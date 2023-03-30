/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


//- Class:	     SensAnalysisGlobal
//- Description: Utility helper class which has correlations and VBD
//- Owner:       Laura Swiler, Brian Adams, Ahmad Rushdi
//- Checked by:
//- Version:

#include "SensAnalysisGlobal.hpp"
#include "ResultsManager.hpp"
#include "dakota_linear_algebra.hpp"
#include "dakota_stat_util.hpp"
#include <algorithm>
#include <boost/iterator/counting_iterator.hpp>

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
  if (num_obs == 0) {
    Cerr << "Error: Number of samples must be nonzero in SensAnalysisGlobal::"
         << "compute_correlations()." << std::endl;
    abort_handler(-1);
  }
  if (resp_samples.size() != num_obs) {
    Cerr << "Error: Mismatch in array lengths in SensAnalysisGlobal::"
         << "compute_correlations()." << std::endl;
    abort_handler(-1);
  }

  numVars = vars_samples[0].cv() + vars_samples[0].div() + 
    vars_samples[0].dsv() + vars_samples[0].drv();
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

/** This version is used when compact samples matrix is being
    processed.  Calculates simple correlation, partial correlation,
    simple rank correlation, and partial rank correlation
    coefficients. */
void SensAnalysisGlobal::
compute_correlations(const RealMatrix&     vars_samples,
                     const IntResponseMap& resp_samples)
{
  int num_obs = vars_samples.numCols();
  if (!num_obs) {
    Cerr << "Error: Number of samples must be nonzero in SensAnalysisGlobal::"
         << "compute_correlations()." << std::endl;
    abort_handler(-1);
  }
  if (resp_samples.size() != num_obs) {
    Cerr << "Error: Mismatch in array lengths in SensAnalysisGlobal::"
         << "compute_correlations()." << std::endl;
    abort_handler(-1);
  }

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
}


void SensAnalysisGlobal::
print_std_regress_coeffs(std::ostream& s, StringMultiArrayConstView cv_labels,
		   const StringArray& resp_labels) const
{
  // output standardized regression coefficients and coefficients of determination (R^2)

  s << "\nStandardized Regression Coefficients and Coefficients of Determination (R^2):\n";

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

  size_t i, j, num_cv = cv_labels.size();
  s << "\nSRCs :\n"
    << "             ";
  for (j=0; j<numFns; ++j)
    s << std::setw(12) << resp_labels[j] << ' ';
  s << '\n';
  for (i=0; i<numVars; ++i) {
    s << std::setw(12) << cv_labels[i] << ' ';
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
}


} // namespace Dakota

