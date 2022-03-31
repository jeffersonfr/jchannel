#include "jchannel/jchannel.h"

#include <iostream>

int main() {
  using namespace jchannel;

  Channel channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  auto p = poll(
    [&](auto & in) mutable {
      assert(in == input);
    }, input, output);

  if (p == false) {
    return 1;
  }

  p.close();

  if (p == true) {
    return 1;
  }

  return 0;
}
