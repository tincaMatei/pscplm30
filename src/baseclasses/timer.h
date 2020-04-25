#ifndef __TIMER_H
#define __TIMER_H

#include <SDL2/SDL.h>

class Timer {
private:
  // Last time since we checked the timer
  Uint32 lastT;
  // Check if the timer is activated
  bool active;
public:
  Timer();
  
  // Start the timer
  void startTimer();
  // Stop the timer
  void stopTimer();
  // Reset the timer
  void resetTimer();
  // Get the time measured by the timer
  Uint32 getTimeAmmount();
  // Get the ammount of milliseconds since SDL was initialized
  Uint32 getTime();
};

#endif
