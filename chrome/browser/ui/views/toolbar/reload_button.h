// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TOOLBAR_RELOAD_BUTTON_H__
#define CHROME_BROWSER_UI_VIEWS_TOOLBAR_RELOAD_BUTTON_H__

#include "base/basictypes.h"
#include "base/gtest_prod_util.h"
#include "base/timer/timer.h"
#include "chrome/browser/ui/views/toolbar/toolbar_button.h"
#include "ui/base/models/simple_menu_model.h"
#include "ui/views/controls/button/button.h"

class CommandUpdater;

////////////////////////////////////////////////////////////////////////////////
//
// ReloadButton
//
// The reload button in the toolbar, which changes to a stop button when a page
// load is in progress. Trickiness comes from the desire to have the 'stop'
// button not change back to 'reload' if the user's mouse is hovering over it
// (to prevent mis-clicks).
//
////////////////////////////////////////////////////////////////////////////////

class ReloadButton : public ToolbarButton,
                     public views::ButtonListener,
                     public ui::SimpleMenuModel::Delegate {
 public:
  enum Mode { MODE_RELOAD = 0, MODE_STOP };

  // The button's class name.
  static const char kViewClassName[];

  explicit ReloadButton(CommandUpdater* command_updater);
  virtual ~ReloadButton();

  // Ask for a specified button state.  If |force| is true this will be applied
  // immediately.
  void ChangeMode(Mode mode, bool force);

  // Enable reload drop-down menu.
  void set_menu_enabled(bool enable) { menu_enabled_ = enable; }

  void LoadImages();

  // ToolbarButton:
  virtual void OnMouseExited(const ui::MouseEvent& event) OVERRIDE;
  virtual bool GetTooltipText(const gfx::Point& p,
                              base::string16* tooltip) const OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;
  virtual void GetAccessibleState(ui::AXViewState* state) OVERRIDE;
  virtual bool ShouldShowMenu() OVERRIDE;
  virtual void ShowDropDownMenu(ui::MenuSourceType source_type) OVERRIDE;

  // views::ButtonListener:
  virtual void ButtonPressed(views::Button* /* button */,
                             const ui::Event& event) OVERRIDE;

  // ui::SimpleMenuModel::Delegate:
  virtual bool IsCommandIdChecked(int command_id) const OVERRIDE;
  virtual bool IsCommandIdEnabled(int command_id) const OVERRIDE;
  virtual bool IsCommandIdVisible(int command_id) const OVERRIDE;
  virtual bool GetAcceleratorForCommandId(
      int command_id,
      ui::Accelerator* accelerator) OVERRIDE;
  virtual void ExecuteCommand(int command_id, int event_flags) OVERRIDE;

 private:
  friend class ReloadButtonTest;

  ui::SimpleMenuModel* CreateMenuModel();

  void ExecuteBrowserCommand(int command, int event_flags);
  void ChangeModeInternal(Mode mode);

  void OnDoubleClickTimer();
  void OnStopToReloadTimer();

  base::OneShotTimer<ReloadButton> double_click_timer_;
  base::OneShotTimer<ReloadButton> stop_to_reload_timer_;

  // This may be NULL when testing.
  CommandUpdater* command_updater_;

  // The mode we should be in assuming no timers are running.
  Mode intended_mode_;

  // The currently-visible mode - this may differ from the intended mode.
  Mode visible_mode_;

  // The delay times for the timers.  These are members so that tests can modify
  // them.
  base::TimeDelta double_click_timer_delay_;
  base::TimeDelta stop_to_reload_timer_delay_;

  // Indicates if reload menu is enabled.
  bool menu_enabled_;

  // TESTING ONLY
  // True if we should pretend the button is hovered.
  bool testing_mouse_hovered_;
  // Increments when we would tell the browser to "reload", so
  // test code can tell whether we did so (as there may be no |browser_|).
  int testing_reload_count_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ReloadButton);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TOOLBAR_RELOAD_BUTTON_H__
