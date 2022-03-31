#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

int main() {
  using namespace jchannel;

  Channel channel;
  auto input = channel.get_input();
  auto output = channel.get_output();

  signal(SIGALRM,
      [](int sig) {
        exit(0);
      });

  alarm(5);
  
  auto p = poll(
    [&](auto & in) mutable {
      assert(in == input);
    }, input);

  auto t = std::thread(
      [&]() {
        std::this_thread::sleep_for(std::chrono::seconds{2});

        p.close();
      });

  p();

  if (p == true) {
    t.join();

    return 1;
  }

  t.join();

  return 0;
}
