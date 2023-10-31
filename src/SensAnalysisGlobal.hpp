/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
                            const StringArray& var_labels,
                            const StringArray& resp_labels, 
                            const size_t &inc_id = 0) const;

  /// save standardized regression coefficients to database
  void archive_std_regress_coeffs(const StrStrSizet& run_identifier,  
                                  ResultsManager& iterator_results,
                                  const StringArray& var_labels,
                                  const StringArray & resp_labels,
                                  const size_t &inc_id = 0) const;

  /// returns corrComputed to indicate whether compute_correlations()
  /// has been invoked
  bool correlations_computed() const;

  /// prints the correlations computed in compute_correlations()
  void print_correlations(std::ostream& s, const StringArray& var_labels,
			  const StringArray& resp_labels) const;

  /// computes standardized regression coefficients and corresponding
  /// R^2 values for input and output data
  void compute_std_regress_coeffs(const RealMatrix&     vars_samples,
                                  const IntResponseMap& resp_samples);

  /// prints the SRCs and R^2 values computed in compute_correlations()
  void print_std_regress_coeffs(std::ostream& s,
                                StringArray var_labels,
			        const StringArray& resp_labels) const;

  /// compute VBD-based Sobol indices
  void compute_vbd_stats_via_sampling( const unsigned short   method
                                     , const int              numBins
                                     , const size_t           numFunctions
                                     , const size_t           num_vars
                                     , const size_t           num_samples
                                     , const RealMatrix &     vars_samples
                                     , const IntResponseMap & resp_samples
                                     );

  /// Printing of VBD results
  void print_sobol_indices( std::ostream      & s
                          , const StringArray & var_labels
                          , const StringArray & resp_labels
                          , const Real          dropTol
                          ) const;

  /// archive VBD-based Sobol indices
  void archive_sobol_indices( const StrStrSizet & run_identifier
                            , ResultsManager    & resultsDB
                            , const StringArray & var_labels
                            , const StringArray & resp_labels
                            , const Real          dropTol
                            ) const;




private:

  //
  //- Heading: Convenience member functions
  //

  /// Checks there are a nonzero number of samples and that the number of samples agree
  /// between variables and responses. Method name passed for error handling.
  void check_num_samples( const size_t num_var_samples, 
                            const size_t num_response_samples, 
                            const char* method_name );

  /// Computes the number of variables for a full variable object.
  size_t get_n_vars( const VariablesArray& vars_samples );

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

  /// Check correlation matrices for nans or infs and print warning message
  void check_correlations_for_nan_or_inf(std::ostream& s) const;

  /// print simple (rank = false) or simple rank (rank = true) correlations
  void print_simple_correlations(std::ostream& s, const StringArray& var_labels, const StringArray& resp_labels, bool rank) const;

  /// print partial (rank = false) or partial rank (rank = true) correlations
  void print_partial_correlations(std::ostream& s, const StringArray& var_labels, const StringArray& resp_labels, bool rank) const;

  /// archive simple (rank = false) or simple rank (rank = true) correlations
  void archive_simple_correlations(const StrStrSizet& run_identifier,  
		     ResultsManager& iterator_results,
		     const StringArray& var_labels,
		     const StringArray& resp_labels,
         const std::vector<const char *>& combined_desc,
         const size_t &inc_id,
         bool rank) const;

  /// archive partial (rank = false) or partial rank (rank = true) correlations
  void archive_partial_correlations(const StrStrSizet& run_identifier,  
		     ResultsManager& iterator_results,
		     const StringArray& var_labels,
		     const StringArray& resp_labels,
         const size_t &inc_id,
         bool rank) const;

  void compute_pick_and_freeze_vbd_stats( const size_t           numFunctions
                                        , const size_t           num_vars
                                        , const size_t           num_samples
                                        , const IntResponseMap & resp_samples
                                        );

  void compute_binned_vbd_stats( const int              numBins
                                       , const size_t           numFunctions
                                       , const size_t           num_vars
                                       , const size_t           num_samples
                                       , const RealMatrix &     vars_samples
                                       , const IntResponseMap & resp_samples
                                       );

  /// Returns a matrix where columns are the indices that would sort
  /// each variable's samples smallest to largest. 
  IntMatrix get_var_samples_argsort( const RealMatrix& valid_data );

  void compute_response_means_and_variances( const RealMatrix& response_samples, 
                                           RealVector& total_means,
                                           RealVector& total_variances );

  void print_main_and_total_effects_indices( std::ostream      & s
                                           , const StringArray & var_labels
                                           , const StringArray & resp_labels
                                           , const Real          dropTol
                                           ) const;
  
  void print_main_effects_indices( std::ostream      & s
                                 , const StringArray & var_labels
                                 , const StringArray & resp_labels
                                 , const Real          dropTol
                                 ) const;


  /// archive VBD-based Sobol main effects indices
  void archive_main_effects_indices( const StrStrSizet & run_identifier
                                   , ResultsManager    & resultsDB
                                   , const StringArray & var_labels
                                   , const StringArray & resp_labels
                                   , const Real          dropTol
                                   ) const;
  
  /// archive VBD-based Sobol total effects indices
  void archive_total_effects_indices( const StrStrSizet & run_identifier
                                    , ResultsManager    & resultsDB
                                    , const StringArray & var_labels
                                    , const StringArray & resp_labels
                                    , const Real          dropTol
                                    ) const;

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

  /// matrix to hold standardized regression coefficients
  RealMatrix stdRegressCoeffs;
  /// vector to hold coefficients of determination, eg R^2 values
  RealVector stdRegressCODs;

  /// array to hold temporary data before sort
  static RealArray rawData;


  /// flag indicating numerical issues in partial raw correlation calculations
  bool numericalIssuesRaw;
  /// flag indicating numerical issues in partial rank correlation calculations
  bool numericalIssuesRank;

  /// flag indicatng whether correlations have been computed
  bool corrComputed;

protected:

  /// compute binned sobol indices from valid samples (having screened out samples with non-numeric response)
  void compute_binned_sobol_indices_from_valid_samples( const RealMatrix& valid_samples,
                                                      size_t num_bins );

  /// Reorder 

  /// number of responses
  size_t numFns;
  /// number of inputs
  size_t numVars;
  
  /// Matrix to hold binned Sobol' indices
  //RealMatrix mainEffects;

  /// VBD main effect indices
  RealVectorArray indexSi;

  /// VBD total effect indices
  RealVectorArray indexTi;
};


inline SensAnalysisGlobal::SensAnalysisGlobal(): corrComputed(false)
{ }


inline SensAnalysisGlobal::~SensAnalysisGlobal()
{ }


inline bool SensAnalysisGlobal::correlations_computed() const
{ return corrComputed; }

} // namespace Dakota

#endif
