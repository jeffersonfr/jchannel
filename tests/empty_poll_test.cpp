#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:

};

TEST_F(ChannelSuite, EmptyPoll) {
  jchannel::poll(
    [](auto &&) {
    });
}
