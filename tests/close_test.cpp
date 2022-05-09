#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<jchannel::PacketMode> mChannel;
    jchannel::Input input = mChannel.get_input();
    jchannel::Output output = mChannel.get_output();

};

TEST_F(ChannelSuite, CloseChannel) {
  auto p = poll(
    [&](auto & in) mutable {
      ASSERT_EQ(in, input);
    }, input, output);

  if (p == false) {
    FAIL();
  }

  p.close();

  if (p == true) {
    FAIL();
  }
}
