#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<> mChannel;
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
        ASSERT_EQ(input->template read<int>().value(), 0x11223344);
      } else if (i == 1) {
        ASSERT_EQ(input->template read<char>().value(), 'A');
      } else if (i == 2) {
        ASSERT_EQ(input->template read<long>().value(), 0x66778899aabbccdd);
      }

      i++;
    }, input);

  p();
  p();
  p();
}

TEST_F(ChannelSuite, UsingFragmentedSize) {
  output->write(static_cast<int>(0x11223344));
  output->write('A');
  output->write(static_cast<long>(0x66778899aabbccdd));

  auto p = poll(
    [](auto && input) {
      static int i = 0;

      if (i == 0) {
        ASSERT_EQ(input->template read<short>().value(), 0x3344);
      } else if (i == 1) {
        ASSERT_EQ(input->template read<short>().value(), 0x1122);
      } else if (i == 2) {
        ASSERT_EQ(input->template read<char>().value(), 'A');
      } else if (i == 3) {
        ASSERT_EQ(input->template read<short>().value(), static_cast<short>(0xccdd));
      } else if (i == 4) {
        ASSERT_EQ(input->template read<short>().value(), static_cast<short>(0xaabb));
      } else if (i == 5) {
        ASSERT_EQ(input->template read<short>().value(), static_cast<short>(0x8899));
      } else if (i == 6) {
        ASSERT_EQ(input->template read<short>().value(), static_cast<short>(0x6677));
      }


      i++;
    }, input);

  p();
  p();
  p();
  p();
  p();
  p();
  p();

}

TEST_F(ChannelSuite, ReadingMoreThanNecessarySize) {
  output->write(static_cast<int>(0x11223344));

  auto p = poll(
    [](auto && input) {
      ASSERT_EQ((input->template read<long>().value() & 0x00000000ffffffff), 0x11223344);
    }, input);

  p();
}

