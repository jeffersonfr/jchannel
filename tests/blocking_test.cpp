#include "jchannel/jchannel.h"

#include <iostream>

using namespace jchannel;

int main() {
  Channel<NonBlocking> nonBlockingChannel;
  auto nonBlockingInput = nonBlockingChannel.get_input();
  auto rNonBlockingInput = nonBlockingInput->read();

  if (!rNonBlockingInput) {
    std::cout << rNonBlockingInput.error() << std::endl;
  }

  Channel blockingChannel;
  auto blockingInput = blockingChannel.get_input();
  auto rBlockingInput = blockingInput->read_for(std::chrono::seconds{2});

  if (!rBlockingInput) {
    std::cout << rBlockingInput.error() << std::endl;
  }

  signal(SIGALRM,
      [](int sig) {
        exit(0);
      });

  alarm(2);

  rBlockingInput = blockingInput->read();

  return 1;
}
