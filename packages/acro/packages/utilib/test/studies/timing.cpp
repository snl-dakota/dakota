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

#include <utilib/std_headers.h>
#include <utilib/seconds.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

int test_timing(int,char**)
{
long int start;
double startme;
double startw;
double x=10.0;
int i;
FILE* foo;

start = clock();
startme = CPUSeconds();
startw  = WallClockSeconds();



for (i=0; i<10000; i++) {
#ifdef _MSC_VER
  fopen_s(&foo,"timing.temp","w+");
#else
  foo = fopen("timing.temp","w+");
#endif
  x += 1/x;
  fprintf(foo,"Val = %g\n",x);
  fclose(foo);
  }

start = clock() - start;
startme = CPUSeconds() - startme;
startw  = WallClockSeconds();

printf("clock time %g\n", ((double)(start)/CLOCKS_PER_SEC));
printf("CPUSeconds time %g\n", startme);
printf("WallClockSeconds time %g\n", startw);

return 0;
}
