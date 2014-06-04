// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_MEDIA_VIDEO_DECODE_ACCELERATOR_IMPL_H_
#define CONTENT_COMMON_GPU_MEDIA_VIDEO_DECODE_ACCELERATOR_IMPL_H_

#include "content/common/content_export.h"
#include "media/video/video_decode_accelerator.h"

namespace content {

class CONTENT_EXPORT VideoDecodeAcceleratorImpl
    : public media::VideoDecodeAccelerator {
 public:
  VideoDecodeAcceleratorImpl();

  // Returns true if VDA::Decode and VDA::Client callbacks can run on the IO
  // thread. Otherwise they will run on the GPU child thread. The purpose of
  // running Decode on the IO thread is to reduce decode latency. Note Decode
  // should return as soon as possible and not block on the IO thread. Also,
  // PictureReady should be run on the child thread if a picture is delivered
  // the first time so it can be cleared.
  virtual bool CanDecodeOnIOThread();

 protected:
  virtual ~VideoDecodeAcceleratorImpl();
};

}  // namespace content

namespace base {

template <class T>
struct DefaultDeleter;

// Specialize DefaultDeleter so that scoped_ptr<VideoDecodeAcceleratorImpl>
// always uses "Destroy()" instead of trying to use the destructor.
template <>
struct DefaultDeleter<content::VideoDecodeAcceleratorImpl> {
 public:
  inline void operator()(void* video_decode_accelerator) const {
    static_cast<content::VideoDecodeAcceleratorImpl*>(video_decode_accelerator)
        ->Destroy();
  }
};

}  // namespace base

#endif  // CONTENT_COMMON_GPU_MEDIA_VIDEO_DECODE_ACCELERATOR_IMPL_H_
