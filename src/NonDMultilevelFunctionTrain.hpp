/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDMultilevelFunctionTrain
//- Description: Iterator to compute/employ Polynomial Chaos expansions
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_MULTILEVEL_FUNCTION_TRAIN_H
#define NOND_MULTILEVEL_FUNCTION_TRAIN_H

#include "NonDC3FunctionTrain.hpp"

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

  void initialize_ml_regression(size_t num_lev, bool& import_pilot);
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

  size_t collocation_points() const;
  size_t start_rank() const;
  size_t start_order() const;

  /// perform specification updates (shared code from
  // {assign,increment}_specification_sequence())
  void update_from_specification();

  // scale sample profile to retain shape while enforcing an upper bound
  //void scale_profile(..., RealVector& new_N_l);

  //
  //- Heading: Data
  //

  /// user specification for start_rank_sequence
  SizetArray startRankSeqSpec;
  /// user specification for start_order_sequence
  SizetArray startOrderSeqSpec;

  /// sequence index for {expOrder,collocPts,expSamples}SeqSpec
  size_t sequenceIndex;
};


inline size_t NonDMultilevelFunctionTrain::collocation_points() const
{
  if (collocPtsSeqSpec.empty()) return std::numeric_limits<size_t>::max();
  else
    return (sequenceIndex < collocPtsSeqSpec.size()) ?
      collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();
}


inline size_t NonDMultilevelFunctionTrain::start_rank() const
{
  if (startRankSeqSpec.empty())
    return startRankSpec; // use default provided by DataMethod
  else
    return (sequenceIndex < startRankSeqSpec.size()) ?
      startRankSeqSpec[sequenceIndex] : startRankSeqSpec.back();
}


inline size_t NonDMultilevelFunctionTrain::start_order() const
{
  if (startOrderSeqSpec.empty())
    return startOrderSpec; // use default provided by DataMethod
  else
    return (sequenceIndex < startOrderSeqSpec.size()) ?
      startOrderSeqSpec[sequenceIndex] : startOrderSeqSpec.back();
}

} // namespace Dakota

#endif
