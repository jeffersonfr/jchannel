#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

using namespace jchannel;

void fork_exec(auto && channel) {
  auto input = channel.get_input();
  auto output = channel.get_output();

  if (int id = fork(); id == 0) { // child
    std::this_thread::sleep_for(std::chrono::seconds{2});

    bool r = output->get_handler().duplicate(1);

    char * const args[] = {
      (char *)("/usr/bin/echo"),
      (char *)("SOMEDATA"),
      nullptr
    };

    execve(args[0], args, environ);

    exit(0);
  } else if (id > 0) {
    input->read();
  }
}

int main() {
  signal(SIGALRM,
      [](int sig) {
        exit(0);
      });

  alarm(10);

  fork_exec(Channel{});
  fork_exec(Channel<CloseOnExec>{});

  return 1;
}
