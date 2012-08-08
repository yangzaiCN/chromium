// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_CHANNEL_MULTIPLEXER_H_
#define REMOTING_PROTOCOL_CHANNEL_MULTIPLEXER_H_

#include "remoting/proto/mux.pb.h"
#include "remoting/protocol/buffered_socket_writer.h"
#include "remoting/protocol/channel_factory.h"
#include "remoting/protocol/message_reader.h"

namespace remoting {
namespace protocol {

class ChannelMultiplexer : public ChannelFactory {
 public:
  static const char kMuxChannelName[];

  // |factory| is used to create the channel upon which to multiplex.
  ChannelMultiplexer(ChannelFactory* factory,
                     const std::string& base_channel_name);
  virtual ~ChannelMultiplexer();

  // ChannelFactory interface.
  virtual void CreateStreamChannel(
      const std::string& name,
      const StreamChannelCallback& callback) OVERRIDE;
  virtual void CreateDatagramChannel(
      const std::string& name,
      const DatagramChannelCallback& callback) OVERRIDE;
  virtual void CancelChannelCreation(const std::string& name) OVERRIDE;

 private:
  struct PendingChannel;
  class MuxChannel;
  class MuxSocket;
  friend class MuxChannel;

  // Callback for |base_channel_| creation.
  void OnBaseChannelReady(scoped_ptr<net::StreamSocket> socket);

  // Helper method used to create channels.
  MuxChannel* GetOrCreateChannel(const std::string& name);

  // Callbacks for |writer_| and |reader_|.
  void OnWriteFailed(int error);
  void OnIncomingPacket(scoped_ptr<MultiplexPacket> packet,
                        const base::Closure& done_task);

  // Called by MuxChannel.
  bool DoWrite(scoped_ptr<MultiplexPacket> packet,
               const base::Closure& done_task);

  // Factory used to create |base_channel_|. Set to NULL once creation is
  // finished or failed.
  ChannelFactory* base_channel_factory_;

  // Name of the underlying channel.
  std::string base_channel_name_;

  // The channel over which to multiplex.
  scoped_ptr<net::StreamSocket> base_channel_;

  // List of requested channels while we are waiting for |base_channel_|.
  std::list<PendingChannel> pending_channels_;

  int next_channel_id_;
  std::map<std::string, MuxChannel*> channels_;

  // Channels are added to |channels_by_receive_id_| only after we receive
  // receive_id from the remote peer.
  std::map<int, MuxChannel*> channels_by_receive_id_;

  BufferedSocketWriter writer_;
  ProtobufMessageReader<MultiplexPacket> reader_;

  // Flag used by OnWriteFailed() to detect when the multiplexer is destroyed.
  bool* destroyed_flag_;

  DISALLOW_COPY_AND_ASSIGN(ChannelMultiplexer);
};

}  // namespace protocol
}  // namespace remoting


#endif  // REMOTING_PROTOCOL_CHANNEL_MULTIPLEXER_H_
