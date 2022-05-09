#include "jchannel/jchannel.h"

#include <gtest/gtest.h>

class ChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<> mChannel;
    jchannel::Input input = mChannel.get_input();
    jchannel::Output output = mChannel.get_output();

};

class NonBlockingChannelSuite : public ::testing::Test {

  protected:
    jchannel::Channel<jchannel::NonBlocking> mChannel;
    jchannel::Input input = mChannel.get_input();
    jchannel::Output output = mChannel.get_output();

};

TEST_F(ChannelSuite, UsingExactSize) {
  output->write(static_cast<int>(0x11223344));
  // output->write("Hello, world !");
  output->write('A');
  output->write(static_cast<long>(0x66778899aabbccdd));

  ASSERT_EQ(input->read<int>().value(), 0x11223344);
  ASSERT_EQ(input->read<char>().value(), 'A');
  ASSERT_EQ(input->read<long>().value(), 0x66778899aabbccdd);
}

TEST_F(ChannelSuite, UsingFragmentedSize) {
  output->write(static_cast<int>(0x11223344));
  output->write("Hello, world !");
  output->write(static_cast<long>(0x66778899aabbccdd));

  ASSERT_EQ(input->read<short>().value(), 0x3344);
  ASSERT_EQ(input->read<short>().value(), 0x1122);
  
  ASSERT_EQ(input->read<char>().value(), 'H');
  ASSERT_EQ(input->read<char>().value(), 'e');
  ASSERT_EQ(input->read<char>().value(), 'l');
  ASSERT_EQ(input->read<char>().value(), 'l');
  ASSERT_EQ(input->read<char>().value(), 'o');
  ASSERT_EQ(input->read<char>().value(), ',');
  ASSERT_EQ(input->read<char>().value(), ' ');
  ASSERT_EQ(input->read<char>().value(), 'w');
  ASSERT_EQ(input->read<char>().value(), 'o');
  ASSERT_EQ(input->read<char>().value(), 'r');
  ASSERT_EQ(input->read<char>().value(), 'l');
  ASSERT_EQ(input->read<char>().value(), 'd');
  ASSERT_EQ(input->read<char>().value(), ' ');
  ASSERT_EQ(input->read<char>().value(), '!');
  ASSERT_EQ(input->read<char>().value(), '\0');
  
  ASSERT_EQ(input->read<short>().value(), static_cast<short>(0xccdd));
  ASSERT_EQ(input->read<short>().value(), static_cast<short>(0xaabb));
  ASSERT_EQ(input->read<short>().value(), static_cast<short>(0x8899));
  ASSERT_EQ(input->read<short>().value(), static_cast<short>(0x6677));
}

TEST_F(NonBlockingChannelSuite, ReadingMoreThanNecessarySize) {
  output->write(static_cast<int>(0x11223344));

  input->read<long>();

  if (input->read()) {
    FAIL();
  }
}

