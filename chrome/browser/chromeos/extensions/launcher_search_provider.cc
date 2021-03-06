// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/extensions/launcher_search_provider.h"

#include "chrome/browser/chromeos/launcher_search_provider/service.h"
#include "chrome/common/extensions/api/launcher_search_provider.h"

namespace extensions {

LauncherSearchProviderSetSearchResultsFunction::
    ~LauncherSearchProviderSetSearchResultsFunction() {
}

bool LauncherSearchProviderSetSearchResultsFunction::RunSync() {
  using chromeos::launcher_search_provider::Service;
  using extensions::api::launcher_search_provider::SetSearchResults::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  Service* const service = Service::Get(GetProfile());
  service->SetSearchResults(extension(), params->query_id, params->results);

  return true;
}

}  // namespace extensions
