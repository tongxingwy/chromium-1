// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/field_trial_synchronizer.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/threading/thread.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/render_messages.h"
#include "chrome/common/variations/variations_util.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"

using content::BrowserThread;

namespace {

// This singleton instance should be constructed during the single threaded
// portion of main(). It initializes globals to provide support for all future
// calls. This object is created on the UI thread, and it is destroyed after
// all the other threads have gone away.
FieldTrialSynchronizer* g_field_trial_synchronizer = NULL;

}  // namespace

FieldTrialSynchronizer::FieldTrialSynchronizer() {
  DCHECK(g_field_trial_synchronizer == NULL);
  g_field_trial_synchronizer = this;
  base::FieldTrialList::AddObserver(this);

  chrome_variations::SetChildProcessLoggingVariationList();
}

void FieldTrialSynchronizer::NotifyAllRenderers(
    const std::string& field_trial_name,
    const std::string& group_name) {
  // To iterate over RenderProcessHosts, or to send messages to the hosts, we
  // need to be on the UI thread.
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  for (content::RenderProcessHost::iterator it(
          content::RenderProcessHost::AllHostsIterator());
       !it.IsAtEnd(); it.Advance()) {
    it.GetCurrentValue()->Send(
        new ChromeViewMsg_SetFieldTrialGroup(field_trial_name, group_name));
  }
}

void FieldTrialSynchronizer::OnFieldTrialGroupFinalized(
    const std::string& field_trial_name,
    const std::string& group_name) {
  CHECK(!field_trial_name.empty() && !group_name.empty());
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&FieldTrialSynchronizer::NotifyAllRenderers,
                 this,
                 field_trial_name,
                 group_name));
  chrome_variations::SetChildProcessLoggingVariationList();
}

FieldTrialSynchronizer::~FieldTrialSynchronizer() {
  base::FieldTrialList::RemoveObserver(this);
  g_field_trial_synchronizer = NULL;
}
