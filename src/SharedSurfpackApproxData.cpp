/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedSurfpackApproxData
//- Description:  Class implementation of Surfpack response surface 
//-               
//- Owner:        Brian Adams

#include <stdexcept>
#include <typeinfo>

#include "SharedSurfpackApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include "SurrogateData.hpp"
#include "dakota_data_io.hpp"

// Headers from Surfpack
#include "SurfData.h"
#include "SurfpackMatrix.h"
 
#include <algorithm>
#include <boost/math/special_functions/round.hpp>

namespace Dakota {


/** Initialize the embedded Surfpack surface object and configure it
    using the specifications from the input file.  Data for the
    surface is created later. */

SharedSurfpackApproxData::
SharedSurfpackApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars),
  diagnosticSet(problem_db.get_sa("model.metrics")),
  crossValidateFlag(problem_db.get_bool("model.surrogate.cross_validate")),
  numFolds(problem_db.get_int("model.surrogate.folds")),
  percentFold(problem_db.get_real("model.surrogate.percent")),
  pressFlag(problem_db.get_bool("model.surrogate.press"))
{
  // For Polynomial surface fits
  if (approxType == "global_polynomial")
    approxOrder = problem_db.get_short("model.surrogate.polynomial_order");
  else if (approxType == "global_kriging") {
    const String& trend_string = 
      problem_db.get_string("model.surrogate.trend_order");
    if (trend_string == "constant")    approxOrder = 0;
    else if (trend_string == "linear") approxOrder = 1;
    else approxOrder = 2; // empty, reduced_quadratic, quadratic
  }
}


/// On-the-fly constructor which uses mostly Surfpack model defaults
SharedSurfpackApproxData::
SharedSurfpackApproxData(const String& approx_type,
			 const UShortArray& approx_order, size_t num_vars,
			 short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level),
  crossValidateFlag(false), numFolds(0), percentFold(0.0), pressFlag(false)
{
  approxType = approx_type;
  if (approx_order.empty())
    approxOrder = 2;
  else {
    approxOrder = approx_order[0];
    if (approx_order.size() != num_vars) {
      Cerr << "Error: bad size of " << approx_order.size()
	   << " for approx_order in SharedSurfpackApproxData lightweight "
	   << "constructor.  Expected " << num_vars << "." << std::endl;
      abort_handler(-1);
    }
    for (size_t i=1; i<num_vars; ++i)
      if (approx_order[i] != approxOrder) {
	Cerr << "Warning: SharedSurfpackApproxData lightweight constructor "
	     << "requires homogeneous approximation order.  Promoting to max "
	     << "value." << std::endl;
	approxOrder = std::max(approx_order[i], approxOrder);
      }
  }
}


void SharedSurfpackApproxData::
merge_variable_arrays(const RealVector& cv,  const IntVector& div,
		      const RealVector& drv, RealArray& ra)
{
  size_t num_cv = cv.length(), num_div = div.length(), num_drv = drv.length(),
         num_v  = num_cv + num_div + num_drv;
  ra.resize(num_v);
  if (num_cv)   copy_data_partial(cv,  ra, 0);
  if (num_div) merge_data_partial(div, ra, num_cv);
  if (num_drv)  copy_data_partial(drv, ra, num_cv+num_div);
}


void SharedSurfpackApproxData::
sdv_to_realarray(const Pecos::SurrogateDataVars& sdv, RealArray& ra)
{
  // check incoming vars for correct length (active or all views)
  const RealVector&  cv = sdv.continuous_variables();
  const IntVector&  div = sdv.discrete_int_variables();
  const RealVector& drv = sdv.discrete_real_variables();
  if (cv.length() + div.length() + drv.length() == numVars)
    merge_variable_arrays(cv, div, drv, ra);
  else {
    Cerr << "Error: bad parameter set length in SharedSurfpackApproxData::"
	 << "sdv_to_realarray(): " << numVars << " != " << cv.length() << " + "
	 << div.length() << " + " << drv.length() << "." << std::endl;
    abort_handler(-1);
  }
}
  

void SharedSurfpackApproxData::
vars_to_realarray(const Variables& vars, RealArray& ra)
{
  // check incoming vars for correct length (active or all views)
  if (vars.cv() + vars.div() + vars.drv() == numVars)
    merge_variable_arrays(vars.continuous_variables(),
			  vars.discrete_int_variables(),
			  vars.discrete_real_variables(), ra);
  else if (vars.acv() + vars.adiv() + vars.adrv() == numVars)
    merge_variable_arrays(vars.all_continuous_variables(),
			  vars.all_discrete_int_variables(),
			  vars.all_discrete_real_variables(), ra);
  else {
    Cerr << "Error: bad parameter set length in SharedSurfpackApproxData::"
	 << "vars_to_realarray()." << std::endl;
    abort_handler(-1);
  }
}
  

void SharedSurfpackApproxData::
add_sd_to_surfdata(const Pecos::SurrogateDataVars& sdv,
		   const Pecos::SurrogateDataResp& sdr, short fail_code,
		   SurfData& surf_data)
{
  // coarse-grained fault tolerance for now: any failure qualifies for omission
  if (fail_code)
    return;

  // Surfpack's RealArray is std::vector<double>; use DAKOTA copy_data helpers.
  // For DAKOTA's compact mode, any active discrete {int,real} variables could
  // be contained within SDV's continuousVars (see Approximation::add(Real*)),
  // although it depends on eval cache lookups as shown in
  // ApproximationInterface::update_approximation().
  RealArray x; 
  sdv_to_realarray(sdv, x);
  Real f = sdr.response_function();

  // for now only allow builds from exactly 1, 3=1+2, or 7=1+2+4; use
  // different set functions so the SurfPoint data remains empty if
  // not present
  switch (buildDataOrder) {

  case 1:
    surf_data.addPoint(SurfPoint(x, f));
    break;

  case 3: {
    RealArray gradient;
    copy_data(sdr.response_gradient(), gradient);
    surf_data.addPoint(SurfPoint(x, f, gradient));
    break;
  }

  case 7: {
    RealArray gradient;
    copy_data(sdr.response_gradient(), gradient);
    SurfpackMatrix<Real> hessian;
    copy_matrix(sdr.response_hessian(), hessian);
    surf_data.addPoint(SurfPoint(x, f, gradient, hessian));
    break;
  }

  default:
    Cerr << "\nError (SharedSurfpackApproxData): derivative data may only be "
	 << "used if all\nlower-order information is also present. Specified "
	 << "buildDataOrder is " << buildDataOrder << "."  << std::endl; 
    abort_handler(-1);
    break;

  }
}


void SharedSurfpackApproxData::
copy_matrix(const RealSymMatrix& rsm, SurfpackMatrix<Real>& surfpack_matrix)
{
  // SymmetricMatrix = symmetric and square, but Dakota::Matrix can be general
  // (e.g., functionGradients = numFns x numVars).  Therefore, have to verify
  // sanity of the copy.  Could copy square submatrix of rsm into sm, but 
  // aborting with an error seems better since this should only currently be
  // used for copying Hessian matrices.
  size_t nr = rsm.numRows(), nc = rsm.numCols();
  if (nr != nc) {
    Cerr << "Error: copy_data(const Dakota::RealSymMatrix& rsm, "
	 << "SurfpackMatrix<Real>& sm) called with nonsquare rsm." << std::endl;
    abort_handler(-1);
  }
  if (surfpack_matrix.getNRows() != nr | surfpack_matrix.getNCols() != nc) 
    surfpack_matrix.resize(nr, nc);
  for (size_t i=0; i<nr; ++i)
    for (size_t j=0; j<nc; ++j)
      surfpack_matrix(i,j) = rsm(i,j);
}

} // namespace Dakota
