#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:

};

TEST_F(ChannelSuite, PipeError) {
  signal(SIGPIPE,
      [](int sig) {
        exit(0);
      });

  jchannel::Channel<jchannel::Empty> myChannel;
  auto input = myChannel.get_input();
  auto output = myChannel.get_output();

  input->close();
  output->write();

  FAIL();
}
