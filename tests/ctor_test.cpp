#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<jchannel::NonBlocking> mNonBlocking;
    jchannel::Channel<jchannel::CloseOnExec> mCloseOnExec;
    jchannel::Channel<jchannel::PacketMode> mPacketMode;
    jchannel::Channel<> mNone;

};

TEST_F(ChannelSuite, Ctor) {
}
