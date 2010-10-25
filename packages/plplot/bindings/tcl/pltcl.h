/* $Id: pltcl.h 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Maurice LeBrun
 * 21-Jun-94
 *
 * Declarations for PLplot/Tcl utility routines.
 * These should not require either Tk or Tcl-DP.
 */

#ifndef __PLTCL_H__
#define __PLTCL_H__

#include "plplot.h"
#include <tcl.h>
#include "tclMatrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* tclMain.c */
/* Main program for a Tcl-based shell that reads Tcl commands from stdin. */

int
pltclMain(int argc, char **argv, char *RcFileName,
	  int (*AppInit)(Tcl_Interp *interp));

/* tclAPI.c */
/* Front-end to PLplot/Tcl API for use from Tcl commands (e.g. plframe). */

int
plTclCmd(char *cmdlist, Tcl_Interp *interp,
	 int argc, char **argv);

/* Initialization routine for PLplot-extended tclsh's (like pltcl). */

int
Pltcl_Init( Tcl_Interp *interp );

int
PlbasicInit( Tcl_Interp *interp );

/* tkshell.c */
/* Sets up auto_path variable */

int
pls_auto_path(Tcl_Interp *interp);

/* Tcl command -- wait until the specified condition is satisfied. */

int
plWait_Until(ClientData, Tcl_Interp *, int, char **);

/* Tcl command -- return the IP address for the current host.  */

int
plHost_ID(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif	/* __PLTCL_H__ */
