#include "jchannel/jchannel.h"

#include <iostream>

int main() {
  using namespace jchannel;

  signal(SIGPIPE,
      [](int sig) {
        exit(0);
      });

  Channel<Empty> myChannel;
  Input input = myChannel.get_input();
  Output output = myChannel.get_output();

  input->close();
  output->write();

  return 1;
}
