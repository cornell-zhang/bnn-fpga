//---------------------------------------------------------
// Timer.cpp
//---------------------------------------------------------
#include "Timer.h"

#ifdef TIMER_ON
//---------------------------------------------------------
// Timer is active
//---------------------------------------------------------

Timer::Timer(const char* Name, bool On) {
  if (On) {
    // record the start time
    gettimeofday(&ts_start, NULL);
    nCalls = 1;
  }
  else {
    nCalls = 0;
  }
  totalTime = 0;	
  strcpy(binName, Name);
}

Timer::~Timer() {
  // on being destroyed, print the average and total time
  if (nCalls > 0) {
    printf ("%-20s: ", binName);
    printf ("%6d calls; ", nCalls);
    if (totalTime < 1e-6)
      printf ("%6.3f usecs total time\n", 1e6*totalTime);
    else if (totalTime < 1e-3)
      printf ("%6.3f msecs total time\n", 1e3*totalTime);
    else
      printf ("%6.3f secs total time\n", totalTime);
  }
}

void Timer::start() {
  // record start time
  gettimeofday(&ts_start, NULL);
  nCalls++;
}
      
void Timer::stop() {
  // get current time, add elapsed time to totalTime
  timeval ts_curr;
  gettimeofday(&ts_curr, NULL);
  totalTime += float(ts_curr.tv_sec - ts_start.tv_sec) +
               float(ts_curr.tv_usec)*1e-6 - float(ts_start.tv_usec)*1e-6;
}

float Timer::get_time() {
  return totalTime;
}

#else
//---------------------------------------------------------
// Timer turned off, methods do nothing
//---------------------------------------------------------
Timer::Timer(const char* Name="", bool On=false) {
}

Timer::~Timer() {
}

void Timer::start() {
}

void Timer::stop() {
}

float Timer::get_time() {
  return 0;
}

#endif
