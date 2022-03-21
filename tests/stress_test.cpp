#include "jchannel/jchannel.h"

#include <iostream>

int main() {
  using namespace jchannel;

  Channel<Empty> myChannel;
  Input input = myChannel.get_input();
  Output output = myChannel.get_output();

  for (int i=0; i<1000000; i++) {
    output->write();
    input->read();
  }

  return 0;
}
