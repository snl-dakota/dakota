/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "SensAnalysisGlobal.hpp"
#include "ResultsManager.hpp"
#include "dakota_linear_algebra.hpp"
#include "dakota_data_util.hpp"
#include "dakota_stat_util.hpp"
#include <algorithm>
#include <boost/iterator/counting_iterator.hpp>
#include "DataMethod.hpp" 

static const char rcsId[]="@(#) $Id: SensAnalysisGlobal.cpp 6170 2009-10-06 22:42:15Z lpswile $";

namespace Dakota {

RealArray SensAnalysisGlobal::rawData = RealArray();


bool SensAnalysisGlobal::rank_sort(const int& x, const int& y)
{ return rawData[x]<rawData[y]; }


size_t SensAnalysisGlobal::
find_valid_samples(const IntResponseMap& resp_samples, BoolDeque& valid_sample)
{
  // TODO: later compute correlation on per-response basis to keep
  // partial faults
  using std::isfinite;

  size_t num_obs = resp_samples.size(), num_valid_samples = 0;
  IntRespMCIter it = resp_samples.begin();
  for (size_t j=0; j<num_obs; ++j, ++it) {
    valid_sample[j] = true;
    for (size_t k=0; k<numFns; ++k)
      if (!isfinite(it->second.function_value(k))) {
        valid_sample[j] = false; 
        break; 
      }
    if (valid_sample[j])
      ++num_valid_samples;
  }

  return num_valid_samples;
}

void SensAnalysisGlobal::
valid_sample_matrix(const VariablesArray& vars_samples,
                    const IntResponseMap& resp_samples,
                    const StringSetArray& dss_vals,
                    const BoolDeque is_valid_sample,
                    RealMatrix& valid_data) 
{
  int num_obs = vars_samples.size(), num_corr = valid_data.numRows();
  IntRespMCIter it = resp_samples.begin();
  for (size_t j=0, s_cntr=0; j<num_obs; ++j, ++it)
    if (is_valid_sample[j]) {
      // get a view of the first numVars rows of the samples col
      RealVector td_col_vars(Teuchos::View, valid_data[s_cntr], (int)numVars);
      vars_samples[j].as_vector(dss_vals, td_col_vars);
      // get a view of the last numFns rows of the samples col
      RealVector td_col_resp(Teuchos::View, valid_data[s_cntr] + numVars, 
                             (int)numFns);
      copy_data(it->second.function_values(), td_col_resp);
      ++s_cntr;
    }
}

void SensAnalysisGlobal::
valid_sample_matrix(const RealMatrix&     vars_samples,
                    const IntResponseMap& resp_samples,
                    const BoolDeque is_valid_sample,
                    RealMatrix& valid_data)
{
  int num_obs = vars_samples.numCols(), num_corr = valid_data.numRows();
  IntRespMCIter it = resp_samples.begin();
  for (int j=0, s_cntr=0; j<num_obs; ++j, ++it)
    if (is_valid_sample[j]) {
      for (int i=0; i<numVars; ++i)
        valid_data(i, s_cntr) = vars_samples(i, j);
      // get a view of the last numFns rows of the samples col
      RealVector td_col_resp(Teuchos::View, valid_data[s_cntr] + numVars, 
                             (int)numFns);
      copy_data(it->second.function_values(), td_col_resp);
      ++s_cntr;
    }
}

/** When converting values to ranks, uses the average ranks of any tied values */
void SensAnalysisGlobal::values_to_ranks(RealMatrix& valid_data)
{
  int num_corr = valid_data.numRows(), num_valid_samples = valid_data.numCols();
  // for each var/resp
  for (int i=0; i<num_corr; ++i) {
    // create a multimap from value to array index (so it is sorted by value and
    // the ranks are given by the map order); don't need a stable sort as we are
    // replacing the tied values by their average rank
    RealIntMultiMap vals_inds;
    for (int j=0; j<num_valid_samples; ++j)
      vals_inds.insert(std::make_pair(valid_data(i,j), j));

    // iterate for each unique value and find tied values
    RealIntMultiMap::const_iterator vi_it = vals_inds.begin();
    RealIntMultiMap::const_iterator vi_end = vals_inds.end();
    for (int rank=0; vi_it != vi_end; ) {
      // find a range of tied values
      double value = vi_it->first;
      std::pair<RealIntMultiMap::const_iterator, RealIntMultiMap::const_iterator>
	tied_range = vals_inds.equal_range(value); 
      int num_ties = std::distance(tied_range.first, tied_range.second);
      double avg_rank = (rank + rank+num_ties-1) / 2.0;
      // all tied values get assigned the average rank
      for ( ; tied_range.first != tied_range.second; ++tied_range.first)
	valid_data(i, tied_range.first->second) = avg_rank;
      // increment to the next unequal value
      vi_it = tied_range.second;
      rank += num_ties;
    }
  }
}

void SensAnalysisGlobal::correl_adjust(Real& corr_value)
{
  if (std::isfinite(corr_value) && std::abs(corr_value) > 1.0)
    corr_value = corr_value / std::abs(corr_value);
}

/** This version is used when full variables objects are being
    processed. Calculates simple correlation, partial correlation,
    simple rank correlation, and partial rank correlation
    coefficients. */
void SensAnalysisGlobal::
compute_correlations(const VariablesArray& vars_samples,
                     const IntResponseMap& resp_samples,
                     const StringSetArray& dss_vals)
{
  size_t num_obs = vars_samples.size();
  check_num_samples( num_obs, resp_samples.size(), "compute_correlations" );

  numVars = get_n_vars( vars_samples ); 
  numFns  = resp_samples.begin()->second.num_functions();
  int num_corr = numVars + numFns;

  // determine which samples have valid responses
  BoolDeque is_valid_sample(num_obs);
  int num_valid_samples = find_valid_samples(resp_samples, is_valid_sample);
  
  // The following calls regenerate and destroy the valid_data matrix
  // to save memory

  // create a matrix containing only the valid sample data
  RealMatrix valid_data(num_corr, num_valid_samples);

  // calculate simple rank correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, dss_vals, is_valid_sample, 
                      valid_data);
  simple_corr(valid_data, num_corr, simpleCorr);

  // calculate partial correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, dss_vals, is_valid_sample, 
                      valid_data);
  partial_corr(valid_data, numVars, simpleCorr, partialCorr, numericalIssuesRaw);

  // calculate simple rank correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, dss_vals, is_valid_sample, 
                      valid_data);
  values_to_ranks(valid_data);
  simple_corr(valid_data, num_corr, simpleRankCorr);

  // calculate partial rank correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, dss_vals, is_valid_sample, 
                      valid_data);
  values_to_ranks(valid_data);
  partial_corr(valid_data, numVars, simpleRankCorr, partialRankCorr, 
               numericalIssuesRank);

  corrComputed = true;
}

void SensAnalysisGlobal::check_num_samples( const size_t n_var_samples,
                                              const size_t n_response_samples,
                                              const char* method_name ){
 if ( n_var_samples == 0 ) {
    Cerr << "Error: Number of samples must be nonzero in SensAnalysisGlobal::"
         << method_name << "()." << std::endl;
    abort_handler(-1);
  }
  if ( n_response_samples != n_var_samples ) {
    Cerr << "Error: Mismatch in array lengths in SensAnalysisGlobal::"
         << method_name << "()." << std::endl;
    abort_handler(-1);
  }
}

size_t SensAnalysisGlobal::get_n_vars(const VariablesArray& vars_samples ){
  // TNP NOTE: There is some question whether this accounts for cases with 
  // design variables, or the special sampling case of all uniform. Need to
  // discuss further with BMA. 
  size_t n_vars = vars_samples[0].cv() + vars_samples[0].div() + 
                    vars_samples[0].dsv() + vars_samples[0].drv();
  return n_vars;
}

/** This version is used when compact samples matrix is being
    processed.  Calculates simple correlation, partial correlation,
    simple rank correlation, and partial rank correlation
    coefficients. */
void SensAnalysisGlobal::
compute_correlations(const RealMatrix&     vars_samples,
                     const IntResponseMap& resp_samples)
{
  size_t num_obs = vars_samples.numCols();
  check_num_samples( num_obs, resp_samples.size(), "compute_correlations");

  numVars = vars_samples.numRows();
  numFns  = resp_samples.begin()->second.num_functions();
  int num_corr = numVars + numFns;

  // determine which samples have valid responses
  BoolDeque is_valid_sample(num_obs);
  int num_valid_samples = find_valid_samples(resp_samples, is_valid_sample);

  // The following calls regenerate and destroy the valid_data matrix
  // to save memory

  // create a matrix containing only the valid sample data
  RealMatrix valid_data(num_corr, num_valid_samples);

  // calculate simple rank correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, is_valid_sample, valid_data);
  simple_corr(valid_data, num_corr, simpleCorr);

  // calculate partial correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, is_valid_sample, valid_data);
  partial_corr(valid_data, numVars, simpleCorr, partialCorr, numericalIssuesRaw);

  // calculate simple rank correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, is_valid_sample, valid_data);
  values_to_ranks(valid_data);
  simple_corr(valid_data, num_corr, simpleRankCorr);

  // calculate partial rank correlation coeff
  valid_sample_matrix(vars_samples, resp_samples, is_valid_sample, valid_data);
  values_to_ranks(valid_data);
  partial_corr(valid_data, numVars, simpleRankCorr, partialRankCorr, 
               numericalIssuesRank);

  corrComputed = true;
}

/** Calculates simple correlation coefficients from a matrix of data
    (oriented factors x observations):
     - num_corr is number of rows of total data 
     - num_in indicates whether only pairs of correlations should be
       calculated between pairs of columns (num_in
       vs. num_corr-num_in); if num_in = num_corr, correlations are
       calculated between all columns */
void SensAnalysisGlobal::
simple_corr(RealMatrix& total_data, const int& num_in, RealMatrix& corr_matrix)
{
  int num_corr = total_data.numRows(), num_obs = total_data.numCols();

  center_matrix_rows(total_data);

  for (int i=0; i<num_corr; i++) {
    // calculate sum of squares for each factor (row)
    Real row_sumsq = 0.0;
    for (int j=0; j<num_obs; j++)
      row_sumsq += total_data(i,j)*total_data(i,j);
    row_sumsq = std::sqrt(row_sumsq);
    // normalize the rows with the sumsquare term
    for (int j=0; j<num_obs; j++)
      total_data(i,j) /= row_sumsq;
  }

  // calculate matrix of simple correlation coefficients
  if (num_corr == num_in) {
    // all-to-all case
    corr_matrix.shape(num_corr, num_corr);
    if (num_obs <= 1)
      corr_matrix.putScalar(std::numeric_limits<double>::quiet_NaN());
    else {
      corr_matrix.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, 1.0, 
			   total_data, total_data, 0.0);
      for (int i=0; i<num_corr; ++i) {
	// set finite diagonal values to 1.0
	if (std::isfinite(corr_matrix(i,i)))
	  corr_matrix(i,i) = 1.0;
	// snap all finite values to [-1.0, 1.0]
	for (int j=0; j<i; ++j) {
	  correl_adjust(corr_matrix(i,j));
	  correl_adjust(corr_matrix(j,i));
	}
      }
    }
  }
  else {  
    // input-to-output case
    int num_out = num_corr - num_in;
    corr_matrix.shape(num_in, num_out);
    if (num_obs <= 1)
      corr_matrix.putScalar(std::numeric_limits<double>::quiet_NaN());
    else {
      RealMatrix total_data_in(Teuchos::View, total_data, num_in, num_obs, 0, 0);
      RealMatrix total_data_out(Teuchos::View, total_data, num_out, num_obs, 
				num_in, 0);
      corr_matrix.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, 1.0, 
			   total_data_in, total_data_out, 0.0);
      // snap all finite values to [-1.0, 1.0]
      for (int i=0; i<num_in; ++i)
	for (int j=0; j<num_out; ++j)
	  correl_adjust(corr_matrix(i,j));
    }
  } 
}

/** Calculates partial correlation coefficients between num_in inputs
    and numRows() - num_in outputs. */
void SensAnalysisGlobal::
partial_corr(RealMatrix& total_data, const int num_in, 
             const RealMatrix& simple_corr_mat,
             RealMatrix& corr_matrix, bool& numerical_issues)
{
  int num_obs = total_data.numCols(), num_out = total_data.numRows() - num_in;

  // initialize output data
  corr_matrix.reshape(num_in, num_out);    
  numerical_issues = false;
  // TODO: return numerical issues per-input
  BoolDeque numerical_except(num_in, false);

  if (num_obs <= 1) {
    corr_matrix.putScalar(std::numeric_limits<double>::quiet_NaN());
    numerical_issues = true;
    return;
  }
  
  // For a single input factor, partial = simple (no controlling factors)
  if (num_in == 1) {
    for (int k=0; k<num_out; ++k)
      corr_matrix(0, k) = simple_corr_mat(0, k+1);
    return;
  }

  center_matrix_rows(total_data);

  // matrix of X = [Vi | R ]; the response cols don't change per variable
  RealMatrix correl_factors_X(num_obs, 1 + num_out);
  for (int j=0; j<num_obs; ++j)
    for (int k=0; k<num_out; ++k)
      correl_factors_X(j, 1+k) = total_data(num_in+k, j);
  // matrix of Z = [V~i]
  RealMatrix control_factors_Z(num_obs, num_in - 1);

  for (int i=0; i<num_in; ++i) {

    // partial correlation analysis for Vi, R, controlling for
    // V~i. Transpose everything to line up with more typical
    // convention Nobs x (Nvar + Nresp)
    for (int j=0; j<num_obs; ++j) {
      correl_factors_X(j, 0) = total_data(i, j); // Vi
      // (response columns are already populated above)
      // V~i
      for (int k=0; k<i; ++k)
        control_factors_Z(j, k) = total_data(k, j);
      for (int k=i+1; k<num_in; ++k)
        control_factors_Z(j, k-1) = total_data(k, j);
    }

    // form partial_cov = X'X - (X'Z)*inv(Z'Z)*(Z'X), preserving
    // symmetric and positive definite
    int mult_err = 0;

    // intialize to X'X
    RealMatrix partial_cov(1 + num_out, 1 + num_out);
    mult_err |= partial_cov.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1.0,
				     correl_factors_X, correl_factors_X, 0.0);

    // Here we use truncated SVD to account for the case where there
    // are fewer data points than variables, including the degenerate
    // case of 2 observations.  Should be equivalent to minimum-norm
    // least squares in the regressions.

    RealMatrix Zinv_Zt_X;  // a bit misnamed, since Z is rectangular
    bool use_qr = false;
    if (use_qr) {
      Zinv_Zt_X.reshape(num_in - 1, 1 + num_out);
      // initialize to Z'X
      mult_err |= Zinv_Zt_X.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1.0, 
                                     control_factors_Z, correl_factors_X, 0.0);

      // factor Z = QR; in-place QR factorization destroys Z (this never errors)
      qr(control_factors_Z); 

      // now backsolve to update to Rinv' * Z'X (a failed backsolve leaves junk)
      int qrs_info = qr_rsolve(control_factors_Z, true, Zinv_Zt_X);
      numerical_except[i] = numerical_except[i] || (qrs_info != 0);
    }
    else {
      // alternative with svd to numerical precision; some concern
      // remains that this might not yield nan/inf when it should

      // initialize to Z'X
      RealMatrix Zt_X(num_in - 1, 1 + num_out);
      mult_err |= Zt_X.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1.0, 
                                control_factors_Z, correl_factors_X, 0.0);
      RealVector sing_vals; 
      RealMatrix v_trans;
      svd(control_factors_Z, sing_vals, v_trans);

      double tol = 
        std::numeric_limits<double>::epsilon() * control_factors_Z.normInf();
      int sv_keep = 0;
      for ( ; sv_keep < sing_vals.length(); ++sv_keep)
        if (sing_vals[sv_keep] < tol)
          break;
      // TODO: Could opt for stricter check here:
      // if (sv_keep < std::min(num_obs, num_in-1))
      if (sv_keep == 0)
        numerical_except[i] = true;
      else {
        v_trans.reshape(sv_keep, num_in - 1);
        Zinv_Zt_X.reshape(sv_keep, 1 + num_out);
        Zinv_Zt_X.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, 
                           v_trans, Zt_X, 0.0);
        for (int j=0; j<sv_keep; ++j)
          for (int k=0; k < (1 + num_out); ++k)
            Zinv_Zt_X(j,k) /= sing_vals[j];
      }
    }

    if (!numerical_except[i])
      // X'X - (X'Z)*inv(Z'Z)*(Z'X) = X'X - (Zinv*Z'X)'(Zinv*Z'X)
      mult_err |= partial_cov.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, -1.0,
				       Zinv_Zt_X, Zinv_Zt_X, 1.0);

    // This should never happen:
    if (mult_err != 0) {
      Cerr << "\nError (partial_corr): multiplying incompatible matrices.\n";
      abort_handler(-1);
    }
    numerical_issues = numerical_issues || numerical_except[i];

    for (int k=0; k<num_out; ++k)
      if (numerical_except[i])
        corr_matrix(i,k) = std::numeric_limits<Real>::quiet_NaN();
      else
        corr_matrix(i,k) = partial_cov(0, k+1) / std::sqrt(partial_cov(0,0)) / 
          std::sqrt(partial_cov(k+1, k+1));
  }

  // snap all finite values to [-1.0, 1.0]
  for (int i=0; i<num_in; ++i)
    for (int j=0; j<num_out; ++j)
      correl_adjust(corr_matrix(i,j));
}

// Return true if any correlation coefficient is NaN or Inf, false otherwise
bool SensAnalysisGlobal::has_nan_or_inf(const RealMatrix &corr) const {
  int num_rows = corr.numRows(), num_cols = corr.numCols();
  for(int j = 0; j < num_cols; ++j) 
    for(int i = 0; i < num_rows; ++i) 
      if( ! std::isfinite(corr(i,j)))
        return true;
  return false;
}

// TODO: combine archive with print
void SensAnalysisGlobal::
archive_correlations(const StrStrSizet& run_identifier,  
		     ResultsManager& iterator_results,
		     const StringArray& var_labels,
		     const StringArray& resp_labels,
         const size_t &inc_id) const
{
  if (!iterator_results.active())  return;

  // Get pointers to the variables and response names to create scales
  std::vector<const char *> combined_desc;
  combined_desc.reserve(var_labels.size() + resp_labels.size());
  for(const String& label : var_labels)
    combined_desc.push_back(label.c_str());
  for(const String& label : resp_labels)
    combined_desc.push_back(label.c_str());

  archive_simple_correlations(run_identifier, iterator_results, var_labels, resp_labels, combined_desc, inc_id, false);
  archive_simple_correlations(run_identifier, iterator_results, var_labels, resp_labels, combined_desc, inc_id, true);
  archive_partial_correlations(run_identifier, iterator_results, var_labels, resp_labels, inc_id, false);
  archive_partial_correlations(run_identifier, iterator_results, var_labels, resp_labels, inc_id, true);
}

void SensAnalysisGlobal::
archive_simple_correlations(const StrStrSizet& run_identifier,  
		     ResultsManager& iterator_results,
		     const StringArray& var_labels,
		     const StringArray& resp_labels,
         const std::vector<const char *>& combined_desc,
         const size_t &inc_id,
         bool rank) const {
  
  int num_in_out = numVars + numFns;
  StringArray location;
  if(inc_id)
    location.push_back(String("increment:") + std::to_string(inc_id));
  if(rank) 
    location.push_back("simple_rank_correlations");
  else 
    location.push_back("simple_correlations");
  auto& corr_matrix = (rank) ? simpleRankCorr : simpleCorr;
  if (corr_matrix.numRows() == num_in_out &&
      corr_matrix.numCols() == num_in_out) {
    DimScaleMap scales;
    scales.emplace(0, StringScale("factors", combined_desc));
    scales.emplace(1, StringScale("factors", combined_desc));
    iterator_results.insert(run_identifier,location, 
        corr_matrix, scales);
  }
  else if (corr_matrix.numRows() == numVars &&
	         corr_matrix.numCols() == numFns) {
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", var_labels));
    scales.emplace(1, StringScale("responses", resp_labels));
    iterator_results.insert(run_identifier,location, 
        corr_matrix, scales);
  }
}

void SensAnalysisGlobal::
archive_partial_correlations(const StrStrSizet& run_identifier,  
		     ResultsManager& iterator_results,
		     const StringArray& var_labels,
		     const StringArray& resp_labels,
         const size_t &inc_id,
         bool rank) const {
  
  StringArray location;
  if(inc_id)
    location.push_back(String("increment:") + std::to_string(inc_id));
  if(rank) 
    location.push_back("partial_rank_correlations");
  else 
    location.push_back("partial_correlations");
  auto & corr_matrix = (rank) ? partialRankCorr : partialCorr;
  
  location.push_back("");
  if (corr_matrix.numRows() == numVars &&
      corr_matrix.numCols() == numFns) {
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", var_labels));
    for (size_t i=0; i<resp_labels.size(); ++i) {
      location.back() = resp_labels[i];
      iterator_results.insert(run_identifier,location,
          Teuchos::getCol<int,Real>(Teuchos::View, *const_cast<RealMatrix*>(&corr_matrix), i), scales);
    }
  }
}

void SensAnalysisGlobal::
archive_std_regress_coeffs(const StrStrSizet& run_identifier,  
                           ResultsManager& iterator_results,
                           const StringArray& var_labels,
                           const StringArray & resp_labels,
                           const size_t &inc_id) const
{
  if(!iterator_results.active())
    return;

  DimScaleMap scales;
  scales.emplace(0, StringScale("variables", var_labels));

  StringArray location;
  if(inc_id) location.push_back(String("increment:") + std::to_string(inc_id));
  location.push_back("std_regression_coeffs");

  location.push_back("");
  for (size_t i=0; i<resp_labels.size(); ++i) {
    location.back() = resp_labels[i];
    iterator_results.insert(run_identifier, location,
        Teuchos::getCol<int,Real>(Teuchos::View, *const_cast<RealMatrix*>(&stdRegressCoeffs), i), scales);
    // Archive Coeff of Determination (R^2) as an attribute
    AttributeArray ns_attr({ResultAttribute<Real>("coefficient_of_determination", stdRegressCODs[i])}); 
    iterator_results.add_metadata_to_object(run_identifier, location, ns_attr);
  }

}

void SensAnalysisGlobal::
print_correlations(std::ostream& s, const StringArray& var_labels,
		   const StringArray& resp_labels) const
{
  // output correlation matrices

  if (!corrComputed) {
    Cout << "Correlation matrices not computed." << std::endl;
    return;
  }
  check_correlations_for_nan_or_inf(s);

  s << std::scientific << std::setprecision(5);

  if (resp_labels.size() != numFns) { 
    Cerr << "Error: Number of response labels (" << resp_labels.size()
	 << ") passed to print_correlations not equal to number of output "
	 << "functions (" << numFns << ") in compute_correlations()."
	 << std::endl;
    abort_handler(-1);
  }

  if (var_labels.size() != numVars) {
    Cerr << "Error: Number of variable labels (" << var_labels.size()
	 << ") passed to print_correlations not equal to number of input "
	 << "variables (" << numVars << ") in compute_correlations()." << std::endl;
    abort_handler(-1);
  }

  print_simple_correlations(s, var_labels, resp_labels, false);
  print_partial_correlations(s, var_labels, resp_labels, false);
  print_simple_correlations(s, var_labels, resp_labels, true);
  print_partial_correlations(s, var_labels, resp_labels, true);
    
  s << std::setprecision(write_precision)  // return to previous precision
    << std::endl;
}


void SensAnalysisGlobal::
check_correlations_for_nan_or_inf(std::ostream& s) const {
  if( has_nan_or_inf(simpleCorr) ||
      has_nan_or_inf(partialCorr) ||
      has_nan_or_inf(simpleRankCorr) ||
      has_nan_or_inf(partialRankCorr) )
    s << "\n\nAt least one correlation coefficient is nan or inf. This " <<
      "commonly occurs when\ndiscrete variables (including histogram " <<
      "variables) are present, a response is\ncompletely insensitive to " <<
      "variables (response variance equal to 0), there are\nfewer samples " <<
      "than variables, or some samples are approximately collinear." << 
      std::endl;
}

void SensAnalysisGlobal::
print_simple_correlations(std::ostream& s, const StringArray& var_labels, const StringArray& resp_labels, bool rank) const {
  int num_in_out = numVars + numFns;
  auto& corr_matrix = (rank) ? simpleRankCorr : simpleCorr;

  if (corr_matrix.numRows() == num_in_out &&
      corr_matrix.numCols() == num_in_out) {
    if (rank)
      s << "\nSimple Rank Correlation Matrix among all inputs and outputs:\n"
        << "             ";
    else
      s << "\nSimple Correlation Matrix among all inputs and outputs:\n"
        << "             ";
    for(const String &label : var_labels)
      s << std::setw(12) << label << ' ';
    for(const String &label : resp_labels)
      s << std::setw(12) << label << ' ';
    s << '\n';
    for (size_t i=0; i<num_in_out; ++i) {
      if (i<numVars)
	      s << std::setw(12) << var_labels[i] << ' ';
      else
	      s << std::setw(12) << resp_labels[i-numVars] << ' ';
      for (size_t j=0; j<=i; ++j)
	      s << std::setw(12) << corr_matrix(i,j) << ' ';
      s << '\n';
    }
  } else if (corr_matrix.numRows() == numVars &&
	   corr_matrix.numCols() == numFns) {
    if(rank)
      s << "\nSimple Rank Correlation Matrix between input and output:\n"
        << "             ";
    else
      s << "\nSimple Correlation Matrix between input and output:\n"
        << "             ";
    for (const String & label : resp_labels)
      s << std::setw(12) << label << ' ';
    s << '\n';
    for (size_t i=0; i<numVars; ++i) {
      s << std::setw(12) << var_labels[i] << ' ';
      for (size_t j=0; j<numFns; ++j)
	      s << std::setw(12) << corr_matrix(i,j) << ' ';
      s << '\n';
    }
  }
}

void SensAnalysisGlobal::
print_partial_correlations(std::ostream& s, const StringArray& var_labels, const StringArray& resp_labels, bool rank) const {
  int num_in_out = numVars + numFns;
  auto& corr_matrix = (rank) ? partialRankCorr : partialCorr;

  if (corr_matrix.numRows() == numVars &&
      corr_matrix.numCols() == numFns) {
    if(rank)
      s << "\nPartial Rank Correlation Matrix between input and output:\n"
        << "             ";
    else
      s << "\nPartial Correlation Matrix between input and output:\n"
        << "             ";
    for (const String& label : resp_labels)
      s << std::setw(12) << label << ' ';
    s << '\n';
    for (size_t i=0; i<numVars; ++i) {
    	s << std::setw(12) << var_labels[i] << ' ';
      for (size_t j=0; j<numFns; ++j)
	      s << std::setw(12) << corr_matrix(i,j) << ' ';
      s << '\n';
    }
  }
}


void SensAnalysisGlobal::
compute_std_regress_coeffs(const RealMatrix&     vars_samples,
                           const IntResponseMap& resp_samples)
{
#ifdef HAVE_DAKOTA_SURROGATES
  int num_obs = vars_samples.numCols();
  if (!num_obs) {
    Cerr << "Error: Number of samples must be nonzero in SensAnalysisGlobal::"
         << "compute_std_regress_coeffs()." << std::endl;
    abort_handler(-1);
  }
  if (resp_samples.size() != num_obs) {
    Cerr << "Error: Mismatch in array lengths in SensAnalysisGlobal::"
         << "compute_std_regress_coeffs()." << std::endl;
    abort_handler(-1);
  }

  numVars = vars_samples.numRows();
  numFns  = resp_samples.begin()->second.num_functions();

  // determine which samples have valid responses
  BoolDeque is_valid_sample(num_obs);
  int num_valid_samples = find_valid_samples(resp_samples, is_valid_sample);

  // create a matrix containing only the valid sample data
  int num_vars_and_resp = numVars + numFns;
  RealMatrix valid_data(num_vars_and_resp, num_valid_samples);
  valid_sample_matrix(vars_samples, resp_samples, is_valid_sample, valid_data);

  // Copy and reformat variables and responses to work with SRC utility
  RealMatrix vars_view(Teuchos::View, valid_data, numVars, valid_data.numCols());
  RealMatrix vars_copy_trans(vars_view, Teuchos::TRANS);
  RealMatrix resp_view(Teuchos::View, valid_data, numFns, valid_data.numCols(), numVars);
  RealMatrix resp_copy_trans(resp_view, Teuchos::TRANS);

  compute_std_regression_coeffs(vars_copy_trans, resp_copy_trans, stdRegressCoeffs, stdRegressCODs);
#endif
}


void SensAnalysisGlobal::
print_std_regress_coeffs(std::ostream& s, StringArray var_labels,
		   const StringArray& resp_labels) const
{
#ifdef HAVE_DAKOTA_SURROGATES
  // output standardized regression coefficients and coefficients of determination (R^2)

  if( has_nan_or_inf(stdRegressCoeffs) )
    s << "\nAt least one standardized regression coefficient is nan or inf. This " <<
      "commonly occurs when a response is\ncompletely insensitive to " <<
      "variables (response variance equal to 0), there are\nfewer samples " <<
      "than variables, or some samples are approximately collinear." << 
      std::endl;

  s << std::scientific << std::setprecision(5);

  if (resp_labels.size() != numFns) { 
    Cerr << "Error: Number of response labels (" << resp_labels.size()
      << ") passed to print_std_regress_coeffs not equal to number of output "
      << "functions (" << numFns << ") in compute_std_regression_coeffs()."
      << std::endl;
    abort_handler(-1);
  }

  size_t i, j;
  s << "\nStandardized Regression Coefficients and Coefficients of Determination (R^2):\n"
    << "             ";
  for (j=0; j<numFns; ++j)
    s << std::setw(12) << resp_labels[j] << ' ';
  s << '\n';
  for (i=0; i<numVars; ++i) {
    s << std::setw(12) << var_labels[i] << ' ';
    for (j=0; j<numFns; ++j)
      s << std::setw(12) << stdRegressCoeffs(i,j) << ' ';
    s << '\n';
  }
  s << std::setw(12) << "R^2" << ' ';
  for (j=0; j<numFns; ++j)
    s << std::setw(12) << stdRegressCODs(j) << ' ';
  s << '\n';

  s << std::setprecision(write_precision)  // return to previous precision
    << std::endl;
#endif
}

void SensAnalysisGlobal::compute_vbd_stats_via_sampling( const unsigned short   method
                                                       , const int              numBins
                                                       , const size_t           numFunctions
                                                       , const size_t           num_vars
                                                       , const size_t           num_samples
                                                       , const RealMatrix &     vars_samples
                                                       , const IntResponseMap & resp_samples
                                                       )
{

  if (method == VBD_BINNED) {
    this->compute_binned_vbd_stats( numBins
                                  , numFunctions
                                  , num_vars
                                  , num_samples
                                  , vars_samples
                                  , resp_samples
                                  );
  }
  else {
    this->compute_pick_and_freeze_vbd_stats( numFunctions
                                           , num_vars
                                           , num_samples
                                           , resp_samples
                                           );
  }
}

void SensAnalysisGlobal::compute_pick_and_freeze_vbd_stats( const size_t           numFunctions
                                                          , const size_t           num_vars
                                                          , const size_t           num_samples
                                                          , const IntResponseMap & resp_samples
                                                          )
{
  if (resp_samples.size() != num_samples * (num_vars+2)) {
    Cerr << "\nError in Analyzer::compute_vbd_stats_with_Saltelli()"
         << ": expected " << num_samples * (num_vars+2) << " responses"
         << "; received " << resp_samples.size()
         << std::endl;
    abort_handler(METHOD_ERROR);
  }
  
  // BMA: for now copy the data to previous data structure 
  //      total_fn_vals[respFn][replicate][sample]
  // This is making the assumption that the responses are ordered as allSamples
  // BMA TODO: compute statistics on finite samples only
  boost::multi_array<Real,3> total_fn_vals(boost::extents[numFunctions][num_vars+2][num_samples]); // [k][i][j]
  IntRespMCIter r_it = resp_samples.begin();
  for (size_t i(0); i < (num_vars+2); ++i) {
    for (size_t j(0); j < num_samples; ++r_it, ++j) {
      for (size_t k(0); k < numFunctions; ++k) {
        total_fn_vals[k][i][j] = r_it->second.function_value(k);
      }
    }
  }

#ifdef DEBUG
  //Cout << "allSamples:\n" << allSamples << '\n';
  for (size_t k(0); k < numFunctions; ++k) {
    for (size_t i(0); i < num_vars+2; ++i) {
      for (size_t j(0); j < num_samples; ++j) {
        Cout << "Response " << k << " for replicate " << i << ", sample " << j
             << ": " << total_fn_vals[k][i][j] << '\n';
      }
    }
  }
#endif

  // We compute variables indexSi and indexTi according to the following paper:
  // - A. Saltelli, P. Annoni, I. Azzini, F. Campolongo, M. Ratto, S. Tarantola,
  //   "Variance based sensitivity analysis of model output. Design and estimator
  //   for the total sensitivity index.", Comp Physics Comm, 181 (2), pp. 259--270,
  //   Feb. 2010.
  //
  // We decided to use formulas indexSi and indexTi based on testing with a shock
  // physics problem that had significant nonlinearities, interactions, and several
  // response functions. The results are documented in the paper:
  // - V. Weirs, J. Kamm, L. Swiler, S. Tarantola, M. Ratto, B. Adams, W. Rider,
  //   M. Eldred, "Sensitivity analysis techniques applied to a system of
  //   hyperbolic conservation laws", RESS, 107, pp. 157--170, Nov. 2012.

  indexSi.resize(numFunctions, RealVector(num_vars));
  indexTi.resize(numFunctions, RealVector(num_vars));

  boost::multi_array<Real,3> total_norm_vals(boost::extents[numFunctions][num_vars+2][num_samples]); // [k][i][j]

  Real dNumSamples( static_cast<Real>(num_samples) );

  // Obtain sensitivity indices for each function
  for (size_t k(0); k < numFunctions; ++k) {
    Real mean_C(0.);
    {
      Real mean_hatY(0.);
      for (size_t j(0); j < num_samples; ++j) {
        mean_hatY += total_fn_vals[k][0][j];
      }
      mean_hatY /= dNumSamples;

      Real mean_hatB(0.); 
      for (size_t j(0); j < num_samples; ++j) {
        mean_hatB += total_fn_vals[k][1][j];
      }
      mean_hatB /= dNumSamples;

      Real overall_mean(0.);
      for (size_t j(0); j < num_samples; ++j) {
        for(size_t i(0); i < (num_vars+2); ++i) { 
          total_norm_vals[k][i][j]  = total_fn_vals[k][i][j];
          overall_mean             += total_norm_vals[k][i][j];
        } 
      }
      overall_mean /= static_cast<Real>( num_samples * (num_vars+2) );
      for (size_t j(0); j < num_samples; ++j) {
        for(size_t i(0); i < (num_vars+2); ++i) {
          total_norm_vals[k][i][j] -= overall_mean;
        }
      }

      mean_C = (mean_hatB + mean_hatY) / 2.;
    }

    Real var_hatYC(0.);
    for (size_t j(0); j < num_samples; ++j) {
      var_hatYC += std::pow(total_fn_vals[k][0][j],2);
    }
    for (size_t j(0); j < num_samples; ++j) {
      var_hatYC += std::pow(total_fn_vals[k][1][j],2);
    }
    var_hatYC = var_hatYC / (2. * dNumSamples)
              - mean_C * mean_C;

    // calculate first order sensitivity indices and first order total indices
    for (size_t i(0); i < num_vars; ++i) {
      Real sum_S(0.);
      Real sum_T(0.);
      for (size_t j(0); j < num_samples; ++j) {
        Real diff(total_norm_vals[k][i+2][j] - total_norm_vals[k][1][j]);
        sum_S += total_norm_vals[k][0][j] * diff;
        sum_T += std::pow(diff, 2);
      }

      indexSi[k][i] = (sum_S /       dNumSamples ) / var_hatYC;
      indexTi[k][i] = (sum_T / (2. * dNumSamples)) / var_hatYC;
    }
  } // for k
}

void SensAnalysisGlobal::compute_binned_vbd_stats( const int              numBins
                                                 , const size_t           numFunctions
                                                 , const size_t           num_vars
                                                 , const size_t           num_samples
                                                 , const RealMatrix &     vars_samples
                                                 , const IntResponseMap & resp_samples
                                                 )
{
  // TNP NOTE: the algorithm would need to be modified to support discrete random variables.
  // Currently there is a check before calling compute_vbs_stats_via_sampling that errors
  // out if there are discrete random variables.

  check_num_samples( num_samples, resp_samples.size(), "compute_binned_vbd_stats");

  numVars = num_vars;
  numFns  = numFunctions;

  // Determine which samples have valid responses (are).
  BoolDeque is_valid_sample(num_samples);
  int num_valid_samples = find_valid_samples(resp_samples, is_valid_sample);

  // Create a matrix containing only the valid sample data.
  // TNP NOTE: This is filtering out samples if any response is non-numeric. 
  // However, since the binned Sobol' indices are computed per response, we could
  // technically do this filtering per response. For now doing it all at once.
  RealMatrix valid_data( numVars+numFns, num_valid_samples);
  valid_sample_matrix(vars_samples, resp_samples, is_valid_sample, valid_data); 

  size_t n_bins;
  if ( numBins <= 0 ){
    n_bins = std::trunc(std::sqrt(num_valid_samples));
  }
  else{
    n_bins = numBins;
  }

  compute_binned_sobol_indices_from_valid_samples( valid_data, n_bins );
}

// Printing of variance based decomposition indices.
void SensAnalysisGlobal::print_sobol_indices( std::ostream      & s
                                            , const StringArray & var_labels
                                            , const StringArray & resp_labels
                                            , const Real          dropTol
                                            ) const
{
  // output explanatory info
  //  << "Variance Based Decomposition Sensitivity Indices\n"
  //  << "These Sobol' indices measure the importance of the uncertain input\n"
  //  << "variables in determining the uncertainty (variance) of the output.\n"
  //  << "Si measures the main effect for variable i itself, while Ti\n"
  //  << "measures the total effect (including the interaction effects\n" 
  //  << "of variable i with other uncertain variables.)\n";

  s << std::scientific 
    << "\nGlobal sensitivity indices for each response function:\n";

  if (indexTi.size()>0){
    print_main_and_total_effects_indices(s,var_labels,resp_labels,dropTol);
  }
  else{
    print_main_effects_indices(s,var_labels,resp_labels,dropTol);
  }
}

void SensAnalysisGlobal::print_main_and_total_effects_indices( std::ostream      & s
                                                             , const StringArray & var_labels
                                                             , const StringArray & resp_labels
                                                             , const Real          dropTol
                                                             ) const
{
  for (size_t k(0); k < resp_labels.size(); ++k) {
    s << resp_labels[k] << " Sobol' indices:\n"; 
    s << std::setw(38) << "Main" << std::setw(19) << "Total\n";
    for (size_t i(0); i < var_labels.size(); ++i) {
      Real main  = indexSi[k][i];
      Real total = indexTi[k][i];
      if (std::abs(main) > dropTol || std::abs(total) > dropTol) {
        s << "                     " << std::setw(write_precision+7) << main
          << ' ' << std::setw(write_precision+7) << total << ' '
          << var_labels[i] << '\n';
      }
    }
  }
}

void SensAnalysisGlobal::print_main_effects_indices( std::ostream      & s
                                                   , const StringArray & var_labels
                                                   , const StringArray & resp_labels
                                                   , const Real          dropTol
                                                   ) const
{
  for (size_t k(0); k < resp_labels.size(); ++k) {
    s << resp_labels[k] << " Sobol' indices:\n"; 
    s << std::setw(38) << "Main\n";
    for (size_t i(0); i < var_labels.size(); ++i) {
      Real main  = indexSi[k][i];
      if (std::abs(main) > dropTol ) {
        s << "                     " << std::setw(write_precision+7) << main
          <<  ' ' << var_labels[i] << '\n';
      }
    }
  }
}

// Archiving of variance based decomposition indices.
void SensAnalysisGlobal::archive_sobol_indices( const StrStrSizet & run_identifier
                                              , ResultsManager    & resultsDB
                                              , const StringArray & var_labels
                                              , const StringArray & resp_labels
                                              , const Real          dropTol
                                              ) const
{

  if(!resultsDB.active())
    return;

  archive_main_effects_indices(run_identifier, resultsDB, var_labels, resp_labels, dropTol);
  if ( indexTi.size() > 0 ){
    archive_total_effects_indices(run_identifier, resultsDB, var_labels, resp_labels, dropTol);
  }
}

void SensAnalysisGlobal::archive_main_effects_indices(const StrStrSizet & run_identifier
                                              , ResultsManager    & resultsDB
                                              , const StringArray & var_labels
                                              , const StringArray & resp_labels
                                              , const Real          dropTol
                                              ) const
{
  for (size_t k(0); k < resp_labels.size(); ++k) {
    RealArray   main_effects;
    StringArray scale_labels;
    for (size_t i(0); i < var_labels.size(); ++i) {
      Real main  = indexSi[k][i];
      if (std::abs(main) > dropTol)  {
        main_effects.push_back(main);
        scale_labels.push_back(var_labels[i]);
      }
    }
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", scale_labels, ScaleScope::UNSHARED));
    resultsDB.insert(run_identifier, {String("main_effects"), resp_labels[k]}, 
                     main_effects, scales);
  }
}

void SensAnalysisGlobal::archive_total_effects_indices(const StrStrSizet & run_identifier
                                              , ResultsManager    & resultsDB
                                              , const StringArray & var_labels
                                              , const StringArray & resp_labels
                                              , const Real          dropTol
                                              ) const
{
  for (size_t k(0); k < resp_labels.size(); ++k) {
    RealArray   total_effects;
    StringArray scale_labels;
    for (size_t i(0); i < var_labels.size(); ++i) {
      Real total = indexTi[k][i];
      if (std::abs(total) > dropTol) {
        total_effects.push_back(total);
        scale_labels.push_back(var_labels[i]);
      }
    }
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", scale_labels, ScaleScope::UNSHARED));
    resultsDB.insert(run_identifier, {String("total_effects"), resp_labels[k]}, 
                     total_effects, scales);
  }
}

void SensAnalysisGlobal::
compute_binned_sobol_indices_from_valid_samples( const RealMatrix& valid_data, size_t num_bins ){
    
  // Algorithm steps, per variable:
  // Reorder variable samples smallest to largest
  // Sort the responses according to that reordering
  // Bin response samples, compute sample variance in each bin
  // Compute sample mean over binned variances
    
  // TODO: only implemented "option 2" algorithm from the paper; could add "option 1" if there is demand.

  indexSi.resize(numFns, numVars); 

  size_t num_samples = valid_data.numCols();
  size_t num_samples_per_bin = num_samples / num_bins;
  
  IntMatrix sorted_var_indices = get_var_samples_argsort(valid_data);

  RealMatrix response_samples( Teuchos::View, valid_data, numFns, num_samples, numVars );
  RealVector total_means, total_variances;
  compute_response_means_and_variances( response_samples, total_means, total_variances );
  
  RealVector binned_means, binned_variances, partial_variances;
  RealMatrix sorted_response_samples( numFns, num_samples );
  RealMatrix binned_variance_samples( numFns, num_bins );
  for (int i=0; i<numVars; ++i ){ 

    IntVector sort_inds = Teuchos::getCol(Teuchos::View, sorted_var_indices, i);

    reorder_matrix_columns_from_index_vector(response_samples, sorted_response_samples, sort_inds );

    // Compute response variances in each bin
    for ( int j=0; j<num_bins; ++j ){ 

      RealMatrix binned_response_samples( Teuchos::View, sorted_response_samples, numFns, num_samples_per_bin, 0, num_samples_per_bin*j );
      RealMatrix binned_response_samples_T( binned_response_samples, Teuchos::TRANS );
      compute_col_means( binned_response_samples_T, binned_means );
      compute_col_variances( binned_response_samples_T, binned_means, binned_variances );
      Teuchos::setCol( binned_variances, j, binned_variance_samples );
    }

    // Compute expectation of binned variances over bins
    RealMatrix binned_variance_samples_T( binned_variance_samples, Teuchos::TRANS );
    compute_col_means( binned_variance_samples_T, partial_variances );
    for ( int k=0; k<numFns; ++k ){
      indexSi[k][i] = 1 - partial_variances[k] / total_variances[k]; 
    }
  }
}

IntMatrix SensAnalysisGlobal::
get_var_samples_argsort( const RealMatrix& valid_data){
  
  // Making a view of only the variable samples so only those are 
  // converted to their rank values.
  RealMatrix valid_samples_T( valid_data, Teuchos::TRANS );
  RealMatrix var_samples_T( Teuchos::View, valid_samples_T, valid_data.numCols(), numVars );

  RealMatrix sorted_var_samples_T;
  IntMatrix sorted_var_indices_T;
  sort_matrix_columns(var_samples_T, sorted_var_samples_T, sorted_var_indices_T );
  return sorted_var_indices_T;
}

void SensAnalysisGlobal::
compute_response_means_and_variances( const RealMatrix& response_samples, 
                                           RealVector& total_means,
                                           RealVector& total_variances ){
  RealMatrix response_samples_T( response_samples, Teuchos::TRANS);
  compute_col_means( response_samples_T, total_means );
  compute_col_variances(response_samples_T, total_means, total_variances);
}

} // namespace Dakota