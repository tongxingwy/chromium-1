// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_DOWNLOAD_METADATA_MANAGER_H_
#define CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_DOWNLOAD_METADATA_MANAGER_H_

#include <map>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/download_manager.h"

namespace base {
class SequencedTaskRunner;
class SequencedWorkerPool;
}

namespace content {
class BrowserContext;
class DownloadItem;
}

namespace safe_browsing {

class ClientDownloadRequest;
class ClientIncidentReport_DownloadDetails;
class DownloadMetadata;

// A browser-wide object that manages the persistent state of metadata
// pertaining to a download.
class DownloadMetadataManager : public content::DownloadManager::Observer {
 public:
  // A callback run when the results of a call to GetDownloadDetails are ready.
  // The supplied parameter may be null, indicating that there are no persisted
  // details for the |browser_context| passed to GetDownloadDetails.
  typedef base::Callback<void(scoped_ptr<ClientIncidentReport_DownloadDetails>)>
      GetDownloadDetailsCallback;

  // Constructs a new instance for which disk IO operations will take place in
  // |worker_pool|.
  explicit DownloadMetadataManager(
      const scoped_refptr<base::SequencedWorkerPool>& worker_pool);

  // Constructor that allows tests to provide a specific runner for
  // asynchronous tasks.
  explicit DownloadMetadataManager(
      const scoped_refptr<base::SequencedTaskRunner>& task_runner);
  ~DownloadMetadataManager() override;

  // Adds |download_manager| to the set observed by the metadata manager.
  void AddDownloadManager(content::DownloadManager* download_manager);

  // Sets |request| as the relevant metadata to persist for |download|. If
  // |request| is null, metadata for the download's BrowserContext are cleared.
  virtual void SetRequest(content::DownloadItem* download,
                          const ClientDownloadRequest* request);

  // Gets the persisted DownloadDetails for |browser_context|. |callback| will
  // be run immediately if the data is available. Otherwise, it will be run
  // later on the caller's thread.
  virtual void GetDownloadDetails(content::BrowserContext* browser_context,
                                  const GetDownloadDetailsCallback& callback);

 protected:
  // Returns the DownloadManager for a given BrowserContext. Virtual for tests.
  virtual content::DownloadManager* GetDownloadManagerForBrowserContext(
      content::BrowserContext* context);

  // content::DownloadManager:Observer methods.
  void OnDownloadCreated(content::DownloadManager* download_manager,
                         content::DownloadItem* item) override;
  void ManagerGoingDown(content::DownloadManager* download_manager) override;

 private:
  class ManagerContext;

  // A mapping of DownloadManagers to their corresponding contexts.
  typedef std::map<content::DownloadManager*, ManagerContext*>
      ManagerToContextMap;

  // A task runner to which read tasks are posted.
  scoped_refptr<base::SequencedTaskRunner> read_runner_;

  // A task runner to which write tasks are posted.
  scoped_refptr<base::SequencedTaskRunner> write_runner_;

  // Contexts for each DownloadManager that has been added and has not yet
  // "gone down".
  ManagerToContextMap contexts_;

  DISALLOW_COPY_AND_ASSIGN(DownloadMetadataManager);
};

}  // namespace safe_browsing

#endif  // CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_DOWNLOAD_METADATA_MANAGER_H_
