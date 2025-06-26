/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef EMBED_HYBRID_META_ITERATOR_H
#define EMBED_HYBRID_META_ITERATOR_H

#include "MetaIterator.hpp"

namespace Dakota {

/// Meta-iterator for closely-coupled hybrid iteration, typically
/// involving the embedding of local search methods within global
/// search methods.

/** This meta-iterator uses multiple methods in close coordination,
    generally using a local search minimizer repeatedly within a
    global minimizer (the local search minimizer refines candidate
    minima which are fed back to the global minimizer). */

class EmbedHybridMetaIterator : public MetaIterator {
 public:
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  EmbedHybridMetaIterator(ProblemDescDB& problem_db,
                          ParallelLibrary& parallel_lib);
  /// alternate constructor
  EmbedHybridMetaIterator(ProblemDescDB& problem_db,
                          ParallelLibrary& parallel_lib,
                          std::shared_ptr<Model> model);
  /// destructor
  ~EmbedHybridMetaIterator() override;

 protected:
  //
  //- Heading: Member functions
  //

  /// Performs the hybrid iteration by executing global and local
  /// iterators, using a set of models that may vary in fidelity
  void core_run() override;

  void derived_init_communicators(ParLevLIter pl_iter) override;
  void derived_set_communicators(ParLevLIter pl_iter) override;
  void derived_free_communicators(ParLevLIter pl_iter) override;

  IntIntPair estimate_partition_bounds() override;

  /// return the final solution from the embedded hybrid (variables)
  const Variables& variables_results() const override;
  /// return the final solution from the embedded hybrid (response)
  const Response& response_results() const override;

 private:
  //
  //- Heading: Data members
  //

  /// the top-level outer iterator (e.g., global minimizer)
  std::shared_ptr<Iterator> globalIterator;
  /// the model employed by the top-level outer iterator
  std::shared_ptr<Model> globalModel;

  /// the inner iterator (e.g., local minimizer)
  std::shared_ptr<Iterator> localIterator;
  /// the model employed by the inner iterator
  std::shared_ptr<Model> localModel;

  /// use of constructor that enforces use of a single passed Model
  bool singlePassedModel;

  /// the probability of running a local search refinement within
  /// phases of the global minimization for tightly-coupled hybrids
  Real localSearchProb;
};

inline const Variables& EmbedHybridMetaIterator::variables_results() const {
  return globalIterator->variables_results();
}

inline const Response& EmbedHybridMetaIterator::response_results() const {
  return globalIterator->response_results();
}

}  // namespace Dakota

#endif
