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

#ifdef ACRO_USING_NPSOL

#ifndef interfaces_NPSOL_h
#define interfaces_NPSOL_h

#include <colin/Solver.h>
#include <colin/ColinUtilib.h>


namespace interfaces {

///
/// An interface to the NPSOL optimization
///
class NPSOL : public colin::Solver<utilib::BasicArray<double> >
{
public:

  /// Constructor
  NPSOL();

  /// Destructor
  virtual ~NPSOL();

  /// Setup NPSOL for optimization.  Call immediately before \ref{minimize}
  void reset();

  /// Perform minimization
  void minimize();

  /// Write out NPSOL parameter information
  void write(std::ostream& os) const;

protected:

  ///
  void initialize_best_point()
        {
        if ((best().point.size() > 0) &&
            (best().point.size() != problem.num_real_vars()))
           EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - user-provided best point has length " << best().point.size() << " but the problem size is " << problem.num_real_vars() << std::endl);
        best().point.resize(problem.num_real_vars());
        }

  /// Constraint function
  static int confun(int *MODE, int *NCNLN, int *N, int *NROWJ, int *NEEDC,
	double *X, double *C, double *CJAC, int *NSTATE);

  /// Objective function
  static int objfun(int *MODE, int *N, double *X, double *F, double *G, int *NSTATE);

  /// For use by the above functions
  static NPSOL *THIS;

  ///
  std::vector<int> asv;

  ///
  utilib::BasicArray<double> point;

  ///
  utilib::BasicArray<colin::real> gradient;

  ///
  colin::AppResponse_Utilib response;

  //
#if 0
  ///
  double ftol;

  ///
  double fdss;

  ///
  int numConstraints;
 
  ///
  int dotInfo;

  ///
  int dotFDSinfo;

  ///
  int dotMethod;

  ///
  int printControl;

  ///
  int optimizationType;

  ///
  std::vector<double> realWorkSpace;

  ///
  vector<int>  intWorkSpace;

  ///
  vector<double> constraintArray;
 
  ///
  vector<double> realCntlParmArray;

  ///
  vector<int> intCntlParmArray;

  ///
  vector<double> X;

  ///
  vector<double> bestX;

  ///
  vector<double> lowerBoundsArray;

  ///
  vector<double> upperBoundsArray;
#endif

};

} // namespace interfaces

#endif

#endif
