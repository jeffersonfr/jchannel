#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

const int MAX_ITERATIONS = 1000;

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<jchannel::Empty> mChannel;

};

TEST_F(ChannelSuite, UnreadedData) {
  auto input = mChannel.get_input();
  auto output = mChannel.get_output();
  
  output->write();

  int counter = 0;

  auto p = poll(
    [&](auto & in) {
      counter++;
    }, input);

  for (int i=0; i<MAX_ITERATIONS; i++) {
    p();
  }

  EXPECT_EQ(counter, MAX_ITERATIONS);
}
