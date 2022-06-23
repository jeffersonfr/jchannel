#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

const int MAX_ITERATIONS = 1000;

TEST(ChannelSuite, UnreadedData) {
  auto [input, output] = jchannel::Channel<jchannel::Empty>{}.get_channels();
  
  output->write();

  int counter = 0;

  auto p = jchannel::poll(
    [&]([[maybe_unused]] auto & in) {
      counter++;
    }, input);

  for (int i=0; i<MAX_ITERATIONS; i++) {
    p();
  }

  EXPECT_EQ(counter, MAX_ITERATIONS);
}
