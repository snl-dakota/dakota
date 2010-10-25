/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 *  DAKOTA's C wrapper to both ANSI and K&R versions of XDR.  Necessary
 *  because C++ routines cannot directly call C routines prototyped in K&R
 *  form, such as those defined in the Cygwin XDR implementation (sunrpc).
 */

#ifndef DAK_XDR_INTERFACE_H
#define DAK_XDR_INTERFACE_H

#ifdef HAVE_CONFIG_H
#include "dakota_config.h"
#endif

#if defined(HAVE_RPC_TYPES_H) && defined(HAVE_RPC_XDR_H)

#include <rpc/types.h>
#include <rpc/xdr.h>

#ifdef __cplusplus
extern "C" {
#endif

int dak_xdr_bool(XDR *xdrs, bool_t *bp);

int dak_xdr_char(XDR *xdrs, char *cp);

int dak_xdr_double(XDR *xdrs, double *dp);

int dak_xdr_float(XDR *xdrs, float *fp);

int dak_xdr_int(XDR *xdrs, int *ip);

int dak_xdr_long(XDR *xdrs, long *lp);

int dak_xdr_short(XDR *xdrs, short *sp);

int dak_xdr_string(XDR *xdrs, char **sp, u_int maxsize);

int dak_xdr_u_char(XDR *xdrs, unsigned char *ucp);

int dak_xdr_u_int(XDR *xdrs, unsigned *up);

int dak_xdr_u_long(XDR *xdrs, unsigned long *ulp);

int dak_xdr_u_short(XDR *xdrs, unsigned short *usp);

int dak_xdr_setpos(XDR *xdrs, u_int pos);

u_int dak_xdr_getpos(XDR *xdrs);

void dak_xdrmem_create(XDR *xdrs, char *addr, u_int size, enum xdr_op op);

void dak_xdr_destroy(XDR *xdrs);

#ifdef __cplusplus
}
#endif

#else

#define NO_XDR

#endif

#endif
