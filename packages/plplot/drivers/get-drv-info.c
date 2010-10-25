/* Get device info from PLplot driver module
 *
 * Copyright (C) 2003  Rafael Laboissiere
 * Copyright (C) 2004  Joao Cardoso
 *
 * This file is part of PLplot.
 *
 * PLplot is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Library General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * PLplot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.

 * You should have received a copy of the GNU Library General Public License
 * along with the GNU C Library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <ltdl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "plConfig.h"

/* SEGV signal handler */
RETSIGTYPE
catch_segv (int sig)
{
  fprintf (stderr, "libltdl error: %s\n", lt_dlerror ());
  exit (1);
}

int
main (int argc, char* argv[])
{
  lt_dlhandle dlhand;
  char sym[300];
  char* drvnam = argv[1];
  char** info;

  /* Establish a handler for SIGSEGV signals. */
  signal (SIGSEGV, catch_segv);

  lt_dlinit ();
  dlhand = lt_dlopenext (drvnam);
  if (dlhand == NULL) {
    fprintf (stderr, "Could not open driver module %s\n"
                     "libltdl error: %s\n", drvnam, lt_dlerror ());
    return 1;
  }
  sprintf (sym, "plD_DEVICE_INFO_%s", drvnam);
  info = (char **) lt_dlsym (dlhand, sym);
  if (info != NULL) {
    printf ("%s", *info);
    return 0;
  }
  else {
    fprintf (stderr, "Could not read symbol %s in driver module %s\n"
                     "libltdl error: %s\n", sym, drvnam, lt_dlerror ());
    return 1;
  }
}
