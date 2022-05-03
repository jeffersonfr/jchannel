#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

TEST(ChannelSuite, Ctor) {
  jchannel::Channel<jchannel::NonBlocking> mNonBlocking;
  jchannel::Channel<jchannel::CloseOnExec> mCloseOnExec;
  jchannel::Channel<jchannel::PacketMode> mPacketMode;
  jchannel::Channel<> mNone;
}
