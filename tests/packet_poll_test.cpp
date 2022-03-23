#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

using namespace jchannel;

void using_exact_size() {
  Channel<PacketMode> channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

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

void using_fragmented_size() {
  Channel<PacketMode> channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

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

void reading_more_than_necessary_size() {
  Channel<PacketMode> channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  output->write(static_cast<int>(0x11223344));

  auto p = poll(
    [](auto && input) {
      assert((input->template read<long>().value() & 0x00000000ffffffff) == 0x11223344);
    }, input);

  p();
}

int main() {
  using_exact_size();
  using_fragmented_size();
  reading_more_than_necessary_size();

  return 0;
}
