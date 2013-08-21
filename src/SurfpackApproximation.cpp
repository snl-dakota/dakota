/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SurfpackApproximation
//- Description:  Class implementation of Surfpack response surface 
//-               
//- Owner:        Brian Adams

#include <stdexcept>
#include <typeinfo>

#include "SurfpackApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include "dakota_data_io.hpp"

// Headers from Surfpack
#include "SurfData.h"
// for Hessian data
#include "SurfpackMatrix.h"
#include "ModelFactory.h"
#include "ModelFitness.h"
#include "surfaces/SurfpackModel.h"
#include "SurfpackInterface.h"
 
#include <algorithm>
#include <boost/math/special_functions/round.hpp>


namespace Dakota {

/// \todo The dakota data structures like RealArray inherit from std::vector.
/// and are passed as is to Surfpack methods and functions-- which expect
/// structures of type std::vector.  This is legal as long as the types 
/// Surfpack expects really are superclasses of their Dakota counterparts.
/// It will fail if a Real is #defined in Dakota to be a float,
/// because then it will be a subclass of std::vector<float> being passed in
/// as a std::vector<double>.  The possible solutions, I think, are:
/// 1. Have the same typedef feature in Surfpack so that everything can
/// be configured to use floats or doubles
/// 2. Explicitly cast each vector component as it passes over
/// the Dakota/Surfpack boundary

/** Initialize the embedded Surfpack surface object and configure it
    using the specifications from the input file.  Data for the
    surface is created later. */

using surfpack::toString;
using surfpack::fromVec;


SurfpackApproximation::
SurfpackApproximation(const ProblemDescDB& problem_db, size_t num_vars):
  Approximation(BaseConstructor(), problem_db, num_vars), //surface(NULL),
  surfData(NULL), model(NULL), factory(NULL), 
  exportModelName(problem_db.get_string("model.surrogate.export_model_file")),
  diagnosticSet(problem_db.get_sa("model.metrics")),
  crossValidateFlag(problem_db.get_bool("model.surrogate.cross_validate")),
  numFolds(problem_db.get_int("model.surrogate.folds")),
  percentFold(problem_db.get_real("model.surrogate.percent")),
  pressFlag(problem_db.get_bool("model.surrogate.press"))
{
    ParamMap args;

    args["verbosity"] = toString<short>(outputLevel);
    args["ndims"] = toString<size_t>(num_vars);

    // For now, not exposing Surfpack random seed in the DAKOTA UI;
    // instead fixing at an arbitrary value (treated as int in Surfpack)
    args["seed"] = "8147";

    // For Polynomial surface fits
    if (approxType == "global_polynomial") {
      args["type"] = "polynomial";

      approxOrder = problem_db.get_short("model.surrogate.polynomial_order");
      args["order"] = toString<unsigned short>(approxOrder);

      // TO DO: activate derivative-based regression
      buildDataOrder = 1;
      if (problem_db.get_bool("model.surrogate.derivative_usage")) {
	if (problem_db.get_string("responses.gradient_type") != "none")
	  buildDataOrder |= 2;
	if (problem_db.get_string("responses.hessian_type")  != "none")
	  buildDataOrder |= 4;
      }
    }

    // For Kriging surface fits
    else if (approxType == "global_kriging") {

      args["type"] = "kriging";

      args["order"] = toString<int>(2);
      args["reduced_polynomial"] = toString<bool>(true);
      const String& trend_string = 
	problem_db.get_string("model.surrogate.trend_order");
      if (!trend_string.empty()) {
	if (trend_string == "constant")
	  args["order"] = toString<unsigned int>(0);
	else if (trend_string == "linear")
	  args["order"] = toString<unsigned int>(1);
	else if (trend_string == "reduced_quadratic") {
	  args["order"] = toString<unsigned int>(2);
	  args["reduced_polynomial"] = toString<bool>(true);
	}
	else if (trend_string == "quadratic") {
	  args["order"] = toString<unsigned int>(2);
	  args["reduced_polynomial"] = toString<bool>(false);
	}
      }

      // activate derivative information if available
      buildDataOrder = 1;
      unsigned short surfpack_derivative_order = 0;
      if (problem_db.get_bool("model.surrogate.derivative_usage")) {
	if (problem_db.get_string("responses.gradient_type") != "none") {
	  buildDataOrder |= 2;
	  surfpack_derivative_order = 1;
	}
	if (problem_db.get_string("responses.hessian_type")  != "none") {
	  buildDataOrder |= 4;
	  if (buildDataOrder | 2) {
	    surfpack_derivative_order = 2;
	  }
	  else {
	    Cerr << "\nError (global_kriging): Hessian information only used "
		 << "if gradients present.\nbuildDataOrder = " << buildDataOrder
		 << std::endl;
	    abort_handler(-1);
	  }
	}
      }
      args["derivative_order"] = 
	toString<unsigned short>(surfpack_derivative_order);

      // optimization options are none | sample | local | global (default)
      args["optimization_method"] = "global";
      const String& optimization_method = 
 	problem_db.get_string("model.surrogate.kriging_opt_method");
      if (!optimization_method.empty()) {
	if (optimization_method == "none" || optimization_method == "sampling" 
	    || optimization_method == "local" 
	    || optimization_method == "global")
	  args["optimization_method"] = optimization_method;
	else {
	  Cerr << "Error (global_kriging): invalid optimization method "
	       << optimization_method << "; valid options are " 
	       << "none, sampling, local, or global" << std::endl;
	  abort_handler(-1);
	}
      }

      short max_trials
	= problem_db.get_short("model.surrogate.kriging_max_trials");
      if (max_trials > 0) {
        args["max_trials"] = toString<short>(max_trials);
      }

      // NIDR support for RealArray (aka std::vector) would eliminate xtra copy!
      // old parameters

      const RealVector& correlation_rv
        = problem_db.get_rv("model.surrogate.kriging_correlations");
      if (!correlation_rv.empty()) {
	RealArray correlation_ra; //std::vector<double>
	copy_data(correlation_rv, correlation_ra);
        args["correlation_lengths"] = fromVec<Real>(correlation_ra);
	args["optimization_method"] = "none";
      }

      /*
      const RealVector& max_correlations_rv 
        = problem_db.get_rv("model.surrogate.kriging_max_correlations");
      if (!max_correlations_rv.empty()) {
	RealArray max_correlation_ra; //std::vector<double>
	copy_data(max_correlations_rv, max_correlation_ra);
        args["max_correlations"] = fromVec<Real>(max_correlation_ra);
      }

      const RealVector& min_correlations_rv 
        = problem_db.get_rv("model.surrogate.kriging_min_correlations");
      if (!min_correlations_rv.empty()) {
	RealArray min_correlation_ra; //std::vector<double>
	copy_data(min_correlations_rv, min_correlation_ra);
        args["min_correlations"] = fromVec<Real>(min_correlation_ra);
      }

      // bounds set at run time within build()
      if (!approxCLowerBnds.empty()) {
	RealArray alb_ra;
	copy_data(approxCLowerBnds, alb_ra);
	args["lower_bounds"] = fromVec<Real>(alb_ra);
      }
      if (!approxCUpperBnds.empty()) {
	RealArray aub_ra;
	copy_data(approxCUpperBnds, aub_ra);
	args["upper_bounds"] = fromVec<Real>(aub_ra);
      }
      */

      // unused for now
      IntVector dimension_groups_iv;
      if (!dimension_groups_iv.empty()) {
	IntArray dg_ra;
	copy_data(dimension_groups_iv, dg_ra);
	args["dimension_groups"] = fromVec<int>(dg_ra);
      }

    }

    // For ANN surface fits
    else if (approxType == "global_neural_network") {
      args["type"] = "ann";
      short random_weight
	= problem_db.get_short("model.surrogate.neural_network_random_weight");
      if (random_weight > 0) {
        args["random_weight"] = toString<short>(random_weight);
      }
      short nodes
	= problem_db.get_short("model.surrogate.neural_network_nodes");
      if (nodes > 0) {
        args["nodes"] = toString<short>(nodes);
      }
      const Real& range
	= problem_db.get_real("model.surrogate.neural_network_range");
      if (range > 0) {
        args["range"] = toString<Real>(range);
      }
    }

    //// For moving least squares
    else if (approxType == "global_moving_least_squares") {
      args["type"] = "mls";
      short weight
	= problem_db.get_short("model.surrogate.mls_weight_function");
      if (weight > 0) {
        args["weight"] = toString<short>(weight);
      }
      short order = problem_db.get_short("model.surrogate.mls_poly_order");
      if (order > 0) {
        args["order"] = toString<short>(order);
      }
    }

    //// For radial basis function networks
    else if (approxType == "global_radial_basis") {
      args["type"] = "rbf";
      short bases = problem_db.get_short("model.surrogate.rbf_bases");
      if (bases > 0) {
        args["bases"] = toString<short>(bases);
      }
      short min_partition
	= problem_db.get_short("model.surrogate.rbf_min_partition");
      if (min_partition > 0) {
        args["min_partition"] = toString<short>(min_partition);
      }
      short max_subsets
	= problem_db.get_short("model.surrogate.rbf_max_subsets");
      if (max_subsets > 0) {
        args["max_iter"] = toString<short>(max_subsets);
      }
      short max_pts = problem_db.get_short("model.surrogate.rbf_max_pts");
      if (max_pts > 0) {
        args["max_pts"] = toString<short>(max_pts);
      }
    }

    //// For Mars surface fits
    else if (approxType == "global_mars") {
      args["type"] = "mars";
      short max_bases = problem_db.get_short("model.surrogate.mars_max_bases");
      if (max_bases > 0) {
        args["max_bases"] = toString<short>(max_bases);
      }
      const String& interpolation
	= problem_db.get_string("model.surrogate.mars_interpolation");
      if (interpolation != "") {
        args["interpolation"] = interpolation; 
      }
    }
   //Cout << "PARAMETERS: " << std::endl;
   //for (ParamMap::iterator itr = args.begin();
   //     itr != args.end(); itr++) {
   //   Cout << "     " << itr->first << ": " << itr->second << std::endl;
   // }

    factory = ModelFactory::createModelFactory(args);


  //}
  //catch(...) {
  //  Cout << "Exception caught in attempt to create Surface object" << std::endl;
  //  abort_handler(-1);
  //}

    if (!exportModelName.empty()) {
      if (SurfpackInterface::HasFeature("model_save")) {
	if (!strends(exportModelName, ".sps") && 
	    !strends(exportModelName, ".bsps")) {
	  Cerr << "\nError: export_model_file name must end in .sps or .bsps."
	       << std::endl;
	  abort_handler(-1);
	}
      }
      else {
	Cerr << "\nError: export_model_file requires compilation with option\n"
	     <<"       HAVE_BOOST_SERIALIZATION." << std::endl;
	abort_handler(-1);
      }
    }

    // validate diagnostic settings (preliminary); TODO: do more at
    // run time and move to both ctors
    bool err_found = false;
    if (!diagnosticSet.empty()) {
      std::set<std::string> valid_metrics;
      valid_metrics.insert("sum_squared");
      valid_metrics.insert("mean_squared");
      valid_metrics.insert("root_mean_squared");
      valid_metrics.insert("sum_abs");
      valid_metrics.insert("mean_abs");
      valid_metrics.insert("max_abs");
      valid_metrics.insert("rsquared");

      int num_diag = diagnosticSet.size();
      for (int j = 0; j < num_diag; ++j)
	if (valid_metrics.find(diagnosticSet[j]) == valid_metrics.end()) {
	  Cerr << "Error: surrogate metric '" << diagnosticSet[j] 
	       << "' is not available in Dakota.\n";
	  err_found = true;
	}	
      if (err_found) {
	Cerr << "Valid surrogate metrics include:\n  ";
	std::copy(valid_metrics.begin(), valid_metrics.end(), 
		  std::ostream_iterator<std::string>(Cerr, " "));
	Cerr << std::endl;
      }
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
    }
    if (err_found)
      abort_handler(-1);

}


/// On-the-fly constructor which uses mostly Surfpack model defaults
SurfpackApproximation::
SurfpackApproximation(const String& approx_type,
		      const UShortArray& approx_order, size_t num_vars,
		      short data_order, short output_level):
  Approximation(NoDBBaseConstructor(), num_vars, data_order, output_level),
  surfData(NULL), model(NULL), factory(NULL),
  crossValidateFlag(false), numFolds(0), percentFold(0.0), pressFlag(false)
{
  approxType = approx_type;
  if (approx_order.empty())
    approxOrder = 2;
  else {
    approxOrder = approx_order[0];
    if (approx_order.size() != num_vars) {
      Cerr << "Error: bad size of " << approx_order.size()
	   << " for approx_order in SurfpackApproximation lightweight "
	   << "constructor.  Expected " << num_vars << "." << std::endl;
      abort_handler(-1);
    }
    for (size_t i=1; i<num_vars; ++i)
      if (approx_order[i] != approxOrder) {
	Cerr << "Warning: SurfpackApproximation lightweight constructor "
	     << "requires homogeneous approximation order.  Promoting to max "
	     << "value." << std::endl;
	approxOrder = std::max(approx_order[i], approxOrder);
      }
  }

  ParamMap args;

  args["verbosity"] = toString<short>(outputLevel);//(QUIET_OUTPUT);
  args["ndims"] = toString<size_t>(num_vars);
  args["seed"] = "8147";

  if (approxType == "global_polynomial") {
    args["type"] = "polynomial";
    args["order"] = toString<unsigned short>(approxOrder);
  }
  else if (approxType == "global_kriging") {

    args["order"] = toString<unsigned int>(approxOrder);
    args["reduced_polynomial"] = toString<bool>(true);
    args["type"] = "kriging";
    /*
    // bounds set at run time within build():
    if (!approxCLowerBnds.empty()) {
      RealArray alb_ra;
      copy_data(approxCLowerBnds, alb_ra);
      args["lower_bounds"] = fromVec<Real>(alb_ra);
    }
    if (!approxCUpperBnds.empty()) {
      RealArray aub_ra;
      copy_data(approxCUpperBnds, aub_ra);
      args["upper_bounds"] = fromVec<Real>(aub_ra);
    }
    */
    //size_t krig_max_trials=(2*num_vars+1)*(num_vars+1)*10;
    //size_t krig_max_trials=20*(2*num_vars+1)*
      //(1+num_vars+((num_vars+1)*num_vars)/2); //#der0 + #der1 + #der2
    //if(krig_max_trials>10000)
      //krig_max_trials=10000;
    //size_t krig_max_trials=1000;
    //args["max_trials"] = toString<size_t>(krig_max_trials);
    args["max_trials"] = toString<size_t>(5000);
    


    // activate derivative information if available
    unsigned short surfpack_derivative_order = 0;
    if (buildDataOrder == 1)
      surfpack_derivative_order = 0;
    else if (buildDataOrder == 3)
      surfpack_derivative_order = 1;
    else if (buildDataOrder == 7)
      surfpack_derivative_order = 2;
    else {
      Cerr << "\nError (global_kriging): Unsupported buildDataOrder = " 
	   << buildDataOrder << std::endl;
      abort_handler(-1);
    }
    args["derivative_order"] = 
      toString<unsigned short>(surfpack_derivative_order);

  }
  else if (approxType == "global_neural_network")
    args["type"] = "ann";
  else if (approxType == "global_moving_least_squares")
    args["type"] = "mls";
  else if (approxType == "global_radial_basis")
    args["type"] = "rbf";
  else if (approxType == "global_mars")
    args["type"] = "mars";
  
  factory = ModelFactory::createModelFactory(args);
}



// Embedded Surfpack objects will need to be deleted
SurfpackApproximation::~SurfpackApproximation()
{
  delete surfData;
  delete model;
  delete factory;
}


int SurfpackApproximation::min_coefficients() const
{
  assert(factory);
  return factory->minPointsRequired();
}


int SurfpackApproximation::recommended_coefficients() const
{
  assert(factory);
  return factory->recommendedNumPoints();
}


void SurfpackApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // Surface object should have been created in constructor
  if (!factory) { 
    Cerr << "Error: surface is null in SurfpackApproximation::build()."
	 << std::endl;  
    abort_handler(-1);
  }

  /// surfData will be deleted in dtor
  /// \todo Right now, we're completely deleting the old data and then
  /// recopying the current data into a SurfData object.  This was just
  /// the easiest way to arrive at a solution that would build and run.
  /// This function is frequently called from addPoint rebuild, however,
  /// and it's not good to go through this whole process every time one
  /// more data point is added.
  try {
    if (surfData) {
      delete surfData;
      surfData = NULL;
    }
    surfData = surrogates_to_surf_data();
 
    // set bounds at run time since they are updated by some methods (e.g., SBO)
    if (!approxCLowerBnds.empty() || !approxDILowerBnds.empty() ||
	!approxDRLowerBnds.empty()) {
      RealArray alb;
      merge_variable_arrays(approxCLowerBnds, approxDILowerBnds,
			    approxDRLowerBnds, alb);
      factory->add("lower_bounds", fromVec<Real>(alb));
    }
    if (!approxCUpperBnds.empty() || !approxDIUpperBnds.empty() ||
	!approxDRUpperBnds.empty()) {
      RealArray aub;
      merge_variable_arrays(approxCUpperBnds, approxDIUpperBnds,
			    approxDRUpperBnds, aub);
      factory->add("upper_bounds", fromVec<Real>(aub));
    }

    model = factory->Build(*surfData); 
    if (outputLevel > NORMAL_OUTPUT) { 

      Cout << model->asString();

      //      ModelFitness* SS_fitness = ModelFitness::Create("sum_squared");
      //Cout << "Sum-squared goodness of fit =  " 
      //   << (*SS_fitness)(*model,*surfData) << "\n" ;
      //delete SS_fitness;

      //ModelFitness* R2_fitness = ModelFitness::Create("rsquared");
      //Cout << "R-squared goodness of fit =  " 
      //   << (*R2_fitness)(*model,*surfData) << "\n" ;
      //delete R2_fitness;

    }
    // TO DO: extract coefficients array
  }
  catch (std::runtime_error& e) {
    Cerr << e.what() << std::endl;
    Cerr << typeid(e).name() << std::endl;
    abort_handler(-1);
  }
  catch (std::string& e) {
    Cerr << "Error: exception with no recourse caught trying to build model:\n"
	 << e << std::endl;
    abort_handler(-1);
  }
  catch (...) {
    Cerr << "Error: exception caught trying to build model" << std::endl;
    abort_handler(-1);
  }

  if (!exportModelName.empty() && SurfpackInterface::HasFeature("model_save")) {
    if (outputLevel >= VERBOSE_OUTPUT)
      Cout << "\nSaving surrogate model to file " << exportModelName 
	   << std::endl;
    SurfpackInterface::Save(model, exportModelName);
  }
}


void SurfpackApproximation::
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


void SurfpackApproximation::
sdv_to_realarray(const Pecos::SurrogateDataVars& sdv, RealArray& ra)
{
  // check incoming vars for correct length (active or all views)
  const RealVector&  cv = sdv.continuous_variables();
  const IntVector&  div = sdv.discrete_int_variables();
  const RealVector& drv = sdv.discrete_real_variables();
  if (cv.length() + div.length() + drv.length() == numVars)
    merge_variable_arrays(cv, div, drv, ra);
  else {
    Cerr << "Error: bad parameter set length in SurfpackApproximation::"
	 << "sdv_to_realarray()." << std::endl;
    abort_handler(-1);
  }
}
  

void SurfpackApproximation::
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
    Cerr << "Error: bad parameter set length in SurfpackApproximation::"
	 << "vars_to_realarray()." << std::endl;
    abort_handler(-1);
  }
}
  

Real SurfpackApproximation::value(const Variables& vars)
{ 
  //static int times_called = 0;
  if (!model) { 
    Cerr << "Error: surface is null in SurfpackApproximation::value()"
	 << std::endl;  
    abort_handler(-1);
  }

  RealArray x_array;
  vars_to_realarray(vars, x_array);
  return (*model)(x_array);
}


const RealVector& SurfpackApproximation::gradient(const Variables& vars)
{
  approxGradient.sizeUninitialized(vars.cv());
  try {
    RealArray x_array;
    vars_to_realarray(vars, x_array);
    VecDbl local_grad = model->gradient(x_array);
    for (unsigned i = 0; i < surfData->xSize(); i++)
      approxGradient[i] = local_grad[i];
  }
  catch (...) {
    Cerr << "Error: gradient() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }
  return approxGradient;
}


const RealSymMatrix& SurfpackApproximation::hessian(const Variables& vars)
{
  size_t num_cv = vars.cv();
  approxHessian.reshape(num_cv);
  try {
    if (approxType == "global_moving_least_squares") {
      Cerr << "Have not implemented analytical hessians in this surfpack class"
	   << std::endl;
      abort_handler(-1);
    }
    RealArray x_array;
    vars_to_realarray(vars, x_array);
    MtxDbl sm = model->hessian(x_array);
    ///\todo Make this acceptably efficient
    for (size_t i = 0; i < num_cv; i++)
      for(size_t j = 0; j < num_cv; j++)
        approxHessian(i,j) = sm(i,j);
  }
  catch (...) {
    Cerr << "Error: hessian() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }
  return approxHessian;
}


Real SurfpackApproximation::prediction_variance(const Variables& vars)
{
  try {
    RealArray x_array;
    vars_to_realarray(vars, x_array);
    return model->variance(x_array);
  }
  catch (...) {
    Cerr << "Error: prediction_variance() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }
}


Real SurfpackApproximation::diagnostic(const String& metric_type)
{ 
  if (!model) { 
    Cerr << "Error: surface is null in SurfpackApproximation::diagnostic()"
	 << std::endl;  
    abort_handler(-1);
  }

  diagnostic(metric_type, *model, *surfData);
}


Real SurfpackApproximation::diagnostic(const String& metric_type,
				       const SurfpackModel& sp_model,
				       const SurfData& s_data)
{ 
  Real approx_diag;
  try {
    ModelFitness* SS_fitness = ModelFitness::Create(metric_type);
    approx_diag = (*SS_fitness)(sp_model, s_data);
    delete SS_fitness;
  }
  catch (const std::string& msg) {
    Cerr << "Error evaluating surrogate metric: " << msg << std::endl;
    abort_handler(-1);
  }

  Cout << metric_type << " goodness of fit: " << approx_diag << '\n';
  return approx_diag;
}

void SurfpackApproximation::primary_diagnostics()
{
  if (diagnosticSet.empty()) {
    // conditionally print default diagnostics
    if (outputLevel > NORMAL_OUTPUT) {
      Cout << "--- Default surrogate metrics" << std::endl;
      diagnostic("rsquared");
      diagnostic("root_mean_squared");	
      diagnostic("mean_abs");
    }
  }
  else {
    Cout << "--- User-requested surrogate metrics" << std::endl;
    int num_diag = diagnosticSet.size();
    for (int j = 0; j < num_diag; ++j)
      diagnostic(diagnosticSet[j]);

    // BMA TODO: at runtime verify (though Surfpack will too) 
    //  * 1/N <= percentFold <= 0.5
    //  * 2 <= numFolds <= N
    if (crossValidateFlag) {
      int num_folds = numFolds;
      // if no folds, try percent, otherwise set default = 10
      if (num_folds == 0) {
	if (percentFold > 0.0) {
	  num_folds = boost::math::iround(1.0/percentFold);
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "Info: cross_validate num_folds = " << num_folds 
		 << " calculated from specified percent = " << percentFold 
		 << "." << std::endl;
	}
	else {
	  num_folds = 10;
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "Info: default num_folds = " << num_folds 
		 << " used." << percentFold << std::endl;
	}
      }

      Cout << "--- Cross validation (" << num_folds << " folds) of user-"
	   << "specified surrogate metrics" << std::endl;

      CrossValidationFitness CV_fitness(num_folds);
      VecDbl cv_metrics;
      CV_fitness.eval_metrics(cv_metrics, *model, *surfData, diagnosticSet);
      
      for (int j = 0; j < num_diag; ++j) {
	const String& metric_type = diagnosticSet[j];
	Cout << metric_type << " goodness of fit: " << cv_metrics[j] << '\n';
      }

    }
    if (pressFlag) {
      Cout << "--- Leave one out cross validation of user-specified "
	   << "surrogate metrics" << std::endl;

      // perform press as CV with N folds
      CrossValidationFitness CV_fitness(surfData->size());
      VecDbl cv_metrics;
      CV_fitness.eval_metrics(cv_metrics, *model, *surfData, diagnosticSet);
     
      for (int j = 0; j < num_diag; ++j) {
	const String& metric_type = diagnosticSet[j];
	Cout << metric_type << " goodness of fit: " << cv_metrics[j] << '\n';
      }

    }
  }
}


void SurfpackApproximation::
challenge_diagnostics(const RealMatrix& challenge_points, int fn_index)
{
  if (!model) { 
    Cerr << "Error: surface is null in SurfpackApproximation::diagnostic()"
	 << std::endl;  
    abort_handler(-1);
  }

  Cout << "--- Surrogate metrics with respect to user challenge data."
       << std::endl;

  // BMA TODO: later don't recopy every time, just copy once and setDefaultIndex
  SurfData chal_data;

  for (size_t row=0; row<challenge_points.numRows(); ++row) {
      RealArray x(numVars);
      for (size_t col=0; col<numVars; ++col)
	x[col] = challenge_points[col][row];
      Real f = challenge_points[numVars + fn_index][row];
      chal_data.addPoint(SurfPoint(x, f));
    }

  int num_diag = diagnosticSet.size();
  for (int j = 0; j < num_diag; ++j)
    diagnostic(diagnosticSet[j], *model, chal_data);
}



/** Copy the data stored in Dakota-style SurrogateData into
    Surfpack-style SurfPoint and SurfData objects. */
SurfData* SurfpackApproximation::surrogates_to_surf_data()
{
  SurfData* surf_data = new SurfData();

  // screen approximation data for failures
  approxData.data_checks();

  // some surrogates, e.g., global_polynomials and kriging, treat the anchor
  // point specially as a constraint; other treat as a regular data point
  if (approxData.anchor()) {
    if (factory->supports_constraints())
      add_anchor_to_surfdata(*surf_data);
    else
      add_sd_to_surfdata(approxData.anchor_variables(),
			 approxData.anchor_response(),
			 approxData.failed_anchor_data(), *surf_data);
  }
  // add the remaining surrogate data points
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Requested build data order is " << buildDataOrder
	 << '\n';
  size_t i, num_data_pts = approxData.size();
  const Pecos::SDVArray& sdv_array = approxData.variables_data();
  const Pecos::SDRArray& sdr_array = approxData.response_data();
  const Pecos::SizetShortMap& failed_resp = approxData.failed_response_data();
  Pecos::SizetShortMap::const_iterator fit = failed_resp.begin();
  for (i=0; i<num_data_pts; ++i) {
    short fail_code = 0;
    if (fit != failed_resp.end() && fit->first == i)
      { fail_code = fit->second; ++fit; }
    add_sd_to_surfdata(sdv_array[i], sdr_array[i], fail_code, *surf_data);
  }

  return surf_data;
}


/** If there is an anchor point, add an equality constraint for its response
    value.  Also add constraints for gradient and hessian, if applicable. */
void SurfpackApproximation::add_anchor_to_surfdata(SurfData& surf_data)
{
  // coarse-grained fault tolerance for now: any failure qualifies for omission
  if (approxData.failed_anchor_data())
    return;

  // Surfpack's RealArray is std::vector<double>
  RealArray x; 
  Real f;
  RealArray gradient;
  SurfpackMatrix<Real> hessian;

  // Print out the anchor continuous variables
  sdv_to_realarray(approxData.anchor_variables(), x);
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Anchor point vars\n" << x;

  // At a minimum, there should be a response value
  unsigned short anchor_data_order = 1;
  f = approxData.anchor_function();
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Anchor response: " << f << '\n';

  // Check for gradient in anchor point
  const RealVector& anchor_grad = approxData.anchor_gradient();
  if (!anchor_grad.empty()) {
    anchor_data_order |= 2;
    copy_data(anchor_grad, gradient);
    if (outputLevel > NORMAL_OUTPUT) {
      Cout << "Anchor gradient:\n";
      write_data(Cout, anchor_grad);
    }
  }
    
  // Check for hessian in anchor point
  const RealSymMatrix& anchor_hess = approxData.anchor_hessian();
  if (!anchor_hess.empty()) {
    anchor_data_order |= 4;
    copy_matrix(anchor_hess, hessian);
    if (outputLevel > NORMAL_OUTPUT) {
      Cout << "Anchor hessian:\n";
      write_data(Cout, anchor_hess, false, true, true);
    }
  }
   
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Requested constraint data order is " << anchor_data_order
	 << '\n';
  
  // for now only allow builds from exactly 1, 3=1+2, or 7=1+2+4; use
  // different set functions so the SurfPoint data remains empty if
  // not present
  switch (anchor_data_order) {

  case 1:
    surf_data.setConstraintPoint(SurfPoint(x, f));
    break;

  case 3:
    surf_data.setConstraintPoint(SurfPoint(x, f, gradient));
    break;

  case 7:
    surf_data.setConstraintPoint(SurfPoint(x, f, gradient, hessian));
    break;

  default:
    Cerr << "\nError (SurfpackApproximation): derivative data may only be used"
	 << "if all\nlower-order information is also present. Specified "
	 << "anchor_data_order is " << anchor_data_order << "."  << std::endl; 
    abort_handler(-1);
    break;

  }

}


void SurfpackApproximation::
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
    Cerr << "\nError (SurfpackApproximation): derivative data may only be used"
	 << "if all\nlower-order information is also present. Specified "
	 << "buildDataOrder is " << buildDataOrder << "."  << std::endl; 
    abort_handler(-1);
    break;

  }
}


void SurfpackApproximation::copy_matrix(const RealSymMatrix& rsm,
					SurfpackMatrix<Real>& surfpack_matrix)
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
