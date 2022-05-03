#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

TEST(ChannelSuite, NonBlockingChannel) {
  jchannel::Channel<jchannel::NonBlocking> channel;
  auto input = channel.get_input();

  if (input->read()) {
    FAIL();
  }
}

TEST(ChannelSuite, BlockingChannel) {
  jchannel::Channel channel;
  auto input = channel.get_input();

  if (input->read_for(std::chrono::seconds{2})) {
    FAIL();
  }

  signal(SIGALRM,
      [](int sig) {
        exit(0);
      });

  alarm(2);

  input->read();

  FAIL();
}
