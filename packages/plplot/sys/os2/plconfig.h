/* -*-C-*-
 *
 *  plConfig.h
 *
 *  Maurice LeBrun
 *  IFS, University of Texas at Austin
 *  18-Jul-1994
 *
 *  Contains macro definitions that determine miscellaneous PLplot library
 *  configuration defaults, such as macros for bin, font, lib, and tcl
 *  install directories, and various system dependencies.  On a Unix
 *  system, typically the configure script builds plConfig.h from
 *  plConfig.h.in.  Elsewhere, it's best to hand-configure a plConfig.h
 *  file and keep it with the system-specific files.
*/

#ifndef __PLCONFIG_H__
#define __PLCONFIG_H__

/* Define if on a POSIX.1 compliant system.  */
#undef _POSIX_SOURCE

/* Define HAVE_UNISTD_H if unistd.h is available. */
#define HAVE_UNISTD_H

/* Define if you have vfork.h.  */
#undef HAVE_VFORK_H

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define to `char *' if <sys/types.h> doesn't define.  */
#undef caddr_t

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the ANSI C header files.  */
#if !defined(__STDC__)
#  undef STDC_HEADERS
#endif

/* Define vfork as fork if vfork does not work.  */
#undef vfork

/* Install directories. */

#undef LIB_DIR
#undef BIN_DIR
#undef TCL_DIR

#endif	/* __PLCONFIG_H__ */
