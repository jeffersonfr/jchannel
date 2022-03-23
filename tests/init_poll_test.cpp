#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

using namespace jchannel;

int main() {
  Channel channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  output->write();
  
  auto p = poll(
    [&](auto & in) mutable {
      assert(in == input);
    }, input);

  p();

  return 0;
}
