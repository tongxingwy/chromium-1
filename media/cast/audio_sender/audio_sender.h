// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAST_AUDIO_SENDER_H_
#define MEDIA_CAST_AUDIO_SENDER_H_

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "base/time/tick_clock.h"
#include "base/time/time.h"
#include "media/base/audio_bus.h"
#include "media/cast/cast_config.h"
#include "media/cast/rtcp/rtcp.h"
#include "media/cast/rtp_timestamp_helper.h"

namespace media {
namespace cast {

class AudioEncoder;

// This class is not thread safe.
// It's only called from the main cast thread.
class AudioSender : public RtcpSenderFeedback,
                    public base::NonThreadSafe,
                    public base::SupportsWeakPtr<AudioSender> {
 public:
  AudioSender(scoped_refptr<CastEnvironment> cast_environment,
              const AudioSenderConfig& audio_config,
              transport::CastTransportSender* const transport_sender);

  virtual ~AudioSender();

  CastInitializationStatus InitializationResult() const {
    return cast_initialization_status_;
  }

  // Note: It is invalid to call this method if InitializationResult() returns
  // anything but STATUS_AUDIO_INITIALIZED.
  void InsertAudio(scoped_ptr<AudioBus> audio_bus,
                   const base::TimeTicks& recorded_time);

  // Only called from the main cast thread.
  void IncomingRtcpPacket(scoped_ptr<Packet> packet);

 private:
  void ResendPackets(
      const MissingFramesAndPacketsMap& missing_frames_and_packets);

  void ScheduleNextRtcpReport();
  void SendRtcpReport(bool schedule_future_reports);

  // Called by the |audio_encoder_| with the next EncodedFrame to send.
  void SendEncodedAudioFrame(scoped_ptr<transport::EncodedFrame> audio_frame);

  virtual void OnReceivedCastFeedback(const RtcpCastMessage& cast_feedback)
      OVERRIDE;

  scoped_refptr<CastEnvironment> cast_environment_;
  transport::CastTransportSender* const transport_sender_;
  scoped_ptr<AudioEncoder> audio_encoder_;
  RtpTimestampHelper rtp_timestamp_helper_;
  Rtcp rtcp_;
  int num_aggressive_rtcp_reports_sent_;

  // If this sender is ready for use, this is STATUS_AUDIO_INITIALIZED.
  CastInitializationStatus cast_initialization_status_;

  // Used to map the lower 8 bits of the frame id to a RTP timestamp. This is
  // good enough as we only use it for logging.
  RtpTimestamp frame_id_to_rtp_timestamp_[256];

  // NOTE: Weak pointers must be invalidated before all other member variables.
  base::WeakPtrFactory<AudioSender> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AudioSender);
};

}  // namespace cast
}  // namespace media

#endif  // MEDIA_CAST_AUDIO_SENDER_H_
