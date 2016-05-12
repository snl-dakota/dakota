/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Approximation
//- Description:  Abstract base class for approximations
//-               
//- Owner:        Mike Eldred

#ifndef DAKOTA_APPROXIMATION_H
#define DAKOTA_APPROXIMATION_H

#include "dakota_data_util.hpp"
#include "SurrogateData.hpp"
#include "SharedApproxData.hpp"

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
  Approximation(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
                const String& approx_label);
   /// alternate constructor
  Approximation(const SharedApproxData& shared_data);
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
  /// exports the approximation
  virtual void export_model(const String& fn_label = "", 
      const String& export_prefix = "", 
      const unsigned short export_format = NO_MODEL_FORMAT );
  /// rebuilds the approximation incrementally
  virtual void rebuild();
  /// removes entries from end of SurrogateData::{vars,resp}Data
  /// (last points appended, or as specified in args)
  virtual void pop(bool save_data);
  /// restores state prior to previous pop()
  virtual void push();
  /// finalize approximation by applying all remaining trial sets
  virtual void finalize();

  /// store current approximation state for later combination
  virtual void store(size_t index = _NPOS);
  /// restore previous approximation state
  virtual void restore(size_t index = _NPOS);
  /// remove a stored approximation prior to combination
  virtual void remove_stored(size_t index = _NPOS);

  /// combine current approximation with previously stored approximation
  virtual void combine(short corr_type, size_t swap_index);

  /// retrieve the approximate function value for a given parameter vector
  virtual Real value(const Variables& vars);
  /// retrieve the approximate function gradient for a given parameter vector
  virtual const RealVector& gradient(const Variables& vars);
  /// retrieve the approximate function Hessian for a given parameter vector
  virtual const RealSymMatrix& hessian(const Variables& vars);
  /// retrieve the variance of the predicted value for a given parameter vector
  virtual Real prediction_variance(const Variables& vars);
    
  /// retrieve the approximate function value for a given parameter vector
  virtual Real value(const RealVector& c_vars);
  /// retrieve the approximate function gradient for a given parameter vector
  virtual const RealVector& gradient(const RealVector& c_vars);
  /// retrieve the approximate function Hessian for a given parameter vector
  virtual const RealSymMatrix& hessian(const RealVector& c_vars);
  /// retrieve the variance of the predicted value for a given parameter vector
  virtual Real prediction_variance(const RealVector& c_vars);
    

  /// check if diagnostics are available for this approximation type
  virtual bool diagnostics_available();
  /// retrieve a single diagnostic metric for the diagnostic type specified
  virtual Real diagnostic(const String& metric_type);
  /// retrieve diagnostic metrics for the diagnostic types specified, applying 
  // num_folds-cross validation
  virtual RealArray cv_diagnostic(const StringArray& metric_types,
				  unsigned num_folds);
  /// compute and print all requested diagnostics and cross-validation 
  virtual void primary_diagnostics(int fn_index);
  /// compute requested diagnostics for user provided challenge pts
  virtual RealArray challenge_diagnostic(const StringArray& metric_types,
			    const RealMatrix& challenge_points,
                            const RealVector& challenge_responses);
  /// compute and print all requested diagnostics for user provided
  /// challenge pts
  virtual void challenge_diagnostics(int fn_index, 
				     const RealMatrix& challenge_points, 
                                     const RealVector& challenge_responses);
  // TODO: private implementation of cross-validation:
  //  void cross_validate(metrics, folds)

  /// return the coefficient array computed by build()/rebuild()
  virtual RealVector approximation_coefficients(bool normalized) const;
  /// set the coefficient array from external sources, rather than
  /// computing with build()/rebuild()
  virtual void approximation_coefficients(const RealVector& approx_coeffs,
					  bool normalized);

  /// print the coefficient array computed in build()/rebuild()
  virtual void coefficient_labels(std::vector<std::string>& coeff_labels) const;

  /// print the coefficient array computed in build()/rebuild()
  virtual void print_coefficients(std::ostream& s, bool normalized);

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

  //
  //- Heading: Member functions
  //

  /// return the minimum number of points required to build the approximation
  /// type in numVars dimensions. Uses *_coefficients() and num_constraints().
  int min_points(bool constraint_flag) const;

  /// return the recommended number of samples to build the approximation type
  /// in numVars dimensions (default same as min_points)
  int recommended_points(bool constraint_flag) const;

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

  /// add data from the provided samples and response matrices,
  /// assuming continuous variables and function values only
  void add(const RealMatrix& sample_vars, const RealVector& sample_resp);

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
  /// clear popCountStack and SurrogateData::popped{Vars,Resp}Trials
  void clear_popped();

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
		const SharedApproxData& shared_data, 
                const String& approx_label);

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Approximation(NoDBBaseConstructor, const SharedApproxData& shared_data);

  //
  //- Heading: Data
  //

  // approximation type identifier
  //String approxType;

  /// gradient of the approximation returned by gradient()
  RealVector approxGradient;
  /// Hessian of the approximation returned by hessian()
  RealSymMatrix approxHessian;

  /// contains the variables/response data for constructing a single
  /// approximation model (one response function)
  Pecos::SurrogateData approxData;

  /// contains the approximation data that is shared among the response set
  SharedApproxData* sharedDataRep;

  /// label for approximation, if applicable
  String approxLabel;

private:

  //
  //- Heading: Member functions
  //

  /// Used only by the standard envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  Approximation* get_approx(ProblemDescDB& problem_db,
			    const SharedApproxData& shared_data,
                            const String& approx_label);

  /// Used only by the alternate envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  Approximation* get_approx(const SharedApproxData& shared_data);

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


inline const Pecos::SurrogateData& Approximation::approximation_data() const
{ return (approxRep) ? approxRep->approxData : approxData; }


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
    RealVector c_vars(Teuchos::View, const_cast<Real*>(sample_c_vars),
		      sharedDataRep->numVars);
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


inline void Approximation::clear_popped()
{
  if (approxRep) // envelope fwd to letter
    approxRep->clear_popped();
  else { // not virtual: base class implementation
    popCountStack.clear();
    approxData.clear_popped();
  }
}

} // namespace Dakota

#endif
