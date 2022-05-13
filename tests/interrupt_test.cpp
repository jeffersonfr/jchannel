#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<> mChannel;
    jchannel::Input input = mChannel.get_input();
    jchannel::Output output = mChannel.get_output();

};

TEST_F(ChannelSuite, InterruptPolling) {
  signal(SIGALRM,
      []([[maybe_unused]] int sig) {
        exit(0);
      });

  alarm(5);
  
  auto p = poll(
    [&](auto & in) mutable {
      ASSERT_EQ(in, input);
    }, input);

  auto t = std::thread(
      [&]() {
        std::this_thread::sleep_for(std::chrono::seconds{2});

        p.close();
      });

  p();

  if (p == true) {
    t.join();

    FAIL();
  }

  t.join();
}
