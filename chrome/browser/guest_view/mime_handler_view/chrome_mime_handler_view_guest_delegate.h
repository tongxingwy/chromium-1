// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_GUEST_VIEW_MIME_HANDLER_VIEW_CHROME_MIME_HANDLER_VIEW_GUEST_DELEGATE_H_
#define CHROME_BROWSER_GUEST_VIEW_MIME_HANDLER_VIEW_CHROME_MIME_HANDLER_VIEW_GUEST_DELEGATE_H_

#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest_delegate.h"

namespace content {
class WebContents;
}  // namespace content

namespace extensions {

class ChromeMimeHandlerViewGuestDelegate : public MimeHandlerViewGuestDelegate {
 public:
  explicit ChromeMimeHandlerViewGuestDelegate(MimeHandlerViewGuest* guest);
  ~ChromeMimeHandlerViewGuestDelegate() override;

  // MimeHandlerViewGuestDelegate.
  void AttachHelpers() override;
  void ChangeZoom(bool zoom_in) override;

 private:
  MimeHandlerViewGuest* guest_;  // Owns us.

  DISALLOW_COPY_AND_ASSIGN(ChromeMimeHandlerViewGuestDelegate);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_GUEST_VIEW_MIME_HANDLER_VIEW_CHROME_MIME_HANDLER_VIEW_GUEST_DELEGATE_H_
