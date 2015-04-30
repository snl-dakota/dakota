/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/*
 * Routines to return time used by processes and wallclock time.
 */

#include <utilib_config.h>

#include <utilib/seconds.h>

#ifdef __cplusplus
extern "C" {
#endif

#if UTILIB_TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if UTILIB_HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#if !defined(UTILIB_HAVE_GETTIMEOFDAY) && !defined(UTILIB_HAVE_LOCALTIME) 
#if defined(HAVE_TIMEB_H)
#include <sys/timeb.h>
#endif
#endif

#if UTILIB_HAVE_UNISTD_H
#include <unistd.h>
#endif
#if UTILIB_HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include <math.h>

static double CPU_start_time=0.0;
static double WallClock_start_time=0.0;
#if defined(UTILIB_HAVE_TIMES) && defined(UTILIB_HAVE_SYSCONF)
static double clock_tick = 1. / (double)sysconf(_SC_CLK_TCK);
#else
static double clock_tick=1.;
#endif

#define MILLISECONDS (1.0e-3)
#define MICROSECONDS (1.0e-6)

/*
 * Initialize Timing
 *
 * Initialize the timing routines.  We get the starting time and use this as a
 * baseline for elapsed time measurements.
 *
 * NOTE: time estimates can be corrupted if multiple calls
 *       are made to this function and the Elapsed routines are used.
 */
int InitializeTiming(void)
{
   CPU_start_time = CPUSeconds();
   WallClock_start_time = WallClockSeconds();
   return 0;
}


/*
 * UserSeconds - Returns user time (in seconds) used by the process so far.
 */
double UserSeconds(void)
  {
#ifdef UTILIB_HAVE_GETRUSAGE
   struct rusage usage;
   long sec, usec;
   double res;
   (void)getrusage(RUSAGE_SELF, &usage);
   sec  = usage.ru_utime.tv_sec;
   usec = usage.ru_utime.tv_usec;
   res = sec + MICROSECONDS*usec;
   return res;
#elif defined(UTILIB_HAVE_TIMES)// && defined(UTILIB_HAVE_SYSCONF)
   struct tms usage;
   (void)times(&usage);
   return(((double)(usage.tms_utime)) * clock_tick);
#elif defined(_WIN32)
   FILETIME unused;
   FILETIME ut;
   GetProcessTimes(NULL, &unused, &unused, &unused, &ut);
   // calculate seconds since the epoch (1 Jan 1970) & add back milliseconds
   ULARGE_INTEGER tmp;
   tmp.LowPart = ut.dwLowDateTime;
   tmp.HighPart = ut.dwHighDateTime;
   return (double)(tmp.QuadPart) * 1e-7;
#else
#error "This platform has no usable functions for tracking user time"
   return 0.0; // Should never get here, configure should have failed 
#endif
  }

/*
 * SystemSeconds - Returns system time (in seconds) used by the process so far.
 */
double SystemSeconds(void)
  {
#ifdef UTILIB_HAVE_GETRUSAGE
   struct rusage usage;
   long sec, usec;
   double res;
   (void)getrusage(RUSAGE_SELF, &usage);
   sec  = usage.ru_stime.tv_sec ;
   usec = usage.ru_stime.tv_usec;
   res = sec + MICROSECONDS*usec;
   return res;
#elif defined(UTILIB_HAVE_TIMES)// && defined(UTILIB_HAVE_SYSCONF)
   struct tms usage;
   (void)times(&usage);
   return(((double)(usage.tms_stime)) * clock_tick);
#elif defined(_WIN32)
   FILETIME unused;
   FILETIME st;
   GetProcessTimes(NULL, &unused, &unused, &st, &unused);
   // calculate seconds since the epoch (1 Jan 1970) & add back milliseconds
   ULARGE_INTEGER tmp;
   tmp.LowPart = st.dwLowDateTime;
   tmp.HighPart = st.dwHighDateTime;
   return (double)(tmp.QuadPart) * 1e-7;
#else
#error "This platform has no usable functions for tracking system time"
   return 0.0; // Should never get here, configure should have failed 
#endif
  }

/*
 * CPUSeconds - Returns sum of system and user time (in seconds)
 *   used by the process so far.
 */
double CPUSeconds(void)
  {
#ifdef UTILIB_HAVE_GETRUSAGE
   struct rusage usage;
   long sec, usec;
   double res;
   (void)getrusage(RUSAGE_SELF, &usage);
   sec  = usage.ru_utime.tv_sec  + usage.ru_stime.tv_sec ;
   usec = usage.ru_utime.tv_usec + usage.ru_stime.tv_usec;
   res = sec + MICROSECONDS*usec;
   return res;
#elif defined(UTILIB_HAVE_TIMES) && defined(UTILIB_HAVE_SYSCONF)
   struct tms usage;
   (void)times(&usage);
   return(((double)(usage.tms_utime + usage.tms_stime)) * clock_tick);
#elif UTILIB_HAVE_CLOCK
   clock_t t = clock();
   return ((double)t)/CLOCKS_PER_SEC; 
#else
#error "This platform has no usable functions for tracking CPU time"
   return 0.0; // Should never get here, configure should have failed 
#endif
  }

/*
 * ElapsedCPUSeconds - Returns time in seconds used by process in the
 *  interval since InitializeTiming was called.
 */
double ElapsedCPUSeconds(void)
{
  return(CPUSeconds() - CPU_start_time);
}

/*
 * WallClockSeconds -  Returns the number of seconds elasped since
 *  some date in the distant past.
 */
double WallClockSeconds(void)
  {
#ifdef UTILIB_HAVE_GETTIMEOFDAY
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return ((double)tv.tv_sec + ((double)tv.tv_usec * MICROSECONDS));
#elif defined(_WIN32)
   SYSTEMTIME sys_time;
   GetSystemTime(&sys_time);
   FILETIME f_time;
   SystemTimeToFileTime(&sys_time, &f_time);
   // calculate seconds since the epoch (1 Jan 1970) & add back milliseconds
   ULARGE_INTEGER tmp;
   tmp.LowPart = f_time.dwLowDateTime;
   tmp.HighPart = f_time.dwHighDateTime;
   return (double)((tmp.QuadPart - 116444736000000000ULL) / 10000000L)
      + (double)sys_time.wMilliseconds * MILLISECONDS;
#elif UTILIB_HAVE_LOCALTIME
   struct tm * tm_str;
   time_t t=time(NULL);
   long int tmp;
   tm_str = localtime(&t);
   if (tm_str == 0) return -999.0;
   /* Compute approximate seconds since Jan 1, 1970 (disregard leap seconds) */
   // Number of days: 365*years + leap years (divisible by 4, but not by
   // 100, unless also by 400)
   // NB: this only works for 1 Jan 2000 <= XXX < 1 Jan 2401.
   // NB: the subtraction for leap years uses 72 and not 68 because if
   //     the current year is a leap year, that extra day is is accounted
   //     for within tm_yday.
   tmp = 365*(tm_str->tm_year - 70) 
      + (long)((tm_str->tm_year - 72)/4) 
      - (long)((tm_str->tm_year - 100)/100) 
      + tm_str->tm_yday;
   tmp = 24*tmp + tm_str->tm_hour;
   tmp = 60*tmp + tm_str->tm_min;
   return (double)( 60*nsecs + tm_str->tm_sec );
#elif UTILIB_HAVE_FTIME
   // NB: ftime is deprecated in favor of time() and may be removed in
   // the future
   struct timeb tb;
   double res;
   (void) ftime(&tb);
   res = tb.time + MILLISECONDS*tb.millitm;
   return res;
#else
#error "This platform has no usable functions for tracking wall time"
  return 0.0;   /* Should never get here; configure should have failed. */
#endif
  }

/*
 * ElapsedWallClockSeconds - Returns the number of seconds elapsed since
 *  InitializeTiming was called.
 */
double ElapsedWallClockSeconds(void)
{
return(WallClockSeconds() - WallClock_start_time);
}

/*
 * CurrentTime
 *
 * Measures the number of seconds from some arbitrary starting point.
 * Useful for initializing RNGs
 */
double CurrentTime(void)
  {
  return WallClockSeconds();
  }

/// The smallest amount of time (in seconds) that the timer on this
/// system can distinguish from zero (experimental).  In practice, it
/// would be good to call this at least twice (with a second between calls)
/// and use the minimum value returned.
/// Note: this assumes the granularity is no larger than a second.

double timerGranularitySeconds()
{
double secondHolder = CPUSeconds();
double granularity = 1.0;
while (secondHolder != floor(secondHolder))
  {
  secondHolder *= 10;
  granularity /= 10;
  }
return(granularity);
}

#ifdef __cplusplus
} // extern "C"
#endif
