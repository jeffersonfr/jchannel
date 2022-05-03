#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

TEST(ChannelSuite, EmptyPoll) {
  jchannel::poll(
    [](auto &&) {
    });
}
