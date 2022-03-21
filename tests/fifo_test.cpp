#include "jchannel/jchannel.h"

#include <iostream>

using namespace jchannel;

class Fifo {

  public:
    Fifo() {
      Channel channel1;
      Channel channel2;

      input1 = channel1.get_input();
      output1 = channel1.get_output();
      
      input2 = channel2.get_input();
      output2 = channel2.get_output();
    }

    ~Fifo() {
    }

    void read() {
      input1->read();
      output2->write();
    }

    void write() {
      output1->write();
      input2->read();
    }

  private:
    Input input1;
    Output output1;
    Input input2;
    Output output2;

};

int main() {

  auto now = std::chrono::steady_clock::now();

  Fifo f;

  if (int id = fork(); id == 0) {
    std::this_thread::sleep_for(std::chrono::seconds{2});

    f.read();

    exit(0);
  } else if (id > 0) {
    f.write();

    if (auto time = std::chrono::steady_clock::now() - now; time < std::chrono::seconds{2}) {
      exit(1);
    }

    exit(0);
  }

  return 0;
}
