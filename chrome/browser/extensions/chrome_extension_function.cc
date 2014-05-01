// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/chrome_extension_function.h"

#include "chrome/browser/extensions/window_controller.h"
#include "chrome/browser/extensions/window_controller_list.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "extensions/browser/extension_function_dispatcher.h"

using content::RenderViewHost;
using content::WebContents;

ChromeAsyncExtensionFunction::ChromeAsyncExtensionFunction() {}

Profile* ChromeAsyncExtensionFunction::GetProfile() const {
  return Profile::FromBrowserContext(context_);
}

bool ChromeAsyncExtensionFunction::CanOperateOnWindow(
    const extensions::WindowController* window_controller) const {
  const extensions::Extension* extension = GetExtension();
  // |extension| is NULL for unit tests only.
  if (extension != NULL && !window_controller->IsVisibleToExtension(extension))
    return false;

  if (GetProfile() == window_controller->profile())
    return true;

  if (!include_incognito())
    return false;

  return GetProfile()->HasOffTheRecordProfile() &&
         GetProfile()->GetOffTheRecordProfile() == window_controller->profile();
}

// TODO(stevenjb): Replace this with GetExtensionWindowController().
Browser* ChromeAsyncExtensionFunction::GetCurrentBrowser() {
  // If the delegate has an associated browser, return it.
  if (dispatcher()) {
    extensions::WindowController* window_controller =
        dispatcher()->delegate()->GetExtensionWindowController();
    if (window_controller) {
      Browser* browser = window_controller->GetBrowser();
      if (browser)
        return browser;
    }
  }

  // Otherwise, try to default to a reasonable browser. If |include_incognito_|
  // is true, we will also search browsers in the incognito version of this
  // profile. Note that the profile may already be incognito, in which case
  // we will search the incognito version only, regardless of the value of
  // |include_incognito|. Look only for browsers on the active desktop as it is
  // preferable to pretend no browser is open then to return a browser on
  // another desktop.
  if (render_view_host_) {
    Profile* profile = Profile::FromBrowserContext(
        render_view_host_->GetProcess()->GetBrowserContext());
    Browser* browser = chrome::FindAnyBrowser(
        profile, include_incognito_, chrome::GetActiveDesktop());
    if (browser)
      return browser;
  }

  // NOTE(rafaelw): This can return NULL in some circumstances. In particular,
  // a background_page onload chrome.tabs api call can make it into here
  // before the browser is sufficiently initialized to return here, or
  // all of this profile's browser windows may have been closed.
  // A similar situation may arise during shutdown.
  // TODO(rafaelw): Delay creation of background_page until the browser
  // is available. http://code.google.com/p/chromium/issues/detail?id=13284
  return NULL;
}

extensions::WindowController*
ChromeAsyncExtensionFunction::GetExtensionWindowController() {
  // If the delegate has an associated window controller, return it.
  if (dispatcher()) {
    extensions::WindowController* window_controller =
        dispatcher()->delegate()->GetExtensionWindowController();
    if (window_controller)
      return window_controller;
  }

  return extensions::WindowControllerList::GetInstance()
      ->CurrentWindowForFunction(this);
}

content::WebContents* ChromeAsyncExtensionFunction::GetAssociatedWebContents() {
  content::WebContents* web_contents =
      UIThreadExtensionFunction::GetAssociatedWebContents();
  if (web_contents)
    return web_contents;

  Browser* browser = GetCurrentBrowser();
  if (!browser)
    return NULL;
  return browser->tab_strip_model()->GetActiveWebContents();
}

ChromeAsyncExtensionFunction::~ChromeAsyncExtensionFunction() {}

ChromeSyncExtensionFunction::ChromeSyncExtensionFunction() {}

bool ChromeSyncExtensionFunction::RunImpl() {
  SendResponse(RunSync());
  return true;
}

ChromeSyncExtensionFunction::~ChromeSyncExtensionFunction() {}
