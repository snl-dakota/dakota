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

/**
 * \file misc.c
 * \author William E. Hart
 */

#include <utilib_config.h>
#include <stdio.h>


int calc_filesize(char* fname);


int calc_filesize(char* fname)
{
FILE* fptr;
int ctr;
char c,prev_c;

#ifdef _MSC_VER
fopen_s(&fptr,fname,"r");
#else
fptr = fopen(fname,"r");
#endif
if (fptr == NULL)
   return 0;

prev_c = '\n';
ctr=0;
c = (char) fgetc(fptr);
while (!feof(fptr)) {
  if (c == '\n') ctr++;
  prev_c = c;
  c = (char) fgetc(fptr);
  }
if (prev_c != '\n') ctr++;

return ctr;
}
