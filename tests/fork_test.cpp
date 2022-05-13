#include "jchannel/jchannel.h"

#include <iostream>

#include <gtest/gtest.h>

using namespace jchannel;

TEST(ChannelSuite, Channel) {
  auto channel = Channel{};
  auto input = channel.get_input();
  auto output = channel.get_output();

  if (int id = fork(); id == 0) { // child
    output->get_handler().duplicate(1);

    char * const args[] = {
      (char *)("/usr/bin/echo"),
      (char *)("-n"),
      (char *)("SOMEDATA"),
      nullptr
    };

    execve(args[0], args, environ);

    exit(0);
  } else if (id > 0) {
    struct result_t {
      char data[256];
    };

    auto result = input->read<result_t>();

    ASSERT_EQ(std::string{result.value().data}, "SOMEDATA");
  }
}

TEST(ChannelSuite, ChannelWithCloseOnExec) {
  auto channel = Channel<CloseOnExec>{};
  auto input = channel.get_input();
  auto output = channel.get_output();

  if (int id = fork(); id == 0) { // child
    output->get_handler().duplicate(1);

    char * const args[] = {
      (char *)("/usr/bin/echo"),
      (char *)("SOMEDATA"),
      nullptr
    };

    execve(args[0], args, environ);

    exit(0);
  } else if (id > 0) {
    struct result_t {
      char data[256];
    };

    auto result = input->read_for(std::chrono::seconds{10});

    ASSERT_EQ(result.has_value(), false);
  }
}

