// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/drm/host/drm_native_display_delegate.h"

#include <stdio.h>

#include "base/logging.h"
#include "base/thread_task_runner_handle.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/worker_pool.h"
#include "ui/display/types/display_snapshot.h"
#include "ui/display/types/native_display_observer.h"
#include "ui/events/ozone/device/device_event.h"
#include "ui/events/ozone/device/device_manager.h"
#include "ui/ozone/common/display_snapshot_proxy.h"
#include "ui/ozone/common/display_util.h"
#include "ui/ozone/common/gpu/ozone_gpu_messages.h"
#include "ui/ozone/platform/drm/host/display_manager.h"
#include "ui/ozone/platform/drm/host/drm_device_handle.h"
#include "ui/ozone/platform/drm/host/drm_gpu_platform_support_host.h"

namespace ui {

namespace {

typedef base::Callback<void(const base::FilePath&, scoped_ptr<DrmDeviceHandle>)>
    OnOpenDeviceReplyCallback;

const char* kDisplayActionString[] = {
    "ADD",
    "REMOVE",
    "CHANGE",
};

void OpenDeviceOnWorkerThread(
    const base::FilePath& path,
    const scoped_refptr<base::TaskRunner>& reply_runner,
    const OnOpenDeviceReplyCallback& callback) {
  scoped_ptr<DrmDeviceHandle> handle(new DrmDeviceHandle());
  handle->Initialize(path);
  reply_runner->PostTask(
      FROM_HERE, base::Bind(callback, path, base::Passed(handle.Pass())));
}

void CloseDeviceOnWorkerThread(
    scoped_ptr<DrmDeviceHandle> handle,
    const scoped_refptr<base::TaskRunner>& reply_runner,
    const base::Closure& callback) {
  handle.reset();
  reply_runner->PostTask(FROM_HERE, callback);
}

class DrmDisplaySnapshotProxy : public DisplaySnapshotProxy {
 public:
  DrmDisplaySnapshotProxy(const DisplaySnapshot_Params& params,
                          DisplayManager* display_manager)
      : DisplaySnapshotProxy(params), display_manager_(display_manager) {
    display_manager_->RegisterDisplay(this);
  }

  ~DrmDisplaySnapshotProxy() override {
    display_manager_->UnregisterDisplay(this);
  }

 private:
  DisplayManager* display_manager_;  // Not owned.

  DISALLOW_COPY_AND_ASSIGN(DrmDisplaySnapshotProxy);
};

}  // namespace

DrmNativeDisplayDelegate::DrmNativeDisplayDelegate(
    DrmGpuPlatformSupportHost* proxy,
    DeviceManager* device_manager,
    DisplayManager* display_manager,
    const base::FilePath& primary_graphics_card_path)
    : proxy_(proxy),
      device_manager_(device_manager),
      display_manager_(display_manager),
      primary_graphics_card_path_(primary_graphics_card_path),
      has_dummy_display_(false),
      task_pending_(false),
      weak_ptr_factory_(this) {
  proxy_->RegisterHandler(this);
}

DrmNativeDisplayDelegate::~DrmNativeDisplayDelegate() {
  device_manager_->RemoveObserver(this);
  proxy_->UnregisterHandler(this);

  for (auto it = drm_devices_.begin(); it != drm_devices_.end(); ++it) {
    base::WorkerPool::PostTask(FROM_HERE,
                               base::Bind(&CloseDeviceOnWorkerThread,
                                          base::Passed(drm_devices_.take(it)),
                                          base::ThreadTaskRunnerHandle::Get(),
                                          base::Bind(&base::DoNothing)),
                               false /* task_is_slow */);
  }
}

void DrmNativeDisplayDelegate::Initialize() {
  {
    // First device needs to be treated specially. We need to open this
    // synchronously since the GPU process will need it to initialize the
    // graphics state.
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    scoped_ptr<DrmDeviceHandle> handle(new DrmDeviceHandle());
    if (!handle->Initialize(primary_graphics_card_path_)) {
      LOG(FATAL) << "Failed to open primary graphics card";
      return;
    }
    drm_devices_.add(primary_graphics_card_path_, handle.Pass());
  }

  device_manager_->AddObserver(this);
  device_manager_->ScanDevices(this);

  if (!displays_.empty())
    return;
  DisplaySnapshot_Params params;
  bool success = false;
  {
    // The file generated by frecon that contains EDID for the 1st display.
    const base::FilePath kEDIDFile("/tmp/display_info.bin");

    // Just read it on current thread as this is necessary information
    // to start. This access only tmpfs, which is fast.
    // TODO(dnicoara|oshima): crbug.com/450886.
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    success = CreateSnapshotFromEDIDFile(kEDIDFile, &params);
  }

  // Fallback to command line if the file doesn't exit or failed to read.
  if (success || CreateSnapshotFromCommandLine(&params)) {
    LOG_IF(ERROR, !success) << "Failed to read display_info.bin.";
    DCHECK_NE(DISPLAY_CONNECTION_TYPE_NONE, params.type);
    displays_.push_back(new DrmDisplaySnapshotProxy(params, display_manager_));
    has_dummy_display_ = true;
  } else {
    LOG(ERROR) << "Failed to obtain initial display info";
  }
}

void DrmNativeDisplayDelegate::GrabServer() {
}

void DrmNativeDisplayDelegate::UngrabServer() {
}

bool DrmNativeDisplayDelegate::TakeDisplayControl() {
  proxy_->Send(new OzoneGpuMsg_TakeDisplayControl());
  return true;
}

bool DrmNativeDisplayDelegate::RelinquishDisplayControl() {
  proxy_->Send(new OzoneGpuMsg_RelinquishDisplayControl());
  return true;
}

void DrmNativeDisplayDelegate::SyncWithServer() {
}

void DrmNativeDisplayDelegate::SetBackgroundColor(uint32_t color_argb) {
  NOTIMPLEMENTED();
}

void DrmNativeDisplayDelegate::ForceDPMSOn() {
}

void DrmNativeDisplayDelegate::GetDisplays(
    const GetDisplaysCallback& callback) {
  get_displays_callback_ = callback;
  // GetDisplays() is supposed to force a refresh of the display list.
  if (!proxy_->Send(new OzoneGpuMsg_RefreshNativeDisplays())) {
    get_displays_callback_.Reset();
    callback.Run(displays_.get());
  }
}

void DrmNativeDisplayDelegate::AddMode(const DisplaySnapshot& output,
                                       const DisplayMode* mode) {
}

void DrmNativeDisplayDelegate::Configure(const DisplaySnapshot& output,
                                         const DisplayMode* mode,
                                         const gfx::Point& origin,
                                         const ConfigureCallback& callback) {
  // The dummy display is used on the first run only. Note: cannot post a task
  // here since there is no task runner.
  if (has_dummy_display_) {
    callback.Run(true);
    return;
  }

  configure_callback_map_[output.display_id()] = callback;

  bool status = false;
  if (mode) {
    status = proxy_->Send(new OzoneGpuMsg_ConfigureNativeDisplay(
        output.display_id(), GetDisplayModeParams(*mode), origin));
  } else {
    status =
        proxy_->Send(new OzoneGpuMsg_DisableNativeDisplay(output.display_id()));
  }

  if (!status)
    OnDisplayConfigured(output.display_id(), false);
}

void DrmNativeDisplayDelegate::CreateFrameBuffer(const gfx::Size& size) {
}

void DrmNativeDisplayDelegate::GetHDCPState(
    const DisplaySnapshot& output,
    const GetHDCPStateCallback& callback) {
  get_hdcp_state_callback_map_[output.display_id()] = callback;
  if (!proxy_->Send(new OzoneGpuMsg_GetHDCPState(output.display_id())))
    OnHDCPStateReceived(output.display_id(), false, HDCP_STATE_UNDESIRED);
}

void DrmNativeDisplayDelegate::SetHDCPState(
    const DisplaySnapshot& output,
    HDCPState state,
    const SetHDCPStateCallback& callback) {
  set_hdcp_state_callback_map_[output.display_id()] = callback;
  if (!proxy_->Send(new OzoneGpuMsg_SetHDCPState(output.display_id(), state)))
    OnHDCPStateUpdated(output.display_id(), false);
}

std::vector<ColorCalibrationProfile>
DrmNativeDisplayDelegate::GetAvailableColorCalibrationProfiles(
    const DisplaySnapshot& output) {
  return std::vector<ColorCalibrationProfile>();
}

bool DrmNativeDisplayDelegate::SetColorCalibrationProfile(
    const DisplaySnapshot& output,
    ColorCalibrationProfile new_profile) {
  NOTIMPLEMENTED();
  return false;
}

void DrmNativeDisplayDelegate::AddObserver(NativeDisplayObserver* observer) {
  observers_.AddObserver(observer);
}

void DrmNativeDisplayDelegate::RemoveObserver(NativeDisplayObserver* observer) {
  observers_.RemoveObserver(observer);
}

void DrmNativeDisplayDelegate::OnDeviceEvent(const DeviceEvent& event) {
  if (event.device_type() != DeviceEvent::DISPLAY)
    return;

  event_queue_.push(DisplayEvent(event.action_type(), event.path()));
  ProcessEvent();
}

void DrmNativeDisplayDelegate::ProcessEvent() {
  while (!event_queue_.empty() && !task_pending_) {
    DisplayEvent event = event_queue_.front();
    event_queue_.pop();
    VLOG(1) << "Got display event " << kDisplayActionString[event.action_type]
            << " for " << event.path.value();
    switch (event.action_type) {
      case DeviceEvent::ADD:
        if (drm_devices_.find(event.path) == drm_devices_.end()) {
          task_pending_ = base::WorkerPool::PostTask(
              FROM_HERE,
              base::Bind(
                  &OpenDeviceOnWorkerThread, event.path,
                  base::ThreadTaskRunnerHandle::Get(),
                  base::Bind(&DrmNativeDisplayDelegate::OnAddGraphicsDevice,
                             weak_ptr_factory_.GetWeakPtr())),
              false /* task_is_slow */);
        }
        break;
      case DeviceEvent::CHANGE:
        task_pending_ = base::ThreadTaskRunnerHandle::Get()->PostTask(
            FROM_HERE,
            base::Bind(&DrmNativeDisplayDelegate::OnUpdateGraphicsDevice,
                       weak_ptr_factory_.GetWeakPtr()));
        break;
      case DeviceEvent::REMOVE:
        DCHECK(event.path != primary_graphics_card_path_)
            << "Removing primary graphics card";
        auto it = drm_devices_.find(event.path);
        if (it != drm_devices_.end()) {
          task_pending_ = base::WorkerPool::PostTask(
              FROM_HERE,
              base::Bind(
                  &CloseDeviceOnWorkerThread,
                  base::Passed(drm_devices_.take_and_erase(it)),
                  base::ThreadTaskRunnerHandle::Get(),
                  base::Bind(&DrmNativeDisplayDelegate::OnRemoveGraphicsDevice,
                             weak_ptr_factory_.GetWeakPtr(), event.path)),
              false /* task_is_slow */);
          return;
        }
        break;
    }
  }
}

void DrmNativeDisplayDelegate::OnAddGraphicsDevice(
    const base::FilePath& path,
    scoped_ptr<DrmDeviceHandle> handle) {
  if (handle->IsValid()) {
    base::ScopedFD file = handle->Duplicate();
    drm_devices_.add(path, handle.Pass());
    proxy_->Send(new OzoneGpuMsg_AddGraphicsDevice(
        path, base::FileDescriptor(file.Pass())));
    FOR_EACH_OBSERVER(NativeDisplayObserver, observers_,
                      OnConfigurationChanged());
  }

  task_pending_ = false;
  ProcessEvent();
}

void DrmNativeDisplayDelegate::OnUpdateGraphicsDevice() {
  FOR_EACH_OBSERVER(NativeDisplayObserver, observers_,
                    OnConfigurationChanged());
  task_pending_ = false;
  ProcessEvent();
}

void DrmNativeDisplayDelegate::OnRemoveGraphicsDevice(
    const base::FilePath& path) {
  proxy_->Send(new OzoneGpuMsg_RemoveGraphicsDevice(path));
  FOR_EACH_OBSERVER(NativeDisplayObserver, observers_,
                    OnConfigurationChanged());
  task_pending_ = false;
  ProcessEvent();
}

void DrmNativeDisplayDelegate::OnChannelEstablished(
    int host_id,
    scoped_refptr<base::SingleThreadTaskRunner> send_runner,
    const base::Callback<void(IPC::Message*)>& send_callback) {
  auto it = drm_devices_.find(primary_graphics_card_path_);
  DCHECK(it != drm_devices_.end());
  // Send the primary device first since this is used to initialize graphics
  // state.
  proxy_->Send(new OzoneGpuMsg_AddGraphicsDevice(
      it->first, base::FileDescriptor(it->second->Duplicate())));

  for (auto pair : drm_devices_) {
    if (pair.second->IsValid() && pair.first != primary_graphics_card_path_) {
      proxy_->Send(new OzoneGpuMsg_AddGraphicsDevice(
          pair.first, base::FileDescriptor(pair.second->Duplicate())));
    }
  }

  device_manager_->ScanDevices(this);
  FOR_EACH_OBSERVER(NativeDisplayObserver, observers_,
                    OnConfigurationChanged());
}

void DrmNativeDisplayDelegate::OnChannelDestroyed(int host_id) {
  // If the channel got destroyed in the middle of a configuration then just
  // respond with failure.
  if (!get_displays_callback_.is_null()) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE,
        base::Bind(&DrmNativeDisplayDelegate::RunUpdateDisplaysCallback,
                   weak_ptr_factory_.GetWeakPtr(), get_displays_callback_));
    get_displays_callback_.Reset();
  }

  for (const auto& pair : configure_callback_map_) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(pair.second, false));
  }
  configure_callback_map_.clear();
}

bool DrmNativeDisplayDelegate::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;

  IPC_BEGIN_MESSAGE_MAP(DrmNativeDisplayDelegate, message)
  IPC_MESSAGE_HANDLER(OzoneHostMsg_UpdateNativeDisplays, OnUpdateNativeDisplays)
  IPC_MESSAGE_HANDLER(OzoneHostMsg_DisplayConfigured, OnDisplayConfigured)
  IPC_MESSAGE_HANDLER(OzoneHostMsg_HDCPStateReceived, OnHDCPStateReceived)
  IPC_MESSAGE_HANDLER(OzoneHostMsg_HDCPStateUpdated, OnHDCPStateUpdated)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void DrmNativeDisplayDelegate::OnUpdateNativeDisplays(
    const std::vector<DisplaySnapshot_Params>& displays) {
  has_dummy_display_ = false;
  displays_.clear();
  for (size_t i = 0; i < displays.size(); ++i)
    displays_.push_back(
        new DrmDisplaySnapshotProxy(displays[i], display_manager_));

  if (!get_displays_callback_.is_null()) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE,
        base::Bind(&DrmNativeDisplayDelegate::RunUpdateDisplaysCallback,
                   weak_ptr_factory_.GetWeakPtr(), get_displays_callback_));
    get_displays_callback_.Reset();
  }
}

void DrmNativeDisplayDelegate::OnDisplayConfigured(int64_t display_id,
                                                   bool status) {
  auto it = configure_callback_map_.find(display_id);
  if (it != configure_callback_map_.end()) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(it->second, status));
    configure_callback_map_.erase(it);
  }
}

void DrmNativeDisplayDelegate::OnHDCPStateReceived(int64_t display_id,
                                                   bool status,
                                                   HDCPState state) {
  auto it = get_hdcp_state_callback_map_.find(display_id);
  if (it != get_hdcp_state_callback_map_.end()) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(it->second, status, state));
    get_hdcp_state_callback_map_.erase(it);
  }
}

void DrmNativeDisplayDelegate::OnHDCPStateUpdated(int64_t display_id,
                                                  bool status) {
  auto it = set_hdcp_state_callback_map_.find(display_id);
  if (it != set_hdcp_state_callback_map_.end()) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(it->second, status));
    set_hdcp_state_callback_map_.erase(it);
  }
}

void DrmNativeDisplayDelegate::RunUpdateDisplaysCallback(
    const GetDisplaysCallback& callback) const {
  callback.Run(displays_.get());
}

}  // namespace ui
