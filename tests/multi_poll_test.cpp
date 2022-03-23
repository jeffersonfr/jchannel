#include "jchannel/jchannel.h"

#include <iostream>
#include <cassert>

using namespace jchannel;

int main() {
  Channel ch1;
  auto in1 = ch1.get_input();
  auto out1 = ch1.get_output();
  
  Channel ch2;
  auto in2 = ch2.get_input();
  auto out2 = ch2.get_output();

  Channel ch3;
  auto in3 = ch3.get_input();
  auto out3 = ch3.get_output();

  out1->write();

  auto p = poll(
    [&](auto && in) {
      in->read();

      if (in == in1) {
        out3->write();
      } else if (in == in2) {
      } else if (in == in3) {
        exit(0);
      }
    }, in1, in2, in3);

  p();
  p();

  return 1;
}
