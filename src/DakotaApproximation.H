/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Approximation
//- Description:  Abstract base class for approximations
//-               
//- Owner:        Mike Eldred

#ifndef DAKOTA_APPROXIMATION_H
#define DAKOTA_APPROXIMATION_H

#include "data_util.h"
#include "SurrogateDataPoint.hpp"

namespace Dakota {

class ProblemDescDB;
class Variables;
class Response;


/// Base class for the approximation class hierarchy.

/** The Approximation class is the base class for the response data
    fit approximation class hierarchy in DAKOTA.  One instance of an
    Approximation must be created for each function to be approximated
    (a vector of Approximations is contained in
    ApproximationInterface).  For memory efficiency and enhanced
    polymorphism, the approximation hierarchy employs the
    "letter/envelope idiom" (see Coplien "Advanced C++", p. 133), for
    which the base class (Approximation) serves as the envelope and
    one of the derived classes (selected in
    Approximation::get_approx()) serves as the letter. */

class Approximation
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Approximation();
  /// standard constructor for envelope
  Approximation(ProblemDescDB& problem_db, size_t num_vars);
   /// alternate constructor
  Approximation(const String& approx_type, const UShortArray& approx_order,
		size_t num_vars, unsigned short data_order);
  /// copy constructor
  Approximation(const Approximation& approx);

  /// destructor
  virtual ~Approximation();

  /// assignment operator
  Approximation operator=(const Approximation& approx);

  //
  //- Heading: Virtual functions
  //

  /// builds the approximation from scratch
  virtual void build();
  /// rebuilds the approximation incrementally
  virtual void rebuild();
  /// removes entries from end of currentPoints (last points appended)
  virtual void pop(bool save_sdp_set);
  /// returns number of entries to pop from end of currentPoints,
  /// based on size of last data set appended
  virtual size_t pop_count();
  /// restores state prior to previous append()
  virtual void restore();
  /// queries availability of restoration for trial set
  virtual bool restore_available();
  /// return index of trial set within restorable bookkeeping sets
  virtual size_t restoration_index();
  /// finalize approximation by applying all remaining trial sets
  virtual void finalize();
  /// return index of i-th trailing trial set within restorable bookkeeping sets
  virtual size_t finalization_index(size_t i);

  /// retrieve the approximate function value for a given parameter vector
  virtual const Real& get_value(const RealVector& x);
  /// retrieve the approximate function gradient for a given parameter vector
  virtual const RealVector& get_gradient(const RealVector& x);
  /// retrieve the approximate function Hessian for a given parameter vector
  virtual const RealSymMatrix& get_hessian(const RealVector& x);
  /// retrieve the variance of the predicted value for a given parameter vector
  virtual const Real& get_prediction_variance(const RealVector& x);
  /// retrieve the diagnostic metric for the diagnostic type specified
  virtual const Real& get_diagnostic(const String& metric_type);

  /// return the coefficient array computed by build()/rebuild()
  virtual const RealVector& approximation_coefficients() const;
  /// set the coefficient array from external sources, rather than
  /// computing with build()/rebuild()
  virtual void approximation_coefficients(const RealVector& approx_coeffs);

  /// print the coefficient array computed in build()/rebuild()
  virtual void print_coefficients(std::ostream& s) const;

  /// return the minimum number of samples (unknowns) required to
  /// build the derived class approximation type in numVars dimensions
  virtual int min_coefficients() const;

  /// return the recommended number of samples (unknowns) required to
  /// build the derived class approximation type in numVars dimensions
  virtual int recommended_coefficients() const;

  /// return the number of constraints to be enforced via anchorPoint
  virtual int num_constraints() const;

  /// clear current build data in preparation for next build
  virtual void clear_current();

  /// check if diagnostics are available for this approximation type
  virtual const bool diagnostics_available();

  //
  //- Heading: Member functions
  //

  /// return the minimum number of points required to build the approximation
  /// type in numVars dimensions. Uses *_coefficients() and num_constraints().
  int min_points(bool constraint_flag) const;

  /// return the recommended number of samples to build the approximation type
  /// in numVars dimensions (default same as min_points)
  int recommended_points(bool constraint_flag) const;

  /// return the number of variables used in the approximation
  int num_variables() const;

  /// return currentPoints
  const SDPList& current_points() const;
  /// return anchorPoint
  const Pecos::SurrogateDataPoint& anchor_point() const;

  /// populates/replaces anchorPoint
  void update(const Variables& vars, const Response& response,
	      int fn_index);
  /// populates/replaces anchorPoint
  void update(const RealVector& c_vars, const Real& fn_val,
	      const RealVector& fn_grad, const RealSymMatrix& fn_hess);
  /// populates/replaces currentPoints
  void update(const RealMatrix& samples, const ResponseArray& resp_array,
	      int fn_index);
  /// populates/replaces currentPoints
  void update(const VariablesArray& vars_array, const ResponseArray& resp_array,
	      int fn_index);

  /// appends one additional entry to currentPoints
  void append(const Variables& vars, const Response& response, int fn_index);
  /// appends one additional entry to currentPoints
  void append(const RealVector& c_vars, const Real& fn_val,
	      const RealVector& fn_grad, const RealSymMatrix& fn_hess);
  /// appends multiple additional entries to currentPoints
  void append(const RealMatrix& samples, const ResponseArray& resp_array,
	      int fn_index);
  /// appends multiple additional entries to currentPoints
  void append(const VariablesArray& vars_array, const ResponseArray& resp_array,
	      int fn_index);

  /// queries the status of anchorPoint
  bool anchor() const;

  /// clear all build data (current and history) to restore original state
  void clear_all();

  /// set approximation lower and upper bounds (currently only used by graphics)
  void set_bounds(const RealVector& lower, const RealVector& upper);

  // render the approximate surface using the 3D graphics (2 variables only)
  //void draw_surface();

  /// returns approxRep for access to derived class member functions
  /// that are not mapped to the top Approximation level
  Approximation* approx_rep() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Approximation(BaseConstructor, const ProblemDescDB& problem_db,
		size_t num_vars);

  //
  //- Heading: Data
  //

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;

  /// number of variables in the approximation
  int numVars;

  /// approximation type identifier
  String approxType;
  /// order of the data used for surrogate construction, in ActiveSet
  /// request vector 3-bit format.
  /** This setting should not be inferred from the responses specification,
      since we may need gradient support for evaluating gradients at a
      single point (e.g., the center of a trust region), but not require
      gradient evaluations at every point. */
  unsigned short dataOrder;

  /// value of the approximation returned by get_value()
  Real approxValue;
  /// gradient of the approximation returned by get_gradient()
  RealVector approxGradient;
  /// Hessian of the approximation returned by get_hessian()
  RealSymMatrix approxHessian;
  /// value of the approximation returned by get_prediction_variance()
  Real approxVariance;
  /// value of the diagnostic returned by get_diagnostic()
  Real approxDiagnostic;

  /// list of samples used to build the approximation.  These sample points
  /// are fit approximately (e.g., using least squares regression).
  SDPList currentPoints;

  /// a special sample (often at the center of the approximation region)
  /// for which exact matching is enforced (e.g., using equality-constrained
  /// least squares regression).
  Pecos::SurrogateDataPoint anchorPoint;

  /// approximation lower bounds (used by 3D graphics and Surfpack KrigingModel)
  RealVector approxLowerBounds;
  /// approximation upper bounds (used by 3D graphics and Surfpack KrigingModel)
  RealVector approxUpperBounds;


private:

  //
  //- Heading: Member functions
  //

  /// Used only by the standard envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  Approximation* get_approx(ProblemDescDB& problem_db, size_t num_vars);

  /// Used only by the alternate envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  Approximation* get_approx(const String& approx_type,
			    const UShortArray& approx_order, size_t num_vars,
			    unsigned short data_order);

  /// extract the relevant RealVector from Variables and invoke add(RealVector&)
  void add(const Variables& vars, const Response& response,
	   int fn_index, bool anchor_flag);
  /// create a RealVector view and invoke add(RealVector&)
  void add(const Real* sample_c_vars, const Response& response,
	   int fn_index, bool anchor_flag);
  /// shared code among add(Variables&) and add(Real*); adds a new data point
  /// by either appending to currentPoints or assigning to anchorPoint, as
  /// dictated by anchor_flag.  Uses add_point() and add_anchor().
  void add(const RealVector& sample_c_vars, const Response& response,
	   int fn_index, bool anchor_flag);

  /// add a new data point by appending to currentPoints
  void add_point(const RealVector& x, const Real& fn_val,
		 const RealVector& fn_grad,
		 const RealSymMatrix& fn_hess);

  /// add a new data point by assigning to anchorPoint
  void add_anchor(const RealVector& x, const Real& fn_val,
		  const RealVector& fn_grad,
		  const RealSymMatrix& fn_hess);

  //
  //- Heading: Data
  //

  /// bookkeeping for saving data popped from currentPoints (in pop())
  /// that is later restored (in restore() and finalize())
  SDP2DArray savedSDPSets;
  /// number of points previously added by append() to be removed by pop()
  size_t popCount;

  /// pointer to the letter (initialized only for the envelope)
  Approximation* approxRep;
  /// number of objects sharing approxRep
  int referenceCount;
};


inline bool Approximation::anchor() const
{
  return (approxRep) ? !approxRep->anchorPoint.is_null() : 
                       !anchorPoint.is_null();
}


inline int Approximation::num_variables() const
{ return (approxRep) ? approxRep->numVars : numVars; }


inline const SDPList& Approximation::current_points() const
{ return (approxRep) ? approxRep->currentPoints : currentPoints; }


inline const Pecos::SurrogateDataPoint& Approximation::anchor_point() const
{ return (approxRep) ? approxRep->anchorPoint : anchorPoint; }


inline void Approximation::
set_bounds(const RealVector& lower, const RealVector& upper)
{
  if (approxRep) {
    // lower/upper may be views
    copy_data(lower, approxRep->approxLowerBounds);
    copy_data(upper, approxRep->approxUpperBounds);
  }
  else {
    // lower/upper may be views
    copy_data(lower, approxLowerBounds);
    copy_data(upper, approxUpperBounds);
  }
}


inline Approximation* Approximation::approx_rep() const
{ return approxRep; }


// private convenience fn -> approxRep forward not needed.
inline void Approximation::
add_point(const RealVector& x,      const Real& fn_val,
	  const RealVector& fn_grad, const RealSymMatrix& fn_hess)
{
  Pecos::SurrogateDataPoint sdp(x, fn_val, fn_grad, fn_hess);
  currentPoints.push_back(sdp);
}


// private convenience fn -> approxRep forward not needed.
inline void Approximation::
add_anchor(const RealVector& x,       const Real& fn_val,
	   const RealVector& fn_grad, const RealSymMatrix& fn_hess)
{ anchorPoint = Pecos::SurrogateDataPoint(x, fn_val, fn_grad, fn_hess); }


inline void Approximation::
update(const Variables& vars, const Response& response, int fn_index)
{
  if (approxRep)
    approxRep->add(vars, response, fn_index, true);
  else
    add(vars, response, fn_index, true); // replace anchorPoint
}


inline void Approximation::
update(const RealVector& c_vars,  const Real& fn_val,
       const RealVector& fn_grad, const RealSymMatrix& fn_hess)
{
  if (approxRep)
    approxRep->add_anchor(c_vars, fn_val, fn_grad, fn_hess);
  else
    add_anchor(c_vars, fn_val, fn_grad, fn_hess); // replace anchorPoint
}


inline void Approximation::
append(const Variables& vars, const Response& response, int fn_index)
{
  if (approxRep)
    approxRep->add(vars, response, fn_index, false);
  else {
    popCount = 1;
    add(vars, response, fn_index, false); // append new point to currentPoints
  }
}


inline void Approximation::
append(const RealVector& c_vars,  const Real& fn_val,
       const RealVector& fn_grad, const RealSymMatrix& fn_hess)
{
  if (approxRep)
    approxRep->add_point(c_vars, fn_val, fn_grad, fn_hess);
  else {
    popCount = 1;
    add_point(c_vars, fn_val, fn_grad, fn_hess); // append new pt to currentPts
  }
}


/** Clears out any history (e.g., TANA3Approximation use for a
    different response function in NonDReliability). */
inline void Approximation::clear_all()
{
  if (approxRep) // envelope fwd to letter
    approxRep->clear_all();
  else { // not virtual: base class implementation
    if (!anchorPoint.is_null())
      anchorPoint = Pecos::SurrogateDataPoint();
    currentPoints.clear();
  }
}


/** Redefined by TANA3Approximation to clear current data but preserve
    history. */
inline void Approximation::clear_current()
{
  if (approxRep) // envelope fwd to letter
    approxRep->clear_current();
  else // default implementation
    clear_all();
}

} // namespace Dakota

#endif
