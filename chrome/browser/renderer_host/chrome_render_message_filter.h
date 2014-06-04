// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_RENDERER_HOST_CHROME_RENDER_MESSAGE_FILTER_H_
#define CHROME_BROWSER_RENDERER_HOST_CHROME_RENDER_MESSAGE_FILTER_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/sequenced_task_runner_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/content_settings.h"
#include "content/public/browser/browser_message_filter.h"
#include "third_party/WebKit/public/web/WebCache.h"

class CookieSettings;
struct ExtensionHostMsg_APIActionOrEvent_Params;
struct ExtensionHostMsg_DOMAction_Params;
struct ExtensionMsg_ExternalConnectionInfo;
class GURL;

namespace extensions {
class InfoMap;
}

namespace net {
class HostResolver;
class URLRequestContextGetter;
}

// This class filters out incoming Chrome-specific IPC messages for the renderer
// process on the IPC thread.
class ChromeRenderMessageFilter : public content::BrowserMessageFilter {
 public:
  ChromeRenderMessageFilter(int render_process_id,
                            Profile* profile,
                            net::URLRequestContextGetter* request_context);

  class V8HeapStatsDetails {
   public:
    V8HeapStatsDetails(size_t v8_memory_allocated,
                       size_t v8_memory_used)
        : v8_memory_allocated_(v8_memory_allocated),
          v8_memory_used_(v8_memory_used) {}
    size_t v8_memory_allocated() const { return v8_memory_allocated_; }
    size_t v8_memory_used() const { return v8_memory_used_; }
   private:
    size_t v8_memory_allocated_;
    size_t v8_memory_used_;
  };

  // content::BrowserMessageFilter methods:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OverrideThreadForMessage(
      const IPC::Message& message,
      content::BrowserThread::ID* thread) OVERRIDE;

  int render_process_id() { return render_process_id_; }
  bool off_the_record() { return off_the_record_; }
  net::HostResolver* GetHostResolver();

 private:
  friend class content::BrowserThread;
  friend class base::DeleteHelper<ChromeRenderMessageFilter>;

  virtual ~ChromeRenderMessageFilter();

  void OnDnsPrefetch(const std::vector<std::string>& hostnames);
  void OnPreconnect(const GURL& url);
  void OnResourceTypeStats(const blink::WebCache::ResourceTypeStats& stats);
  void OnUpdatedCacheStats(const blink::WebCache::UsageStats& stats);
  void OnFPS(int routing_id, float fps);
  void OnV8HeapStats(int v8_memory_allocated, int v8_memory_used);

#if defined(ENABLE_EXTENSIONS)
  // TODO(jamescook): Move these functions into the extensions module. Ideally
  // this would be in extensions::ExtensionMessageFilter but that will require
  // resolving the MessageService and ActivityLog dependencies on src/chrome.
  // http://crbug.com/339637
  void OnOpenChannelToExtension(int routing_id,
                                const ExtensionMsg_ExternalConnectionInfo& info,
                                const std::string& channel_name,
                                bool include_tls_channel_id,
                                int* port_id);
  void OpenChannelToExtensionOnUIThread(
      int source_process_id,
      int source_routing_id,
      int receiver_port_id,
      const ExtensionMsg_ExternalConnectionInfo& info,
      const std::string& channel_name,
      bool include_tls_channel_id);
  void OnOpenChannelToNativeApp(int routing_id,
                                const std::string& source_extension_id,
                                const std::string& native_app_name,
                                int* port_id);
  void OpenChannelToNativeAppOnUIThread(int source_routing_id,
                                        int receiver_port_id,
                                        const std::string& source_extension_id,
                                        const std::string& native_app_name);
  void OnOpenChannelToTab(int routing_id, int tab_id,
                          const std::string& extension_id,
                          const std::string& channel_name, int* port_id);
  void OpenChannelToTabOnUIThread(int source_process_id, int source_routing_id,
                                  int receiver_port_id,
                                  int tab_id, const std::string& extension_id,
                                  const std::string& channel_name);
  void OnGetExtensionMessageBundle(const std::string& extension_id,
                                   IPC::Message* reply_msg);
  void OnGetExtensionMessageBundleOnFileThread(
      const base::FilePath& extension_path,
      const std::string& extension_id,
      const std::string& default_locale,
      IPC::Message* reply_msg);
  void OnExtensionCloseChannel(int port_id, const std::string& error_message);
  void OnAddAPIActionToExtensionActivityLog(
      const std::string& extension_id,
      const ExtensionHostMsg_APIActionOrEvent_Params& params);
  void OnAddBlockedCallToExtensionActivityLog(
      const std::string& extension_id,
      const std::string& function_name);
  void OnAddDOMActionToExtensionActivityLog(
      const std::string& extension_id,
      const ExtensionHostMsg_DOMAction_Params& params);
  void OnAddEventToExtensionActivityLog(
      const std::string& extension_id,
      const ExtensionHostMsg_APIActionOrEvent_Params& params);
#endif  // defined(ENABLE_EXTENSIONS)
  void OnAllowDatabase(int render_frame_id,
                       const GURL& origin_url,
                       const GURL& top_origin_url,
                       const base::string16& name,
                       const base::string16& display_name,
                       bool* allowed);
  void OnAllowDOMStorage(int render_frame_id,
                         const GURL& origin_url,
                         const GURL& top_origin_url,
                         bool local,
                         bool* allowed);
  void OnRequestFileSystemAccessSync(int render_frame_id,
                                     const GURL& origin_url,
                                     const GURL& top_origin_url,
                                     bool* allowed);
  void OnRequestFileSystemAccessAsync(int render_frame_id,
                                      int request_id,
                                      const GURL& origin_url,
                                      const GURL& top_origin_url);
  void OnAllowIndexedDB(int render_frame_id,
                        const GURL& origin_url,
                        const GURL& top_origin_url,
                        const base::string16& name,
                        bool* allowed);
#if defined(ENABLE_EXTENSIONS)
  void OnCanTriggerClipboardRead(const GURL& origin, bool* allowed);
  void OnCanTriggerClipboardWrite(const GURL& origin, bool* allowed);
#endif
#if defined(ENABLE_PLUGINS)
  void OnIsCrashReportingEnabled(bool* enabled);
#endif

  const int render_process_id_;

  // The Profile associated with our renderer process.  This should only be
  // accessed on the UI thread!
  Profile* profile_;
  // Copied from the profile so that it can be read on the IO thread.
  const bool off_the_record_;
  // The Predictor for the associated Profile. It is stored so that it can be
  // used on the IO thread.
  chrome_browser_net::Predictor* predictor_;
  scoped_refptr<net::URLRequestContextGetter> request_context_;

#if defined(ENABLE_EXTENSIONS)
  scoped_refptr<extensions::InfoMap> extension_info_map_;
#endif

  // Used to look up permissions at database creation time.
  scoped_refptr<CookieSettings> cookie_settings_;

  DISALLOW_COPY_AND_ASSIGN(ChromeRenderMessageFilter);
};

#endif  // CHROME_BROWSER_RENDERER_HOST_CHROME_RENDER_MESSAGE_FILTER_H_
