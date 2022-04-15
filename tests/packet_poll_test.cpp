#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<jchannel::PacketMode> mChannel;
    jchannel::Input input = mChannel.get_input();
    jchannel::Output output = mChannel.get_output();

};

TEST_F(ChannelSuite, UsingExactSize) {
  output->write(static_cast<int>(0x11223344));
  output->write('A');
  output->write(static_cast<long>(0x66778899aabbccdd));

  auto p = poll(
    [](auto && input) {
      static int i = 0;

      if (i == 0) {
        assert(input->template read<int>().value() == 0x11223344);
      } else if (i == 1) {
        assert(input->template read<char>().value() == 'A');
      } else if (i == 2) {
        assert(input->template read<long>().value() == 0x66778899aabbccdd);
      }

      i++;
    }, input);

  p();
  p();
  p();
}

TEST_F(ChannelSuite, UsingFragmentedSize) {
  output->write(static_cast<int>(0x11223344));
  output->write("Hello, world !");
  output->write(static_cast<long>(0x66778899aabbccdd));

  auto p = poll(
    [](auto && input) {
      static int i = 0;

      if (i == 0) {
        assert(input->template read<int>().value() == 0x11223344);
      } else if (i == 1) {
        assert(input->template read<char>().value() == 'H');
      } else if (i == 2) {
        assert(input->template read<long>().value() == 0x66778899aabbccdd);
      }

      i++;
    }, input);

  p();
  p();
  p();
}

TEST_F(ChannelSuite, ReadingMoreThanNecessarySize) {
  output->write(static_cast<int>(0x11223344));

  auto p = poll(
    [](auto && input) {
      assert((input->template read<long>().value() & 0x00000000ffffffff) == 0x11223344);
    }, input);

  p();
}

