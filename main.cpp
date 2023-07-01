// cmake . && make -j32  && ./main
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#define MAIN_LOOP_DELAY 25000 // microseconds
using namespace std;

////////////////////
// TapTempo class //
////////////////////
class TapTempo {
  uint32_t counter;
  uint32_t last_press;

public:
  void Init() { printf("[taptempo] inited\n"); }

  // TapTempo state machine
  void Update(bool &button_press) {
    counter++;
    if (button_press) {
      double time_since_last_press =
          ((double)(counter - last_press) * MAIN_LOOP_DELAY / 1e6);
      if (time_since_last_press > 3) {
        printf("[taptempo] reset!\n");
        printf("[taptempo] time_since_last_press: %2.1f",
               time_since_last_press);
      } else {
        double bpm = 60.0 / time_since_last_press;
        printf("[taptempo] pressed: %d, last_press: %d, bpm: %2.1f\n", counter,
               last_press, bpm);
        // TODO: call function to set device bpm
      }
      // depress button
      button_press = false;
      // set last press time
      last_press = counter;
    }
  }
};
////////////////////
////////////////////

/** tap tempo simulator */
TapTempo taptempo;
bool button_press;

void button_emulator() {
  // https://stackoverflow.com/questions/24708700/c-detect-when-user-presses-arrow-key
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag &= ~ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  char a;
  while (true) {
    cin >> a;
    button_press = true;
    if (a == 120) { // "x"
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  // intialize button emulator
  thread first(button_emulator);
  // initialize tap tempo
  taptempo.Init();

  // main state machine
  while (true) {
    taptempo.Update(button_press);
    usleep(MAIN_LOOP_DELAY);
  }
  first.join();
  return 0;
}
