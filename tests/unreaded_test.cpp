#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

using namespace jchannel;

const int MAX_ITERATIONS = 1000;

int main() {
  Channel channel;
  auto input = channel.get_input();
  auto output = channel.get_output();
  
  output->write();

  int counter = 0;

  auto p = poll(
    [&](auto & in) {
      counter++;
    }, input);

  for (int i=0; i<MAX_ITERATIONS; i++) {
    p();
  }

  assert(counter == MAX_ITERATIONS);

  return 0;
}
