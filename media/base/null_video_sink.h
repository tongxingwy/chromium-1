// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_AUDIO_NULL_VIDEO_SINK_H_
#define MEDIA_AUDIO_NULL_VIDEO_SINK_H_

#include "base/cancelable_callback.h"
#include "base/md5.h"
#include "base/memory/scoped_ptr.h"
#include "base/time/default_tick_clock.h"
#include "base/time/tick_clock.h"
#include "media/base/video_renderer_sink.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace media {

class MEDIA_EXPORT NullVideoSink : NON_EXPORTED_BASE(public VideoRendererSink) {
 public:
  using NewFrameCB = base::Callback<void(const scoped_refptr<VideoFrame>&)>;

  // Periodically calls |callback| every |interval| on |task_runner| once the
  // sink has been started.  If |clockless| is true, the RenderCallback will
  // be called back to back by repeated post tasks. Optionally, if specified,
  // |new_frame_cb| will be called for each new frame received.
  NullVideoSink(bool clockless,
                base::TimeDelta interval,
                const NewFrameCB& new_frame_cb,
                const scoped_refptr<base::SingleThreadTaskRunner>& task_runner);
  ~NullVideoSink() override;

  // VideoRendererSink implementation.
  void Start(RenderCallback* callback) override;
  void Stop() override;
  void PaintFrameUsingOldRenderingPath(
      const scoped_refptr<VideoFrame>& frame) override;

  // Allows tests to simulate suspension of Render() callbacks.
  void PauseRenderCallbacks(base::TimeTicks pause_until);

  void set_tick_clock_for_testing(base::TickClock* tick_clock) {
    tick_clock_ = tick_clock;
  }

  // Sets |stop_cb_|, which will be fired when Stop() is called.
  void set_stop_cb(const base::Closure& stop_cb) {
    stop_cb_ = stop_cb;
  }

 private:
  // Task that periodically calls Render() to consume video data.
  void CallRender();

  const bool clockless_;
  const base::TimeDelta interval_;
  const NewFrameCB new_frame_cb_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  bool started_;
  RenderCallback* callback_;

  // Manages cancellation of periodic Render() callback task.
  base::CancelableClosure cancelable_worker_;

  // Used to determine when a new frame is received.
  scoped_refptr<VideoFrame> last_frame_;

  // Used to determine the interval given to RenderCallback::Render() as well as
  // to maintain stable periodicity of callbacks.
  base::TimeTicks current_render_time_;

  // Used to suspend Render() callbacks to |callback_| for some time.
  base::TimeTicks pause_end_time_;

  // Allow for an injectable tick clock for testing.
  base::DefaultTickClock default_tick_clock_;
  base::TimeTicks last_now_;

  // If specified, used instead of |default_tick_clock_|.
  base::TickClock* tick_clock_;

  // If set, called when Stop() is called.
  base::Closure stop_cb_;

  DISALLOW_COPY_AND_ASSIGN(NullVideoSink);
};

}  // namespace media

#endif  // MEDIA_AUDIO_NULL_VIDEO_SINK_H_
