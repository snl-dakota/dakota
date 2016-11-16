/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
#include "SharedSurfpackApproxData.hpp"
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

using surfpack::toString;
using surfpack::fromVec;

/** Initialize the embedded Surfpack surface object and configure it
    using the specifications from the input file.  Data for the
    surface is created later. */
SurfpackApproximation::
SurfpackApproximation(const ProblemDescDB& problem_db,
		      const SharedApproxData& shared_data,
                      const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label), //surface(NULL),
  surfData(NULL), model(NULL), factory(NULL)
  //sharedDataRep((SharedSurfpackApproxData*)shared_data.data_rep())
{
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;

  ParamMap args;

  args["verbosity"] = toString<short>(sharedDataRep->outputLevel);
  args["ndims"] = toString<size_t>(sharedDataRep->numVars);

  // For now, not exposing Surfpack random seed in the DAKOTA UI;
  // instead fixing at an arbitrary value (treated as int in Surfpack)
  args["seed"] = "8147";

  // For Polynomial surface fits
  if (sharedDataRep->approxType == "global_polynomial") {
    args["type"] = "polynomial";
    args["order"] = toString<unsigned short>(shared_surf_data_rep->approxOrder);
    // TO DO: activate derivative-based regression
  }

  // For Kriging surface fits
  else if (sharedDataRep->approxType == "global_kriging") {

    args["type"]  = "kriging";
    args["order"] = toString<unsigned short>(shared_surf_data_rep->approxOrder);
    args["reduced_polynomial"] =
      (problem_db.get_string("model.surrogate.trend_order") == "quadratic") ?
      toString<bool>(false) : toString<bool>(true);

    // activate derivative information if available
    unsigned short surfpack_derivative_order = 0;
    short bdo = sharedDataRep->buildDataOrder;
    if (bdo & 2)
      surfpack_derivative_order = 1;
    if (bdo & 4) {
      if (bdo & 2)
	surfpack_derivative_order = 2;
      else {
	Cerr << "\nError (global_kriging): Hessian information only used "
	     << "if gradients present.\nbuildDataOrder = " << bdo << std::endl;
	abort_handler(-1);
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
    if (max_trials > 0)
      args["max_trials"] = toString<short>(max_trials);

    // NIDR support for RealArray (aka std::vector) would eliminate xtra copy!
    // old parameters
    Real nugget = problem_db.get_real("model.surrogate.nugget");
    short find_nugget = problem_db.get_short("model.surrogate.find_nugget");
    if (nugget > 0) {
      args["nugget"] = toString<Real>(nugget);
    } 
    else { 
      if (find_nugget > 0) {
	if (find_nugget == 1)  
	  args["find_nugget"] = toString<bool>(false);
	else if (find_nugget == 2)  
	  args["find_nugget"] = toString<bool>(true);
	else {
	  Cerr << " find_nugget must be 1 or 2" << '\n'; 
	  abort_handler(-1);
	}
      }
    }

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
    if (!sharedDataRep->approxCLowerBnds.empty()) {
      RealArray alb_ra;
      copy_data(sharedDataRep->approxCLowerBnds, alb_ra);
      args["lower_bounds"] = fromVec<Real>(alb_ra);
    }
    if (!sharedDataRep->approxCUpperBnds.empty()) {
      RealArray aub_ra;
      copy_data(sharedDataRep->approxCUpperBnds, aub_ra);
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
  else if (sharedDataRep->approxType == "global_neural_network") {
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
  else if (sharedDataRep->approxType == "global_moving_least_squares") {
    args["type"] = "mls";
    short weight
      = problem_db.get_short("model.surrogate.mls_weight_function");
    if (weight > 0) {
      args["weight"] = toString<short>(weight);
    }
    short order = problem_db.get_short("model.surrogate.polynomial_order");
    if (order > 0) {
      args["order"] = toString<short>(order);
    }
  }

  //// For radial basis function networks
  else if (sharedDataRep->approxType == "global_radial_basis") {
    args["type"] = "rbf";
    // mapping number bases to number of centers
    short bases = problem_db.get_short("model.surrogate.rbf_bases");
    if (bases > 0) {
      args["centers"] = toString<short>(bases);
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
    // mapping max_pts to cvt_pts
    short max_pts = problem_db.get_short("model.surrogate.rbf_max_pts");
    if (max_pts > 0) {
      args["cvt_pts"] = toString<short>(max_pts);
    }
  }

  //// For Mars surface fits
  else if (sharedDataRep->approxType == "global_mars") {
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
  //for (ParamMap::iterator itr = args.begin(); itr != args.end(); itr++)
  //   Cout << "     " << itr->first << ": " << itr->second << std::endl;

  factory = ModelFactory::createModelFactory(args);

  //catch(...) {
  //  Cout << "Exception caught in attempt to create Surface object"
  //       << std::endl;
  //  abort_handler(-1);
  //}

  // validate diagnostic settings (preliminary); TODO: do more at
  // run time and move to both ctors
  bool err_found = false;
  const StringArray& diag_set = shared_surf_data_rep->diagnosticSet;
  if (!diag_set.empty()) {
    std::set<std::string> valid_metrics;
    valid_metrics.insert("sum_squared");
    valid_metrics.insert("mean_squared");
    valid_metrics.insert("root_mean_squared");
    valid_metrics.insert("sum_abs");
    valid_metrics.insert("mean_abs");
    valid_metrics.insert("max_abs");
    valid_metrics.insert("rsquared");

    int num_diag = diag_set.size();
    for (int j = 0; j < num_diag; ++j)
      if (valid_metrics.find(diag_set[j]) == valid_metrics.end()) {
	Cerr << "Error: surrogate metric '" << diag_set[j] 
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
  if (shared_surf_data_rep->crossValidateFlag) {
    if (shared_surf_data_rep->numFolds > 0 &&
	shared_surf_data_rep->numFolds < 2) {
      Cerr << "Error: cross_validation folds must be 2 or greater."
	   << std::endl;
      err_found = true;
    }
    if (shared_surf_data_rep->percentFold < 0.0 ||
	shared_surf_data_rep->percentFold > 0.5) {
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
SurfpackApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data),
  surfData(NULL), model(NULL), factory(NULL)
  //sharedDataRep((SharedSurfpackApproxData*)shared_data.data_rep())
{
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;

  ParamMap args;
  args["verbosity"] = toString<short>(sharedDataRep->outputLevel);
  args["ndims"]     = toString<size_t>(sharedDataRep->numVars);
  args["seed"]      = "8147";

  if (sharedDataRep->approxType == "global_polynomial") {
    args["type"] = "polynomial";
    args["order"] = toString<unsigned short>(shared_surf_data_rep->approxOrder);
  }
  else if (sharedDataRep->approxType == "global_kriging") {

    args["type"] = "kriging";
    args["order"] = toString<unsigned short>(shared_surf_data_rep->approxOrder);
    args["reduced_polynomial"] = toString<bool>(true);
    /*
    // bounds set at run time within build():
    if (!sharedDataRep->approxCLowerBnds.empty()) {
      RealArray alb_ra;
      copy_data(sharedDataRep->approxCLowerBnds, alb_ra);
      args["lower_bounds"] = fromVec<Real>(alb_ra);
    }
    if (!sharedDataRep->approxCUpperBnds.empty()) {
      RealArray aub_ra;
      copy_data(sharedDataRep->approxCUpperBnds, aub_ra);
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
    short bdo = sharedDataRep->buildDataOrder;
    if (bdo == 1)      surfpack_derivative_order = 0;
    else if (bdo == 3) surfpack_derivative_order = 1;
    else if (bdo == 7) surfpack_derivative_order = 2;
    else {
      Cerr << "\nError (global_kriging): Unsupported buildDataOrder = " << bdo
	   << std::endl;
      abort_handler(-1);
    }
    args["derivative_order"] = 
      toString<unsigned short>(surfpack_derivative_order);

  }
  else if (sharedDataRep->approxType == "global_neural_network")
    args["type"] = "ann";
  else if (sharedDataRep->approxType == "global_moving_least_squares") {
    args["type"] = "mls";
    args["order"] = toString<unsigned short>(shared_surf_data_rep->approxOrder);
  }
  else if (sharedDataRep->approxType == "global_radial_basis")
    args["type"] = "rbf";
  else if (sharedDataRep->approxType == "global_mars")
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

  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;

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
    if (!sharedDataRep->approxCLowerBnds.empty() ||
	!sharedDataRep->approxDILowerBnds.empty() ||
	!sharedDataRep->approxDRLowerBnds.empty()) {
      RealArray lb;
      shared_surf_data_rep->merge_variable_arrays(
	sharedDataRep->approxCLowerBnds, sharedDataRep->approxDILowerBnds,
	sharedDataRep->approxDRLowerBnds, lb);
      factory->add("lower_bounds", fromVec<Real>(lb));
    }
    if (!sharedDataRep->approxCUpperBnds.empty() ||
	!sharedDataRep->approxDIUpperBnds.empty() ||
	!sharedDataRep->approxDRUpperBnds.empty()) {
      RealArray ub;
      shared_surf_data_rep->merge_variable_arrays(
	sharedDataRep->approxCUpperBnds, sharedDataRep->approxDIUpperBnds,
	sharedDataRep->approxDRUpperBnds, ub);
      factory->add("upper_bounds", fromVec<Real>(ub));
    }

    if (model) {
      delete model;
      model = NULL;
    }
    model = factory->Build(*surfData); 
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

/*  if (!shared_surf_data_rep->exportModelName.empty() &&
      SurfpackInterface::HasFeature("model_save")) {
    if (sharedDataRep->outputLevel >= VERBOSE_OUTPUT)
      Cout << "\nSaving surrogate model to file "
	   << shared_surf_data_rep->exportModelName << std::endl;
    SurfpackInterface::Save(model, shared_surf_data_rep->exportModelName);
  }*/
}


void SurfpackApproximation::export_model(const String& fn_label,
 					 const String& export_prefix, 
                                         const unsigned short export_format)
{
  String without_extension;
  unsigned short formats;
  if(export_format) {
    without_extension = export_prefix + "." + fn_label;
    formats = export_format;
  } else {
    without_extension = sharedDataRep->modelExportPrefix + "." + approxLabel;
    formats = sharedDataRep->modelExportFormat;
  }
  //unsigned short formats = export_format; 
  const bool &can_save = SurfpackInterface::HasFeature("model_save");
  // Saving to text archive
  if(formats & TEXT_ARCHIVE) {
    if(can_save) {
      String filename = without_extension + ".sps";
      SurfpackInterface::Save(model,filename);
    } else
        Cerr << "\nRequested surrogate export to text archive failed: "
		<< "Surfpack lacks support for model saving.\n";
  }
  // Saving to binary archive
  if(formats & BINARY_ARCHIVE) {
    if(can_save) {
      String filename = without_extension + ".bsps";
      SurfpackInterface::Save(model,filename);
    } else
        Cerr << "\nRequested surrogate export to binary archive failed: "
	        << "Surfpack lacks support for model saving.\n";
  }
  // Saving to algebraic file
  if(formats & ALGEBRAIC_FILE) {
    String filename = without_extension + ".alg";
    std::ofstream af(filename.c_str(),std::ofstream::out);
    af << "Model for response " << fn_label << ":\n" << model->asString();
    af.close();
    Cout << "Model saved in algebraic format to file '" << filename << "'.\n";
  }
  // Writing in algebraic format to screen
  if(formats & ALGEBRAIC_CONSOLE) {
    Cout << "\nModel for response " << fn_label << ":\n";
    Cout << model->asString();
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
  ((SharedSurfpackApproxData*)sharedDataRep)->vars_to_realarray(vars, x_array);
  return (*model)(x_array);
}


const RealVector& SurfpackApproximation::gradient(const Variables& vars)
{
  approxGradient.sizeUninitialized(vars.cv());
  try {
    RealArray x_array;
    ((SharedSurfpackApproxData*)sharedDataRep)
      ->vars_to_realarray(vars, x_array);
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
    if (sharedDataRep->approxType == "global_moving_least_squares") {
      Cerr << "Have not implemented analytical hessians in this surfpack class"
	   << std::endl;
      abort_handler(-1);
    }
    RealArray x_array;
    ((SharedSurfpackApproxData*)sharedDataRep)
      ->vars_to_realarray(vars, x_array);
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
    ((SharedSurfpackApproxData*)sharedDataRep)
      ->vars_to_realarray(vars, x_array);
    return model->variance(x_array);
  }
  catch (...) {
    Cerr << "Error: prediction_variance() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }
}

Real SurfpackApproximation::value(const RealVector& c_vars)
{
    //static int times_called = 0;
    if (!model) {
        Cerr << "Error: surface is null in SurfpackApproximation::value()"
        << std::endl;
        abort_handler(-1);
    }
        
    RealArray x_array;
    size_t num_vars = c_vars.length();
    for (size_t i = 0; i < num_vars; i++) x_array.push_back(c_vars[i]);
    return (*model)(x_array);
}
    
    
const RealVector& SurfpackApproximation::gradient(const RealVector& c_vars)
{
    approxGradient.sizeUninitialized(c_vars.length());
    try {
        RealArray x_array;
        size_t num_vars = c_vars.length();
        for (size_t i = 0; i < num_vars; i++) x_array.push_back(c_vars[i]);
        
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
    
    
const RealSymMatrix& SurfpackApproximation::hessian(const RealVector& c_vars)
{
    size_t num_cv = c_vars.length();
    approxHessian.reshape(num_cv);
    try {
        if (sharedDataRep->approxType == "global_moving_least_squares") {
            Cerr << "Have not implemented analytical hessians in this surfpack class"
            << std::endl;
            abort_handler(-1);
        }
        RealArray x_array;
        for (size_t i = 0; i < num_cv; i++) x_array.push_back(c_vars[i]);
        
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
    
    
Real SurfpackApproximation::prediction_variance(const RealVector& c_vars)
{
    try {
        RealArray x_array;
        size_t num_vars = c_vars.length();
        for (size_t i = 0; i < num_vars; i++) x_array.push_back(c_vars[i]);
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

  return diagnostic(metric_type, *model, *surfData);
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

  Cout << std::setw(20) << metric_type << std::setw(20) << approx_diag << '\n';
  return approx_diag;
}


void SurfpackApproximation::primary_diagnostics(int fn_index)
{
  String func_description = approxLabel.empty() ? 
    "function " + boost::lexical_cast<std::string>(fn_index+1) : approxLabel;  
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;
  const StringArray& diag_set = shared_surf_data_rep->diagnosticSet;
  if (diag_set.empty()) {
    // conditionally print default diagnostics
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "\nSurrogate quality metrics for " << func_description << ":\n";
      diagnostic("root_mean_squared");	
      diagnostic("mean_abs");
      diagnostic("rsquared");
    }
  }
  else {
    Cout << "\nSurrogate quality metrics for " << func_description << ":\n";
    int num_diag = diag_set.size();
    for (int j = 0; j < num_diag; ++j)
      diagnostic(diag_set[j]);
   
    // BMA TODO: at runtime verify (though Surfpack will too) 
    //  * 1/N <= percentFold <= 0.5
    //  * 2 <= numFolds <= N
    if (shared_surf_data_rep->crossValidateFlag) {
      int num_folds = shared_surf_data_rep->numFolds;
      // if no folds, try percent, otherwise set default = 10
      if (num_folds == 0) {
        if (shared_surf_data_rep->percentFold > 0.0) {
          num_folds = boost::math::iround(1./shared_surf_data_rep->percentFold);
          if (sharedDataRep->outputLevel >= DEBUG_OUTPUT)
            Cout << "Info: cross_validate num_folds = " << num_folds 
                 << " calculated from specified percent = "
                 << shared_surf_data_rep->percentFold << "." << std::endl;
        }
        else {
          num_folds = 10;
          if (sharedDataRep->outputLevel >= DEBUG_OUTPUT)
            Cout << "Info: default num_folds = " << num_folds << " used."
                 << std::endl;
        }
      }

      Cout << "\nSurrogate quality metrics (" << num_folds << "-fold CV) for " 
           << func_description << ":\n";
      RealArray cv_metrics = cv_diagnostic(diag_set, num_folds);
      //CrossValidationFitness CV_fitness(num_folds);
      //VecDbl cv_metrics;
      //CV_fitness.eval_metrics(cv_metrics, *model, *surfData, diag_set);
      
      for (int j = 0; j < num_diag; ++j) {
        const String& metric_type = diag_set[j];
        if (metric_type == "rsquared")
          Cout << std::setw(20) << metric_type
               << std::setw(20) << std::numeric_limits<Real>::quiet_NaN()
               << "  (n/a for cross-validation)" 
               << std::endl;
        else
          Cout << std::setw(20) << metric_type << std::setw(20) << cv_metrics[j] 
               << std::endl;
      }

    }
    if (shared_surf_data_rep->pressFlag) {
      Cout << "\nSurrogate quality metrics (PRESS/leave-one-out) for " 
           << func_description << ":\n";
     
      // perform press as CV with N folds
      RealArray cv_metrics = cv_diagnostic(diag_set, surfData->size());
      //CrossValidationFitness CV_fitness(surfData->size());
      //VecDbl cv_metrics;
      //CV_fitness.eval_metrics(cv_metrics, *model, *surfData, diag_set);
     
      for (int j = 0; j < num_diag; ++j) {
        const String& metric_type = diag_set[j];
        if (metric_type == "rsquared")
          Cout << std::setw(20) << metric_type 
               << std::setw(20) << std::numeric_limits<Real>::quiet_NaN()
               << "  (n/a for PRESS)" << std::endl;
        else
          Cout << std::setw(20) << metric_type << std::setw(20) << cv_metrics[j] 
               << std::endl;
      }

    }
  }
}


void SurfpackApproximation::
challenge_diagnostics(int fn_index, const RealMatrix& challenge_points,
                      const RealVector& challenge_responses)
{
  if (!model) { 
    Cerr << "Error: surface is null in SurfpackApproximation::diagnostic()"
	 << std::endl;  
    abort_handler(-1);
  }
  
  String func_description = approxLabel.empty() ? 
    "function " + boost::lexical_cast<std::string>(fn_index+1) : approxLabel;  

  // copy
  StringArray diag_set = 
    ((SharedSurfpackApproxData*)sharedDataRep)->diagnosticSet;
  if (diag_set.empty()) {
    // conditionally print default diagnostics
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "\nSurrogate quality metrics (challenge data) for " 
	   << func_description << ":\n";
      diag_set.push_back("root_mean_squared");	
      diag_set.push_back("mean_abs");
      diag_set.push_back("rsquared");
      challenge_diagnostic(diag_set, challenge_points, challenge_responses);
    }
  }
  else {
    Cout << "\nSurrogate quality metrics (challenge data) for " 
	 << func_description << ":\n";
    challenge_diagnostic(diag_set, challenge_points, challenge_responses);
  }

}

RealArray SurfpackApproximation::cv_diagnostic(const StringArray& metric_types, 
                                               unsigned num_folds) {
  CrossValidationFitness CV_fitness(num_folds);
  VecDbl cv_metrics;
  try {
    CV_fitness.eval_metrics(cv_metrics, *model, *surfData, metric_types);
  } catch(String cv_error) {
    Cerr << "Error: Exception caught while computing CV score:\n" << cv_error << std::endl;
    cv_metrics.resize(metric_types.size());
    std::fill(cv_metrics.begin(), cv_metrics.end(), std::numeric_limits<Real>::quiet_NaN());
  }
  return cv_metrics;
}

RealArray SurfpackApproximation::challenge_diagnostic(const StringArray& metric_types,
			    const RealMatrix& challenge_points,
                            const RealVector& challenge_responses) {
  // JAS: painful but probably unavoidable data copy on every call.
  SurfData chal_data;
  RealArray chal_metrics;
  size_t num_v = sharedDataRep->numVars;
  for (size_t row=0; row<challenge_points.numRows(); ++row) {
    RealArray x(num_v);
    for (size_t col=0; col<num_v; ++col)
      x[col] = challenge_points[col][row];
    Real f = challenge_responses[row];
    chal_data.addPoint(SurfPoint(x, f));
  }
  for (int j = 0; j < metric_types.size(); ++j)
    chal_metrics.push_back(diagnostic(metric_types[j], *model, chal_data));
  return chal_metrics;
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
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;
  if (approxData.anchor()) {
    if (factory->supports_constraints())
      add_anchor_to_surfdata(*surf_data);
    else
      shared_surf_data_rep->
	add_sd_to_surfdata(approxData.anchor_variables(),
			   approxData.anchor_response(),
			   approxData.failed_anchor_data(), *surf_data);
  }
  // add the remaining surrogate data points
  if (sharedDataRep->outputLevel >= DEBUG_OUTPUT)
    Cout << "Requested build data order is " << sharedDataRep->buildDataOrder
	 << '\n';
  size_t i, num_data_pts = approxData.points();
  const Pecos::SDVArray& sdv_array = approxData.variables_data();
  const Pecos::SDRArray& sdr_array = approxData.response_data();
  const Pecos::SizetShortMap& failed_resp = approxData.failed_response_data();
  Pecos::SizetShortMap::const_iterator fit = failed_resp.begin();
  for (i=0; i<num_data_pts; ++i) {
    short fail_code = 0;
    if (fit != failed_resp.end() && fit->first == i)
      { fail_code = fit->second; ++fit; }
    shared_surf_data_rep->add_sd_to_surfdata(sdv_array[i], sdr_array[i],
					     fail_code, *surf_data);
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
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;
  shared_surf_data_rep->sdv_to_realarray(approxData.anchor_variables(), x);
  if (sharedDataRep->outputLevel > NORMAL_OUTPUT)
    Cout << "Anchor point vars\n" << x;

  // At a minimum, there should be a response value
  unsigned short anchor_data_order = 1;
  f = approxData.anchor_function();
  if (sharedDataRep->outputLevel > NORMAL_OUTPUT)
    Cout << "Anchor response: " << f << '\n';

  // Check for gradient in anchor point
  const RealVector& anchor_grad = approxData.anchor_gradient();
  if (!anchor_grad.empty()) {
    anchor_data_order |= 2;
    copy_data(anchor_grad, gradient);
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "Anchor gradient:\n";
      write_data(Cout, anchor_grad);
    }
  }

  // Check for hessian in anchor point
  const RealSymMatrix& anchor_hess = approxData.anchor_hessian();
  if (!anchor_hess.empty()) {
    anchor_data_order |= 4;
    shared_surf_data_rep->copy_matrix(anchor_hess, hessian);
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "Anchor hessian:\n";
      write_data(Cout, anchor_hess, false, true, true);
    }
  }

  if (sharedDataRep->outputLevel > NORMAL_OUTPUT)
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

} // namespace Dakota
