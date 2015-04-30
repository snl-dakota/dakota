/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */
#include <acro_config.h>
//
// TODO MIGRATE FENV support into COLIN
//

//#define ACRO_USING_NPSOL

#if 0
#ifdef ACRO_USING_NPSOL
#include <interfaces/NPSOL.h>
#ifdef ACRO_USING_FENV
#include <fenv.h>
#endif


interfaces::NPSOL *interfaces::NPSOL::THIS;

typedef int (*c_fp)(int*,int*,int*,int*,int*,double*,double*,double*,int*);

typedef int (*o_fp)(int*,int*,double*,double*,double*,int*);

extern "C" int npsol_(int *n, int *nclin, int *ncnln, int *nrowa,
	     int *nrowuj, int *nrowr, double *a, double *bl, double *bu,
	     c_fp confun, o_fp objfun, int *inform, int *iter,
	     int *istate, double *c, double *ujac, double *clamda, double *objf,
	     double *ugrad, double *r, double *x, int *iw, int *leniw,
	     double *w, int *lenw);

extern "C" void npoptn_(const char*, int);



namespace interfaces {

using colin::real;


#ifdef ACRO_USING_FENV
#define NPSOL_NAXINF_CORRECTION 1.0e30

void nan_correct(double &x)
{
if (!finite(x)) {
   if (x < 0)
      x = -NPSOL_NAXINF_CORRECTION;
   else
      x = NPSOL_NAXINF_CORRECTION;
   }
}
#endif



NPSOL::NPSOL()
{
THIS=this;
}


void NPSOL::reset()
{
}


void NPSOL::minimize()
{
response.resize(1, 
		problem.numNonlinearConstraints(), 
		problem.num_real_vars(),
		true);
#if 0

int N = problem.num_real_vars();
if (N == 0) return;

opt_init();

int WR=1000, WI=300;
{
problem.EvalF(best_point,best_val);
}
X << best_point;
double objf_value = best_val;

lowerBoundsArray.resize(N);
upperBoundsArray.resize(N);
vector<real> tmpl(N), tmpu(N);
problem.get_real_bounds(tmpl,tmpu);
for (int i=0; i<N; i++) {
  lowerBoundsArray[i] = tmpl[i];
  upperBoundsArray[i] = tmpu[i];
  }

  for( ; ; )
  {
    // dotFDSinfo accepts the NGOTOZ parameter from DOT.  This involves a
    // change to the DOT source (adding NGOTOZ to the SUBROUTINE DOT
    // parameter list in ddot1.f).  dotFDSinfo is nonzero when DOT is
    // calculating finite difference gradients, and is zero otherwise.
 
    dotFDSinfo = 0; // Vanderplaats: Initialize to 0 before DOT call
 
    DEBUGPR(10,
      ucout << dotInfo << endl;
      ucout << dotFDSinfo << endl;
      ucout << dotMethod << endl;
      ucout << printControl << endl;
      ucout << N << endl;
      ucout << numConstraints << endl;
      ucout << X << endl;
      ucout << lowerBoundsArray << endl;
      ucout << upperBoundsArray << endl;
      ucout << objf_value << endl;
      ucout << optimizationType << endl;
      ucout << constraintArray << endl;
      ucout << realCntlParmArray << endl;
      ucout << intCntlParmArray << endl;
      ucout << WR << endl;
      ucout << WI << endl;
      ucout.flush();
            );

    dot(dotInfo, dotFDSinfo, dotMethod, printControl, N, 
	numConstraints, &(X[0]), 
	&(lowerBoundsArray[0]), &(upperBoundsArray[0]),
	objf_value, optimizationType,
        &(constraintArray[0]), &(realCntlParmArray[0]),
        &(intCntlParmArray[0]), &(realWorkSpace[0]),
        WR, &(intWorkSpace[0]), WI);
 
    if (dotInfo == 0) break;

    DEBUGPR(10,
      ucout << dotInfo << endl;
      ucout << dotFDSinfo << endl;
      ucout << dotMethod << endl;
      ucout << printControl << endl;
      ucout << N << endl;
      ucout << numConstraints << endl;
      ucout << X << endl;
      ucout << bestX << endl;
      ucout << lowerBoundsArray << endl;
      ucout << "B: " << lowerBoundsArray;
      ucout.flush();
      ucout << upperBoundsArray << endl;
      ucout << objf_value << endl;
      ucout << optimizationType << endl;
      ucout << constraintArray << endl;
      ucout << realCntlParmArray << endl;
      ucout << intCntlParmArray << endl;
      ucout << intWorkSpace << endl;
      ucout << WR << endl;
      ucout << WI << endl;
      ucout.flush();
            );

    real tmp_value;
    problem.EvalF(X,tmp_value);
    if (best_val > tmp_value) {
       best_val = tmp_value;
       best_point << X;
       DEBUGPR(10,
          ucout << "New Best: " << best_val << "::" << X << "\n";
               );
       }
    objf_value = tmp_value;

    if (check_convergence(best_val)) break;
  }

best().point << x;
problem.Eval(x, best().response, colin::mode_f | colin::mode_c);
compute_response_info(best().response,problem.state->constraint_lower_bounds,  problem.state->constraint_upper_bounds, best().value,best().constraint_violation);
#endif
}


int NPSOL::objfun(int *MODE, int *N, double *X, double *F, 
						double *G, int *NSTATE)
{
//
// Setup the point and/or gradient arrays
//
int mode=0;
THIS->point.set_data(*N,X,utilib::DataNotOwned);
if (*MODE != 1) {
   mode += 1;
   }
if (*MODE > 0) {
   THIS->gradient.resize(*N);
   mode += 2;
   }
//
// Evaluate the point
//
colin::real f;
#ifdef ACRO_USING_FENV
feclearexcept(FE_ALL_EXCEPT);
#endif
if (*MODE == 0)
   THIS->problem.EvalF(THIS->point,f);
else if (*MODE == 1)
   THIS->problem.EvalG(THIS->point,THIS->gradient);
else
   THIS->problem.EvalFG(THIS->point,f,THIS->gradient);
if (*MODE > 0) {
   for (unsigned int i=0; i<THIS->gradient.size(); i++)
     G[i] = THIS->gradient[i];
   }
//
// Handle errors
//
#ifdef ACRO_USING_FENV
if (fetestexcept(FE_DIVBYZERO|FE_INVALID)) {
   if (*MODE != 1)
      nan_correct(f);
   if (*MODE > 0) {
      for (unsigned int i=0; i<gradient.size(); i++)
        nan_correct(gradient[i]);
      }
   }
#endif
//
// Reset the value based on the sense of optimization (NPSOL only
// minimizes).
//
if (THIS->problem.sense == colin::maximization) {
   if (*MODE != 1)
      *F = -f;
   if (*MODE > 0) {
      for (unsigned int i=0; i<THIS->gradient.size(); i++)
        THIS->gradient[i] = - THIS->gradient[i];
      }
   }

return 0;
}



int NPSOL::confun(int *MODE, int *NCNLN, int *N, int *NROWJ, int *NEEDC,
                             double *X, double *C, double *CJAC, int *NSTATE)
{
//double c, *cj, *cje, *g;
//int i, n, nlc, nrj, wantJ;

//
// Setup point and active set vector
//
THIS->point.set_data(*N,X,utilib::DataNotOwned);
THIS->asv.resize(*NCNLN+1);
THIS->asv << 0;
for (unsigned int i=0; i<THIS->asv.size(); i++) {
  if (NEEDC[i] > 0) {
     THIS->asv[i+1] = *MODE + 1;
     }
  }
//
// Evaluate the point
//
#ifdef ACRO_USING_FENV
feclearexcept(FE_ALL_EXCEPT);
#endif
THIS->problem.Eval(THIS->point,THIS->asv,THIS->response);
//
// Set the data
//
int nrj=*NROWJ;
double *cje = CJAC + (*N)*nrj;
for (unsigned int i=0; i<*NCNLN; i++) {
  if (NEEDC[i] > 0) {
     if (*MODE != 1)
        C[i] = THIS->response.constraint_values()[i];
     if (*MODE > 0) {
        int j=0;
        for (double* cj = CJAC + i; cj < cje; cj += nrj, j++)
          *cj = THIS->response.constraint_gradients()[i][j];
        }
     }
  }


#ifdef ACRO_USING_FENV
if (fetestexcept(FE_DIVBYZERO|FE_INVALID)) {
   if (*MODE != 1)
      for(int i = 0; i < *NCNLN; i++)
        nan_correct(C[i]);
   if (*MODE > 0)
      for(int i = 0; i < *NCNLN; i++)
        for(cj = CJAC + i*nrj, cje = cj + n; cj < cje; cj++)
          nan_correct(*cj);
   }
#endif

return 0;
}


} // namespace interfaces

#endif
#endif
