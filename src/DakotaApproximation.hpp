/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  /// activate an approximation state based on its multi-index key
  virtual void active_model_key(const Pecos::ActiveKey& sd_key);
  /// reset initial state by removing all model keys for an approximation
  virtual void clear_model_keys();

  /// builds the approximation from scratch
  virtual void build();

  /// overloaded build to support field-based approximations; builds from scratch
  virtual void build(int num_resp);

  /// exports the approximation; if export_format > NO_MODEL_FORMAT,
  /// uses all 3 parameters, otherwise extracts these from the
  /// Approximation's sharedDataRep to build a filename
  virtual void export_model(const StringArray& var_labels = StringArray(),
			    const String& fn_label = "",
			    const String& export_prefix = "",
			    const unsigned short export_format = NO_MODEL_FORMAT);
  /// approximation export that generates labels from the passed
  /// Variables, since only the derived classes know how the variables
  /// are ordered w.r.t. the surrogate build; if export_format >
  /// NO_MODEL_FORMAT, uses all 3 parameters, otherwise extracts these
  /// from the Approximation's sharedDataRep to build a filename
  virtual void export_model(const Variables& vars,
			    const String& fn_label = "",
			    const String& export_prefix = "",
			    const unsigned short export_format = NO_MODEL_FORMAT);

  /// rebuilds the approximation incrementally
  virtual void rebuild();

  /// replace the response data 
  virtual void replace(const IntResponsePair& response_pr, size_t fn_index);
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
  /// retrieve the approximate function values for a given parameter vector
  virtual RealVector values(const Variables& vars);
  /// retrieve the approximate function gradient for a given parameter vector
  virtual const RealVector& gradient(const Variables& vars);
  /// retrieve the approximate function Hessian for a given parameter vector
  virtual const RealSymMatrix& hessian(const Variables& vars);
  /// retrieve the variance of the predicted value for a given parameter vector
  virtual Real prediction_variance(const Variables& vars);
    
  /// retrieve the approximate function value for a given parameter vector
  virtual Real value(const RealVector& c_vars);
  /// retrieve the approximate function value for a given parameter vector
  virtual RealVector values(const RealVector& c_vars);
  /// retrieve the approximate function gradient for a given parameter vector
  virtual const RealVector& gradient(const RealVector& c_vars);
  /// retrieve the approximate function Hessian for a given parameter vector
  virtual const RealSymMatrix& hessian(const RealVector& c_vars);
  /// retrieve the variance of the predicted value for a given parameter vector
  virtual Real prediction_variance(const RealVector& c_vars);

  /// return the mean of the expansion, where all active vars are random
  virtual Real mean();
  /// return the mean of the expansion for a given parameter vector,
  /// where a subset of the active variables are random
  virtual Real mean(const RealVector& x);
  /// return the mean of the combined expansion, where all active vars
  /// are random
  virtual Real combined_mean();
  /// return the mean of the combined expansion for a given parameter vector,
  /// where a subset of the active variables are random
  virtual Real combined_mean(const RealVector& x);
  /// return the gradient of the expansion mean
  virtual const RealVector& mean_gradient();
  /// return the gradient of the expansion mean
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
  virtual Real combined_covariance(const RealVector& x,
				   Approximation& approx_2);

  virtual void compute_moments(bool full_stats = true,
			       bool combined_stats = false);
  virtual void compute_moments(const RealVector& x, bool full_stats = true,
			       bool combined_stats = false);

  virtual const RealVector& moments() const;
  virtual const RealVector& expansion_moments() const;
  virtual const RealVector& numerical_integration_moments() const;
  virtual const RealVector& combined_moments() const;

  virtual Real moment(size_t i) const;
  virtual void moment(Real mom, size_t i);
  virtual Real combined_moment(size_t i) const;
  virtual void combined_moment(Real mom, size_t i);

  virtual void clear_component_effects();
  virtual void compute_component_effects();
  virtual void compute_total_effects();
  virtual const RealVector& sobol_indices() const;
  virtual const RealVector& total_sobol_indices() const;
  virtual ULongULongMap sparse_sobol_index_map() const;

  /// check if resolution advancement (e.g., order, rank) is available
  /// for this approximation instance
  virtual bool advancement_available();

  /// check if diagnostics are available for this approximation type
  virtual bool diagnostics_available();
  /// retrieve a single diagnostic metric for the diagnostic type specified
  virtual Real diagnostic(const String& metric_type);
  /// retrieve diagnostic metrics for the diagnostic types specified, applying 
  // num_folds-cross validation
  virtual RealArray cv_diagnostic(const StringArray& metric_types,
				  unsigned num_folds);
  /// compute and print all requested diagnostics and cross-validation 
  virtual void primary_diagnostics(size_t fn_index);
  /// compute requested diagnostics for user provided challenge pts
  virtual RealArray challenge_diagnostic(const StringArray& metric_types,
			    const RealMatrix& challenge_points,
                            const RealVector& challenge_responses);
  /// compute and print all requested diagnostics for user provided
  /// challenge pts
  virtual void challenge_diagnostics(size_t fn_index, 
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

  // link more than once approxData instance for aggregated response data
  // (PecosApproximation)
  //virtual void link_multilevel_surrogate_data();

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

  /// return the number of approximation components (1 for scalars)
  virtual size_t num_components() const;

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

  /// if needed, map passed all variable labels to approximation's labels
  virtual void map_variable_labels(const Variables& dfsm_vars);

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

  /// return approxData
  const Pecos::SurrogateData& surrogate_data() const;
  /// return approxData
  Pecos::SurrogateData& surrogate_data();

  /// create SurrogateData{Vars,Resp} and append to SurrogateData::
  /// {varsData,respData,dataIdentifiers}
  void add(const Variables& vars, bool v_copy, const Response& response,
	   size_t fn_index, bool r_copy, bool anchor_flag, int eval_id,
	   size_t key_index = _NPOS);
  /// create SurrogateData{Vars,Resp} and append to SurrogateData::
  /// {varsData,respData,dataIdentifiers}
  void add(const Real* c_vars, bool v_copy, const Response& response,
	   size_t fn_index, bool r_copy, bool anchor_flag, int eval_id,
	   size_t key_index = _NPOS);
  /// create a SurrogateDataResp and append to SurrogateData::
  /// {varsData,respData,dataIdentifiers}
  void add(const Pecos::SurrogateDataVars& sdv, bool v_copy,
	   const Response& response, size_t fn_index, bool r_copy,
	   bool anchor_flag, int eval_id, size_t key_index = _NPOS);
  /// append to SurrogateData::{varsData,respData,dataIdentifiers}
  void add(const Pecos::SurrogateDataVars& sdv, bool v_copy,
	   const Pecos::SurrogateDataResp& sdr, bool r_copy, bool anchor_flag,
	   int eval_id, size_t key_index = _NPOS);

  /// add surrogate data from the provided sample and response data,
  /// assuming continuous variables and function values only
  void add_array(const RealMatrix& sample_vars, bool v_copy,
		 const RealVector& sample_resp, bool r_copy,
		 size_t key_index = _NPOS);

  /// appends to SurrogateData::popCountStack (number of entries to pop from
  /// end of SurrogateData::{vars,resp}Data, based on size of last data append)
  void pop_count(size_t count, size_t key_index);
  // returns SurrogateData::popCountStack.back() (number of entries to pop from
  // end of SurrogateData::{vars,resp}Data, based on size of last data append)
  //size_t pop_count(size_t key_index) const;

  /// clear SurrogateData::{vars,resp}Data for activeKey + embedded keys
  void clear_data();
  /// clear active approximation data
  void clear_active_data();
  /// clear inactive approximation data
  void clear_inactive_data();
  /// clear SurrogateData::popped{Vars,Resp}Trials,popCountStack for activeKey
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
  std::shared_ptr<Approximation> approx_rep() const;

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

  /// Used only by the standard envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  std::shared_ptr<Approximation>
  get_approx(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
	     const String& approx_label);

  /// Used only by the alternate envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  std::shared_ptr<Approximation>
  get_approx(const SharedApproxData& shared_data);

  /// create a SurrogateDataVars instance from a Real*
  Pecos::SurrogateDataVars variables_to_sdv(const Real* sample_c_vars);
  /// create a SurrogateDataVars instance by extracting data from a
  /// Variables object
  Pecos::SurrogateDataVars variables_to_sdv(const Variables& vars);
  /// create a SurrogateDataResp instance by extracting data for a
  /// particular QoI from a Response object
  Pecos::SurrogateDataResp response_to_sdr(const Response& response,
					   size_t fn_index);

  /// tracks a new data point by appending to SurrogateData::{vars,Resp}Data
  void add(const Pecos::SurrogateDataVars& sdv, bool v_copy,
	   const Pecos::SurrogateDataResp& sdr, bool r_copy, bool anchor_flag);
  /// tracks a new data point by appending to SurrogateData::dataIdentifiers
  void add(int eval_id);

  /// Check number of build points against minimum required
  void check_points(size_t num_build_pts);

  /// extract and assign i-th embedded active key
  void assign_key_index(size_t key_index);

  //
  //- Heading: Data
  //

  // approximation type identifier
  //String approxType;

  /// contains the variables/response data for constructing a single
  /// approximation model (one response function).  There is only one
  /// SurrogateData instance per Approximation, although it may contain
  /// keys for different model forms/resolutions and aggregations (e.g.,
  /// discrepancies) among forms/resolutions.
  Pecos::SurrogateData approxData;

  /// gradient of the approximation returned by gradient()
  RealVector approxGradient;
  /// Hessian of the approximation returned by hessian()
  RealSymMatrix approxHessian;

  /// label for approximation, if applicable
  String approxLabel;

  // BMA: left this as pointer to rep even though could be to envelope
  /// contains the approximation data that is shared among the response set
  std::shared_ptr<SharedApproxData> sharedDataRep;

  /// pointer to the letter (initialized only for the envelope)
  std::shared_ptr<Approximation> approxRep;

private:

  //
  //- Heading: Member functions
  //

  //
  //- Heading: Data
  //
};


inline const Pecos::SurrogateData& Approximation::surrogate_data() const
{
  if (approxRep) return approxRep->surrogate_data();
  else           return approxData;
}


inline Pecos::SurrogateData& Approximation::surrogate_data()
{
  if (approxRep) return approxRep->surrogate_data();
  else           return approxData;
}


inline void Approximation::assign_key_index(size_t key_index)
{
  // extract a particular raw key if activeKey is aggregated
  const Pecos::ActiveKey& key = sharedDataRep->activeKey;
  if (key.aggregated() && key_index != _NPOS) {
    Pecos::ActiveKey embedded_key; key.extract_key(key_index, embedded_key);
    approxData.active_key(embedded_key);
  }
  else
    approxData.active_key(key); // no-op if key already active
}


inline void Approximation::
add(const Pecos::SurrogateDataVars& sdv, bool v_copy,
    const Pecos::SurrogateDataResp& sdr, bool r_copy, bool anchor_flag)
{
  Pecos::SurrogateDataVars push_sdv = (v_copy) ? sdv.copy() : sdv;
  Pecos::SurrogateDataResp push_sdr = (r_copy) ? sdr.copy() : sdr;
  if (anchor_flag) approxData.anchor_point(push_sdv, push_sdr);
  else             approxData.push_back(push_sdv, push_sdr);
}


inline void Approximation::
add(const Variables& vars, bool v_copy, const Response& response,
    size_t fn_index, bool r_copy, bool anchor_flag, int eval_id,
    size_t key_index)
{
  if (approxRep)
    approxRep->add(vars, v_copy, response, fn_index, r_copy, anchor_flag,
		   eval_id, key_index);
  else { // not virtual: all derived classes use following definition
    assign_key_index(key_index);
    Pecos::SurrogateDataVars sdv = variables_to_sdv(vars);             //shallow
    Pecos::SurrogateDataResp sdr = response_to_sdr(response, fn_index);//shallow
    add(sdv, v_copy, sdr, r_copy, anchor_flag); // deep copies managed here
    add(eval_id);
  }
}


inline void Approximation::
add(const Real* c_vars, bool v_copy, const Response& response, size_t fn_index,
    bool r_copy, bool anchor_flag, int eval_id, size_t key_index)
{
  if (approxRep)
    approxRep->add(c_vars, v_copy, response, fn_index, r_copy, anchor_flag,
		   eval_id, key_index);
  else { // not virtual: all derived classes use following definition
    assign_key_index(key_index);
    Pecos::SurrogateDataVars sdv = variables_to_sdv(c_vars);          // shallow
    Pecos::SurrogateDataResp sdr = response_to_sdr(response, fn_index);//shallow
    add(sdv, v_copy, sdr, r_copy, anchor_flag); // deep copies managed here
    add(eval_id);
  }
}


inline void Approximation::
add(const Pecos::SurrogateDataVars& sdv, bool v_copy, const Response& response,
    size_t fn_index, bool r_copy, bool anchor_flag, int eval_id,
    size_t key_index)
{
  if (approxRep)
    approxRep->add(sdv, v_copy, response, fn_index, r_copy, anchor_flag,
		   eval_id, key_index);
  else { // not virtual: all derived classes use following definition
    assign_key_index(key_index);
    Pecos::SurrogateDataResp sdr = response_to_sdr(response, fn_index);//shallow
    add(sdv, v_copy, sdr, r_copy, anchor_flag); // deep copies managed here
    add(eval_id);
  }
}


inline void Approximation::
add(const Pecos::SurrogateDataVars& sdv, bool v_copy,
    const Pecos::SurrogateDataResp& sdr, bool r_copy, bool anchor_flag,
    int eval_id, size_t key_index)
{
  if (approxRep)
    approxRep->add(sdv, v_copy, sdr, r_copy, anchor_flag, eval_id, key_index);
  else {
    assign_key_index(key_index);
    add(sdv, v_copy, sdr, r_copy, anchor_flag);
    add(eval_id);
  }
}


inline void Approximation::add(int eval_id)
{ if (eval_id != INT_MAX) approxData.push_back(eval_id); }


/*
inline size_t Approximation::pop_count(size_t key_index) const
{
  if (approxRep) return approxRep->pop_count(key_index);
  else {
    assign_key_index(key_index);
    return approxData.pop_count();
  }
}
*/


inline void Approximation::pop_count(size_t count, size_t key_index)
{
  if (approxRep) approxRep->pop_count(count, key_index);
  else {
    assign_key_index(key_index);
    approxData.pop_count(count);
  }
}


inline void Approximation::active_model_key(const Pecos::ActiveKey& sd_key)
{
  if (approxRep) approxRep->active_model_key(sd_key);
  else approxData.active_key(sd_key);
}


inline void Approximation::clear_model_keys()
{
  if (approxRep) approxRep->clear_model_keys();
  else approxData.clear_all(false); // don't re-initialize
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
  else approxData.clear_data(); // re-initializes by default
}


inline void Approximation::clear_active_data()
{
  if (approxRep) approxRep->clear_active_data();
  else approxData.clear_active_data(sharedDataRep->activeKey);
}


inline void Approximation::clear_inactive_data()
{
  if (approxRep) approxRep->clear_inactive_data();
  else // This is used after combination, so don't enumerate embedded keys
    approxData.clear_inactive_data();
}


inline void Approximation::clear_active_popped()
{
  if (approxRep) approxRep->clear_active_popped();
  else approxData.clear_active_popped(sharedDataRep->activeKey);
}


inline void Approximation::clear_popped()
{
  if (approxRep) approxRep->clear_popped();
  else approxData.clear_popped();
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


inline size_t Approximation::num_components() const
{ 
  if (approxRep) return approxRep->num_components();
  else return 1;
}


inline std::shared_ptr<Approximation> Approximation::approx_rep() const
{ return approxRep; }

} // namespace Dakota

#endif
