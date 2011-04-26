/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Approximation
//- Description:  Class implementation of base class for approximations
//-               
//- Owner:        Mike Eldred, Sandia National Laboratories

#include "DakotaApproximation.H"
#include "ProblemDescDB.H"
#include "DakotaVariables.H"
#include "DakotaResponse.H"
#include "TaylorApproximation.H"
#include "TANA3Approximation.H"
#include "PecosApproximation.H"
#include "GaussProcApproximation.H"
#ifdef HAVE_SURFPACK
#include "SurfpackApproximation.H"
#endif // HAVE_SURFPACK
#include "DakotaGraphics.H"

#define DEBUG


namespace Dakota {

/** This constructor is the one which must build the base class data
    for all derived classes.  get_approx() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_approx() again).  Since the
    letter IS the representation, its rep pointer is set to NULL (an
    uninitialized pointer causes problems in ~Approximation). */
Approximation::Approximation(BaseConstructor, const ProblemDescDB& problem_db,
			     size_t num_vars):
  // See base constructor in DakotaIterator.C for full discussion of output
  // verbosity.  For approximations, verbose adds quad poly coeff reporting.
  outputLevel(problem_db.get_short("method.output")),
  numVars(num_vars), approxType(problem_db.get_string("model.surrogate.type")),
  buildDataOrder(1), popCount(1), approxRep(NULL), referenceCount(1)
{
  if (problem_db.get_bool("model.surrogate.derivative_usage") &&
      approxType != "global_polynomial"                       &&
      approxType != "global_kriging"                          &&
      approxType != "global_orthogonal_polynomial"            &&
      approxType != "piecewise_interpolation_polynomial")
    Cerr << "\nWarning: use_derivatives is not currently supported by "
	 << approxType << ".\n\n";

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
Approximation::Approximation(BaseConstructor, const String& approx_type,
			     size_t num_vars, short data_order):
  outputLevel(NORMAL_OUTPUT), numVars(num_vars), approxType(approx_type),
  buildDataOrder(data_order), popCount(1), approxRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(BaseConstructor) called to build base "
       << "class for letter." << std::endl;
#endif
}


/** The default constructor is used in Array<Approximation> instantiations
    and by the alternate envelope constructor.  approxRep is NULL in this
    case (problem_db is needed to build a meaningful Approximation object).
    This makes it necessary to check for NULL in the copy constructor,
    assignment operator, and destructor. */
Approximation::Approximation(): buildDataOrder(1), outputLevel(NORMAL_OUTPUT),
  popCount(1), approxRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation() called to build empty approximation "
       << "object." << std::endl;
#endif
}


/** Envelope constructor only needs to extract enough data to properly
    execute get_approx, since Approximation(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
Approximation::Approximation(ProblemDescDB& problem_db, size_t num_vars):
  referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(ProblemDescDB&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate derived type
  approxRep = get_approx(problem_db, num_vars);
  if ( !approxRep ) // bad type or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
Approximation* Approximation::
get_approx(ProblemDescDB& problem_db, size_t num_vars)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_approx(ProblemDescDB&)."
       << std::endl;
#endif

  const String& approx_type = problem_db.get_string("model.surrogate.type");
  if (approx_type == "local_taylor")
    return new TaylorApproximation(problem_db, num_vars);
  else if (approx_type == "multipoint_tana")
    return new TANA3Approximation(problem_db, num_vars);
  else if (approx_type == "global_orthogonal_polynomial" ||
           approx_type == "global_interpolation_polynomial" ||
           approx_type == "piecewise_orthogonal_polynomial" ||  // multi-element
           approx_type == "piecewise_interpolation_polynomial") // local spline
    return new PecosApproximation(problem_db, num_vars);
  else if (approx_type == "global_gaussian")
    return new GaussProcApproximation(problem_db, num_vars);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares")
    return new SurfpackApproximation(problem_db, num_vars);
#endif // HAVE_SURFPACK
  else {
    Cerr << "Error: Approximation type " << approx_type << " not available."
	 << std::endl;
    return NULL;
  }
}


/** This is the alternate envelope constructor for instantiations on
    the fly.  Since it does not have access to problem_db, the letter
    class is not fully populated.  This constructor executes
    get_approx(type), which invokes the default constructor of the
    derived letter class, which in turn invokes the default
    constructor of the base class. */
Approximation::
Approximation(const String& approx_type, const UShortArray& approx_order,
	      size_t num_vars, short data_order):
  referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Approximation::Approximation(String&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate derived type
  approxRep = get_approx(approx_type, approx_order, num_vars, data_order);
  if ( !approxRep ) // bad type or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
Approximation* Approximation::
get_approx(const String& approx_type, const UShortArray& approx_order, 
	   size_t num_vars, short data_order)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_approx(String&)." << std::endl;
#endif

  Approximation* approx;
  if (approx_type == "local_taylor")
    approx = new TaylorApproximation(num_vars, data_order);
  else if (approx_type == "multipoint_tana")
    approx = new TANA3Approximation(num_vars, data_order);
  else if (approx_type == "global_orthogonal_polynomial" ||
	   approx_type == "global_interpolation_polynomial" ||
           approx_type == "piecewise_orthogonal_polynomial" ||  // multi-element
           approx_type == "piecewise_interpolation_polynomial") // local spline
    approx = new PecosApproximation(approx_type, approx_order,
				    num_vars,    data_order);
  else if (approx_type == "global_gaussian")
    approx = new GaussProcApproximation(num_vars, data_order);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares")
    approx = new SurfpackApproximation(approx_type, approx_order,
				       num_vars,    data_order);
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
    approxRep->referenceCount++;

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
      approxRep->referenceCount++;
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


void Approximation::build()
{
  if (approxRep)
    approxRep->build();
  else { // virtual fn: this is the common base class portion and is
         // insufficient on its own; derived implementations should
         // explicitly invoke (or reimplement) this base class contribution.
    size_t num_curr_pts = currentPoints.size();
    int ms = min_points(true); // account for anchorPoint & buildDataOrder
    if (num_curr_pts < ms) {
      Cerr << "\nError: not enough samples to build approximation.  "
	   << "Construction of this approximation\n       requires at least "
	   << ms << " samples for " << numVars << " variables.  Only "
	   << num_curr_pts << " samples were provided." << std::endl;
      abort_handler(-1);
    }
  }
}


void Approximation::rebuild()
{
  if (approxRep)
    approxRep->rebuild();
  else  { // virtual fn: no default, error if not supplied in derived class
    Cerr << "Error: rebuild() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }
}


void Approximation::pop(bool save_sdp_set)
{
  if (approxRep)
    approxRep->pop(save_sdp_set);
  else  {
    // virtual fn: this is the common base class portion and is
    // insufficient on its own; derived implementations should
    // explicitly invoke (or reimplement) this base class contribution.
    size_t curr_size = currentPoints.size(), num_pop = pop_count();
    if (curr_size >= num_pop) {
      // save currentPoints data within savedSDPSets instance
      if (save_sdp_set) {
	SDPArray sdp; savedSDPSets.push_back(sdp); // copy empty
	SDPArray& last_sdp = savedSDPSets.back();  // update in place
	// prevent underflow portability issue w/ compiler coercion of -num_pop
	SDPLDiffT reverse_advance = -(SDPLDiffT)num_pop;
	SDPLIter it = currentPoints.end(); std::advance(it, reverse_advance);
	for (; it!=currentPoints.end(); ++it)
	  last_sdp.push_back(*it);
      }
      // drop num_data_pts off of end of currentPoints
      currentPoints.resize(curr_size - num_pop);
    }
    else {
      Cerr << "Error: cannot pop " << num_pop << " points from current list of "
	   << curr_size << " entries in Approximation::pop()." << std::endl;
      abort_handler(-1);
    }
  }
}


size_t Approximation::pop_count()
{
  return (approxRep) ? approxRep->pop_count() // derived class redefinition
                     : popCount; // default definition
}


void Approximation::restore()
{
  if (approxRep)
    approxRep->restore();
  else { // virtual fn: this is the common base class portion and is
         // insufficient on its own; derived implementations should
         // explicitly invoke (or reimplement) this base class contribution.
    SDP2AIter it = savedSDPSets.begin();
    std::advance(it, restoration_index());
    currentPoints.insert(currentPoints.end(), it->begin(), it->end());
    savedSDPSets.erase(it);
  }
}


bool Approximation::restore_available()
{
  if (!approxRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: restore_available() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(-1);
  }

  return approxRep->restore_available();
}


size_t Approximation::restoration_index()
{
  if (!approxRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: restoration_index() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(-1);
  }

  return approxRep->restoration_index();
}


void Approximation::finalize()
{
  if (approxRep)
    approxRep->finalize();
  else { // virtual fn: this is the common base class portion and is
         // insufficient on its own; derived implementations should
         // explicitly invoke (or reimplement) this base class contribution.
    size_t i, index, num_restore = savedSDPSets.size();
    for (i=0; i<num_restore; ++i) {
      index = finalization_index(i);
      currentPoints.insert(currentPoints.end(), savedSDPSets[index].begin(),
			   savedSDPSets[index].end());
    }
    savedSDPSets.clear();
  }
}


size_t Approximation::finalization_index(size_t i)
{
  if (!approxRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: finalization_index(size_t) not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }

  return approxRep->finalization_index(i);
}


const Real& Approximation::get_value(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error: get_value() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }

  return approxRep->get_value(x);
}


const RealVector& Approximation::get_gradient(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error: get_gradient() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }

  return approxRep->get_gradient(x);
}


const RealSymMatrix& Approximation::get_hessian(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error: get_hessian() not available for this approximation type."
	 << std::endl;
    abort_handler(-1);
  }
    
  return approxRep->get_hessian(x);
}


const Real& Approximation::get_prediction_variance(const RealVector& x)
{
  if (!approxRep) {
    Cerr << "Error: get_prediction_variance() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }

  return approxRep->get_prediction_variance(x);
}


const bool Approximation::diagnostics_available()
{
  if (approxRep) // envelope fwd to letter
    return approxRep->diagnostics_available();
  else // default for letter lacking virtual fn redefinition
    return false;
}


const Real& Approximation::get_diagnostic(const String& metric_type)
{
  //if (!approxRep) {
  //  Cerr << "Error: get_diagnostic() not available for this" 
  //	 << " approximation type."
  //	 << std::endl;
  // return 0.;
  //}

  return approxRep->get_diagnostic(metric_type);
}


const RealVector& Approximation::approximation_coefficients() const
{
  if (!approxRep) {
    Cerr << "Error: approximation_coefficients() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }
   
  return approxRep->approximation_coefficients();
}


void Approximation::approximation_coefficients(const RealVector& approx_coeffs)
{
  if (approxRep)
    approxRep->approximation_coefficients(approx_coeffs);
  else {
    Cerr << "Error: approximation_coefficients() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(-1);
  }
}


void Approximation::print_coefficients(std::ostream& s) const
{
  if (approxRep)
    approxRep->print_coefficients(s);
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
    if (anchorPoint.is_null())
      return 0;
    else { // anchorPoint data may differ from buildDataOrder setting
      int ng = anchorPoint.response_gradient().length(),
          nh = anchorPoint.response_hessian().numRows();
      return 1 + ng + nh*(nh + 1)/2;
    }
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
    int data_per_pt = 0;
    if (buildDataOrder & 1) data_per_pt += 1;
    if (buildDataOrder & 2) data_per_pt += numVars;
    if (buildDataOrder & 4) data_per_pt += numVars*(numVars + 1)/2;
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
    int data_per_pt = 0;
    if (buildDataOrder & 1) data_per_pt += 1;
    if (buildDataOrder & 2) data_per_pt += numVars;
    if (buildDataOrder & 4) data_per_pt += numVars*(numVars + 1)/2;
    return (data_per_pt > 1) ?
      (int)std::ceil((Real)coeffs/(Real)data_per_pt) : coeffs;
  }
}



void Approximation::
update(const RealMatrix& samples, const ResponseArray& resp_array, int fn_index)
{
  if (approxRep)
    approxRep->update(samples, resp_array, fn_index);
  else { // not virtual: all derived classes use following definition:
    currentPoints.clear(); // replace currentPoints with incoming samples
    size_t num_points = std::min((size_t)samples.numCols(), resp_array.size());
    for (size_t i=0; i<num_points; ++i)
      add(samples[i], resp_array[i], fn_index, false);
  }
}


void Approximation::
update(const VariablesArray& vars_array, const ResponseArray& resp_array,
       int fn_index)
{
  if (approxRep)
    approxRep->update(vars_array, resp_array, fn_index);
  else { // not virtual: all derived classes use following definition:
    currentPoints.clear(); // replace currentPoints with incoming arrays
    size_t i, num_points = std::min(vars_array.size(), resp_array.size());
    for (i=0; i<num_points; ++i)
      add(vars_array[i], resp_array[i], fn_index, false);
  }
}


void Approximation::
append(const RealMatrix& samples, const ResponseArray& resp_array, int fn_index)
{
  if (approxRep)
    approxRep->append(samples, resp_array, fn_index);
  else { // not virtual: all derived classes use following definition
    popCount = std::min((size_t)samples.numCols(), resp_array.size());
    for (size_t i=0; i<popCount; ++i)
      add(samples[i], resp_array[i], fn_index, false);
  }
}


void Approximation::
append(const VariablesArray& vars_array, const ResponseArray& resp_array,
       int fn_index)
{
  if (approxRep)
    approxRep->append(vars_array, resp_array, fn_index);
  else { // not virtual: all derived classes use following definition
    popCount = std::min(vars_array.size(), resp_array.size());
    for (size_t i=0; i<popCount; ++i)
      add(vars_array[i], resp_array[i], fn_index, false);
  }
}


void Approximation::
add(const Real* sample_c_vars, const Response& response, int fn_index,
    bool anchor_flag)
{
  // create view of first numVars entries within column of sample Matrix
  // --> any discrete {int,real} vars (e.g., from NonDSampling) are ignored.
  RealVector c_vars(Teuchos::View, (Real*)sample_c_vars, numVars);
  add(c_vars, response, fn_index, anchor_flag);
}


void Approximation::
add(const Variables& vars, const Response& response, int fn_index,
    bool anchor_flag)
{
  // Approximation does not know about view mappings; therefore, take the
  // simple approach of matching up vars.cv() or vars.acv() with numVars.
  bool active_vars;
  if (vars.cv() == numVars)
    active_vars = true;
  else if (vars.acv() == numVars)
    active_vars = false;
  else {
    Cerr << "Error: variable size mismatch in Approximation::add()."
	 << std::endl;
    abort_handler(-1);
  }
  const RealVector& c_vars = (active_vars) ? vars.continuous_variables() :
    vars.all_continuous_variables();

  add(c_vars, response, fn_index, anchor_flag);
}


void Approximation::
add(const RealVector& c_vars, const Response& response, int fn_index,
    bool anchor_flag)
{
  // private fn used only by build() and update() functions -> approxRep
  // forward not needed.  Recomputing coefficients, if needed, is managed
  // by the calling functions.

  const ShortArray& asv = response.active_set_request_vector();
  short asv_val = asv[fn_index];
  if (asv_val) {
    const RealVector&         fn_vals     = response.function_values();
    const RealMatrix&         fn_grads    = response.function_gradients();
    const RealSymMatrixArray& fn_hessians = response.function_hessians();
    size_t num_fns = asv.size();

    /*
    if ( vars.cv() != numVars) {// ||
	 //( (asv_val & 2) && fn_grads.numRows()               != numVars) ||
	 //( (asv_val & 4) && (fn_hessians[fn_index].numRows() != numVars ||
	 //		       fn_hessians[fn_index].numCols() != numVars))) {
      Cerr << "Error: variable size mismatch in Approximation::add()."
	   << std::endl;
      abort_handler(-1);
    }
    */
    if ( ( (asv_val & 1) && fn_vals.length()   != num_fns ) ||
	 ( (asv_val & 2) && fn_grads.numCols() != num_fns ) || 
	 ( (asv_val & 4) && fn_hessians.size() != num_fns ) ) {
      Cerr << "Error: function size mismatch in Approximation::add()."
	   << std::endl;
      abort_handler(-1);
    }

    // Since SurrogateDataPoint does not contain an asv, assure the
    // use of empty vectors/matrices if data is not active.
    Real          empty_r = 0.;
    RealSymMatrix empty_hess;
    const Real&   fn_val  = (asv_val & 1) ? fn_vals[fn_index] : empty_r;
    RealVector    fn_grad;
    if (asv_val & 2)
      fn_grad = response.function_gradient_copy(fn_index); // copy->copy
    const RealSymMatrix& fn_hess
      = (asv_val & 4) ? fn_hessians[fn_index] : empty_hess;
    if (anchor_flag)
      add_anchor(c_vars, fn_val, fn_grad, fn_hess);
    else
      add_point(c_vars,  fn_val, fn_grad, fn_hess);
  }
  // else no data point added for this index
}


/* Old 3D graphics capability:
void Approximation::draw_surface()
{
  if (approxRep)
    approxRep->draw_surface();
  else { // draw surface of 2 independent variables

    if (approxLowerBounds.length() != 2 ||
	approxUpperBounds.length() != 2 || numVars != 2)
      return;

    // define number of equally spaced points in each direction
    int i, j, num_intervals = 50, num_axis_pts = num_intervals + 1;
    Real dx = (approxUpperBounds[0] - approxLowerBounds[0])/num_intervals,
         dy = (approxUpperBounds[1] - approxLowerBounds[1])/num_intervals;
    RealMatrix F(num_axis_pts, num_axis_pts);
    RealVector X(num_axis_pts), Y(num_axis_pts);
  
    // fill X & Y with grid of values to be displayed in graphics
    X[0] = approxLowerBounds[0];
    Y[0] = approxLowerBounds[1];
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
        F(j,i)   = get_value(xtemp);
      }
    }
    extern Graphics dakota_graphics; // defined in ParallelLibrary.C
    dakota_graphics.show_data_3d(X, Y, F);
  }
}
*/

} // namespace Dakota
