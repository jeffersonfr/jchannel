#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class Fifo {

  public:
    Fifo() {
      jchannel::Channel channel1;
      jchannel::Channel channel2;

      input1 = channel1.get_input();
      output1 = channel1.get_output();
      
      input2 = channel2.get_input();
      output2 = channel2.get_output();
    }

    void read() {
      input1->read();
      output2->write();
    }

    void write() {
      output1->write();
      input2->read();
    }

  private:
    jchannel::Input input1;
    jchannel::Output output1;
    jchannel::Input input2;
    jchannel::Output output2;

};


class ChannelSuite : public ::testing::Test {

  protected:
    Fifo mFifo;

};

TEST_F(ChannelSuite, Fifo) {
  auto now = std::chrono::steady_clock::now();

  if (int id = fork(); id == 0) {
    std::this_thread::sleep_for(std::chrono::seconds{2});

    mFifo.read();
  } else if (id > 0) {
    mFifo.write();

    if (auto time = std::chrono::steady_clock::now() - now; time < std::chrono::seconds{2}) {
      FAIL();
    }
  }
}
