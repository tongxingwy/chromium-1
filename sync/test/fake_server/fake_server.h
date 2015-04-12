// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_TEST_FAKE_SERVER_FAKE_SERVER_H_
#define SYNC_TEST_FAKE_SERVER_FAKE_SERVER_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/values.h"
#include "sync/internal_api/public/base/model_type.h"
#include "sync/protocol/sync.pb.h"
#include "sync/test/fake_server/fake_server_entity.h"

namespace fake_server {

// A fake version of the Sync server used for testing.
class FakeServer {
 public:
  typedef base::Callback<void(int, int, const std::string&)>
      HandleCommandCallback;

  class Observer {
   public:
    virtual ~Observer() {}

    // Called after FakeServer has processed a successful commit. The types
    // updated as part of the commit are passed in |committed_model_types|.
    virtual void OnCommit(
        const std::string& committer_id,
        syncer::ModelTypeSet committed_model_types) = 0;
  };

  FakeServer();
  virtual ~FakeServer();

  // Asynchronously handles a /command POST to the server. If the error_code is
  // passed to |callback| is 0 (success), the POST's response code and content
  // will also be passed.
  void HandleCommand(const std::string& request,
                     const HandleCommandCallback& callback);

  // Creates a DicionaryValue representation of all entities present in the
  // server. The dictionary keys are the strings generated by ModelTypeToString
  // and the values are ListValues containing StringValue versions of entity
  // names.
  scoped_ptr<base::DictionaryValue> GetEntitiesAsDictionaryValue();

  // Returns all entities stored by the server of the given |model_type|.
  // This method returns SyncEntity protocol buffer objects (instead of
  // FakeServerEntity) so that callers can inspect datatype-specific data
  // (e.g., the URL of a session tab).
  std::vector<sync_pb::SyncEntity> GetSyncEntitiesByModelType(
      syncer::ModelType model_type);

  // Adds the FakeServerEntity* owned by |entity| to the server's collection
  // of entities. This method makes no guarantees that the added entity will
  // result in successful server operations.
  void InjectEntity(scoped_ptr<FakeServerEntity> entity);

  // Sets a new store birthday so that tests may trigger a NOT_MY_BIRTHDAY
  // error. If |store_birthday| is the same as |store_birthday_|, false is
  // returned and this method has no effect.
  bool SetNewStoreBirthday(const std::string& store_birthday);

  // Puts the server in a state where it acts as if authentication has
  // succeeded.
  void SetAuthenticated();

  // Puts the server in a state where all commands will fail with an
  // authentication error.
  void SetUnauthenticated();

  // Force the server to return |error_type| in the error_code field of
  // ClientToServerResponse on all subsequent sync requests. This method should
  // not be called if TriggerActionableError has previously been called. Returns
  // true if error triggering was successfully configured.
  bool TriggerError(const sync_pb::SyncEnums::ErrorType& error_type);

  // Force the server to return the given data as part of the error field of
  // ClientToServerResponse on all subsequent sync requests. This method should
  // not be called if TriggerError has previously been called. Returns true if
  // error triggering was successfully configured.
  bool TriggerActionableError(
    const sync_pb::SyncEnums::ErrorType& error_type,
    const std::string& description,
    const std::string& url,
    const sync_pb::SyncEnums::Action& action);

  // Instructs the server to send triggered errors on every other request
  // (starting with the first one after this call). This feature can be used to
  // test the resiliency of the client when communicating with a problematic
  // server or flaky network connection. This method should only be called
  // after a call to TriggerError or TriggerActionableError. Returns true if
  // triggered error alternating was successful.
  bool EnableAlternatingTriggeredErrors();

  // Adds |observer| to FakeServer's observer list. This should be called
  // before the Profile associated with |observer| is connected to the server.
  void AddObserver(Observer* observer);

  // Removes |observer| from the FakeServer's observer list. This method
  // must be called if AddObserver was ever called with |observer|.
  void RemoveObserver(Observer* observer);

  // Undoes the effects of DisableNetwork.
  void EnableNetwork();

  // Forces every request to fail in a way that simulates a network failure.
  // This can be used to trigger exponential backoff in the client.
  void DisableNetwork();

 private:
  typedef std::map<std::string, FakeServerEntity*> EntityMap;

  // Processes a GetUpdates call.
  bool HandleGetUpdatesRequest(const sync_pb::GetUpdatesMessage& get_updates,
                               sync_pb::GetUpdatesResponse* response);

  // Processes a Commit call.
  bool HandleCommitRequest(const sync_pb::CommitMessage& message,
                           const std::string& invalidator_client_id,
                           sync_pb::CommitResponse* response);

  bool CreatePermanentBookmarkFolder(const char* server_tag, const char* name);

  // Inserts the default permanent items in |entities_|.
  bool CreateDefaultPermanentItems();

  // Inserts the mobile bookmarks folder entity in |entities_|.
  bool CreateMobileBookmarksPermanentItem();

  // Saves a |entity| to |entities_|.
  void SaveEntity(FakeServerEntity* entity);

  // Commits a client-side SyncEntity to the server as a FakeServerEntity.
  // The client that sent the commit is identified via |client_guid|. The
  // parent ID string present in |client_entity| should be ignored in favor
  // of |parent_id|. If the commit is successful, the entity's server ID string
  // is returned and a new FakeServerEntity is saved in |entities_|.
  std::string CommitEntity(
      const sync_pb::SyncEntity& client_entity,
      sync_pb::CommitResponse_EntryResponse* entry_response,
      std::string client_guid,
      std::string parent_id);

  // Populates |entry_response| based on |entity|. It is assumed that
  // SaveEntity has already been called on |entity|.
  void BuildEntryResponseForSuccessfulCommit(
      sync_pb::CommitResponse_EntryResponse* entry_response,
      FakeServerEntity* entity);

  // Determines whether the SyncEntity with id_string |id| is a child of an
  // entity with id_string |potential_parent_id|.
  bool IsChild(const std::string& id, const std::string& potential_parent_id);

  // Creates and saves tombstones for all children of the entity with the given
  // |id|. A tombstone is not created for the entity itself.
  bool DeleteChildren(const std::string& id);

  // Returns whether a triggered error should be sent for the request.
  bool ShouldSendTriggeredError() const;

  // This is the last version number assigned to an entity. The next entity will
  // have a version number of version_ + 1.
  int64 version_;

  // The current store birthday value.
  std::string store_birthday_;

  // Whether the server should act as if incoming connections are properly
  // authenticated.
  bool authenticated_;

  // All SyncEntity objects saved by the server. The key value is the entity's
  // id string.
  EntityMap entities_;

  // All Keystore keys known to the server.
  std::vector<std::string> keystore_keys_;

  // Used as the error_code field of ClientToServerResponse on all responses
  // except when |triggered_actionable_error_| is set.
  sync_pb::SyncEnums::ErrorType error_type_;

  // Used as the error field of ClientToServerResponse when its pointer is not
  // NULL.
  scoped_ptr<sync_pb::ClientToServerResponse_Error> triggered_actionable_error_;

  // These values are used in tandem to return a triggered error (either
  // |error_type_| or |triggered_actionable_error_|) on every other request.
  // |alternate_triggered_errors_| is set if this feature is enabled and
  // |request_counter_| is used to send triggered errors on odd-numbered
  // requests. Note that |request_counter_| can be reset and is not necessarily
  // indicative of the total number of requests handled during the object's
  // lifetime.
  bool alternate_triggered_errors_;
  int request_counter_;

  // FakeServer's observers.
  ObserverList<Observer, true> observers_;

  // When true, the server operates normally. When false, a failure is returned
  // on every request. This is used to simulate a network failure on the client.
  bool network_enabled_;
};

}  // namespace fake_server

#endif  // SYNC_TEST_FAKE_SERVER_FAKE_SERVER_H_
