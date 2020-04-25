#include "baseclasses/timer.h"

Timer::Timer() {
  active = false;
  lastT = 0;
}

Uint32 Timer::getTimeAmmount() {
  if(active)
    return getTime() - lastT;
  return 0;
}

void Timer::resetTimer() {
  lastT = getTime();
}

void Timer::startTimer() {
  active = true;
  lastT = getTime();
}

void Timer::stopTimer() {
  active = false;
}

Uint32 Timer::getTime() {
  return SDL_GetTicks();
}
