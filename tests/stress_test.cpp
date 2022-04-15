#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<jchannel::Empty> mChannel;

};

TEST_F(ChannelSuite, Stress) {
  auto input = mChannel.get_input();
  auto output = mChannel.get_output();

  for (int i=0; i<1000000; i++) {
    output->write();
    input->read();
  }
}
