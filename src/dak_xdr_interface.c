/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 * DAKOTA's C wrapper to both ANSI and K&R versions of XDR.  Necessary
 * because C++ routines cannot directly call C routines prototyped in K&R
 * form, such as those defined in the Cygwin XDR implementation (sunrpc).
 */

#include "dak_xdr_interface.h"

#if defined(HAVE_RPC_TYPES_H) && defined(HAVE_RPC_XDR_H)

int dak_xdr_bool(XDR *xdrs, bool_t *bp)
{ return xdr_bool(xdrs, bp); }

int dak_xdr_char(XDR *xdrs, char *cp)
{ return xdr_char(xdrs, cp); }

int dak_xdr_double(XDR *xdrs, double *dp)
{ return xdr_double(xdrs, dp); }

int dak_xdr_float(XDR *xdrs, float *fp)
{ return xdr_float(xdrs, fp); }

int dak_xdr_int(XDR *xdrs, int *ip)
{ return xdr_int(xdrs, ip); }

int dak_xdr_long(XDR *xdrs, long *lp)
{ return xdr_long(xdrs, lp); }

int dak_xdr_short(XDR *xdrs, short *sp)
{ return xdr_short(xdrs, sp); }

int dak_xdr_string(XDR *xdrs, char **sp, u_int maxsize)
{ return xdr_string(xdrs, sp, maxsize); }

int dak_xdr_u_char(XDR *xdrs, unsigned char *ucp)
{ return xdr_u_char(xdrs, ucp); }

int dak_xdr_u_int(XDR *xdrs, unsigned *up)
{ return xdr_u_int(xdrs, up); }

int dak_xdr_u_long(XDR *xdrs, unsigned long *ulp)
{ return xdr_u_long(xdrs, ulp); }

int dak_xdr_u_short(XDR *xdrs, unsigned short *usp)
{ return xdr_u_short(xdrs, usp); }

int dak_xdr_setpos(XDR *xdrs, u_int pos)
{ return xdr_setpos(xdrs, pos); }

u_int dak_xdr_getpos(XDR *xdrs)
{ return xdr_getpos(xdrs); }

void dak_xdrmem_create(XDR *xdrs, char *addr, u_int size, enum xdr_op op)
{ xdrmem_create(xdrs, addr, size, op); }

void dak_xdr_destroy(XDR *xdrs)
{ xdr_destroy(xdrs); }

#endif
