/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDIncremLHSSampling
//- Description: Implementation for the incremental version of LHS sampling
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_INCREM_LHS_SAMPLING_H
#define NOND_INCREM_LHS_SAMPLING_H

#include "NonDSampling.hpp"


namespace Dakota {

/// Performs icremental LHS sampling for uncertainty quantification.

/** The Latin Hypercube Sampling (LHS) package from Sandia
    Albuquerque's Risk and Reliability organization provides
    comprehensive capabilities for Monte Carlo and Latin Hypercube
    sampling within a broad array of user-specified probabilistic
    parameter distributions.  The icremental LHS sampling capability
    allows one to supplement an initial sample of size n to size 2n
    while maintaining the correct stratification of the 2n samples and
    also maintaining the specified correlation structure.  The
    icremental version of LHS will return a sample of size n, which
    when combined with the original sample of size n, allows one to
    double the size of the sample. */

class NonDIncremLHSSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDIncremLHSSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDIncremLHSSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  /// performs a forward uncertainty propagation by using LHS to generate a
  /// set of parameter samples, performing function evaluations on these
  /// parameter samples, and computing statistics on the ensemble of results.
  void quantify_uncertainty();

  /// print the final statistics
  void print_results(std::ostream& s);

protected:

  /// sort algorithm to compute ranks for rank correlations
  static bool rank_sort(const int& x, const int& y);

private:

  //
  //- Heading: Data
  //

  int previousSamples;      ///< number of samples in previous LHS run
  bool varBasedDecompFlag;  ///< flags computation of VBD
  static RealArray rawData; ///< static data used by static rank_sort() fn
  static IntArray rankCol; ///< workspace for store_ranks and combine_discrete_ranks
  static IntArray finalRank; ///< workspace for store_ranks and combine_discrete_ranks
  template<typename T>
  void store_ranks(IntMatrix &sample_ranks,
                 const size_t num_vars,
                 const size_t num_samples,
                 const size_t offset_values,
                 const size_t offset_ranks,
                 const T& values);

  template <typename T>
  void combine_discrete_ranks(const T& values, const size_t num_vars,
      const size_t offset);

};

} // namespace Dakota

#endif
