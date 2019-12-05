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
    const unsigned short export_format = NO_MODEL_FORMAT);
  /// rebuilds the approximation incrementally
  virtual void rebuild();
  /// removes entries from end of SurrogateData::{vars,resp}Data
  /// (last points appended, or as specified in args)
  virtual void pop_coefficients(bool save_data);
  /// restores state prior to previous pop()
  virtual void push_coefficients();
  /// finalize approximation by applying all remaining trial sets
  virtual void finalize_coefficients();

  /// clear current build data in preparation for next build
  virtual void clear_current_active_data();

  /// combine all level approximations into a single aggregate approximation
  virtual void combine_coefficients();
  /// promote combined approximation into active approximation
  virtual void combined_to_active_coefficients(bool clear_combined = true);
  /// prune inactive coefficients following combination and promotion to active
  virtual void clear_inactive_coefficients();

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

  /// Statistics
  virtual Real mean();                            
  virtual Real mean(const RealVector& x);          
  virtual const RealVector& mean_gradient();      
  virtual const RealVector& mean_gradient(const RealVector& x,
					  const SizetArray& dvv);     

  /// return the variance of the expansion, where all active vars are random
  virtual Real variance();
  /// return the variance of the expansion for a given parameter vector,
  /// where a subset of the active variables are random
  virtual Real variance(const RealVector& x);           
  virtual const RealVector& variance_gradient();      
  virtual const RealVector& variance_gradient(const RealVector& x,
					      const SizetArray& dvv);
  /// return the covariance between two response expansions, treating
  /// all variables as random
  virtual Real covariance(Approximation& approx_2);
  /// return the covariance between two response expansions, treating
  /// a subset of the variables as random
  virtual Real covariance(const RealVector& x, Approximation& approx_2);
  /// return the covariance between two combined response expansions,
  /// where all active variables are random
  virtual Real combined_covariance(Approximation& approx_2);
  /// return the covariance between two combined response expansions,
  /// where a subset of the active variables are random
  virtual Real combined_covariance(const Pecos::RealVector& x,
				   Approximation& approx_2);

  virtual void compute_moments(bool full_stats = true,
			       bool combined_stats = false);
  virtual void compute_moments(const RealVector& x, bool full_stats = true,
			       bool combined_stats = false);
  virtual const RealVector& moments() const;
  virtual Real moment(size_t i) const;
  virtual void moment(Real mom, size_t i);

  virtual void clear_component_effects();
  virtual void compute_component_effects();
  virtual void compute_total_effects();
  virtual const RealVector& sobol_indices() const;
  virtual const RealVector& total_sobol_indices() const;
  virtual ULongULongMap sparse_sobol_index_map() const;

  virtual const RealVector& expansion_moments() const;
  virtual const RealVector& numerical_integration_moments() const;

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

  /// link more than once approxData instance for aggregated response data
  /// (PecosApproximation)
  virtual void link_multilevel_surrogate_data();

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

  /* *** Additions for C3 ***
  /// clear current build data in preparation for next build
  virtual void clear_current();
  */
  virtual void expansion_coefficient_flag(bool);
  virtual bool expansion_coefficient_flag() const;    
  virtual void expansion_gradient_flag(bool);
  virtual bool expansion_gradient_flag() const;

  /// clear tracking of computed moments, due to (expansion) change
  /// that invalidates previous results
  virtual void clear_computed_bits();

  //
  //- Heading: Member functions
  //

  /// return the minimum number of points required to build the approximation
  /// type in numVars dimensions. Uses *_coefficients() and num_constraints().
  int min_points(bool constraint_flag) const;

  /// return the recommended number of samples to build the approximation type
  /// in numVars dimensions (default same as min_points)
  int recommended_points(bool constraint_flag) const;

  /// removes entries from end of SurrogateData::{vars,resp}Data
  /// (last points appended, or as specified in args)
  void pop_data(bool save_data);
  /// restores SurrogateData state prior to previous pop()
  void push_data();
  /// finalize SurrogateData by applying all remaining trial sets
  void finalize_data();

  /// return approxData[sharedDataRep->activeDataIndex]
  const Pecos::SurrogateData& surrogate_data() const;
  /// return approxData[sharedDataRep->activeDataIndex]
  Pecos::SurrogateData& surrogate_data();
  /// return approxData[d_index]
  const Pecos::SurrogateData& surrogate_data(size_t d_index) const;

  /// append to SurrogateData::varsData
  void add(const Pecos::SurrogateDataVars& sdv, bool anchor_flag,
	   bool deep_copy, size_t key_index = _NPOS);
  /// extract the relevant vectors from Variables and invoke
  /// add(RealVector&, IntVector&, RealVector&)
  void add(const Variables& vars, bool anchor_flag, bool deep_copy,
	   size_t key_index = _NPOS);
  /// adds a new data point by appending to SurrogateData::varsData
  void add(const RealVector& c_vars, const IntVector& di_vars,
	   const RealVector& dr_vars, bool anchor_flag, bool deep_copy,
	   size_t key_index = _NPOS);
  /// create a RealVector view and invoke add(SurrogateDataVars&)
  void add(const Real* sample_c_vars, bool anchor_flag, bool deep_copy,
	   size_t key_index = _NPOS);

  /// append to SurrogateData::respData
  void add(const Pecos::SurrogateDataResp& sdr, bool anchor_flag,
	   bool deep_copy, size_t key_index = _NPOS);
  /// adds a new data point by appending to SurrogateData::respData
  void add(const Response& response, int fn_index, bool anchor_flag,
	   bool deep_copy, size_t key_index = _NPOS);

  /// add surrogate data from the provided sample and response data,
  /// assuming continuous variables and function values only
  void add_array(const RealMatrix& sample_vars, const RealVector& sample_resp,
		 bool deep_copy = true, size_t key_index = _NPOS);

  /// appends to SurrogateData::popCountStack (number of entries to pop from
  /// end of SurrogateData::{vars,resp}Data, based on size of last data append)
  void pop_count(size_t count, size_t key_index);
  // returns SurrogateData::popCountStack.back() (number of entries to pop from
  // end of SurrogateData::{vars,resp}Data, based on size of last data append)
  //size_t pop_count(size_t key_index) const;

  /// activate an approximation state based on its multi-index key
  void active_model_key(const UShortArray& sd_key);
  /// reset initial state by removing all model keys for an approximation
  void clear_model_keys();
  /// clear SurrogateData::{vars,resp}Data for all approxDataKeys
  void clear_data();
  /// clear active approximation data
  void clear_active_data();
  /// clear inactive approximation data
  void clear_inactive_data();
  /// clear SurrogateData::popped{Vars,Resp}Trials,popCountStack for active key
  void clear_active_popped();
  /// clear SurrogateData::popped{Vars,Resp}Trials,popCountStack for all keys
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
  //- Heading: Member functions
  //

  /// Check number of build points against minimum required
  void check_points(size_t num_build_pts);

  //
  //- Heading: Data
  //

  // approximation type identifier
  //String approxType;

  /// gradient of the approximation returned by gradient()
  RealVector approxGradient;
  /// Hessian of the approximation returned by hessian()
  RealSymMatrix approxHessian;

  /// label for approximation, if applicable
  String approxLabel;

  /// contains the variables/response data for constructing a single
  /// approximation model (one response function).  Typically there is
  /// only one SurrogateData instance per Approximation, although
  /// SurrogateModels in AGGREGATED_MODELS mode require two instances.
  std::vector<Pecos::SurrogateData> approxData;

  /// contains the approximation data that is shared among the response set
  SharedApproxData* sharedDataRep;

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

  /// pointer to the letter (initialized only for the envelope)
  Approximation* approxRep;
  /// number of objects sharing approxRep
  int referenceCount;
};


inline const Pecos::SurrogateData& Approximation::surrogate_data() const
{
  if (approxRep)
    return approxRep->surrogate_data();
  else
    return approxData[sharedDataRep->activeDataIndex];
}


inline Pecos::SurrogateData& Approximation::surrogate_data()
{
  if (approxRep)
    return approxRep->surrogate_data();
  else
    return approxData[sharedDataRep->activeDataIndex];
}


inline const Pecos::SurrogateData& Approximation::
surrogate_data(size_t d_index) const
{
  if (approxRep)
    return approxRep->surrogate_data(d_index);
  else if (d_index == _NPOS)
    return approxData[sharedDataRep->activeDataIndex]; // defaults to front()
  else {
    if (d_index >= approxData.size()) {
      Cerr << "Error: index out of range in Approximation::surrogate_data()."
	   << std::endl;
      abort_handler(APPROX_ERROR);
    }
    return approxData[d_index];
  }
}


inline void Approximation::
add(const Pecos::SurrogateDataVars& sdv, bool anchor_flag, bool deep_copy,
    size_t key_index)
{
  if (approxRep)
    approxRep->add(sdv, anchor_flag, deep_copy, key_index);
  else { // not virtual: all derived classes use following definition
    size_t data_index = sharedDataRep->activeDataIndex;
    Pecos::SurrogateData& approx_data = approxData[data_index];
    const UShort2DArray&  data_keys = sharedDataRep->approxDataKeys[data_index];
    if (key_index == _NPOS) key_index = 0; // make front() the default
    if (key_index >= data_keys.size()) {
      Cerr << "Error: index out of range in Approximation::add()" << std::endl;
      abort_handler(APPROX_ERROR);
    }

    approx_data.active_key(data_keys[key_index]);// no-op if key already active
    if (deep_copy) {
      if (anchor_flag) approx_data.anchor_variables(sdv.copy());
      else             approx_data.push_back(sdv.copy());
    }
    else { // incoming sdv is already a shallow copy
      if (anchor_flag) approx_data.anchor_variables(sdv);
      else             approx_data.push_back(sdv);
    }

    /*
    size_t d, num_d = approxData.size();
    if (d_index >= num_d) { // append new SurrogateData instances
      const UShortArray& key = approxData.back().active_key(); // ctor pushes 1
      for (d=num_d; d<=d_index; ++d)
	approxData.push_back(Pecos::SurrogateData(key));
    }
    */
  }
}


inline void Approximation::
add(const RealVector& c_vars, const IntVector& di_vars,
    const RealVector& dr_vars, bool anchor_flag, bool deep_copy,
    size_t key_index)
{
  if (approxRep)
    approxRep->add(c_vars, di_vars, dr_vars, anchor_flag, deep_copy, key_index);
  else { // not virtual: all derived classes use following definition
    // deep_copy requests are applied downstream in add(SurrogateDataVars)
    Pecos::SurrogateDataVars sdv(c_vars, di_vars, dr_vars, Pecos::SHALLOW_COPY);
    add(sdv, anchor_flag, deep_copy, key_index);// deep copy applied here
  }
}


inline void Approximation::
add(const Real* sample_c_vars, bool anchor_flag, bool deep_copy,
    size_t key_index)
{
  if (approxRep)
    approxRep->add(sample_c_vars, anchor_flag, deep_copy, key_index);
  else { // not virtual: all derived classes use following definition
    // create view of numVars entries within column of sample Matrix;
    // for compact mode, any active discrete {int,real} vars are managed
    // as real values (e.g., NonDSampling::update_model_from_sample())
    // and we do not convert them back to {di,dr}_vars here.
    RealVector c_vars(Teuchos::View, const_cast<Real*>(sample_c_vars),
		      sharedDataRep->numVars);
    // deep_copy requests are applied downstream in add(SurrogateDataVars)
    Pecos::SurrogateDataVars sdv(c_vars, Pecos::SHALLOW_COPY);
    add(sdv, anchor_flag, deep_copy, key_index); // deep copy applied here
  }
}


inline void Approximation::
add(const Pecos::SurrogateDataResp& sdr, bool anchor_flag, bool deep_copy,
    size_t key_index)
{
  if (approxRep)
    approxRep->add(sdr, anchor_flag, deep_copy, key_index);
  else { // not virtual: all derived classes use following definition
    size_t data_index = sharedDataRep->activeDataIndex;
    Pecos::SurrogateData& approx_data = approxData[data_index];
    const UShort2DArray&  data_keys = sharedDataRep->approxDataKeys[data_index];
    if (key_index == _NPOS) key_index = 0; // make front() the default
    if (key_index >= data_keys.size()) {
      Cerr << "Error: index out of range in Approximation::add()" << std::endl;
      abort_handler(APPROX_ERROR);
    }

    approx_data.active_key(data_keys[key_index]);// no-op if key already active
    if (deep_copy) {
      if (anchor_flag) approx_data.anchor_response(sdr.copy());
      else             approx_data.push_back(sdr.copy());
    }
    else { // incoming sdr is already a shallow copy
      if (anchor_flag) approx_data.anchor_response(sdr);
      else             approx_data.push_back(sdr);
    }

    /*
    size_t d, num_d = approxData.size();
    if (d_index >= num_d) { // append new SurrogateData instances
      const UShortArray& key = approxData.back().active_key(); // ctor pushes 1
      for (size_t d=num_d; d<=d_index; ++d)
	approxData.push_back(Pecos::SurrogateData(key));
    }
    */
  }
}


/*
inline size_t Approximation::pop_count(size_t key_index) const
{
  if (approxRep) return approxRep->pop_count(key_index);
  else {
    size_t data_index = sharedDataRep->activeDataIndex;
    Pecos::SurrogateData& approx_data = approxData[data_index];
    const UShort2DArray&  data_keys = sharedDataRep->approxDataKeys[data_index];
    approx_data.active_key(data_keys[key_index]);// no-op if key already active
    return approx_data.pop_count();
  }
}
*/


inline void Approximation::pop_count(size_t count, size_t key_index)
{
  if (approxRep) approxRep->pop_count(count, key_index);
  else {
    size_t data_index = sharedDataRep->activeDataIndex;
    Pecos::SurrogateData& approx_data = approxData[data_index];
    const UShort2DArray&  data_keys = sharedDataRep->approxDataKeys[data_index];
    approx_data.active_key(data_keys[key_index]);// no-op if key already active
    approx_data.pop_count(count);
  }
}


inline void Approximation::active_model_key(const UShortArray& sd_key)
{
  if (approxRep) approxRep->active_model_key(sd_key);
  else {
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].active_key(sd_key);
  }
}


inline void Approximation::clear_model_keys()
{
  if (approxRep) approxRep->clear_model_keys();
  else {
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].clear_all(false); // don't re-initialize
  }
}


/** Clear current but preserve hisory for active key (virtual function
    redefined by {TANA3,QMEA}Approximation to demote current while
    preserving previous points). */
inline void Approximation::clear_current_active_data()
{
  if (approxRep) // envelope fwd to letter
    approxRep->clear_current_active_data();
  else // default implementation
    clear_active_data();
}
    

/** Clears out current + history for each tracked key (not virtual). */
inline void Approximation::clear_data()
{
  if (approxRep) approxRep->clear_data();
  else {
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].clear_data(); // re-initializes by default
  }
}


inline void Approximation::clear_active_data()
{
  if (approxRep) approxRep->clear_active_data();
  else {
    const UShort3DArray& keys = sharedDataRep->approxDataKeys;
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].clear_active_data(keys[d]);
  }
}


inline void Approximation::clear_inactive_data()
{
  if (approxRep) approxRep->clear_inactive_data();
  else {
    // This is used after combination, so can ignore approxDataKeys
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].clear_inactive_data();
  }
}


inline void Approximation::clear_active_popped()
{
  if (approxRep) approxRep->clear_active_popped();
  else {
    const UShort3DArray& keys = sharedDataRep->approxDataKeys;
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].clear_active_popped(keys[d]);
  }
}


inline void Approximation::clear_popped()
{
  if (approxRep) approxRep->clear_popped();
  else {
    size_t d, num_d = approxData.size();
    for (d=0; d<num_d; ++d)
      approxData[d].clear_popped();
  }
}


inline void Approximation::check_points(size_t num_build_pts)
{
  int min_samp = min_points(true); // account for anchor point & buildDataOrder
  if (num_build_pts < min_samp) {
    Cerr << "\nError: not enough samples to build approximation.  Construction "
	 << "of this approximation\n       requires at least " << min_samp
	 << " samples for " << sharedDataRep->numVars << " variables.  Only "
	 << num_build_pts << " samples were provided." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


inline Approximation* Approximation::approx_rep() const
{ return approxRep; }

} // namespace Dakota

#endif
