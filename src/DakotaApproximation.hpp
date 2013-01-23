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

#include "data_util.hpp"
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
  /// (last points appended, or as specified in args)
  virtual void pop(bool save_data);
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

  /// store current approximation for later combination
  virtual void store();
  /// combine current approximation with previously stored approximation
  virtual void combine(short corr_type);

  /// retrieve the approximate function value for a given parameter vector
  virtual Real value(const Variables& vars);
  /// retrieve the approximate function gradient for a given parameter vector
  virtual const RealVector& gradient(const Variables& vars);
  /// retrieve the approximate function Hessian for a given parameter vector
  virtual const RealSymMatrix& hessian(const Variables& vars);
  /// retrieve the variance of the predicted value for a given parameter vector
  virtual Real prediction_variance(const Variables& vars);
  /// retrieve the diagnostic metric for the diagnostic type specified
  virtual Real diagnostic(const String& metric_type);

  /// return the coefficient array computed by build()/rebuild()
  virtual const RealVector& approximation_coefficients() const;
  /// set the coefficient array from external sources, rather than
  /// computing with build()/rebuild()
  virtual void approximation_coefficients(const RealVector& approx_coeffs);

  /// print the coefficient array computed in build()/rebuild()
  virtual void coefficient_labels(std::vector<std::string>& coeff_labels) const;

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
  virtual bool diagnostics_available();

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

  /// return approxData
  const Pecos::SurrogateData& approximation_data() const;

  /// append to SurrogateData::varsData or assign to SurrogateData::anchorVars
  void add(const Pecos::SurrogateDataVars& sdv, bool anchor_flag);
  /// extract the relevant vectors from Variables and invoke
  /// add(RealVector&, IntVector&, RealVector&)
  void add(const Variables& vars, bool anchor_flag, bool deep_copy);
  /// create a RealVector view and invoke add(RealVector&, empty, empty)
  void add(const Real* sample_c_vars, bool anchor_flag, bool deep_copy);
  /// shared code among add(Variables&) and add(Real*); adds a new
  /// data point by either appending to SurrogateData::varsData or
  /// assigning to SurrogateData::anchorVars, as dictated by anchor_flag.
  /// Uses add_point() and add_anchor().
  void add(const RealVector& c_vars, const IntVector& di_vars,
	   const RealVector& dr_vars, bool anchor_flag, bool deep_copy);

  /// append to SurrogateData::respData or assign to SurrogateData::anchorResp
  void add(const Pecos::SurrogateDataResp& sdr, bool anchor_flag);
  /// adds a new data point by either appending to SurrogateData::respData
  /// or assigning to SurrogateData::anchorResp, as dictated by anchor_flag.
  /// Uses add_point() and add_anchor().
  void add(const Response& response, int fn_index, bool anchor_flag,
	   bool deep_copy);

  /// appends to popCountStack (number of entries to pop from end of
  /// SurrogateData::{vars,resp}Data, based on size of last data set appended)
  void pop_count(size_t count);
  // returns popCountStack.back() (number of entries to pop from end of
  // SurrogateData::{vars,resp}Data, based on size of last data set appended)
  //size_t pop_count() const;
  // clear popCountStack
  //void clear_stack();

  /// clear all build data (current and history) to restore original state
  void clear_all();
  /// clear SurrogateData::anchor{Vars,Resp}
  void clear_anchor();
  /// clear SurrogateData::{vars,resp}Data
  void clear_data();
  /// clear popCountStack and SurrogateData::saved{Vars,Resp}Trials
  void clear_saved();

  /// set approximation lower and upper bounds (currently only used by graphics)
  void set_bounds(const RealVector&  c_l_bnds, const RealVector&  c_u_bnds,
		  const IntVector&  di_l_bnds, const IntVector&  di_u_bnds,
		  const RealVector& dr_l_bnds, const RealVector& dr_u_bnds);

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

  /// gradient of the approximation returned by gradient()
  RealVector approxGradient;
  /// Hessian of the approximation returned by hessian()
  RealSymMatrix approxHessian;

  /// contains the variables/response data for constructing a single
  /// approximation model (one response function)
  Pecos::SurrogateData approxData;

  /// approximation continuous lower bounds (used by 3D graphics and
  /// Surfpack KrigingModel)
  RealVector approxCLowerBnds;
  /// approximation continuous upper bounds (used by 3D graphics and
  /// Surfpack KrigingModel)
  RealVector approxCUpperBnds;
  /// approximation continuous lower bounds
  IntVector approxDILowerBnds;
  /// approximation continuous upper bounds
  IntVector approxDIUpperBnds;
  /// approximation continuous lower bounds
  RealVector approxDRLowerBnds;
  /// approximation continuous upper bounds
  RealVector approxDRUpperBnds;

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

  //
  //- Heading: Data
  //

  /// a stack managing the number of points previously added by calls
  /// to append() that can be removed by calls to pop()
  SizetArray popCountStack;

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
set_bounds(const RealVector&  c_l_bnds, const RealVector&  c_u_bnds,
	   const IntVector&  di_l_bnds, const IntVector&  di_u_bnds,
	   const RealVector& dr_l_bnds, const RealVector& dr_u_bnds)
{
  // enforce deep copies since lower/upper may be active views
  if (approxRep) {
    copy_data(c_l_bnds,  approxRep->approxCLowerBnds);
    copy_data(c_u_bnds,  approxRep->approxCUpperBnds);
    copy_data(di_l_bnds, approxRep->approxDILowerBnds);
    copy_data(di_u_bnds, approxRep->approxDIUpperBnds);
    copy_data(dr_l_bnds, approxRep->approxDRLowerBnds);
    copy_data(dr_u_bnds, approxRep->approxDRUpperBnds);
  }
  else {
    copy_data(c_l_bnds,  approxCLowerBnds);
    copy_data(c_u_bnds,  approxCUpperBnds);
    copy_data(di_l_bnds, approxDILowerBnds);
    copy_data(di_u_bnds, approxDIUpperBnds);
    copy_data(dr_l_bnds, approxDRLowerBnds);
    copy_data(dr_u_bnds, approxDRUpperBnds);
  }
}


inline Approximation* Approximation::approx_rep() const
{ return approxRep; }


inline void Approximation::
add(const Pecos::SurrogateDataVars& sdv, bool anchor_flag)
{
  if (approxRep)
    approxRep->add(sdv, anchor_flag);
  else { // not virtual: all derived classes use following definition
    if (anchor_flag) approxData.anchor_variables(sdv);
    else             approxData.push_back(sdv);
  }
}


inline void Approximation::
add(const RealVector& c_vars, const IntVector& di_vars,
    const RealVector& dr_vars, bool anchor_flag, bool deep_copy)
{
  if (approxRep)
    approxRep->add(c_vars, di_vars, dr_vars, anchor_flag, deep_copy);
  else { // not virtual: all derived classes use following definition
    // Map DAKOTA's deep_copy bool into Pecos' copy mode
    // (Pecos::DEFAULT_COPY is not supported through DAKOTA).
    short mode = (deep_copy) ? Pecos::DEEP_COPY : Pecos::SHALLOW_COPY;
    Pecos::SurrogateDataVars sdv(c_vars, di_vars, dr_vars, mode);
    if (anchor_flag) approxData.anchor_variables(sdv);
    else             approxData.push_back(sdv);
  }
}


inline void Approximation::
add(const Real* sample_c_vars, bool anchor_flag, bool deep_copy)
{
  if (approxRep)
    approxRep->add(sample_c_vars, anchor_flag, deep_copy);
  else { // not virtual: all derived classes use following definition
    // create view of numVars entries within column of sample Matrix;
    // for compact mode, any active discrete {int,real} vars are managed
    // as real values (e.g., NonDSampling::update_model_from_sample())
    // and we do not convert them back to {di,dr}_vars here.
    RealVector c_vars(Teuchos::View, const_cast<Real*>(sample_c_vars), numVars);
    IntVector di_vars; RealVector dr_vars; // empty
    add(c_vars, di_vars, dr_vars, anchor_flag, deep_copy);
  }
}


inline void Approximation::
add(const Pecos::SurrogateDataResp& sdr, bool anchor_flag)
{
  if (approxRep)
    approxRep->add(sdr, anchor_flag);
  else { // not virtual: all derived classes use following definition
    if (anchor_flag) approxData.anchor_response(sdr);
    else             approxData.push_back(sdr);
  }
}


/*
inline size_t Approximation::pop_count() const
{
  if (approxRep) // envelope fwd to letter
    return approxRep->pop_count();
  else { // not virtual: base class implementation
    if (popCountStack.empty()) {
      Cerr << "\nError: empty count stack in Approximation::pop_count()."
	   << std::endl;
      abort_handler(-1);
      return 0;
    }
    else
      return popCountStack.back();
  }
}
*/


inline void Approximation::pop_count(size_t count)
{
  if (approxRep) approxRep->popCountStack.push_back(count);
  else           popCountStack.push_back(count);
}


//inline void Approximation::clear_stack()
//{
//  if (approxRep) approxRep->popCountStack.clear();
//  else           popCountStack.clear();
//}


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


inline void Approximation::clear_anchor()
{
  if (approxRep) // envelope fwd to letter
    approxRep->approxData.clear_anchor();
  else // not virtual: base class implementation
    approxData.clear_anchor();
}


inline void Approximation::clear_data()
{
  if (approxRep) // envelope fwd to letter
    approxRep->approxData.clear_data();
  else // not virtual: base class implementation
    approxData.clear_data();
}


inline void Approximation::clear_saved()
{
  if (approxRep) // envelope fwd to letter
    approxRep->clear_saved();
  else { // not virtual: base class implementation
    popCountStack.clear();
    approxData.clear_saved();
  }
}

} // namespace Dakota

#endif
