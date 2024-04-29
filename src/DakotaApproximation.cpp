/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "TaylorApproximation.hpp"
#include "TANA3Approximation.hpp"
#include "QMEApproximation.hpp"
#include "PecosApproximation.hpp"
#include "GaussProcApproximation.hpp"
#ifdef HAVE_C3
#include "C3Approximation.hpp"
#endif
#include "VPSApproximation.hpp"
#ifdef HAVE_SURFPACK
#include "SurfpackApproximation.hpp"
#endif // HAVE_SURFPACK
#ifdef HAVE_DAKOTA_SURROGATES
#include "DakotaSurrogatesGP.hpp"
#include "DakotaSurrogatesPoly.hpp"
#ifdef HAVE_DAKOTA_PYTHON_SURROGATES
#include "DakotaSurrogatesPython.hpp"
#endif // HAVE_DAKOTA_PYTHON_SURROGATES
#endif // HAVE_DAKOTA_SURROGATES
#include "DakotaGraphics.hpp"

//#define ALLOW_GLOBAL_HERMITE_INTERPOLATION
//#define DEBUG


namespace Dakota {

/** This constructor is the one which must build the base class data
    for all derived classes.  get_approx() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_approx() again).  Since the
    letter IS the representation, its rep pointer is set to NULL. */
Approximation::Approximation(BaseConstructor, const ProblemDescDB& problem_db,
			     const SharedApproxData& shared_data,
                             const String& approx_label):
  approxData(true), approxLabel(approx_label),
  sharedDataRep(shared_data.data_rep())
{ /* empty ctor */ }


/** This constructor is the one which must build the base class data
    for all derived classes.  get_approx() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_approx() again).  Since the
    letter IS the representation, its rep pointer is set to NULL. */
Approximation::
Approximation(NoDBBaseConstructor, const SharedApproxData& shared_data):
  approxData(true), sharedDataRep(shared_data.data_rep())
{ /* empty ctor */ }


/** The default constructor is used in Array<Approximation> instantiations
    and by the alternate envelope constructor.  approxRep is NULL in this
    case (problem_db is needed to build a meaningful Approximation object). */
Approximation::Approximation()
{ /* empty ctor */ }


/** Envelope constructor only needs to extract enough data to properly
    execute get_approx, since Approximation(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
Approximation::
Approximation(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
              const String& approx_label):
  // Set the rep pointer to the appropriate derived type
  approxRep(get_approx(problem_db, shared_data, approx_label))
{
  if ( !approxRep ) // bad type or insufficient memory
    abort_handler(APPROX_ERROR);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
std::shared_ptr<Approximation> Approximation::
get_approx(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
           const String& approx_label)
{
  bool pw_decomp = problem_db.get_bool("model.surrogate.domain_decomp");
  if (pw_decomp) {
    return std::make_shared<VPSApproximation>
      (problem_db, shared_data, approx_label);
  }
  else {
    const String& approx_type = shared_data.data_rep()->approxType;
    if (approx_type == "local_taylor")
      return std::make_shared<TaylorApproximation>
	(problem_db, shared_data, approx_label);
    else if (approx_type == "multipoint_tana")
      return std::make_shared<TANA3Approximation>
	(problem_db, shared_data, approx_label);
    else if (approx_type == "multipoint_qmea")
      return std::make_shared<QMEApproximation>
	(problem_db, shared_data, approx_label);
    else if (strends(approx_type, "_orthogonal_polynomial") ||
	     strends(approx_type, "_interpolation_polynomial"))
      return std::make_shared<PecosApproximation>
	(problem_db, shared_data, approx_label);
    else if (approx_type == "global_gaussian")
      return std::make_shared<GaussProcApproximation>
	(problem_db, shared_data, approx_label);
#ifdef HAVE_C3
    else if (approx_type == "global_function_train")
      return std::make_shared<C3Approximation>
	(problem_db, shared_data, approx_label);
#endif
#ifdef HAVE_SURFPACK
    else if (approx_type == "global_polynomial"     ||
	     approx_type == "global_kriging"        ||
	     approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	     approx_type == "global_radial_basis"   ||
	     approx_type == "global_mars"           ||
	     approx_type == "global_moving_least_squares")
      return std::make_shared<SurfpackApproximation>
	(problem_db, shared_data, approx_label);
#endif // HAVE_SURFPACK
#ifdef HAVE_DAKOTA_SURROGATES
    else if (approx_type == "global_exp_gauss_proc")
      return std::make_shared<SurrogatesGPApprox>
	(problem_db, shared_data, approx_label);
    else if (approx_type == "global_exp_poly")
      return std::make_shared<SurrogatesPolyApprox>
	(problem_db, shared_data, approx_label);
#ifdef HAVE_DAKOTA_PYTHON_SURROGATES
    else if (approx_type == "global_exp_python")
      return std::make_shared<SurrogatesPythonApprox>
	(problem_db, shared_data, approx_label);
#endif // HAVE_DAKOTA_PYTHON_SURROGATES
#endif // HAVE_DAKOTA_SURROGATES
    else {
      Cerr << "Error: Approximation type " << approx_type << " not available."
	   << std::endl;
    }
  }
  return std::shared_ptr<Approximation>();
}


/** This is the alternate envelope constructor for instantiations on
    the fly.  Since it does not have access to problem_db, it utilizes
    the NoDBBaseConstructor constructor chain. */
Approximation::Approximation(const SharedApproxData& shared_data):
  // Set the rep pointer to the appropriate derived type
  approxRep(get_approx(shared_data))
{
  if ( !approxRep ) // bad type or insufficient memory
    abort_handler(APPROX_ERROR);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
std::shared_ptr<Approximation>
Approximation::get_approx(const SharedApproxData& shared_data)
{
  const String&  approx_type = shared_data.data_rep()->approxType;
  if (approx_type == "local_taylor")
    return std::make_shared<TaylorApproximation>(shared_data);
  else if (approx_type == "multipoint_tana")
    return std::make_shared<TANA3Approximation>(shared_data);
  else if (approx_type == "multipoint_qmea")
    return std::make_shared<QMEApproximation>(shared_data);
  else if (strends(approx_type, "_orthogonal_polynomial") ||
	   strends(approx_type, "_interpolation_polynomial"))
    return std::make_shared<PecosApproximation>(shared_data);
#ifdef HAVE_C3
  else if (approx_type == "global_function_train")
    return std::make_shared<C3Approximation>(shared_data);
#endif
  else if (approx_type == "global_gaussian")
    return std::make_shared<GaussProcApproximation>(shared_data);
  else if (approx_type == "global_voronoi_surrogate")
    return std::make_shared<VPSApproximation>(shared_data);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares")
    return std::make_shared<SurfpackApproximation>(shared_data);
#endif // HAVE_SURFPACK
#ifdef HAVE_DAKOTA_SURROGATES
  else if (approx_type == "global_exp_gauss_proc")
    return std::make_shared<SurrogatesGPApprox>(shared_data);
  else if (approx_type == "global_exp_poly")
    return std::make_shared<SurrogatesPolyApprox>(shared_data);
#ifdef HAVE_DAKOTA_PYTHON_SURROGATES
  else if (approx_type == "global_exp_python")
    return std::make_shared<SurrogatesPythonApprox>(shared_data);
#endif // HAVE_DAKOTA_PYTHON_SURROGATES
#endif // HAVE_DAKOTA_SURROGATES
  else
    Cerr << "Error: Approximation type " << approx_type << " not available."
	 << std::endl;

  return std::shared_ptr<Approximation>();
}


/** Copy constructor manages sharing of approxRep. */
Approximation::Approximation(const Approximation& approx):
  approxRep(approx.approxRep)
{ /* empty ctor */ }


Approximation Approximation::operator=(const Approximation& approx)
{
  approxRep = approx.approxRep;
  return *this; // calls copy constructor since returned by value
}


Approximation::~Approximation()
{ /* empty dtor */ }


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::build()
{
  if (approxRep)
    approxRep->build();
  else { // default is only a data check; augmented/replaced by derived class
    check_points(approxData.points());

    // Could also enumerate embedded keys:
    //std::vector<Pecos::ActiveKey> embedded_keys;
    //sharedDataRep->activeKey.extract_keys(embedded_keys);
    //size_t i, num_checks = embedded_keys.size();
    //for (i=0; i<num_checks; ++i)
    //  check_points(approxData.points(embedded_keys[i]));
  }
}


void Approximation::
export_model(const StringArray& var_labels, const String& fn_label,
	     const String& export_prefix, const unsigned short export_format)
{
  if (approxRep)
    approxRep->export_model(var_labels, fn_label, export_prefix, export_format);
  // if no export_model, return without doing anything
}

void Approximation::
export_model(const Variables& vars, const String& fn_label,
	     const String& export_prefix, const unsigned short export_format)
{
  if (approxRep)
    approxRep->export_model(vars, fn_label, export_prefix, export_format);
  // if no export_model, return without doing anything
}


void Approximation::rebuild()
{
  if (approxRep)
    approxRep->rebuild();
  else
    build(); // if no incremental rebuild(), fall back on full build()
}


void Approximation::replace(const IntResponsePair& response_pr, size_t fn_index)
{
  if (approxRep)
    approxRep->replace(response_pr, fn_index);
  else {
    Pecos::SurrogateDataResp sdr
      = response_to_sdr(response_pr.second, fn_index);// *** Note: SHALLOW_COPY *** the referenced responses will be cleared from the Model queue but *may* persist in the eval cache; this is what ApproximationInterface::cache_lookup() manages ... Only matters for gradient/Hessian views ...
    approxData.replace(/*sharedDataRep->approxDataKeys,*/
		       sdr, response_pr.first);
  }
}


void Approximation::pop_data(bool save_data)
{
  if (approxRep) approxRep->pop_data(save_data);
  else approxData.pop(sharedDataRep->activeKey, save_data);
}


void Approximation::push_data()
{
  if (approxRep) approxRep->push_data();
  else {
    const Pecos::ActiveKey& key = sharedDataRep->activeKey;
    // want aggregated active key (not embedded keys) for retrieval index
    // as this is what is activated through the Model
    size_t r_index = sharedDataRep->push_index(key);
    approxData.push(key, r_index); // preserves active state
    /*
    const UShort2DArray& keys = sharedDataRep->approxDataKeys;
    if (!keys.empty()) {
      // Only want active key for retrieval index as this is what is
      // activated through the Model.  Other keys are only used for
      // enumerating SurrogateData updates using approxDataKeys.
      const UShortArray& active_key = keys.back();//= sharedDataRep->activeKey;
      size_t r_index = sharedDataRep->push_index(active_key);
      // preserves active state
      approxData.push(keys, r_index); // preserves active state
    }
    */
  }
}


void Approximation::finalize_data()
{
  // finalization has to apply restorations in the correct order

  if (approxRep) approxRep->finalize_data();
  else {
    const Pecos::ActiveKey& key = sharedDataRep->activeKey;
    // want aggregated active key (not embedded keys) for retrieval index
    // as this is what is activated through the Model
    size_t f_index, p, num_popped = approxData.popped_sets(key);
    for (p=0; p<num_popped; ++p) {
      f_index = sharedDataRep->finalize_index(p, key);
      approxData.push(key, f_index, false);
    }
    /*
    const UShort2DArray& keys = sharedDataRep->approxDataKeys;
    if (!keys.empty()) {
      // Only need truth model key for finalization indices (see above)
      const UShortArray& active_key = keys.back();//= sharedDataRep->activeKey;
      // assume number of popped trials is consistent across approxData
      size_t f_index, p, num_popped = approxData.popped_sets(active_key);
      for (p=0; p<num_popped; ++p) {
	f_index = sharedDataRep->finalize_index(p, active_key);
	approxData.push(keys, f_index, false);
      }
    }
    */

    clear_active_popped(); // after all finalization indices processes
  }
}


void Approximation::pop_coefficients(bool save_data)
{
  if (approxRep) approxRep->pop_coefficients(save_data);
  //else no op
}


void Approximation::push_coefficients()
{
  if (approxRep) approxRep->push_coefficients();
  //else no op
}


void Approximation::finalize_coefficients()
{
  if (approxRep) approxRep->finalize_coefficients();
  //else no op
}


void Approximation::combine_coefficients()
{
  if (approxRep) approxRep->combine_coefficients();
  //else no op
}


void Approximation::clear_inactive_coefficients()
{
  if (approxRep) approxRep->clear_inactive_coefficients();
  //else no op
}


/*
void Approximation::combined_to_active_data()
{
  if (approxRep) approxRep->combined_to_active();
  else approxData.active_key(sharedDataRep->active_model_key());
}
*/


void Approximation::combined_to_active_coefficients(bool clear_combined)
{
  if (approxRep) approxRep->combined_to_active_coefficients(clear_combined);
  //else no-op
}


Real Approximation::value(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: value() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->value(vars);
}


const RealVector& Approximation::gradient(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: gradient() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->gradient(vars);
}


const RealSymMatrix& Approximation::hessian(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: hessian() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }
    
  return approxRep->hessian(vars);
}


Real Approximation::prediction_variance(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: prediction_variance() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->prediction_variance(vars);
}

Real Approximation::mean()
{
  if (!approxRep) {
    Cerr << "Error: mean() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->mean();
}

Real Approximation::mean(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error: mean(x) not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->mean(x);
}

Real Approximation::combined_mean()
{
  if (!approxRep) {
    Cerr << "Error: combined_mean() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->combined_mean();
}

Real Approximation::combined_mean(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error: combined_mean(x) not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->combined_mean(x);
}

const RealVector& Approximation::mean_gradient()
{
  if (!approxRep) {
    Cerr << "Error: mean_gradient() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->mean_gradient();
}

const RealVector& Approximation::
mean_gradient(const RealVector& x, const SizetArray& ind)
{
  if (!approxRep) {
    Cerr << "Error: mean_gradient(x, ind) not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->mean_gradient(x,ind);
} 

Real Approximation::variance()
{
  if (!approxRep) {
    Cerr << "Error:variance() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->variance();
}

Real Approximation::variance(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error:variance(x) not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->variance(x);
}    

const RealVector& Approximation::variance_gradient()
{
  if (!approxRep) {
    Cerr << "Error: variance_gradient() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->variance_gradient();
}

const RealVector& Approximation::
variance_gradient(const RealVector& x, const SizetArray& ind)
{
  if (!approxRep) {
    Cerr << "Error: variance_gradient(x, ind) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->variance_gradient(x,ind);
}

Real Approximation::covariance(Approximation& approx_2)
{
  if (!approxRep) {
    Cerr << "Error: covariance(other) not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->covariance(approx_2);
}
   
Real Approximation::covariance(const RealVector& x, Approximation& approx_2)
{
  if (!approxRep) {
    Cerr << "Error: covariance(x, other) not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->covariance(x, approx_2);
}

Real Approximation::combined_covariance(Approximation& approx_2)
{
  if (!approxRep) {
    Cerr << "Error: combined_covariance(other) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->combined_covariance(approx_2);
}
   
Real Approximation::
combined_covariance(const RealVector& x, Approximation& approx_2)
{
  if (!approxRep) {
    Cerr << "Error:combined_covariance(x, other) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->combined_covariance(x, approx_2);
}

void Approximation::compute_moments(bool full_stats, bool combined_stats)
{
  if (approxRep)
    approxRep->compute_moments(full_stats, combined_stats);
  else {
    Cerr << "Error: compute_moments() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

void Approximation::
compute_moments(const RealVector& x, bool full_stats, bool combined_stats)
{
  if (approxRep)
    approxRep->compute_moments(x, full_stats, combined_stats);
  else {
    Cerr << "Error: compute_moments(RealVector) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

const RealVector& Approximation::moments() const
{
  if (!approxRep) {
    Cerr << "Error: moments() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->moments();
}

const RealVector& Approximation::expansion_moments() const
{
  if (!approxRep) {
    Cerr << "Error: expansion_moments() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->expansion_moments();
}

const RealVector& Approximation::numerical_integration_moments() const
{
  if (!approxRep) {
    Cerr << "Error: numerical_integration_moments() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->numerical_integration_moments();
}

const RealVector& Approximation::combined_moments() const
{
  if (!approxRep) {
    Cerr << "Error: combined_moments() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->combined_moments();
}

Real Approximation::moment(size_t i) const
{
  if (!approxRep) {
    Cerr << "Error: moment(size_t) not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->moment(i);
}

void Approximation::moment(Real mom, size_t i)
{
  if (approxRep)
    approxRep->moment(mom, i);
  else {
    Cerr << "Error: moment(Real, size_t) not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

Real Approximation::combined_moment(size_t i) const
{
  if (!approxRep) {
    Cerr << "Error: combined_moment(size_t) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->combined_moment(i);
}

void Approximation::combined_moment(Real mom, size_t i)
{
  if (approxRep)
    approxRep->combined_moment(mom, i);
  else {
    Cerr << "Error: combined_moment(Real, size_t) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

void Approximation::clear_component_effects()
{
  if (approxRep)
    approxRep->clear_component_effects();
  else {
    Cerr << "Error: clear_component_effects() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

void Approximation::compute_component_effects()
{
  if (approxRep)
    approxRep->compute_component_effects();
  else {
    Cerr << "Error: compute_component_effects() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

void Approximation::compute_total_effects()
{
  if (approxRep)
    approxRep->compute_total_effects();
  else {
    Cerr << "Error: compute_total_effects() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

const RealVector& Approximation::sobol_indices() const
{
  if (!approxRep) {
    Cerr << "Error: sobol_indices() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->sobol_indices();
}

const RealVector& Approximation::total_sobol_indices() const
{
  if (!approxRep) {
    Cerr << "Error: total_sobol_indices() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->total_sobol_indices();
}

ULongULongMap Approximation::sparse_sobol_index_map() const
{
  if (!approxRep) {
    Cerr << "Error: sparse_sobol_index_map() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->sparse_sobol_index_map();
}

Real Approximation::value(const RealVector& c_vars)
{
  if (!approxRep) {
    Cerr << "Error: value() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->value(c_vars);
}

const RealVector& Approximation::gradient(const RealVector& c_vars)
{
  if (!approxRep) {
    Cerr << "Error: gradient() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->gradient(c_vars);
}
    
    
const RealSymMatrix& Approximation::hessian(const RealVector& c_vars)
{
  if (!approxRep) {
    Cerr << "Error: hessian() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }
        
  return approxRep->hessian(c_vars);
}
    
    
Real Approximation::prediction_variance(const RealVector& c_vars)
{
  if (!approxRep) {
    Cerr << "Error: prediction_variance() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
        
  return approxRep->prediction_variance(c_vars);
}


bool Approximation::advancement_available()
{
  if (approxRep) return approxRep->advancement_available();
  else           return true; // only a few cases throttle advancements
}


bool Approximation::diagnostics_available()
{
  if (approxRep) // envelope fwd to letter
    return approxRep->diagnostics_available();
  else // default for letter lacking virtual fn redefinition
    return false;
}


Real Approximation::diagnostic(const String& metric_type)
{
  if (!approxRep) {
    Cerr << "Error: diagnostic() not available for this approximation type." 
  	   << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->diagnostic(metric_type);
}


RealArray Approximation::
cv_diagnostic(const StringArray& metric_types, unsigned num_folds)
{
  if (!approxRep) {
    Cerr << "Error: cv_diagnostic() not available for this approximation type." 
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->cv_diagnostic(metric_types, num_folds);
}


void Approximation::primary_diagnostics(size_t fn_index)
{
  if (approxRep)
    approxRep->primary_diagnostics(fn_index);
  // else no-op
}


RealArray Approximation::
challenge_diagnostic(const StringArray& metric_types,
		     const RealMatrix& challenge_points,
                     const RealVector& challenge_resps)
{
  if (!approxRep) {
    Cerr << "Error: challenge_diagnostic() not available for this approximation"
	 << " type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->
    challenge_diagnostic(metric_types, challenge_points, challenge_resps);
}


void Approximation::
challenge_diagnostics(size_t fn_index, const RealMatrix& challenge_points,
                      const RealVector& challenge_resps)
{
  if (approxRep)
    approxRep->challenge_diagnostics(fn_index, challenge_points, 
				     challenge_resps);
  // else no-op
}


RealVector Approximation::approximation_coefficients(bool normalized) const
{
  if (!approxRep) {
    Cerr << "Error: approximation_coefficients() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
   
  return approxRep->approximation_coefficients(normalized);
}


void Approximation::
approximation_coefficients(const RealVector& approx_coeffs, bool normalized)
{
  if (approxRep)
    approxRep->approximation_coefficients(approx_coeffs, normalized);
  else {
    Cerr << "Error: approximation_coefficients() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


/*
void Approximation::link_multilevel_surrogate_data()
{
  if (approxRep) approxRep->link_multilevel_surrogate_data();
  //else no-op (no linkage required for derived Approximation)
}
*/


void Approximation::
coefficient_labels(std::vector<std::string>& coeff_labels) const
{
  if (approxRep)
    approxRep->coefficient_labels(coeff_labels);
  else {
    Cerr << "Error: coefficient_labels() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void Approximation::print_coefficients(std::ostream& s, bool normalized)
{
  if (approxRep)
    approxRep->print_coefficients(s, normalized);
  else {
    Cerr << "Error: print_coefficients() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


int Approximation::min_coefficients() const
{
  if (!approxRep) { // no default implementation
    Cerr << "Error: min_coefficients() not defined for this approximation type."
         << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return approxRep->min_coefficients(); // fwd to letter
}


int Approximation::recommended_coefficients() const
{
  // derived class may override, else return minimum coefficients
  if (approxRep)
    return approxRep->recommended_coefficients(); // fwd to letter
  else
    return min_coefficients();
}


int Approximation::num_constraints() const
{
  if (approxRep) // fwd to letter
    return approxRep->num_constraints();
  // default implementation:
  else if (approxData.anchor()) { // anchor data may differ from buildDataOrder
    const Pecos::SurrogateDataResp& anchor_sdr = approxData.anchor_response();
    int ng = anchor_sdr.response_gradient().length(),
        nh = anchor_sdr.response_hessian().numRows();
    return 1 + ng + nh*(nh + 1)/2;
  }
  else
    return 0;
}


int Approximation::min_points(bool constraint_flag) const
{
  if (approxRep)
    return approxRep->min_points(constraint_flag); 
  else { // not virtual: implementation for all derived classes
    int coeffs = min_coefficients();
    if (constraint_flag)
      coeffs -= num_constraints();
    short bdo = sharedDataRep->buildDataOrder;
    size_t data_per_pt = 0, num_v = sharedDataRep->numVars;
    if (bdo & 1) data_per_pt += 1;
    if (bdo & 2) data_per_pt += num_v;
    if (bdo & 4) data_per_pt += num_v*(num_v + 1)/2;
    return (data_per_pt > 1) ?
      (int)std::ceil((Real)coeffs/(Real)data_per_pt) : coeffs;
  }
}


int Approximation::recommended_points(bool constraint_flag) const
{
  if (approxRep)
    return approxRep->recommended_points(constraint_flag); 
  else { // not virtual: implementation for all derived classes
    int coeffs = recommended_coefficients();
    if (constraint_flag)
      coeffs -= num_constraints();
    short bdo = sharedDataRep->buildDataOrder;
    size_t data_per_pt = 0, num_v = sharedDataRep->numVars;
    if (bdo & 1) data_per_pt += 1;
    if (bdo & 2) data_per_pt += num_v;
    if (bdo & 4) data_per_pt += num_v*(num_v + 1)/2;
    return (data_per_pt > 1) ?
      (int)std::ceil((Real)coeffs/(Real)data_per_pt) : coeffs;
  }
}


void Approximation::expansion_coefficient_flag(bool flag)
{
  if (approxRep)
    approxRep->expansion_coefficient_flag(flag);
  else {
    Cerr << "Error: expansion_coefficient_flag() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


bool Approximation::expansion_coefficient_flag() const
{
  if (approxRep)
    return approxRep->expansion_coefficient_flag();
  else
    return false;
}


void Approximation::expansion_gradient_flag(bool flag)
{
  if (approxRep)
    approxRep->expansion_gradient_flag(flag);
  else {
    Cerr << "Error: expansion_gradient_flag() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


bool Approximation::expansion_gradient_flag() const
{
  if (approxRep)
    return approxRep->expansion_gradient_flag();
  else
    return false;
}    


void Approximation::clear_computed_bits()
{
  if (approxRep) approxRep->clear_computed_bits();
  //else no-op
}


void Approximation::map_variable_labels(const Variables& dfsm_vars)
{
  if (approxRep)
    approxRep->map_variable_labels(dfsm_vars);
  else {
    Cerr << "Error: Approximation::map_variable_labels() called on unsupported "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


/*
void Approximation::
variables_to_sdv(const RealVector& c_vars, const IntVector& di_vars,
                 const RealVector& dr_vars)
{
  // deep_copy requests are applied downstream in add(SurrogateDataVars)
  return Pecos::SurrogateDataVars(c_vars, di_vars, dr_vars,Pecos::SHALLOW_COPY);
}
*/


Pecos::SurrogateDataVars Approximation::
variables_to_sdv(const Real* sample_c_vars)
{
  // create view of numVars entries within column of sample Matrix;
  // for compact mode, any active discrete {int,real} vars are managed
  // as real values (e.g., NonDSampling::update_model_from_sample())
  // and we do not convert them back to {di,dr}_vars here.
  RealVector c_vars(Teuchos::View, const_cast<Real*>(sample_c_vars),
		    sharedDataRep->numVars);
  // deep_copy requests are applied downstream in add(SurrogateDataVars)
  return Pecos::SurrogateDataVars(c_vars, Pecos::SHALLOW_COPY);
}


Pecos::SurrogateDataVars Approximation::variables_to_sdv(const Variables& vars)
{
  // Approximation does not know about view mappings; therefore, take the
  // simple approach of matching up active or all counts with numVars.
  size_t num_v = sharedDataRep->numVars;
  // these two options take precedence
  if (vars.cv() + vars.div() + vars.drv() == num_v)
    return Pecos::SurrogateDataVars(vars.continuous_variables(),
      vars.discrete_int_variables(), vars.discrete_real_variables(),
      Pecos::SHALLOW_COPY);
  else if (vars.acv() + vars.adiv() + vars.adrv() == num_v)
    return Pecos::SurrogateDataVars(vars.all_continuous_variables(),
      vars.all_discrete_int_variables(), vars.all_discrete_real_variables(),
      Pecos::SHALLOW_COPY);
  // fallback options; currently treat ambiguous case as error
  else if (vars.cv() == num_v)
    return Pecos::SurrogateDataVars(vars.continuous_variables(),
      Pecos::SHALLOW_COPY);
  //else if (vars.acv() == num_v) // potential conflict with cv/div/drv
  //  return Pecos::SurrogateDataVars(vars.all_continuous_variables(),
  //    Pecos::SHALLOW_COPY);
  else {
    Cerr << "Error: variable size mismatch in Approximation::variables_to_sdv()"
	 << std::endl;
    abort_handler(APPROX_ERROR);
    return Pecos::SurrogateDataVars();
  }
}


Pecos::SurrogateDataResp Approximation::
response_to_sdr(const Response& response, size_t fn_index)
{
  short asv_val = response.active_set_request_vector()[fn_index];
  switch (asv_val) {
  case 0: // should not happen: ASV dropouts managed at higher level
    return Pecos::SurrogateDataResp(); break;
  case 1: // special case with lightweight ctor
    return Pecos::SurrogateDataResp(response.function_value(fn_index));
    break;
  default: {
    // general ASV: use empty vectors/matrices if data is not active.
    Real fn_val = (asv_val & 1) ? response.function_value(fn_index) : 0.;
    RealVector fn_grad;  RealSymMatrix fn_hess;
    if (asv_val & 2) fn_grad = response.function_gradient_view(fn_index);
    if (asv_val & 4) fn_hess = response.function_hessian_view(fn_index);
    // deep_copy requests are managed in add(SurrogateDataResp)
    return Pecos::SurrogateDataResp(fn_val, fn_grad, fn_hess, asv_val,
				    Pecos::SHALLOW_COPY);
    break;
  }
  }
}


/** Short cut function (not used by ApproximationInterface). */
void Approximation::
add_array(const RealMatrix& sample_vars, bool v_copy,
	  const RealVector& sample_resp, bool r_copy, size_t key_index)
{
  if (approxRep)
    approxRep->add_array(sample_vars, v_copy, sample_resp, r_copy, key_index);
  else { // not virtual: all derived classes use following definition
    size_t i, num_samples = sample_vars.numCols();
    if (sample_resp.length() != num_samples) {
      Cerr << "\nError: incompatible data sizes in Approximation::add_array"
	   << "(RealMatrix&, RealVector&)." << std::endl;
      abort_handler(APPROX_ERROR);
    }
    bool anchor_flag = false;
    assign_key_index(key_index);
    for (i=0; i<num_samples; ++i) {

      // add variable values (column of samples matrix)
      Pecos::SurrogateDataVars sdv = variables_to_sdv(sample_vars[i]);
      // add response value (scalar)
      Pecos::SurrogateDataResp sdr(sample_resp[i]);

      add(sdv, v_copy, sdr, r_copy, anchor_flag);
      //add(eval_id); // not supported by this API
    }
  }
}


/* Old 3D graphics capability:
void Approximation::draw_surface()
{
  if (approxRep)
    approxRep->draw_surface();
  else { // draw surface of 2 independent variables

    if (approxCLowerBnds.length() != 2 ||
	approxCUpperBnds.length() != 2 || sharedDataRep->numVars != 2)
      return;

    // define number of equally spaced points in each direction
    int i, j, num_intervals = 50, num_axis_pts = num_intervals + 1;
    Real dx = (approxCUpperBnds[0] - approxCLowerBnds[0])/num_intervals,
         dy = (approxCUpperBnds[1] - approxCLowerBnds[1])/num_intervals;
    RealMatrix F(num_axis_pts, num_axis_pts);
    RealVector X(num_axis_pts), Y(num_axis_pts);
  
    // fill X & Y with grid of values to be displayed in graphics
    X[0] = approxCLowerBnds[0];
    Y[0] = approxCLowerBnds[1];
    for (i=1; i<num_axis_pts; i++) {
      X[i] = X[i-1] + dx;
      Y[i] = Y[i-1] + dy;
    }

    // evaluate approximate response values at all grid intersections to
    // populate F matrix
    RealVector xtemp(2);
    for (i=0; i<num_axis_pts; i++) {
      for (j=0; j<num_axis_pts; j++) {
        xtemp[0] = X[i];
        xtemp[1] = Y[j];
        F(j,i)   = value(xtemp);
      }
    }
    dakota_graphics.show_data_3d(X, Y, F);
  }
}
*/

} // namespace Dakota
