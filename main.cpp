// cmake . && make -j32  && ./main
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#define MAIN_LOOP_DELAY 30000 // microseconds
#define TAP_TEMPO_AVERAGES 10
#define TAP_TEMPO_RESET_TIME 2
using namespace std;

////////////////////
// TapTempo class //
////////////////////
class TapTempo {
  uint8_t bpm_index;
  double bpms[TAP_TEMPO_AVERAGES];
  uint32_t counter;
  uint32_t last_press;
  const double weights[10] = {1, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};

public:
  void Init() { printf("[taptempo] inited\n"); }

  // TapTempo state machine
  void Update(bool &button_press) {
    counter++;
    if (button_press) {
      double time_since_last_press =
          ((double)(counter - last_press) * MAIN_LOOP_DELAY / 1e6);
      if (time_since_last_press > TAP_TEMPO_RESET_TIME) {
        printf("[taptempo] reset!\n");
        printf("[taptempo] time_since_last_press: %2.1f",
               time_since_last_press);
        for (uint8_t i = 0; i < TAP_TEMPO_AVERAGES; i++) {
          bpms[i] = 0;
        }
      } else if (last_press > 0) {
        double bpm = 60.0 / time_since_last_press;
        bpms[bpm_index] = bpm;
        bpm_index++;
        if (bpm_index >= TAP_TEMPO_AVERAGES) {
          bpm_index = 0;
        }
        // average all non-zero bpms
        double total_bpm = 0;
        double total_counter = 0;
        for (uint8_t j = 0; j < TAP_TEMPO_AVERAGES; j++) {
          int i = bpm_index - j;
          if (i < 0) {
            i += TAP_TEMPO_AVERAGES;
          }
          if (bpms[i] > 0) {
            total_bpm += bpms[i] * weights[j];
            total_counter += weights[j];
          }
        }
        double bpm_average = total_bpm / total_counter;

        printf("[taptempo] pressed: %d, last_press: %d, bpm: %2.1f, "
               "bpm_average: %2.1f\n",
               counter, last_press, bpm, bpm_average);
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
