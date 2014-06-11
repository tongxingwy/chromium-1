// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_NATIVE_PERMISSION_PROTECTED_MEDIA_ID_PERMISSION_REQUEST_H
#define ANDROID_WEBVIEW_NATIVE_PERMISSION_PROTECTED_MEDIA_ID_PERMISSION_REQUEST_H

#include "android_webview/native/permission/aw_permission_request_delegate.h"
#include "base/callback.h"
#include "content/public/browser/browser_context.h"

namespace android_webview {

// The AwPermissionRequestDelegate implementation for protected media id
// permission request.
class ProtectedMediaIdPermissionRequest : public AwPermissionRequestDelegate {
 public:
  ProtectedMediaIdPermissionRequest(const GURL& origin,
      const content::BrowserContext::
          ProtectedMediaIdentifierPermissionCallback& callback);
  virtual ~ProtectedMediaIdPermissionRequest();

  // AwPermissionRequestDelegate implementation.
  virtual const GURL& GetOrigin() OVERRIDE;
  virtual int64 GetResources() OVERRIDE;
  virtual void NotifyRequestResult(bool allowed) OVERRIDE;

 private:
  const GURL origin_;
  const content::BrowserContext::ProtectedMediaIdentifierPermissionCallback
      callback_;

  DISALLOW_COPY_AND_ASSIGN(ProtectedMediaIdPermissionRequest);
};

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_NATIVE_PERMISSION_PROTECTED_MEDIA_ID_PERMISSION_REQUEST_H
