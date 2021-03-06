// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/mime_util/mime_util.h"

#include "base/containers/hash_tables.h"
#include "base/lazy_instance.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"

namespace mime_util {

namespace {

// Dictionary of cryptographic file mime types.
struct CertificateMimeTypeInfo {
  const char* const mime_type;
  net::CertificateMimeType cert_type;
};

const CertificateMimeTypeInfo kSupportedCertificateTypes[] = {
    {"application/x-x509-user-cert", net::CERTIFICATE_MIME_TYPE_X509_USER_CERT},
#if defined(OS_ANDROID)
    {"application/x-x509-ca-cert", net::CERTIFICATE_MIME_TYPE_X509_CA_CERT},
    {"application/x-pkcs12", net::CERTIFICATE_MIME_TYPE_PKCS12_ARCHIVE},
#endif
};

// From WebKit's WebCore/platform/MIMETypeRegistry.cpp:

const char* const kSupportedImageTypes[] = {"image/jpeg",
                                            "image/pjpeg",
                                            "image/jpg",
                                            "image/webp",
                                            "image/png",
                                            "image/gif",
                                            "image/bmp",
                                            "image/vnd.microsoft.icon",  // ico
                                            "image/x-icon",              // ico
                                            "image/x-xbitmap",           // xbm
                                            "image/x-png"};

//  Mozilla 1.8 and WinIE 7 both accept text/javascript and text/ecmascript.
//  Mozilla 1.8 accepts application/javascript, application/ecmascript, and
// application/x-javascript, but WinIE 7 doesn't.
//  WinIE 7 accepts text/javascript1.1 - text/javascript1.3, text/jscript, and
// text/livescript, but Mozilla 1.8 doesn't.
//  Mozilla 1.8 allows leading and trailing whitespace, but WinIE 7 doesn't.
//  Mozilla 1.8 and WinIE 7 both accept the empty string, but neither accept a
// whitespace-only string.
//  We want to accept all the values that either of these browsers accept, but
// not other values.
const char* const kSupportedJavascriptTypes[] = {"text/javascript",
                                                 "text/ecmascript",
                                                 "application/javascript",
                                                 "application/ecmascript",
                                                 "application/x-javascript",
                                                 "text/javascript1.1",
                                                 "text/javascript1.2",
                                                 "text/javascript1.3",
                                                 "text/jscript",
                                                 "text/livescript"};

// These types are excluded from the logic that allows all text/ types because
// while they are technically text, it's very unlikely that a user expects to
// see them rendered in text form.
static const char* const kUnsupportedTextTypes[] = {
    "text/calendar",
    "text/x-calendar",
    "text/x-vcalendar",
    "text/vcalendar",
    "text/vcard",
    "text/x-vcard",
    "text/directory",
    "text/ldif",
    "text/qif",
    "text/x-qif",
    "text/x-csv",
    "text/x-vcf",
    "text/rtf",
    "text/comma-separated-values",
    "text/csv",
    "text/tab-separated-values",
    "text/tsv",
    "text/ofx",                         // http://crbug.com/162238
    "text/vnd.sun.j2me.app-descriptor"  // http://crbug.com/176450
};

// Note:
// - does not include javascript types list (see supported_javascript_types)
// - does not include types starting with "text/" (see
//   IsSupportedNonImageMimeType())
static const char* const kSupportedNonImageTypes[] = {
    "image/svg+xml",  // SVG is text-based XML, even though it has an image/
                      // type
    "application/xml",
    "application/atom+xml",
    "application/rss+xml",
    "application/xhtml+xml",
    "application/json",
    "multipart/related",  // For MHTML support.
    "multipart/x-mixed-replace"
    // Note: ADDING a new type here will probably render it AS HTML. This can
    // result in cross site scripting.
};

// Singleton utility class for mime types
class MimeUtil {
 public:
  bool IsSupportedImageMimeType(const std::string& mime_type) const;
  bool IsSupportedNonImageMimeType(const std::string& mime_type) const;
  bool IsUnsupportedTextMimeType(const std::string& mime_type) const;
  bool IsSupportedJavascriptMimeType(const std::string& mime_type) const;

  bool IsSupportedMimeType(const std::string& mime_type) const;

 private:
  friend struct base::DefaultLazyInstanceTraits<MimeUtil>;

  using MimeTypes = base::hash_set<std::string>;

  MimeUtil();

  MimeTypes image_types_;
  MimeTypes non_image_types_;
  MimeTypes unsupported_text_types_;
  MimeTypes javascript_types_;

  DISALLOW_COPY_AND_ASSIGN(MimeUtil);
};

MimeUtil::MimeUtil() {
  for (size_t i = 0; i < arraysize(kSupportedNonImageTypes); ++i)
    non_image_types_.insert(kSupportedNonImageTypes[i]);
  for (size_t i = 0; i < arraysize(kSupportedImageTypes); ++i)
    image_types_.insert(kSupportedImageTypes[i]);
  for (size_t i = 0; i < arraysize(kUnsupportedTextTypes); ++i)
    unsupported_text_types_.insert(kUnsupportedTextTypes[i]);
  for (size_t i = 0; i < arraysize(kSupportedJavascriptTypes); ++i) {
    javascript_types_.insert(kSupportedJavascriptTypes[i]);
    non_image_types_.insert(kSupportedJavascriptTypes[i]);
  }
  for (size_t i = 0; i < arraysize(kSupportedCertificateTypes); ++i)
    non_image_types_.insert(kSupportedCertificateTypes[i].mime_type);
}

bool MimeUtil::IsSupportedImageMimeType(const std::string& mime_type) const {
  return image_types_.find(base::StringToLowerASCII(mime_type)) !=
         image_types_.end();
}

bool MimeUtil::IsSupportedNonImageMimeType(const std::string& mime_type) const {
  return non_image_types_.find(base::StringToLowerASCII(mime_type)) !=
             non_image_types_.end() ||
         (StartsWithASCII(mime_type, "text/", false /* case insensitive */) &&
          !IsUnsupportedTextMimeType(mime_type)) ||
         (StartsWithASCII(mime_type, "application/", false) &&
          net::MatchesMimeType("application/*+json", mime_type)) ||
         net::IsSupportedMediaMimeType(mime_type);
}

bool MimeUtil::IsUnsupportedTextMimeType(const std::string& mime_type) const {
  return unsupported_text_types_.find(base::StringToLowerASCII(mime_type)) !=
         unsupported_text_types_.end();
}

bool MimeUtil::IsSupportedJavascriptMimeType(
    const std::string& mime_type) const {
  return javascript_types_.find(mime_type) != javascript_types_.end();
}

bool MimeUtil::IsSupportedMimeType(const std::string& mime_type) const {
  return (StartsWithASCII(mime_type, "image/", false) &&
          IsSupportedImageMimeType(mime_type)) ||
         IsSupportedNonImageMimeType(mime_type);
}

// This variable is Leaky because it is accessed from WorkerPool threads.
static base::LazyInstance<MimeUtil>::Leaky g_mime_util =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

bool IsSupportedImageMimeType(const std::string& mime_type) {
  return g_mime_util.Get().IsSupportedImageMimeType(mime_type);
}

bool IsSupportedNonImageMimeType(const std::string& mime_type) {
  return g_mime_util.Get().IsSupportedNonImageMimeType(mime_type);
}

bool IsUnsupportedTextMimeType(const std::string& mime_type) {
  return g_mime_util.Get().IsUnsupportedTextMimeType(mime_type);
}

bool IsSupportedJavascriptMimeType(const std::string& mime_type) {
  return g_mime_util.Get().IsSupportedJavascriptMimeType(mime_type);
}

bool IsSupportedCertificateMimeType(const std::string& mime_type) {
  net::CertificateMimeType file_type =
      GetCertificateMimeTypeForMimeType(mime_type);
  return file_type != net::CERTIFICATE_MIME_TYPE_UNKNOWN;
}

bool IsSupportedMimeType(const std::string& mime_type) {
  return g_mime_util.Get().IsSupportedMimeType(mime_type);
}

net::CertificateMimeType GetCertificateMimeTypeForMimeType(
    const std::string& mime_type) {
  // Don't create a map, there is only one entry in the table,
  // except on Android.
  for (size_t i = 0; i < arraysize(kSupportedCertificateTypes); ++i) {
    if (base::strcasecmp(mime_type.c_str(),
                         kSupportedCertificateTypes[i].mime_type) == 0) {
      return kSupportedCertificateTypes[i].cert_type;
    }
  }

  return net::CERTIFICATE_MIME_TYPE_UNKNOWN;
}

}  // namespace mime_util
