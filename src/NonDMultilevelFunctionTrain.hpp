/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MULTILEVEL_FUNCTION_TRAIN_H
#define NOND_MULTILEVEL_FUNCTION_TRAIN_H

#include "NonDC3FunctionTrain.hpp"
#include "SharedC3ApproxData.hpp"


namespace Dakota {

/// Nonintrusive polynomial chaos expansion approaches to uncertainty
/// quantification

/** The NonDMultilevelFunctionTrain class uses a set of function train
    (FT) expansions, one per model fidelity or resolution, to
    approximate the effect of parameter uncertainties on response
    functions of interest. */

class NonDMultilevelFunctionTrain: public NonDC3FunctionTrain
{
public:

  //
  //- Heading: Constructors and destructor
  //
 
  /// standard constructor
  NonDMultilevelFunctionTrain(ProblemDescDB& problem_db, Model& model);
  /*
  /// alternate constructor for helper iterator
  NonDMultilevelFunctionTrain(unsigned short method_name, Model& model,
			      const SizetArray& colloc_pts_seq,
			      const RealVector& dim_pref,
			      Real colloc_ratio, const SizetArray& pilot,
			      int seed, short u_space_type, short refine_type,
			      short refine_control, short covar_control,
			      short ml_alloc_cntl, short ml_discrep,
			      //short rule_nest, short rule_growth,
			      bool piecewise_basis, bool use_derivs,
			      bool cv_flag,
			      const String& import_build_pts_file,
			      unsigned short import_build_format,
			      bool import_build_active_only);
  */
  /// destructor
  ~NonDMultilevelFunctionTrain();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_u_space_model();
  void core_run();

  void assign_specification_sequence();
  void increment_specification_sequence();

  size_t collocation_points() const;
  int random_seed() const;
  int first_seed() const;

  void initialize_ml_regression(size_t num_lev, bool& import_pilot);
  void infer_pilot_sample(/*Real ratio, */size_t num_steps,
			  SizetArray& delta_N_l);
  void increment_sample_sequence(size_t new_samp, size_t total_samp,
				 size_t step);
  void compute_sample_increment(const RealVector& regress_metrics,
				const SizetArray& N_l, SizetArray& delta_N_l);

  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: Member functions
  //

  /// assign defaults related to allocation control (currently for ML
  /// regression approaches)
  void assign_allocation_control();

private:

  //
  //- Heading: Utility functions
  //

  // sequence handlers:

  size_t start_rank(size_t index) const;
  size_t start_rank() const;

  unsigned short start_order(size_t index) const;
  unsigned short start_order() const;

  void push_c3_active(const UShortArray& orders);
  void push_c3_active();

  /// return the regression size used for different refinement options;
  /// the index identifies the point in the specification sequence 
  size_t regression_size(size_t index);

  // scale sample profile to retain shape while enforcing an upper bound
  //void scale_profile(..., RealVector& new_N_l);

  //
  //- Heading: Data
  //

  /// user specification for start_rank_sequence
  SizetArray startRankSeqSpec;
  /// user specification for start_order_sequence
  UShortArray startOrderSeqSpec;

  /// sequence index for start{Rank,Order}SeqSpec
  size_t sequenceIndex;
};


inline size_t NonDMultilevelFunctionTrain::collocation_points() const
{ return NonDExpansion::collocation_points(sequenceIndex); }


inline int NonDMultilevelFunctionTrain::random_seed() const
{ return NonDExpansion::seed_sequence(sequenceIndex); }


inline int NonDMultilevelFunctionTrain::first_seed() const
{ return NonDExpansion::seed_sequence(0); }


inline size_t NonDMultilevelFunctionTrain::start_rank(size_t index) const
{
  if (startRankSeqSpec.empty())
    return startRankSpec; // use single-level default provided by DataMethod
  else
    return (index < startRankSeqSpec.size()) ?
      startRankSeqSpec[index] : startRankSeqSpec.back();
}


inline size_t NonDMultilevelFunctionTrain::start_rank() const
{ return start_rank(sequenceIndex); }


inline unsigned short NonDMultilevelFunctionTrain::
start_order(size_t index) const
{
  if (startOrderSeqSpec.empty())
    return startOrderSpec; // use single-level default provided by DataMethod
  else
    return (index < startOrderSeqSpec.size()) ?
      startOrderSeqSpec[index] : startOrderSeqSpec.back();
}


inline unsigned short NonDMultilevelFunctionTrain::start_order() const
{ return start_order(sequenceIndex); }


inline void NonDMultilevelFunctionTrain::
push_c3_active(const UShortArray& orders)
{
  push_c3_start_rank(start_rank());
  push_c3_max_rank(maxRankSpec);    // restore if adapted (no sequence)

  push_c3_seed(random_seed());

  push_c3_start_orders(orders);
  push_c3_max_order(maxOrderSpec); // restore if adapted (no sequence)
  std::shared_ptr<SharedC3ApproxData> shared_data_rep =
    std::static_pointer_cast<SharedC3ApproxData>(
    uSpaceModel.shared_approximation().data_rep());
  shared_data_rep->update_basis(); // propagate order updates to oneApproxOpts
}


inline void NonDMultilevelFunctionTrain::push_c3_active()
{
  UShortArray orders;
  configure_expansion_orders(start_order(), dimPrefSpec, orders);
  push_c3_active(orders);
}

} // namespace Dakota

#endif
