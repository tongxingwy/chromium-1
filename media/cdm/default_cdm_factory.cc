// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/cdm/default_cdm_factory.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "media/base/key_systems.h"
#include "media/cdm/aes_decryptor.h"
#include "url/gurl.h"

namespace media {

DefaultCdmFactory::DefaultCdmFactory() {
}

DefaultCdmFactory::~DefaultCdmFactory() {
}

void DefaultCdmFactory::Create(
    const std::string& key_system,
    bool allow_distinctive_identifier,
    bool allow_persistent_state,
    const GURL& security_origin,
    const SessionMessageCB& session_message_cb,
    const SessionClosedCB& session_closed_cb,
    const LegacySessionErrorCB& legacy_session_error_cb,
    const SessionKeysChangeCB& session_keys_change_cb,
    const SessionExpirationUpdateCB& session_expiration_update_cb,
    const CdmCreatedCB& cdm_created_cb) {
  if (!security_origin.is_valid() || !CanUseAesDecryptor(key_system)) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(cdm_created_cb, nullptr));
    return;
  }

  scoped_ptr<MediaKeys> cdm(
      new AesDecryptor(security_origin, session_message_cb, session_closed_cb,
                       session_keys_change_cb));
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::Bind(cdm_created_cb, base::Passed(&cdm)));
}

}  // namespace media
