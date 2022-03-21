#include "jchannel/jchannel.h"

#include <iostream>

int main() {
  using namespace jchannel;

  Channel<NonBlocking> ch1;
  Channel<CloseOnExec> ch2;
  Channel<PacketMode> ch3;
  Channel ch4;

  return 0;
}
