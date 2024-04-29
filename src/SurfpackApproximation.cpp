/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
  //sharedDataRep((SharedSurfpackApproxData*)shared_data.data_rep())
{
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);

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

  spFactory.reset(ModelFactory::createModelFactory(args));

  //catch(...) {
  //  Cout << "Exception caught in attempt to create Surface object"
  //       << std::endl;
  //  abort_handler(-1);
  //}

  // validate diagnostic settings (preliminary); TODO: do more at
  // run time and move to both ctors
  std::set<std::string> allowed_metrics =
    { "sum_squared", "mean_squared", "root_mean_squared",
      "sum_abs", "mean_abs", "max_abs",
      "rsquared" };
  shared_surf_data_rep->validate_metrics(allowed_metrics);

  if (problem_db.get_bool("model.surrogate.import_surrogate"))
    import_model(problem_db);
}


/// On-the-fly constructor which uses mostly Surfpack model defaults
SurfpackApproximation::
SurfpackApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
  //sharedDataRep((SharedSurfpackApproxData*)shared_data.data_rep())
{
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);

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
  
  spFactory.reset(ModelFactory::createModelFactory(args));
}


int SurfpackApproximation::min_coefficients() const
{
  assert(spFactory);
  return spFactory->minPointsRequired();
}


int SurfpackApproximation::recommended_coefficients() const
{
  assert(spFactory);
  return spFactory->recommendedNumPoints();
}


void SurfpackApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // Surface factory should have been created in constructor
  if (!spFactory) {
    Cerr << "Error: Uninitialized model factory in SurfpackApproximation::build()."
	 << std::endl;  
    abort_handler(-1);
  }

  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);

  // clear any imported model mapping
  modelIsImported = false;
  shared_surf_data_rep->varsMapIndices.clear();

  /// \todo Right now, we're completely deleting the old data and then
  /// recopying the current data into a SurfData object.  This was just
  /// the easiest way to arrive at a solution that would build and run.
  /// This function is frequently called from addPoint rebuild, however,
  /// and it's not good to go through this whole process every time one
  /// more data point is added.
  try {
    surrogates_to_surf_data();
 
    // set bounds at run time since they are updated by some methods (e.g., SBO)
    if (!sharedDataRep->approxCLowerBnds.empty() ||
	!sharedDataRep->approxDILowerBnds.empty() ||
	!sharedDataRep->approxDRLowerBnds.empty()) {
      RealArray lb(sharedDataRep->numVars);
      shared_surf_data_rep->merge_variable_arrays(
	sharedDataRep->approxCLowerBnds, sharedDataRep->approxDILowerBnds,
	sharedDataRep->approxDRLowerBnds, lb);
      spFactory->add("lower_bounds", fromVec<Real>(lb));
    }
    if (!sharedDataRep->approxCUpperBnds.empty() ||
	!sharedDataRep->approxDIUpperBnds.empty() ||
	!sharedDataRep->approxDRUpperBnds.empty()) {
      RealArray ub(sharedDataRep->numVars);
      shared_surf_data_rep->merge_variable_arrays(
	sharedDataRep->approxCUpperBnds, sharedDataRep->approxDIUpperBnds,
	sharedDataRep->approxDRUpperBnds, ub);
      spFactory->add("upper_bounds", fromVec<Real>(ub));
    }

    spModel.reset(spFactory->Build(*surfData));
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


void
SurfpackApproximation::map_variable_labels(const Variables& dfsm_vars)
{
  // When importing, always map from all variables to the
  // subset needed by the surrogate...
  const auto& approx_labels = spModel->input_labels();
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    map_variable_labels(dfsm_vars, approx_labels);
}


void SurfpackApproximation::
export_model(const Variables& vars, const String& fn_label,
	     const String& export_prefix,
	     const unsigned short export_format)
{
  StringArray var_labels =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    variable_labels(vars);
  export_model(var_labels, fn_label, export_prefix, export_format);
}


void SurfpackApproximation::
export_model(const StringArray& var_labels, const String& fn_label,
	     const String& export_prefix, const unsigned short export_format)
{
  // TODO: This needs protection against empty model too (maybe)
  spModel->input_labels(var_labels);

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
      SurfpackInterface::Save(spModel.get(),filename);
    } else
        Cerr << "\nRequested surrogate export to text archive failed: "
		<< "Surfpack lacks support for model saving.\n";
  }
  // Saving to binary archive
  if(formats & BINARY_ARCHIVE) {
    if(can_save) {
      String filename = without_extension + ".bsps";
      SurfpackInterface::Save(spModel.get(),filename);
    } else
        Cerr << "\nRequested surrogate export to binary archive failed: "
	        << "Surfpack lacks support for model saving.\n";
  }
  // Saving to algebraic file
  if(formats & ALGEBRAIC_FILE) {
    String filename = without_extension + ".alg";
    std::ofstream af(filename.c_str(),std::ofstream::out);
    af << "Model for response " << fn_label << ":\n" << spModel->asString();
    af.close();
    Cout << "Model saved in algebraic format to file '" << filename << "'.\n";
  }
  // Writing in algebraic format to screen
  if(formats & ALGEBRAIC_CONSOLE) {
    Cout << "\nModel for response " << fn_label << ":\n";
    Cout << spModel->asString();
  }    
}

Real SurfpackApproximation::value(const Variables& vars)
{ 
  //static int times_called = 0;
  if (!spModel) {
    Cerr << "Error: surface is null in SurfpackApproximation::value()"
	 << std::endl;  
    abort_handler(-1);
  }

  return (*spModel)(map_eval_vars(vars));
}


const RealVector& SurfpackApproximation::gradient(const Variables& vars)
{
  approxGradient.sizeUninitialized(vars.cv());
  try {
    VecDbl local_grad = spModel->gradient(map_eval_vars(vars));
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
    MtxDbl sm = spModel->hessian(map_eval_vars(vars));
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
    return spModel->variance(map_eval_vars(vars));
  }
  catch (...) {
    Cerr << "Error: prediction_variance() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }
}


RealArray SurfpackApproximation::map_eval_vars(const Variables& vars)
{
  if (modelIsImported)
    return std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
      imported_eval_vars<RealArray>(vars);
  else {
    // active or all variables
    RealArray surr_vars(sharedDataRep->numVars);
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
      vars_to_realarray(vars, surr_vars);
    return surr_vars;
  }
}


Real SurfpackApproximation::value(const RealVector& c_vars)
{
    //static int times_called = 0;
    if (!spModel) {
        Cerr << "Error: surface is null in SurfpackApproximation::value()"
        << std::endl;
        abort_handler(-1);
    }
        
    RealArray x_array;
    size_t num_vars = c_vars.length();
    for (size_t i = 0; i < num_vars; i++) x_array.push_back(c_vars[i]);
    return (*spModel)(x_array);
}
    
    
const RealVector& SurfpackApproximation::gradient(const RealVector& c_vars)
{
    approxGradient.sizeUninitialized(c_vars.length());
    try {
        RealArray x_array;
        size_t num_vars = c_vars.length();
        for (size_t i = 0; i < num_vars; i++) x_array.push_back(c_vars[i]);
        
        VecDbl local_grad = spModel->gradient(x_array);
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
        
        MtxDbl sm = spModel->hessian(x_array);
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
        return spModel->variance(x_array);
    }
    catch (...) {
        Cerr << "Error: prediction_variance() not available for this "
        << "approximation type." << std::endl;
        abort_handler(-1);
    }
}


Real SurfpackApproximation::diagnostic(const String& metric_type)
{ 
  if (!spModel) {
    Cerr << "Error: surface is null in SurfpackApproximation::diagnostic()"
         << std::endl;  
    abort_handler(-1);
  }

  return diagnostic(metric_type, *spModel, *surfData);
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

  Cout << std::setw(20) << metric_type << "  " << approx_diag << '\n';
  return approx_diag;
}


void SurfpackApproximation::primary_diagnostics(size_t fn_index)
{
  String func_description = approxLabel.empty() ?
    "function " + std::to_string(fn_index+1) : approxLabel;
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
  const StringArray& diag_set = shared_surf_data_rep->diagnosticSet;
  if (diag_set.empty()) {
    // conditionally print default diagnostics
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "\nSurrogate quality metrics at build (training) points for "
	   << func_description << ":\n";
      for (const auto& req_diag : {"root_mean_squared", "mean_abs", "rsquared"})
	diagnostic(req_diag);
    }
  }
  else {
    Cout << "\nSurrogate quality metrics at build (training) points for "
	 << func_description << ":\n";
    for (const auto& req_diag : diag_set)
      diagnostic(req_diag);
   
    // BMA TODO: at runtime verify (though Surfpack will too) 
    if (shared_surf_data_rep->crossValidateFlag) {
      unsigned num_folds = shared_surf_data_rep->numFolds;
      Cout << "\nSurrogate quality metrics (" << num_folds << "-fold CV) for " 
           << func_description << ":\n";
      RealArray cv_metrics = cv_diagnostic(diag_set, num_folds);
      for (int j = 0; j < diag_set.size(); ++j) {
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
      for (int j = 0; j < diag_set.size(); ++j) {
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
challenge_diagnostics(size_t fn_index, const RealMatrix& challenge_points,
                      const RealVector& challenge_responses)
{
  if (!spModel) {
    Cerr << "Error: surface is null in SurfpackApproximation::diagnostic()"
	 << std::endl;  
    abort_handler(-1);
  }
  
  String func_description = approxLabel.empty() ? 
    "function " + std::to_string(fn_index+1) : approxLabel;
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
  StringArray diag_set = shared_surf_data_rep->diagnosticSet;
  // conditionally print default diagnostics
  if (diag_set.empty() && sharedDataRep->outputLevel > NORMAL_OUTPUT) {
    diag_set.push_back("root_mean_squared");
    diag_set.push_back("mean_abs");
    diag_set.push_back("rsquared");
  }
  Cout << "\nSurrogate quality metrics at challenge (test) points for "
       << func_description << ":\n";
  challenge_diagnostic(diag_set, challenge_points, challenge_responses);
}


RealArray SurfpackApproximation::cv_diagnostic(const StringArray& metric_types, 
                                               unsigned num_folds) {
  CrossValidationFitness CV_fitness(num_folds);
  VecDbl cv_metrics;
  try {
    CV_fitness.eval_metrics(cv_metrics, *spModel, *surfData, metric_types);
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
    chal_metrics.push_back(diagnostic(metric_types[j], *spModel, chal_data));
  return chal_metrics;
}


void SurfpackApproximation::import_model(const ProblemDescDB& problem_db)
{
  auto import_prefix =
    problem_db.get_string("model.surrogate.model_import_prefix");
  auto import_format =
    problem_db.get_ushort("model.surrogate.model_import_format");
  bool is_binary = import_format & BINARY_ARCHIVE;
  std::string filename = import_prefix + "." + approxLabel +
    (is_binary ? ".bsps" : ".sps");

  spModel.reset(SurfpackInterface::LoadModel(filename));

  if (sharedDataRep->outputLevel >= NORMAL_OUTPUT)
    Cout << "Imported surrogate for response '" << approxLabel
	 << "' from file '" << filename << "'." << std::endl;

  modelIsImported = true;
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    varsMapIndices.clear();
}


/** Copy the data stored in Dakota-style SurrogateData into
    Surfpack-style SurfPoint and SurfData objects. Updates surfData */
void SurfpackApproximation::surrogates_to_surf_data()
{
  surfData.reset(new SurfData());

  // screen approximation data for failures
  approxData.data_checks();
  // add the remaining surrogate data points
  if (sharedDataRep->outputLevel >= DEBUG_OUTPUT)
    Cout << "Requested build data order is " << sharedDataRep->buildDataOrder
	 << '\n';

  std::shared_ptr<SharedSurfpackApproxData> data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
  size_t i, num_data_pts = approxData.points();
  const Pecos::SDVArray& sdv_array = approxData.variables_data();
  const Pecos::SDRArray& sdr_array = approxData.response_data();
  const Pecos::SizetShortMap& failed_resp = approxData.failed_response_data();
  Pecos::SizetShortMap::const_iterator fit = failed_resp.begin();

  // some surrogates, e.g., global_polynomials and kriging, treat the anchor
  // point specially as a constraint; other treat as a regular data point
  size_t constr_index = approxData.anchor_index();
  if (spFactory->supports_constraints() && constr_index != _NPOS) {
    for (i=0; i<num_data_pts; ++i) {
      short fail_code = 0;
      if (fit != failed_resp.end() && fit->first == i)
	{ fail_code = fit->second; ++fit; }
      if (i == constr_index)
	add_constraints_to_surfdata(sdv_array[i], sdr_array[i], fail_code);
      else
	data_rep->
	  add_sd_to_surfdata(sdv_array[i], sdr_array[i], fail_code, *surfData);
    }
  }
  else
    for (i=0; i<num_data_pts; ++i) {
      short fail_code = 0;
      if (fit != failed_resp.end() && fit->first == i)
	{ fail_code = fit->second; ++fit; }
      data_rep->add_sd_to_surfdata(sdv_array[i], sdr_array[i], fail_code,
				   *surfData);
    }
}


/** If there is an anchor point, add an equality constraint for its response
    value.  Also add constraints for gradient and hessian, if applicable. */
void SurfpackApproximation::
add_constraints_to_surfdata(const Pecos::SurrogateDataVars& anchor_vars,
			    const Pecos::SurrogateDataResp& anchor_resp,
			    short fail_code)
{
  // coarse-grained fault tolerance for now: any failure qualifies for omission
  if (fail_code)
    return;

  // Surfpack's RealArray is std::vector<double>
  RealArray x(sharedDataRep->numVars);
  Real f;
  RealArray gradient;
  SurfpackMatrix<Real> hessian;

  // Print out the anchor continuous variables
  std::shared_ptr<SharedSurfpackApproxData> data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
  data_rep->sdv_to_realarray(anchor_vars, x);
  if (sharedDataRep->outputLevel > NORMAL_OUTPUT)
    Cout << "Anchor point vars\n" << x;

  // At a minimum, there should be a response value
  unsigned short anchor_data_order = 1;
  f = anchor_resp.response_function();
  if (sharedDataRep->outputLevel > NORMAL_OUTPUT)
    Cout << "Anchor response: " << f << '\n';

  // Check for gradient in anchor point
  const RealVector& anchor_grad = anchor_resp.response_gradient();
  if (!anchor_grad.empty()) {
    anchor_data_order |= 2;
    copy_data(anchor_grad, gradient);
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "Anchor gradient:\n" << anchor_grad;
    }
  }

  // Check for hessian in anchor point
  const RealSymMatrix& anchor_hess = anchor_resp.response_hessian();
  if (!anchor_hess.empty()) {
    anchor_data_order |= 4;
    data_rep->copy_matrix(anchor_hess, hessian);
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
    surfData->setConstraintPoint(SurfPoint(x, f));
    break;

  case 3:
    surfData->setConstraintPoint(SurfPoint(x, f, gradient));
    break;

  case 7:
    surfData->setConstraintPoint(SurfPoint(x, f, gradient, hessian));
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
