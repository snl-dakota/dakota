/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Iterator
//- Description:  Dynamic library load manager for third-party solver packages.
//- Version: $Id$

// NOTE:  THIS IS NOT ACTIVE CODE
//
// It is just a "buffer" to maintain a record of a commit that is no longer
// maintained.  Revs related to DLib management (in the mid-May 2010)
// were deemed inappropriate to add to the top of DakotaIterator.cpp from
// the perperspective of DAKOTA code design/modularity.  Therefore, the
// code has been de-activated and moved into this source file for posterity.
//
// Assuming we do want to maintain "DL LoadManagement" capabability, these
// typedefs/functions/"iterator" prototypes should be re-designed and 
// implemented in a new component for DAKOTA (e.g. DynLoadLib Manager?).

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"

/* WJB: re-enable dependencies class-wrappers only if necessary
#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif
#ifdef HAVE_NLPQL
#include "NLPQLPOptimizer.hpp"
#endif
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#include "NLSSOLLeastSq.hpp"
#endif
// WJB: end of re-enable dependencies comment */

#if(defined(HAVE_DOT) || defined(HAVE_JEGA) || defined(HAVE_NLPQL) ||               defined(HAVE_NPSOL)) 

#ifdef DAKOTA_SHLIB
#undef DAKOTA_DYNLIB

typedef void (*p_vf)(void);

// WJB - ToDo: prefer function over macro
#ifdef _WIN32
#include "dakota_windows.h"
#define find_dlsym(a,b,c) (a = (p_vf)GetProcAddress((HINSTANCE)(b),c))
#define NO_DLERROR
#else
#include <dlfcn.h>
#define find_dlsym(a,b,c) (a = dlsym(b,c))
#undef NO_DLERROR
#endif


typedef struct Libentry
{
  const char *name;
  p_vf f;
} Libentry;

typedef struct SharedLib
{
  const char *libname;
  size_t nentries;
  Libentry *Entries;
} SharedLib;

struct NIDR_KWlib;
extern "C" NIDR_KWlib *nidr_lib_record(void *, const char*);
extern "C" void *nidr_dlopen(const char*);


static p_vf Lib_load(SharedLib *L, int k)
{
  Libentry *e, *ee;
  const char *lname;

  void* h = nidr_dlopen(lname = L->libname);
  if (!h) {
#ifndef NO_DLERROR
    if ((const char* s = dlerror()))
      std::cerr << "Cannot open library \"" << lname << "\":\n\t" << s;
    else
#endif
      std::cerr << "Cannot open library \"" << lname << "\n";
    std::exit(1);
  }
  nidr_lib_record(h, L->libname); // for cleanup (e.g., dlclose()) at endOf exec
  e = L->Entries;
  ee = e + L->nentries;

  for(ee = e + L->nentries; e < ee; ++e)
    if (!find_dlsym(e->f, h, e->name)) {
      std::cerr << "Could not find " << e->name << " in library "
                << lname << "\n";
      std::exit(2);
    }

  return L->Entries[k].f;
}


static Libentry
  Dot_entries[] = { {"dot_"}, {"dot510_"} },
  Npsol_entries[] = { {"npsol_"}, {"nlssol_"}, {"npoptn2_"} },
  Nlpql_entries[] = { {"nlpqlp_"}, {"ql_"} };

#define NumberOf(x) (sizeof(x)/sizeof(x[0]))

static SharedLib
  Dot_lib = { "libdot.dll", NumberOf(Dot_entries), Dot_entries },
  Npsol_lib = { "libnpsol.dll", NumberOf(Npsol_entries), Npsol_entries },
  Nlpql_lib = { "libnlpql.dll", NumberOf(Nlpql_entries), Nlpql_entries };


// WJB - Dakota C++ style: obtain prototypes from their respective header files
#define DOT F77_FUNC(dot,DOT)
#define DOT510 F77_FUNC(dot510,DOT510)
#define NPSOL F77_FUNC(npsol,NPSOL)
#define NLSSOL F77_FUNC(nlssol,NLSSOL)
// BMA (20160315): Changed to use Fortran 2003 ISO C bindings.
// The Fortran symbol will be lowercase with same name as if in C
//#define NPOPTN2 F77_FUNC(npoptn2,NPOPTN2)
#define NPOPTN2 npoptn2
#define NLPQLP F77_FUNC(nlpqlp,NLPQLP)
#define QL F77_FUNC(ql,QL)

/* WJB - ToDo:  verify redeclaration of an existing function and REMOVE!
extern "C" void DOT(int *info, int *ngotoz, int *method,
        int *iprint, int *ndv, int *ncon, double *x,
        double *xl, double *xu, double *obj, int *minmax,
        double *g, double *rprm, int *iprm, double *wk,
        int *nrwk, int *iwk, int *nriwk)
{
        typedef void (*DOT_t)(int *, int *, int *,
                        int *, int *, int *, double *,
                        double *, double *, double *, int *,
                        double *, double *, int *, double *,
                        int *, int *, int *);
        DOT_t f;
        if (!(f = (DOT_t)Dot_entries[0].f))
                f = (DOT_t)Lib_load(&Dot_lib, 0);
        f(info, ngotoz, method, iprint, ndv, ncon, x, xl, xu, obj,
          minmax, g, rprm, iprm, wk, nrwk, iwk, nriwk);
}

extern "C" void DOT510(int *ndv, int *ncon, int *ncola,  int *method, int *nrwk,
        int *nriwk, int *nrb, int *ngmax, double *xl, double *xu)
{
        typedef void (*DOT510_t)(int *, int *, int *,  int *, int *, int *,
                                int *, int *, double *, double *);
        DOT510_t f;
        if (!(f = (DOT510_t)Dot_entries[1].f))
                f = (DOT510_t)Lib_load(&Dot_lib, 1);
        f(ndv, ncon, ncola,  method, nrwk, nriwk, nrb, ngmax, xl, xu);
}

extern "C" void NPSOL(int *n, int *nclin, int *ncnln,
        int *lda, int *ldju, int *ldr, double *a, double *bl, double *bu,
        p_vf funcon, p_vf funobj, int *inform,  int *iter, int *istate,
        double *c, double *cjacu, double *clamda, double *objf,
        double *gradu, double *r, double *x, int *iw, int *leniw,
        double *w, int *lenw)
{
        typedef void (*NPSOL_t)(int *, int *, int *, int *, int *,
                        int *, double *, double *, double *, p_vf, p_vf,
                        int *, int *, int *, double *, double *, double *,
                        double *, double *, double *, double *, int *,
                        int *, double *, int *);
        NPSOL_t f;
        if (!(f = (NPSOL_t)Npsol_entries[0].f))
                f = (NPSOL_t)Lib_load(&Npsol_lib, 0);
        f(n, nclin, ncnln, lda, ldju, ldr, a, bl, bu, funcon, funobj,
          inform, iter, istate, c, cjacu, clamda, objf, gradu, r, x,
          iw, leniw, w, lenw);
}

extern "C" void NLSSOL(int *m, int *n, int *nclin, int *ncnln, int *lda,
        int *ldcju, int *ldfju, int *ldr, double *a, double *bl, double *bu,
        p_vf funcon, p_vf funobj, int *inform, int *iter, int *istate,
        double *c, double *cjacu, double *y, double *f, double *fjacu,
        double *clamda, double *objf, double *r, double *x, int *iw,
        int *leniw, double *w, int *lenw)
{
        typedef void (*NLSSOL_t)(int *, int *, int *, int *, int *, int *,
                int *, int *, double *, double *, double *, p_vf, p_vf,
                int *, int *, int *, double *, double *, double *,
                double *, double *, double *, double *, double *,
                double *, int *, int *, double *, int *);
        NLSSOL_t F;
        if (!(F = (NLSSOL_t)Npsol_entries[1].f))
                F = (NLSSOL_t)Lib_load(&Npsol_lib, 1);
        F(m, n, nclin, ncnln, lda, ldcju, ldfju, ldr, a, bl, bu,
          funcon, funobj, inform, iter, istate, c, cjacu, y, f,
          fjacu, clamda, objf, r, x, iw, leniw, w, lenw);
}

extern "C" void NPOPTN2(char *string, size_t string_len)
{
        typedef void (*NPOPTN2_t)(char *, size_t);
        NPOPTN2_t f;
        if (!(f = (NPOPTN2_t)Npsol_entries[2].f))
                f = (NPOPTN2_t)Lib_load(&Npsol_lib, 2);
        f(string, string_len);
}


extern "C" void NLPQLP(int *l, int *m, int *me, int *mmax, int *n, int *nmax, in
t *mnn2,
        double *x, double *f, double *g, double *df, double *dg, double *u,
        double *xl, double *xu, double *c, double *d, double *acc,
        double *accqp, double *stpmin, int *maxfun, int *maxit, int *max_nm,
        double *tol_nm, int *iprint, int *mode, int *iout, int *ifail,
        double *wa, int *lwa, int *kwa, int *lkwa, int *active,
        int *lactiv, int *lql, p_vf qpsolve)
{
        typedef void (*NLPQLP_t)(int *, int *, int *, int *, int *, int *,
                        int *mnn2, double *, double *, double *, double *,
                        double *, double *, double *, double *, double *,                               double *, double *, double *, double *, int *, int *,
                        int *, double *, int *, int *, int *, int *, double *,
                        int *, int *, int *, int *, int *, int *, p_vf);
        NLPQLP_t F;
        if (!(F = (NLPQLP_t)Nlpql_entries[0].f))
                F = (NLPQLP_t)Lib_load(&Nlpql_lib, 0);
        F(l, m, me, mmax, n, nmax, mnn2, x, f, g, df, dg, u, xl, xu, c, d, acc,
          accqp, stpmin, maxfun, maxit, max_nm, tol_nm, iprint, mode, iout,
          ifail, wa, lwa, kwa, lkwa, active, lactiv, lql, qpsolve);
}

extern "C" void QL(int *m, int *me, int *mmax, int *n, int *nmax, int *mnn,
   double *c, double *d, double *a, double *b, double *xl, double *xu,double *x,
   double *u, double *eps, int *mode, int *iout, int *ifail, int *iprint,
   double *war, int *lwar, int *iwar, int *liwar)
{
        typedef void (*QL_t)(int *, int *, int *, int *, int *, int *,
                        double *, double *, double *, double *, double *,
                        double *, double *, double *, double *, int *, int *,
                        int *, int *, double *, int *, int *, int *);
        QL_t f;
        if (!(f = (QL_t)Nlpql_entries[1].f))
                f = (QL_t)Lib_load(&Nlpql_lib, 1);
        f(m, me, mmax, n, nmax, mnn, c, d, a, b, xl, xu, x, u, eps, mode,
         iout, ifail, iprint, war, lwar, iwar, liwar);
}
// WJB: end of long ToDo verify redeclaration comment block */

#endif // DAKOTA_SHLIB
#endif // HAVE_DOT or NPSOL or JEGA or NPPQL

