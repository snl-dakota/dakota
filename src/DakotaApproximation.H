/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
#include "SurrogateData.hpp"

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
		size_t num_vars, short data_order);
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
  /// removes entries from end of SurrogateData::{vars,resp}Data
  /// (last points appended)
  virtual void pop(bool save_data);
  /// returns number of entries to pop from end of
  /// SurrogateData::{vars,resp}Data, based on size of last data set appended
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

  /// return the number of constraints to be enforced via an anchor point
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

  // return approxData
  const Pecos::SurrogateData& approximation_data() const;

  /// populates/replaces anchor point within approxData
  void update(const Variables& vars, const Response& response,
	      int fn_index, bool deep_copy = false);
  /// populates/replaces SurrogateData::{vars,resp}Data
  void update(const RealMatrix& samples, const ResponseArray& resp_array,
	      int fn_index, bool deep_copy = false);
  /// populates/replaces SurrogateData::{vars,resp}Data
  void update(const VariablesArray& vars_array, const ResponseArray& resp_array,
	      int fn_index, bool deep_copy = false);

  /// appends one additional entry to SurrogateData::{vars,resp}Data
  void append(const Variables& vars, const Response& response, int fn_index,
	      bool deep_copy = false);
  /// appends multiple additional entries to SurrogateData::{vars,resp}Data
  void append(const RealMatrix& samples, const ResponseArray& resp_array,
	      int fn_index, bool deep_copy = false);
  /// appends multiple additional entries to SurrogateData::{vars,resp}Data
  void append(const VariablesArray& vars_array, const ResponseArray& resp_array,
	      int fn_index, bool deep_copy = false);

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

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Approximation(BaseConstructor, const String& approx_type, size_t num_vars,
		short data_order);

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
  /** This setting distinguishes derivative data intended for use in
      construction (includes derivatives w.r.t. the build variables) from
      derivative data that may be approximated separately (excludes derivatives
      w.r.t. auxilliary variables).  This setting should also not be inferred
      directly from the responses specification, since we may need gradient
      support for evaluating gradients at a single point (e.g., the center of
      a trust region), but not require gradient evaluations at every point. */
  short buildDataOrder;

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

  /// contains the variables/response data for constructing a single
  /// approximation model (one response function)
  Pecos::SurrogateData approxData;

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
			    short data_order);

  /// extract the relevant RealVector from Variables and invoke add(RealVector&)
  void add(const Variables& vars, const Response& response,
	   int fn_index, bool anchor_flag, bool deep_copy);
  /// create a RealVector view and invoke add(RealVector&)
  void add(const Real* sample_c_vars, const Response& response,
	   int fn_index, bool anchor_flag, bool deep_copy);
  /// shared code among add(Variables&) and add(Real*); adds a new data point
  /// by either appending to SurrogateData::{vars,Resp}Data or assigning to
  /// SurrogateData::anchor{Vars,Resp}, as dictated by anchor_flag.
  /// Uses add_point() and add_anchor().
  void add(const RealVector& sample_c_vars, const Response& response,
	   int fn_index, bool anchor_flag, bool deep_copy);

  /// add a new data point by appending to SurrogateData::{vars,Resp}Data
  void add_point(const RealVector& x, const Real& fn_val,
		 const RealVector& fn_grad, const RealSymMatrix& fn_hess,
		 bool deep_copy);

  /// add a new data point by assigning to SurrogateData::anchor{Vars,Resp}
  void add_anchor(const RealVector& x, const Real& fn_val,
		  const RealVector& fn_grad, const RealSymMatrix& fn_hess,
		  bool deep_copy);

  //
  //- Heading: Data
  //

  /// number of points previously added by append() to be removed by pop()
  size_t popCount;

  /// pointer to the letter (initialized only for the envelope)
  Approximation* approxRep;
  /// number of objects sharing approxRep
  int referenceCount;
};


inline int Approximation::num_variables() const
{ return (approxRep) ? approxRep->numVars : numVars; }


inline const Pecos::SurrogateData& Approximation::approximation_data() const
{ return (approxRep) ? approxRep->approxData : approxData; }


inline void Approximation::
set_bounds(const RealVector& lower, const RealVector& upper)
{
  // enforce deep copies since lower/upper may be active views
  if (approxRep) {
    copy_data(lower, approxRep->approxLowerBounds);
    copy_data(upper, approxRep->approxUpperBounds);
  }
  else {
    copy_data(lower, approxLowerBounds);
    copy_data(upper, approxUpperBounds);
  }
}


inline Approximation* Approximation::approx_rep() const
{ return approxRep; }


// private convenience fn -> approxRep forward not needed.
inline void Approximation::
add_point(const RealVector& x,       const Real& fn_val,
	  const RealVector& fn_grad, const RealSymMatrix& fn_hess,
	  bool deep_copy)
{
  // Map DAKOTA's deep_copy bool into Pecos' copy mode.
  // Note that Pecos::DEFAULT_COPY is not supported through DAKOTA and the
  // SurrogateDataVars/SurrogateDataResp modes are currently synchronized.
  short mode = (deep_copy) ? Pecos::DEEP_COPY : Pecos::SHALLOW_COPY;
  Pecos::SurrogateDataVars sdv(x, mode);
  Pecos::SurrogateDataResp sdr(fn_val, fn_grad, fn_hess, mode);
  approxData.push_back(sdv, sdr);
}


// private convenience fn -> approxRep forward not needed.
inline void Approximation::
add_anchor(const RealVector& x,       const Real& fn_val,
	   const RealVector& fn_grad, const RealSymMatrix& fn_hess,
	   bool deep_copy)
{
  // Map DAKOTA's deep_copy bool into Pecos' copy mode.
  // Note that Pecos::DEFAULT_COPY is not supported through DAKOTA and the
  // SurrogateDataVars/SurrogateDataResp modes are currently synchronized.
  short mode = (deep_copy) ? Pecos::DEEP_COPY : Pecos::SHALLOW_COPY;
  Pecos::SurrogateDataVars sdv(x, mode);
  Pecos::SurrogateDataResp sdr(fn_val, fn_grad, fn_hess, mode);
  approxData.anchor_point(sdv, sdr);
}


inline void Approximation::
update(const Variables& vars, const Response& response, int fn_index,
       bool deep_copy)
{
  if (approxRep)
    approxRep->add(vars, response, fn_index, true, deep_copy);
  else
    add(vars, response, fn_index, true, deep_copy); // replace anchor point
}


inline void Approximation::
append(const Variables& vars, const Response& response, int fn_index,
       bool deep_copy)
{
  if (approxRep)
    approxRep->append(vars, response, fn_index, deep_copy);
  else {
    popCount = 1;
    // append new point to SurrogateData::{vars,resp}Data
    add(vars, response, fn_index, false, deep_copy);
  }
}


/** Clears out any history (e.g., TANA3Approximation use for a
    different response function in NonDReliability). */
inline void Approximation::clear_all()
{
  if (approxRep) // envelope fwd to letter
    approxRep->clear_all();
  else { // not virtual: base class implementation
    if (approxData.anchor())
      approxData.clear_anchor();
    approxData.clear_data();
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
