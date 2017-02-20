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
    struct timespec ts_start;
    //timeval ts_start;
    double totalTime;
    
    public:
      //------------------------------------------------------------------
      // constructor
      //------------------------------------------------------------------
      Timer (const char* Name="", bool On=false) {
        if (On) {
          // record the start time
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_start); 
          //gettimeofday(&ts_start, NULL);
          nCalls = 1;
        }
        else {
          nCalls = 0;
        }
        totalTime = 0;	
        strcpy(binName, Name);
      }

      //------------------------------------------------------------------
      // destructor
      //------------------------------------------------------------------
      ~Timer () {
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
      
      //------------------------------------------------------------------
      // start timer
      //------------------------------------------------------------------
      void start() {
        // record start time
        //gettimeofday(&ts_start, NULL);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_start);
        nCalls++;
      }
      
      //------------------------------------------------------------------
      // stop timer
      //------------------------------------------------------------------
      void stop() {
        // get current time, add elapsed time to totalTime
        struct timespec ts_curr;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_curr);
        //timeval ts_curr;
        //gettimeofday(&ts_curr, NULL);
        totalTime += double(ts_curr.tv_sec - ts_start.tv_sec) +
                     double(ts_curr.tv_nsec)*1e-9 - double(ts_start.tv_nsec)*1e-9;
      }

  #else

    //--------------------------------------------------------------------
    // all methods do nothing if TIMER_ON is not set
    //--------------------------------------------------------------------
    public:
      Timer (const char* Name, bool On=true) {}
      void start() {}
      void stop() {}

  #endif
};

#endif
