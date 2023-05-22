/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
  RealArray x(numVars);
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


StringArray
SharedSurfpackApproxData::variable_labels(const Variables& vars) const
{
  // order the variable labels the way the surrogate inputs are ordered
  // check incoming vars for correct length (active or all views)
  StringArray var_labels;
  if (vars.cv() + vars.div() + vars.drv() == numVars) {
    var_labels.insert(var_labels.end(),
		      vars.continuous_variable_labels().begin(),
		      vars.continuous_variable_labels().end());
    var_labels.insert(var_labels.end(),
		      vars.discrete_int_variable_labels().begin(),
		      vars.discrete_int_variable_labels().end());
    var_labels.insert(var_labels.end(),
		      vars.discrete_real_variable_labels().begin(),
		      vars.discrete_real_variable_labels().end());
  }
  else if (vars.acv() + vars.adiv() + vars.adrv() == numVars) {
    var_labels.insert(var_labels.end(),
		      vars.all_continuous_variable_labels().begin(),
		      vars.all_continuous_variable_labels().end());
    var_labels.insert(var_labels.end(),
		      vars.all_discrete_int_variable_labels().begin(),
		      vars.all_discrete_int_variable_labels().end());
    var_labels.insert(var_labels.end(),
		      vars.all_discrete_real_variable_labels().begin(),
		      vars.all_discrete_real_variable_labels().end());
  }
  else {
    Cerr << "Error: bad variable size in SharedSurfpackApproxData::"
	 << "variable_labels()." << std::endl;
    abort_handler(-1);
  }
  return var_labels;
}


void SharedSurfpackApproxData::
validate_metrics(const std::set<std::string>& allowed_metrics)
{
  bool err_found = false;
  for (const auto& req_metric : diagnosticSet)
    if (allowed_metrics.count(req_metric) == 0) {
      Cerr << "Error: surrogate metric '" << req_metric
	   << "' invalid for " << approxType << " surrogate.\n";
      err_found = true;
    }
  if (err_found) {
    Cerr << "Valid metrics for " << approxType << " surrogate include:\n  ";
    std::copy(allowed_metrics.begin(), allowed_metrics.end(),
	      std::ostream_iterator<std::string>(Cerr, " "));
    Cerr << std::endl;
  }

  if (crossValidateFlag) {
    if (numFolds > 0 && numFolds < 2) {
      Cerr << "Error: cross_validation folds must be 2 or greater."
	   << std::endl;
      err_found = true;
    }
    if (percentFold < 0.0 || percentFold > 0.5) {
      Cerr << "Error: cross_validation percent must be between 0.0 and 0.5"
	   << std::endl;
      err_found = true;
    }

    // calculate folds from default or percent if needed
    if (numFolds == 0) {
      if (percentFold > 0.0) {
	numFolds = boost::math::iround(1./percentFold);
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "Info: cross_validate num_folds = " << numFolds
	       << " calculated from specified percent = "
	       << percentFold << "." << std::endl;
      }
      else {
	numFolds = 10;
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "Info: default num_folds = " << numFolds << " used."
	       << std::endl;
      }
    }

  }

  if (err_found)
    abort_handler(PARSE_ERROR);
}


void SharedSurfpackApproxData::
map_variable_labels(const Variables& dfsm_vars,
		    const StringArray& approx_labels)
{
  // When importing, always map from all variables to the
  // subset needed by the surrogate...
  // the surrogate was built over active or all cont, int, real
  const auto& cv_labels = dfsm_vars.all_continuous_variable_labels();
  StringArray all_labels(cv_labels.begin(), cv_labels.end());
  const auto& div_labels = dfsm_vars.all_discrete_int_variable_labels();
  all_labels.insert(all_labels.end(), div_labels.begin(), div_labels.end());
  const auto& drv_labels = dfsm_vars.all_discrete_real_variable_labels();
  all_labels.insert(all_labels.end(), drv_labels.begin(), drv_labels.end());

  bool vars_equal = (all_labels == approx_labels);
  if (!vars_equal) {
    if (approx_labels.empty()) {
      Cerr << "\nError: Imported surrogate has no variable labels; cannot "
	   << "determine variable map." << std::endl;
      abort_handler(APPROX_ERROR);
    }

    // each approx model var must be in the wrapping model's var set
    varsMapIndices.clear();
    varsMapIndices.reserve(approx_labels.size());
    StringArray missing_labels;
    for (const auto& approx_label : approx_labels) {
      size_t ind = find_index(all_labels, approx_label);
      if (ind == _NPOS)
	missing_labels.push_back(approx_label);
      else
	varsMapIndices.push_back(ind);
    }
    if (!missing_labels.empty()) {
      Cerr << "\nError: Imported surrogate includes variable labels\n"
	   << missing_labels << "\nnot present in model's variables:\n"
	   << all_labels << std::endl;
      abort_handler(APPROX_ERROR);
    }

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Info: mapping model's variables to imported surrogate."
	   << std::endl;
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Model all_vars by domain type\n" << all_labels << std::endl;
      Cout << "Indices s.t. surr_vars[i] = all_vars[index[i]]\n"
	   << varsMapIndices << std::endl;
      Cout << "Imported surrogate labels\n" << approx_labels << std::endl;
      Cout << "Model all_vars mapped to surrogate\n";
      for (size_t i=0; i<varsMapIndices.size(); ++i)
	Cout << all_labels[varsMapIndices[i]] << "\n";
    }
  }
}


} // namespace Dakota
