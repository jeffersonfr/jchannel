#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

using namespace jchannel;

void using_exact_size() {
  Channel channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  output->write(static_cast<int>(0x11223344));
  // output->write("Hello, world !");
  output->write('A');
  output->write(static_cast<long>(0x66778899aabbccdd));

  assert(input->read<int>().value() == 0x11223344);
  assert(input->read<char>().value() == 'A');
  assert(input->read<long>().value() == 0x66778899aabbccdd);
}

void using_fragmented_size() {
  Channel channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  output->write(static_cast<int>(0x11223344));
  output->write("Hello, world !");
  output->write(static_cast<long>(0x66778899aabbccdd));

  assert(input->read<short>().value() == 0x3344);
  assert(input->read<short>().value() == 0x1122);
  
  assert(input->read<char>().value() == 'H');
  assert(input->read<char>().value() == 'e');
  assert(input->read<char>().value() == 'l');
  assert(input->read<char>().value() == 'l');
  assert(input->read<char>().value() == 'o');
  assert(input->read<char>().value() == ',');
  assert(input->read<char>().value() == ' ');
  assert(input->read<char>().value() == 'w');
  assert(input->read<char>().value() == 'o');
  assert(input->read<char>().value() == 'r');
  assert(input->read<char>().value() == 'l');
  assert(input->read<char>().value() == 'd');
  assert(input->read<char>().value() == ' ');
  assert(input->read<char>().value() == '!');
  assert(input->read<char>().value() == '\0');
  
  assert(input->read<short>().value() == static_cast<short>(0xccdd));
  assert(input->read<short>().value() == static_cast<short>(0xaabb));
  assert(input->read<short>().value() == static_cast<short>(0x8899));
  assert(input->read<short>().value() == static_cast<short>(0x6677));
}

void reading_more_than_necessary_size() {
  Channel<NonBlocking> channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  output->write(static_cast<int>(0x11223344));

  input->read<long>();

  if (input->read()) {
    exit(1);
  }
}

int main() {
  using_exact_size();
  using_fragmented_size();
  reading_more_than_necessary_size();

  return 0;
}
