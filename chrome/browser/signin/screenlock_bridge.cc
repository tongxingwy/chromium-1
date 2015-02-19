// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/signin/screenlock_bridge.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "chrome/browser/profiles/profile_window.h"
#include "chrome/browser/signin/signin_manager_factory.h"
#include "components/signin/core/browser/signin_manager.h"

#if defined(OS_CHROMEOS)
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/session_manager_client.h"
#endif

namespace {

base::LazyInstance<ScreenlockBridge> g_screenlock_bridge_bridge_instance =
    LAZY_INSTANCE_INITIALIZER;

// Ids for the icons that are supported by lock screen and signin screen
// account picker as user pod custom icons.
// The id's should be kept in sync with values used by user_pod_row.js.
const char kLockedUserPodCustomIconId[] = "locked";
const char kLockedToBeActivatedUserPodCustomIconId[] = "locked-to-be-activated";
const char kLockedWithProximityHintUserPodCustomIconId[] =
    "locked-with-proximity-hint";
const char kUnlockedUserPodCustomIconId[] = "unlocked";
const char kHardlockedUserPodCustomIconId[] = "hardlocked";
const char kSpinnerUserPodCustomIconId[] = "spinner";

// Given the user pod icon, returns its id as used by the user pod UI code.
std::string GetIdForIcon(ScreenlockBridge::UserPodCustomIcon icon) {
  switch (icon) {
    case ScreenlockBridge::USER_POD_CUSTOM_ICON_LOCKED:
      return kLockedUserPodCustomIconId;
    case ScreenlockBridge::USER_POD_CUSTOM_ICON_LOCKED_TO_BE_ACTIVATED:
      return kLockedToBeActivatedUserPodCustomIconId;
    case ScreenlockBridge::USER_POD_CUSTOM_ICON_LOCKED_WITH_PROXIMITY_HINT:
      return kLockedWithProximityHintUserPodCustomIconId;
    case ScreenlockBridge::USER_POD_CUSTOM_ICON_UNLOCKED:
      return kUnlockedUserPodCustomIconId;
    case ScreenlockBridge::USER_POD_CUSTOM_ICON_HARDLOCKED:
      return kHardlockedUserPodCustomIconId;
    case ScreenlockBridge::USER_POD_CUSTOM_ICON_SPINNER:
      return kSpinnerUserPodCustomIconId;
    default:
      return "";
  }
}

}  // namespace

// static
ScreenlockBridge* ScreenlockBridge::Get() {
  return g_screenlock_bridge_bridge_instance.Pointer();
}

ScreenlockBridge::UserPodCustomIconOptions::UserPodCustomIconOptions()
    : autoshow_tooltip_(false),
      hardlock_on_click_(false) {
}

ScreenlockBridge::UserPodCustomIconOptions::~UserPodCustomIconOptions() {}

scoped_ptr<base::DictionaryValue>
ScreenlockBridge::UserPodCustomIconOptions::ToDictionaryValue() const {
  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue());
  std::string icon_id = GetIdForIcon(icon_);
  result->SetString("id", icon_id);

  if (!tooltip_.empty()) {
    base::DictionaryValue* tooltip_options = new base::DictionaryValue();
    tooltip_options->SetString("text", tooltip_);
    tooltip_options->SetBoolean("autoshow", autoshow_tooltip_);
    result->Set("tooltip", tooltip_options);
  }

  if (!aria_label_.empty())
    result->SetString("ariaLabel", aria_label_);

  if (hardlock_on_click_)
    result->SetBoolean("hardlockOnClick", true);

  return result.Pass();
}

void ScreenlockBridge::UserPodCustomIconOptions::SetIcon(
    ScreenlockBridge::UserPodCustomIcon icon) {
  icon_ = icon;
}

void ScreenlockBridge::UserPodCustomIconOptions::SetTooltip(
    const base::string16& tooltip,
    bool autoshow) {
  tooltip_ = tooltip;
  autoshow_tooltip_ = autoshow;
}

void ScreenlockBridge::UserPodCustomIconOptions::SetAriaLabel(
    const base::string16& aria_label) {
  aria_label_ = aria_label;
}

void ScreenlockBridge::UserPodCustomIconOptions::SetHardlockOnClick() {
  hardlock_on_click_ = true;
}

// static
std::string ScreenlockBridge::GetAuthenticatedUserEmail(
    const Profile* profile) {
  // |profile| has to be a signed-in profile with SigninManager already
  // created. Otherwise, just crash to collect stack.
  const SigninManagerBase* signin_manager =
      SigninManagerFactory::GetForProfileIfExists(profile);
  return signin_manager->GetAuthenticatedUsername();
}

ScreenlockBridge::ScreenlockBridge() : lock_handler_(NULL) {
}

ScreenlockBridge::~ScreenlockBridge() {
}

void ScreenlockBridge::SetLockHandler(LockHandler* lock_handler) {
  DCHECK(lock_handler_ == NULL || lock_handler == NULL);

  // Don't notify observers if there is no change -- i.e. if the screen was
  // already unlocked, and is remaining unlocked.
  if (lock_handler == lock_handler_)
    return;

  // TODO(isherman): If |lock_handler| is null, then |lock_handler_| might have
  // been freed. Cache the screen type rather than querying it below.
  LockHandler::ScreenType screen_type;
  if (lock_handler_)
    screen_type = lock_handler_->GetScreenType();
  else
    screen_type = lock_handler->GetScreenType();

  lock_handler_ = lock_handler;
  if (lock_handler_)
    FOR_EACH_OBSERVER(Observer, observers_, OnScreenDidLock(screen_type));
  else
    FOR_EACH_OBSERVER(Observer, observers_, OnScreenDidUnlock(screen_type));
}

void ScreenlockBridge::SetFocusedUser(const std::string& user_id) {
  if (user_id == focused_user_id_)
    return;
  focused_user_id_ = user_id;
  FOR_EACH_OBSERVER(Observer, observers_, OnFocusedUserChanged(user_id));
}

bool ScreenlockBridge::IsLocked() const {
  return lock_handler_ != NULL;
}

void ScreenlockBridge::Lock(Profile* profile) {
#if defined(OS_CHROMEOS)
  chromeos::SessionManagerClient* session_manager =
      chromeos::DBusThreadManager::Get()->GetSessionManagerClient();
  session_manager->RequestLockScreen();
#else
  profiles::LockProfile(profile);
#endif
}

void ScreenlockBridge::Unlock(Profile* profile) {
  if (lock_handler_)
    lock_handler_->Unlock(GetAuthenticatedUserEmail(profile));
}

void ScreenlockBridge::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ScreenlockBridge::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}
