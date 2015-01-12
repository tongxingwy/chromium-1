// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_WEBRTC_LOGGING_PRIVATE_WEBRTC_LOGGING_PRIVATE_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_WEBRTC_LOGGING_PRIVATE_WEBRTC_LOGGING_PRIVATE_API_H_

#include "chrome/browser/extensions/chrome_extension_function.h"
#include "chrome/common/extensions/api/webrtc_logging_private.h"

namespace content {

class RenderProcessHost;

}

namespace extensions {

class WebrtcLoggingPrivateFunction : public ChromeAsyncExtensionFunction {
 protected:
  ~WebrtcLoggingPrivateFunction() override {}

  // Returns the RenderProcessHost associated with the given |request|
  // authorized by the |security_origin|. Returns null if unauthorized or
  // the RPH does not exist.
  content::RenderProcessHost* RphFromRequest(
      const api::webrtc_logging_private::RequestInfo& request,
      const std::string& security_origin);
};

class WebrtcLoggingPrivateSetMetaDataFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.setMetaData",
                             WEBRTCLOGGINGPRIVATE_SETMETADATA)
  WebrtcLoggingPrivateSetMetaDataFunction();

 private:
  ~WebrtcLoggingPrivateSetMetaDataFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void SetMetaDataCallback(bool success, const std::string& error_message);
};

class WebrtcLoggingPrivateStartFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.start",
                             WEBRTCLOGGINGPRIVATE_START)
  WebrtcLoggingPrivateStartFunction();

 private:
  ~WebrtcLoggingPrivateStartFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void StartCallback(bool success, const std::string& error_message);
};

class WebrtcLoggingPrivateSetUploadOnRenderCloseFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.setUploadOnRenderClose",
                             WEBRTCLOGGINGPRIVATE_SETUPLOADONRENDERCLOSE)
  WebrtcLoggingPrivateSetUploadOnRenderCloseFunction();

 private:
  ~WebrtcLoggingPrivateSetUploadOnRenderCloseFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;
};

class WebrtcLoggingPrivateStopFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.stop",
                             WEBRTCLOGGINGPRIVATE_STOP)
  WebrtcLoggingPrivateStopFunction();

 private:
  ~WebrtcLoggingPrivateStopFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void StopCallback(bool success, const std::string& error_message);
};

class WebrtcLoggingPrivateUploadFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.upload",
                             WEBRTCLOGGINGPRIVATE_UPLOAD)
  WebrtcLoggingPrivateUploadFunction();

 private:
  ~WebrtcLoggingPrivateUploadFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void UploadCallback(bool success, const std::string& report_id,
                      const std::string& error_message);
};

class WebrtcLoggingPrivateDiscardFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.discard",
                             WEBRTCLOGGINGPRIVATE_DISCARD)
  WebrtcLoggingPrivateDiscardFunction();

 private:
  ~WebrtcLoggingPrivateDiscardFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void DiscardCallback(bool success, const std::string& error_message);
};

class WebrtcLoggingPrivateStartRtpDumpFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.startRtpDump",
                             WEBRTCLOGGINGPRIVATE_STARTRTPDUMP)
  WebrtcLoggingPrivateStartRtpDumpFunction();

 private:
  ~WebrtcLoggingPrivateStartRtpDumpFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void StartRtpDumpCallback(bool success, const std::string& error_message);
};

class WebrtcLoggingPrivateStopRtpDumpFunction
    : public WebrtcLoggingPrivateFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcLoggingPrivate.stopRtpDump",
                             WEBRTCLOGGINGPRIVATE_STOPRTPDUMP)
  WebrtcLoggingPrivateStopRtpDumpFunction();

 private:
  ~WebrtcLoggingPrivateStopRtpDumpFunction() override;

  // ExtensionFunction overrides.
  bool RunAsync() override;

  // Must be called on UI thread.
  void StopRtpDumpCallback(bool success, const std::string& error_message);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_WEBRTC_LOGGING_PRIVATE_WEBRTC_LOGGING_PRIVATE_API_H_
