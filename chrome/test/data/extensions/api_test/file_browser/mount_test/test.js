// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// These have to be sync'd with file_manager_private_apitest.cc
var expectedVolume1 = {
  volumeId: 'removable:mount_path1',
  volumeLabel: 'mount_path1',
  sourcePath: 'device_path1',
  volumeSource: 'device',
  volumeType: 'removable',
  deviceType: 'usb',
  devicePath: 'system_path_prefix1',
  isParentDevice: false,
  isReadOnly: false,
  hasMedia: false,
  profile: {profileId: "", displayName: "", isCurrentProfile: true}
};

var expectedVolume2 = {
  volumeId: 'removable:mount_path2',
  volumeLabel: 'mount_path2',
  sourcePath: 'device_path2',
  volumeSource: 'device',
  volumeType: 'removable',
  deviceType: 'mobile',
  devicePath: 'system_path_prefix2',
  isParentDevice: true,
  isReadOnly: true,
  hasMedia: true,
  profile: {profileId: "", displayName: "", isCurrentProfile: true}
};

var expectedVolume3 = {
  volumeId: 'removable:mount_path3',
  volumeLabel: 'mount_path3',
  sourcePath: 'device_path3',
  volumeSource: 'device',
  volumeType: 'removable',
  deviceType: 'optical',
  devicePath: 'system_path_prefix3',
  isParentDevice: true,
  isReadOnly: false,
  hasMedia: false,
  profile: {profileId: "", displayName: "", isCurrentProfile: true}
};

var expectedDownloadsVolume = {
  volumeId: /^downloads:Downloads[^\/]*$/,
  volumeLabel: '',
  volumeSource: 'device',
  volumeType: 'downloads',
  isReadOnly: false,
  hasMedia: false,
  profile: {profileId: "", displayName: "", isCurrentProfile: true}
};

var expectedDriveVolume = {
  volumeId: /^drive:drive[^\/]*$/,
  volumeLabel: '',
  sourcePath: /^\/special\/drive[^\/]*$/,
  volumeSource: 'network',
  volumeType: 'drive',
  isReadOnly: false,
  hasMedia: false,
  profile: {profileId: "", displayName: "", isCurrentProfile: true}
};

var expectedArchiveVolume = {
  volumeId: 'archive:archive_mount_path',
  volumeLabel: 'archive_mount_path',
  sourcePath: /removable\/mount_path3\/archive.zip$/,
  volumeSource: 'file',
  volumeType: 'archive',
  isReadOnly: true,
  hasMedia: false,
  profile: {profileId: "", displayName: "", isCurrentProfile: true}
};

// List of expected mount points.
// NOTE: this has to be synced with values in file_manager_private_apitest.cc
//       and values sorted by volumeId.
var expectedVolumeList = [
  expectedArchiveVolume,
  expectedDownloadsVolume,
  expectedDriveVolume,
  expectedVolume1,
  expectedVolume2,
  expectedVolume3,
];

function validateObject(received, expected, name) {
  for (var key in expected) {
    if (expected[key] instanceof RegExp) {
      if (!expected[key].test(received[key])) {
        console.warn('Expected "' + key + '" ' + name + ' property to match: ' +
                     expected[key] + ', but got: "' + received[key] + '".');
        return false;
      }
    } else if (expected[key] instanceof Object) {
      if (!validateObject(received[key], expected[key], name + "." + key))
        return false;
    } else if (received[key] != expected[key]) {
      console.warn('Expected "' + key + '" ' + name + ' property to be: "' +
                  expected[key] + '"' + ', but got: "' + received[key] +
                  '" instead.');
      return false;
    }
  }

  var expectedKeys = Object.keys(expected);
  var receivedKeys = Object.keys(received);
  if (expectedKeys.length !== receivedKeys.length) {
    var unexpectedKeys = [];
    for (var i = 0; i < receivedKeys.length; i++) {
      if (!(receivedKeys[i] in expected))
        unexpectedKeys.push(receivedKeys[i]);
    }

    console.warn('Unexpected properties found: ' + unexpectedKeys);
    return false;
  }
  return true;
}

chrome.test.runTests([
  function removeMount() {
    chrome.fileManagerPrivate.removeMount('archive:archive_mount_path');

    // We actually check this one on C++ side. If MountLibrary.RemoveMount
    // doesn't get called, test will fail.
    chrome.test.succeed();
  },

  function getVolumeMetadataList() {
    chrome.fileManagerPrivate.getVolumeMetadataList(
        chrome.test.callbackPass(function(result) {
          chrome.test.assertEq(expectedVolumeList.length, result.length,
              'getMountPoints returned wrong number of mount points.');
          for (var i = 0; i < expectedVolumeList.length; i++) {
            chrome.test.assertTrue(
                validateObject(
                    result[i], expectedVolumeList[i], 'volumeMetadata'),
                'getMountPoints result[' + i + '] not as expected');
          }
    }));
  }
]);
