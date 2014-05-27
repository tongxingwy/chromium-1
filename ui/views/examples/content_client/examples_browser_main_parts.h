// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_BROWSER_MAIN_PARTS_H_
#define UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_BROWSER_MAIN_PARTS_H_

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

namespace content {
class ShellBrowserContext;
struct MainFunctionParams;
}

namespace wm {
class WMState;
class WMTestHelper;
}

namespace views {
class ViewsDelegate;

namespace examples {

class ExamplesBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit ExamplesBrowserMainParts(
      const content::MainFunctionParams& parameters);
  virtual ~ExamplesBrowserMainParts();

  // content::BrowserMainParts:
  virtual void ToolkitInitialized() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  content::ShellBrowserContext* browser_context() {
    return browser_context_.get();
  }

 private:
  scoped_ptr<content::ShellBrowserContext> browser_context_;

  scoped_ptr<ViewsDelegate> views_delegate_;

#if defined(OS_CHROMEOS)
  // Enable a minimal set of views::corewm to be initialized.
  scoped_ptr<wm::WMTestHelper> wm_test_helper_;
#endif

  scoped_ptr<wm::WMState> wm_state_;

  DISALLOW_COPY_AND_ASSIGN(ExamplesBrowserMainParts);
};

}  // namespace examples
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_BROWSER_MAIN_PARTS_H_
