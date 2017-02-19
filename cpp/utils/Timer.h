//---------------------------------------------------------
// Timer.h
//---------------------------------------------------------
#ifndef __TIMER_H__
#define __TIMER_H__
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>

#define TIMER_ON

//---------------------------------------------------------
// Timer is an object which helps profile programs using
// the clock() function.
// - By default, a timer is stopped when you instantiate it
//   and must be started manually
// - Passing True to the constructor starts the timer when
//   it is constructed
// - When the timer is destructed it prints stats to stdout
//---------------------------------------------------------
class Timer {

  #ifdef TIMER_ON

    char binName[50];
    unsigned nCalls;
    timeval ts_start;
    float totalTime;

  #endif
    
    public:
      // constructor
      Timer(const char* Name="", bool On=false);
      // destructor
      ~Timer();
      
      // start/stop timer
      void start();
      void stop();

      // returns time in seconds
      float get_time();
};

#endif
