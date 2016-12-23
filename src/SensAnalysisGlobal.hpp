/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 SensAnalysisGlobal
//- Description: Utility helper class which has correlations and VBD
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef SENS_ANALYSIS_GLOBAL
#define SENS_ANALYSIS_GLOBAL

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "dakota_global_defs.hpp"
#include "dakota_results_types.hpp"

namespace Dakota {

class ResultsManager;

/// Class for a utility class containing correlation calculations
/// and variance-based decomposition

/** This class provides code for several of the sampling methods both
    in the NonD branch and in the PStudyDACE branch.  Currently, the
    utility functions provide global sensitivity analysis through
    correlation calculations (e.g. simple, partial, rank, raw) as well
    as variance-based decomposition. */
class SensAnalysisGlobal
{
public:
 
  //
  //- Heading: Constructors and destructor
  //

  SensAnalysisGlobal();  ///< constructor
  ~SensAnalysisGlobal(); ///< destructor

  //
  //- Heading: Member functions
  //

  /// computes four correlation matrices for input and output data
  /// simple, partial, simple rank, and partial rank
  void compute_correlations(const VariablesArray& vars_samples,
                            const IntResponseMap& resp_samples,
                            const StringSetArray& dss_vals);
  /// computes four correlation matrices for input and output data
  /// simple, partial, simple rank, and partial rank
  void compute_correlations(const RealMatrix&     vars_samples,
                            const IntResponseMap& resp_samples);

  /// save correlations to database
  void archive_correlations(const StrStrSizet& run_identifier,  
                            ResultsManager& iterator_results,
                            StringMultiArrayConstView cv_labels,
                            StringMultiArrayConstView div_labels,
                            StringMultiArrayConstView dsv_labels,
                            StringMultiArrayConstView drv_labels,
                            const StringArray& resp_labels) const;

  /// returns corrComputed to indicate whether compute_correlations()
  /// has been invoked
  bool correlations_computed() const;

  /// prints the correlations computed in compute_correlations()
  void print_correlations(std::ostream& s, StringMultiArrayConstView cv_labels,
			  StringMultiArrayConstView div_labels,
			  StringMultiArrayConstView dsv_labels,
			  StringMultiArrayConstView drv_labels,
			  const StringArray& resp_labels) const;

private:

  //
  //- Heading: Convenience member functions
  //

  /// find samples with finite response (any sample with any Nan or
  /// +/-Inf observation will be dropped)
  size_t find_valid_samples(const IntResponseMap& resp_samples, 
			    BoolDeque& valid_sample);

  /// extract a compact valid sample (vars/resp) matrix from the passed data
  void valid_sample_matrix(const VariablesArray& vars_samples,
                           const IntResponseMap& resp_samples,
                           const StringSetArray& dss_vals,
                           const BoolDeque is_valid_sample,
                           RealMatrix& valid_data);

  /// extract a compact valid sample (vars/resp) matrix from the passed data
  void valid_sample_matrix(const RealMatrix&     vars_samples,
                           const IntResponseMap& resp_samples,
                           const BoolDeque is_valid_sample,
                           RealMatrix& valid_samples);

  /// replace sample values with their ranks, in-place
  void values_to_ranks(RealMatrix& valid_data);

  /// sort algorithm to compute ranks for rank correlations
  static bool rank_sort(const int& x, const int& y);

  /// center the passed matrix by its mean, in-place
  void center_rows(RealMatrix& data_matrix);

  /// if result was NaN/Inf, preserve it, otherwise truncate to [-1.0, 1.0]
  void correl_adjust(Real& corr_value);

  /// computes simple correlations, populating corr_matrix
  void simple_corr(RealMatrix& total_data, const int& num_in,
                   RealMatrix& corr_matrix);
  /// computes partial correlations, populating corr_matrix and numerical_issues
  void partial_corr(RealMatrix& total_data, const int num_in, 
                    const RealMatrix& simple_corr_mat,
                    RealMatrix& corr_matrix, bool& numerical_issues);

  /// Return true if there are any NaN or Inf entries in the matrix
  bool has_nan_or_inf(const RealMatrix &corr) const;

  //- Heading: Data
  //

  /// matrix to hold simple raw correlations
  RealMatrix simpleCorr;
  /// matrix to hold simple rank correlations
  RealMatrix simpleRankCorr;
  /// matrix to hold partial raw correlations
  RealMatrix partialCorr;
  /// matrix to hold partial rank correlations
  RealMatrix partialRankCorr;

  /// array to hold temporary data before sort
  static RealArray rawData;

  /// number of responses
  size_t numFns;
  /// number of inputs
  size_t numVars;

  /// flag indicating numerical issues in partial raw correlation calculations
  bool numericalIssuesRaw;
  /// flag indicating numerical issues in partial rank correlation calculations
  bool numericalIssuesRank;

  /// flag indictaing whether correlations have been computed
  bool corrComputed;
};


inline SensAnalysisGlobal::SensAnalysisGlobal(): corrComputed(false)
{ }


inline SensAnalysisGlobal::~SensAnalysisGlobal()
{ }


inline bool SensAnalysisGlobal::correlations_computed() const
{ return corrComputed; }

} // namespace Dakota

#endif
