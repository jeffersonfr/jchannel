#include "jchannel/jchannel.h"

#include <iostream>

int main() {
  using namespace jchannel;

  poll(
    [](auto &&) {
    });

  return 0;
}
