/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Approximation
//- Description:  Class implementation of base class for approximations
//-               
//- Owner:        Mike Eldred, Sandia National Laboratories

#include "DakotaApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "TaylorApproximation.hpp"
#include "TANA3Approximation.hpp"
#include "PecosApproximation.hpp"
#include "GaussProcApproximation.hpp"
#include "VPSApproximation.hpp"
#ifdef HAVE_SURFPACK
#include "SurfpackApproximation.hpp"
#endif // HAVE_SURFPACK
#include "DakotaGraphics.hpp"

//#define ALLOW_GLOBAL_HERMITE_INTERPOLATION
//#define DEBUG


namespace Dakota {

/** This constructor is the one which must build the base class data
    for all derived classes.  get_approx() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_approx() again).  Since the
    letter IS the representation, its rep pointer is set to NULL (an
    uninitialized pointer causes problems in ~Approximation). */
Approximation::Approximation(BaseConstructor, const ProblemDescDB& problem_db,
			     const SharedApproxData& shared_data,
                             const String& approx_label):
  sharedDataRep(shared_data.data_rep()), approxRep(NULL),
  approxLabel(approx_label), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(BaseConstructor) called to build base "
       << "class for letter." << std::endl;
#endif
}


/** This constructor is the one which must build the base class data
    for all derived classes.  get_approx() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_approx() again).  Since the
    letter IS the representation, its rep pointer is set to NULL (an
    uninitialized pointer causes problems in ~Approximation). */
Approximation::
Approximation(NoDBBaseConstructor, const SharedApproxData& shared_data):
  sharedDataRep(shared_data.data_rep()), approxRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(NoDBBaseConstructor) called to build "
       << "base class for letter." << std::endl;
#endif
}


/** The default constructor is used in Array<Approximation> instantiations
    and by the alternate envelope constructor.  approxRep is NULL in this
    case (problem_db is needed to build a meaningful Approximation object).
    This makes it necessary to check for NULL in the copy constructor,
    assignment operator, and destructor. */
Approximation::Approximation():
  sharedDataRep(NULL), approxRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation() called to build empty approximation "
       << "object." << std::endl;
#endif
}


/** Envelope constructor only needs to extract enough data to properly
    execute get_approx, since Approximation(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
Approximation::
Approximation(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
              const String& approx_label):
  sharedDataRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(ProblemDescDB&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate derived type
  approxRep = get_approx(problem_db, shared_data, approx_label);
  if ( !approxRep ) // bad type or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
Approximation* Approximation::
get_approx(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
           const String& approx_label)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_approx(ProblemDescDB&)."
       << std::endl;
#endif

  bool pw_decomp = problem_db.get_bool("model.surrogate.domain_decomp");
  if (pw_decomp) {
    return new VPSApproximation(problem_db, shared_data, approx_label);
  } else {
    const String& approx_type = shared_data.data_rep()->approxType;
    if (approx_type == "local_taylor")
      return new TaylorApproximation(problem_db, shared_data, approx_label);
    else if (approx_type == "multipoint_tana")
      return new TANA3Approximation(problem_db, shared_data, approx_label);
    else if (strends(approx_type, "_orthogonal_polynomial") ||
	     strends(approx_type, "_interpolation_polynomial"))
      return new PecosApproximation(problem_db, shared_data, approx_label);
    else if (approx_type == "global_gaussian")
      return new GaussProcApproximation(problem_db, shared_data, approx_label);
#ifdef HAVE_SURFPACK
    else if (approx_type == "global_polynomial"     ||
	     approx_type == "global_kriging"        ||
	     approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	     approx_type == "global_radial_basis"   ||
	     approx_type == "global_mars"           ||
	     approx_type == "global_moving_least_squares")
      return new SurfpackApproximation(problem_db, shared_data, approx_label);
#endif // HAVE_SURFPACK
    else {
      Cerr << "Error: Approximation type " << approx_type << " not available."
	   << std::endl;
      return NULL;
    }
  }
}


/** This is the alternate envelope constructor for instantiations on
    the fly.  Since it does not have access to problem_db, it utilizes
    the NoDBBaseConstructor constructor chain. */
Approximation::Approximation(const SharedApproxData& shared_data):
  sharedDataRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(String&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate derived type
  approxRep = get_approx(shared_data);
  if ( !approxRep ) // bad type or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
Approximation* Approximation::get_approx(const SharedApproxData& shared_data)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_approx(String&)." << std::endl;
#endif

  Approximation* approx;
  const String&  approx_type = shared_data.data_rep()->approxType;
  if (approx_type == "local_taylor")
    approx = new TaylorApproximation(shared_data);
  else if (approx_type == "multipoint_tana")
    approx = new TANA3Approximation(shared_data);
  else if (strends(approx_type, "_orthogonal_polynomial") ||
	   strends(approx_type, "_interpolation_polynomial"))
    approx = new PecosApproximation(shared_data);
  else if (approx_type == "global_gaussian")
    approx = new GaussProcApproximation(shared_data);
  else if (approx_type == "global_voronoi_surrogate")
    approx = new VPSApproximation(shared_data);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares")
    approx = new SurfpackApproximation(shared_data);
#endif // HAVE_SURFPACK
  else {
    Cerr << "Error: Approximation type " << approx_type << " not available."
	 << std::endl;
    approx = NULL;
  }
  return approx;
}


/** Copy constructor manages sharing of approxRep and incrementing
    of referenceCount. */
Approximation::Approximation(const Approximation& approx)
{
  // Increment new (no old to decrement)
  approxRep = approx.approxRep;
  if (approxRep) // Check for an assignment of NULL
    ++approxRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(Approximation&)" << std::endl;
  if (approxRep)
    Cout << "approxRep referenceCount = " << approxRep->referenceCount
	 << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old approxRep, assigns
    new approxRep, and increments referenceCount for new approxRep. */
Approximation Approximation::operator=(const Approximation& approx)
{
  if (approxRep != approx.approxRep) { // normal case: old != new
    // Decrement old
    if (approxRep) // Check for NULL
      if ( --approxRep->referenceCount == 0 ) 
	delete approxRep;
    // Assign and increment new
    approxRep = approx.approxRep;
    if (approxRep) // Check for NULL
      ++approxRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::operator=(Approximation&)" << std::endl;
  if (approxRep)
    Cout << "approxRep referenceCount = " << approxRep->referenceCount
	 << std::endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes approxRep
    when referenceCount reaches zero. */
Approximation::~Approximation()
{ 
  // Check for NULL pointer 
  if (approxRep) {
    --approxRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    Cout << "approxRep referenceCount decremented to " 
	 << approxRep->referenceCount << std::endl;
#endif
    if (approxRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting approxRep" << std::endl;
#endif
      delete approxRep;
    }
  }
}


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::build()
{
  if (approxRep)
    approxRep->build();
  else {
    size_t num_curr_pts = approxData.points();
    int ms = min_points(true); // account for anchor point & buildDataOrder
    if (num_curr_pts < ms) {
      Cerr << "\nError: not enough samples to build approximation.  "
	   << "Construction of this approximation\n       requires at least "
	   << ms << " samples for " << sharedDataRep->numVars << " variables.  "
	   << "Only " << num_curr_pts << " samples were provided." << std::endl;
      abort_handler(-1);
    }
  }
}


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::export_model(const String& fn_label, 
                                 const String& export_prefix, 
                                 const unsigned short export_format)
{
  if (approxRep)
    approxRep->export_model(fn_label, export_prefix, export_format);
  // if no export_model, return without doing anything
}


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::rebuild()
{
  if (approxRep)
    approxRep->rebuild();
  else // virtual fn: default definition
    build(); // if no special rebuild optimization, fall back on full build()
}


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::pop(bool save_data)
{
  if (approxRep)
    approxRep->pop(save_data);
  else {
    if (popCountStack.empty()) {
      Cerr << "\nError: empty count stack in Approximation::pop()."
	   << std::endl;
      abort_handler(-1);
    }
    approxData.pop(popCountStack.back(), save_data);
    popCountStack.pop_back();
  }
}


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::push()
{
  if (approxRep)
    approxRep->push();
  else
    popCountStack.push_back(approxData.push(sharedDataRep->retrieval_index()));
}


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void Approximation::finalize()
{
  if (approxRep)
    approxRep->finalize();
  else {
    // finalization has to apply restorations in the correct order
    size_t i, num_popped = approxData.popped_trials(); // # of popped trials
    for (i=0; i<num_popped; ++i)
      approxData.push(sharedDataRep->finalization_index(i), false);
    clear_popped(); // clear only after process completed
  }
}


void Approximation::store(size_t index)
{
  if (approxRep) approxRep->store(index);
  else           approxData.store(index); // derived classes augment
}


void Approximation::restore(size_t index)
{
  if (approxRep) approxRep->restore(index);
  else           approxData.restore(index); // derived classes augment
}


void Approximation::remove_stored(size_t index)
{
  if (approxRep) approxRep->remove_stored(index);
  else           approxData.remove_stored(index); // derived classes augment
}


void Approximation::combine(short corr_type, size_t swap_index)
{
  if (approxRep) approxRep->combine(corr_type, swap_index);
//else           approxData.combine(); // base contribution; derived augments
}


Real Approximation::value(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: value() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }

  return approxRep->value(vars);
}


const RealVector& Approximation::gradient(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: gradient() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }

  return approxRep->gradient(vars);
}


const RealSymMatrix& Approximation::hessian(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: hessian() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }
    
  return approxRep->hessian(vars);
}


Real Approximation::prediction_variance(const Variables& vars)
{
  if (!approxRep) {
    Cerr << "Error: prediction_variance() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(-1);
  }

  return approxRep->prediction_variance(vars);
}

    
Real Approximation::value(const RealVector& c_vars)
{
    if (!approxRep) {
        Cerr << "Error: value() not available for this approximation type."
        << std::endl;
        abort_handler(-1);
    }
        
    return approxRep->value(c_vars);
}
    
    
const RealVector& Approximation::gradient(const RealVector& c_vars)
{
    if (!approxRep) {
        Cerr << "Error: gradient() not available for this approximation type."
        << std::endl;
        abort_handler(-1);
    }
        
    return approxRep->gradient(c_vars);
}
    
    
const RealSymMatrix& Approximation::hessian(const RealVector& c_vars)
{
    if (!approxRep) {
        Cerr << "Error: hessian() not available for this approximation type."
        << std::endl;
        abort_handler(-1);
    }
        
    return approxRep->hessian(c_vars);
}
    
    
Real Approximation::prediction_variance(const RealVector& c_vars)
{
    if (!approxRep) {
        Cerr << "Error: prediction_variance() not available for this approximation "
        << "type." << std::endl;
        abort_handler(-1);
    }
        
    return approxRep->prediction_variance(c_vars);
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
    abort_handler(-1);
  }

  return approxRep->diagnostic(metric_type);
}


RealArray Approximation::cv_diagnostic(const StringArray& metric_types, unsigned num_folds)
{
  if (!approxRep) {
    Cerr << "Error: cv_diagnostic() not available for this approximation type." 
  	   << std::endl;
    abort_handler(-1);
  }

  return approxRep->cv_diagnostic(metric_types, num_folds);
}


void Approximation::primary_diagnostics(int fn_index)
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
    Cerr << "Error: challenge_diagnostic() not available for this approximation " 
	 << " type." << std::endl;
    abort_handler(-1);
  }

  return approxRep->challenge_diagnostic(metric_types, challenge_points, challenge_resps);
}


void Approximation::
challenge_diagnostics(int fn_index, const RealMatrix& challenge_points,
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
    abort_handler(-1);
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
    abort_handler(-1);
  }
}


void Approximation::
coefficient_labels(std::vector<std::string>& coeff_labels) const
{
  if (approxRep)
    approxRep->coefficient_labels(coeff_labels);
  else {
    Cerr << "Error: coefficient_labels() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(-1);
  }
}


void Approximation::print_coefficients(std::ostream& s, bool normalized)
{
  if (approxRep)
    approxRep->print_coefficients(s, normalized);
  else {
    Cerr << "Error: print_coefficients() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(-1);
  }
}


int Approximation::min_coefficients() const
{
  if (!approxRep) { // no default implementation
    Cerr << "Error: min_coefficients() not defined for this approximation type."
         << std::endl;
    abort_handler(-1);
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
  else { // default implementation
    if (approxData.anchor()) { // anchor data may differ from buildDataOrder
      int ng = approxData.anchor_gradient().length(),
          nh = approxData.anchor_hessian().numRows();
      return 1 + ng + nh*(nh + 1)/2;
    }
    else
      return 0;
  }
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


void Approximation::
add(const Variables& vars, bool anchor_flag, bool deep_copy)
{
  if (approxRep)
    approxRep->add(vars, anchor_flag, deep_copy);
  else { // not virtual: all derived classes use following definition
    // Approximation does not know about view mappings; therefore, take the
    // simple approach of matching up active or all counts with numVars.
    size_t num_v = sharedDataRep->numVars;
    if (vars.cv() + vars.div() + vars.drv() == num_v)
      add(vars.continuous_variables(), vars.discrete_int_variables(),
	  vars.discrete_real_variables(), anchor_flag, deep_copy);
    else if (vars.acv() + vars.adiv() + vars.adrv() == num_v)
      add(vars.all_continuous_variables(), vars.all_discrete_int_variables(),
	  vars.all_discrete_real_variables(), anchor_flag, deep_copy);
    /*
    else if (vars.cv() == num_v) {  // compactMode does not affect vars
      IntVector empty_iv; RealVector empty_rv;
      add(vars.continuous_variables(), empty_iv, empty_rv,
	  anchor_flag, deep_copy);
    }
    else if (vars.acv() == num_v) { // potential conflict with cv/div/drv
      IntVector empty_iv; RealVector empty_rv;
      add(vars.all_continuous_variables(), empty_iv, empty_rv,
	  anchor_flag, deep_copy);
    }
    */
    else {
      Cerr << "Error: variable size mismatch in Approximation::add()."
	   << std::endl;
      abort_handler(-1);
    }
  }
}


void Approximation::
add(const Response& response, int fn_index, bool anchor_flag, bool deep_copy)
{
  if (approxRep)
    approxRep->add(response, fn_index, anchor_flag, deep_copy);
  else { // not virtual: all derived classes use following definition
    short asv_val = response.active_set_request_vector()[fn_index];
    //if (asv_val) { // ASV dropouts are now managed at a higher level

    // use empty vectors/matrices if data is not active.
    Real fn_val = (asv_val & 1) ? response.function_value(fn_index) : 0.;
    RealVector fn_grad;
    if (asv_val & 2)
      fn_grad = response.function_gradient_view(fn_index); // view of column
    RealSymMatrix empty_hess;
    const RealSymMatrix& fn_hess = (asv_val & 4) ?
      response.function_hessian(fn_index) : empty_hess;

    // Map DAKOTA's deep_copy bool into Pecos' copy mode
    // (Pecos::DEFAULT_COPY is not supported through DAKOTA).
    short mode = (deep_copy) ? Pecos::DEEP_COPY : Pecos::SHALLOW_COPY;
    Pecos::SurrogateDataResp sdr(fn_val, fn_grad, fn_hess, asv_val, mode);
    if (anchor_flag) approxData.anchor_response(sdr);
    else             approxData.push_back(sdr);

    //}
  }
}


void Approximation::
add(const RealMatrix& sample_vars, const RealVector& sample_resp)
{
  if (approxRep)
    approxRep->add(sample_vars, sample_resp);
  else { // not virtual: all derived classes use following definition
    size_t num_samples = sample_vars.numCols();
    if (sample_resp.length() != num_samples) {
      Cerr << "\nError: incompatible approx build data sizes.\n";
      abort_handler(-1);
    }
    size_t num_vars = sample_vars.numRows();
    bool anchor_flag = false, deep_copy = true;
    for (size_t i=0; i<num_samples; ++i) {

      // add variable values (column of samples matrix)
      add(sample_vars[i], anchor_flag, deep_copy);

      // add response value
      short asv_val = 1;
      Real fn_val = sample_resp[i];
      RealVector empty_grad;
      RealSymMatrix empty_hess;
      short mode = (deep_copy) ? Pecos::DEEP_COPY : Pecos::SHALLOW_COPY;
      Pecos::SurrogateDataResp sdr(fn_val, empty_grad, empty_hess, asv_val, mode);
      approxData.push_back(sdr);
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
