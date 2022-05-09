#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<> mChannel;
    jchannel::Input input = mChannel.get_input();
    jchannel::Output output = mChannel.get_output();

};

TEST_F(ChannelSuite, PollChannel) {
  output->write();
  
  auto p = poll(
    [&](auto & in) mutable {
      ASSERT_EQ(in, input);
    }, input);

  p();
}
