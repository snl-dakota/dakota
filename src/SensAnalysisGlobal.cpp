/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


//- Class:	 SensAnalysisGlobal
//- Description: Utility helper class which has correlations and VBD
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "SensAnalysisGlobal.hpp"
#include "ResultsManager.hpp"
#include <algorithm>

#include <boost/math/special_functions/fpclassify.hpp>

static const char rcsId[]="@(#) $Id: SensAnalysisGlobal.cpp 6170 2009-10-06 22:42:15Z lpswile $";


namespace Dakota {

RealArray SensAnalysisGlobal::rawData = RealArray();


bool SensAnalysisGlobal::rank_sort(const int& x, const int& y)
{ return rawData[x]<rawData[y]; }


void SensAnalysisGlobal::
compute_correlations(const VariablesArray& vars_samples,
		     const IntResponseMap& resp_samples)
{
  using boost::math::isfinite;

  // this method calculates four correlation matrices:
  // simple correlation coefficients, partial correlation coefficients
  // simple rank correlation coefficients, and partial rank correlation coeff.

  size_t num_obs = vars_samples.size();
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
  size_t i, j, k, num_cv = vars_samples[0].cv(),
    num_div = vars_samples[0].div(), num_drv = vars_samples[0].drv();
  numVars = num_cv + num_div + num_drv;
  numFns  = resp_samples.begin()->second.num_functions();

  //simple correlation coefficients
  int num_corr = numVars + numFns;
  size_t num_true_samples = 0;
  BoolDeque valid_sample(num_obs);
  IntRespMCIter it;
  for (it=resp_samples.begin(), j=0; j<num_obs; ++it, ++j) {
    valid_sample[j] = true;
    for (k=0; k<numFns; ++k) // any Nan or +/-Inf observation will be dropped
      if (!isfinite(it->second.function_value(k)))
	{ valid_sample[j] = false; break; }
    if (valid_sample[j])
      ++num_true_samples;
  }
  
  RealMatrix total_data(num_corr, num_true_samples);
  size_t s_cntr = 0;
  for (it=resp_samples.begin(), j=0; j<num_obs; ++it, ++j)
    if (valid_sample[j]) {
      const Variables& vars_j = vars_samples[j];
      for (i=0; i<num_cv; ++i)
	total_data(i, s_cntr) = vars_j.continuous_variable(i);
      for (i=0; i<num_div; ++i)
	total_data(i+num_cv, s_cntr) = (Real)vars_j.discrete_int_variable(i);
      for (i=0; i<num_drv; ++i)
	total_data(i+num_cv+num_div, s_cntr) = vars_j.discrete_real_variable(i);
      const Response& resp_j = it->second;
      for (i=0; i<numFns; ++i)
	total_data(i+numVars, s_cntr) = resp_j.function_value(i);
      ++s_cntr;
    }
 
  //calculate simple rank correlation coeff
  simple_corr(total_data, false, num_corr);

  //calculate partial correlation coeff
  partial_corr(total_data, false, numVars);

  //simple rank correlations
  IntArray rank_col(num_true_samples);
  IntArray final_rank(num_true_samples);
  rawData.resize(num_true_samples);
  for (i=0; i<num_corr; ++i) {
    s_cntr = 0;
    if (i<numVars) {
      for (j=0; j<num_obs; ++j)
	if (valid_sample[j]) {
	  if (i<num_cv)
	    rawData[s_cntr] = vars_samples[j].continuous_variable(i);
	  else if (i<num_cv+num_div)
	    rawData[s_cntr]
	      = (Real)vars_samples[j].discrete_int_variable(i-num_cv);
	  else if (i<numVars)
	    rawData[s_cntr]
	      = vars_samples[j].discrete_real_variable(i-num_cv-num_div);
	  rank_col[s_cntr] = s_cntr; ++s_cntr;
	}
    }
    else {
      for (it=resp_samples.begin(), j=0; j<num_obs; ++it, ++j)
	if (valid_sample[j]) {
	  rawData[s_cntr] = it->second.function_value(i-numVars);
	  rank_col[s_cntr] = s_cntr; ++s_cntr;
	}
    }
    std::sort(rank_col.begin(),rank_col.end(),rank_sort);
    for (j=0; j<num_true_samples; ++j)
      final_rank[rank_col[j]] = j;
    for (j=0; j<num_true_samples; ++j)
      total_data(i, j) = (Real)final_rank[j];
  }

  //calculate simple rank correlation coeff
  simple_corr(total_data, true, num_corr);

  //calculate partial rank correlation coeff
  partial_corr(total_data, true, numVars);

  corrComputed = true;
}


void SensAnalysisGlobal::
compute_correlations(const RealMatrix&     vars_samples,
		     const IntResponseMap& resp_samples)
{
  using boost::math::isfinite;

  // this method calculates four correlation matrices:
  // simple correlation coefficients, partial correlation coefficients
  // simple rank correlation coefficients, and partial rank correlation coeff.

  size_t num_obs = vars_samples.numCols();
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
  size_t i, j, k;
  numVars = vars_samples.numRows();
  numFns  = resp_samples.begin()->second.num_functions();

  //simple correlation coefficients
  int num_corr = numVars + numFns;
  size_t num_true_samples = 0;
  BoolDeque valid_sample(num_obs);
  IntRespMCIter it;
  for (it=resp_samples.begin(), j=0; j<num_obs; ++it, ++j) {
    valid_sample[j] = true;
    for (k=0; k<numFns; ++k) // any Nan or +/-Inf observation will be dropped
      if (!isfinite(it->second.function_value(k)))
	{ valid_sample[j] = false; break; }
    if (valid_sample[j])
      ++num_true_samples;
  }

  RealMatrix total_data(num_corr, num_true_samples);
  size_t s_cntr = 0;
  for (it=resp_samples.begin(), j=0; j<num_obs; ++it, ++j)
    if (valid_sample[j]) {
      for (i=0; i<numVars; ++i)
	total_data(i, s_cntr) = vars_samples(i,j);
      const Response& resp_j = it->second;
      for (i=0; i<numFns; ++i)
	total_data(i+numVars, s_cntr) = resp_j.function_value(i);
      ++s_cntr;
    }
 
  //calculate simple rank correlation coeff
  simple_corr(total_data, false, num_corr);

  //calculate partial correlation coeff
  partial_corr(total_data, false, numVars);

  //simple rank correlations
  IntArray rank_col(num_true_samples);
  IntArray final_rank(num_true_samples);
  rawData.resize(num_true_samples);
  for (i=0; i<num_corr; ++i) {
    s_cntr = 0;
    if (i<numVars) {
      for (j=0; j<num_obs; ++j)
	if (valid_sample[j]) {
	  rawData[s_cntr] = vars_samples(i,j);
	  rank_col[s_cntr] = s_cntr; ++s_cntr;
	}
    }
    else {
      for (it=resp_samples.begin(), j=0; j<num_obs; ++it, ++j)
	if (valid_sample[j]) {
	  rawData[s_cntr] = it->second.function_value(i-numVars);
	  rank_col[s_cntr] = s_cntr; ++s_cntr;
	}
    }
    std::sort(rank_col.begin(), rank_col.end(), rank_sort);
    for (j=0; j<num_true_samples; ++j)
      final_rank[rank_col[j]] = j;
    for (j=0; j<num_true_samples; ++j)
      total_data(i, j) = (Real)final_rank[j];
  }

  //calculate simple rank correlation coeff
  simple_corr(total_data, true, num_corr);

  //calculate partial rank correlation coeff
  partial_corr(total_data, true, numVars);

  corrComputed = true;
}


void SensAnalysisGlobal::
simple_corr(RealMatrix& total_data, bool rank_on, const int& num_in)
{
  //this method calculates simple correlation coefficients from a matrix of data
  //num_corr is number of columns of total data
  //rank_on indicates if rank correlations should be calculated
  //num_in indicate if only pairs of correlations 
  //should be calculated between pairs of columns (num_in vs. num_corr-num_in) 
  //if num_in =  num_corr, correlations are calculated between all columns

  size_t i,j,k;
  int num_corr = total_data.numRows(), num_obs = total_data.numCols(),
    num_out = num_corr - num_in;

  if (num_corr == num_in) {
    if (rank_on)
      simpleRankCorr.shape(num_corr,num_corr);
    else
      simpleCorr.shape(num_corr,num_corr);
  }
  else {
    if (rank_on)
      simpleRankCorr.shape(num_in,num_out);
    else
      simpleCorr.shape(num_in,num_out);
  }

  RealVector mean_column(num_corr, false), absmax_column(num_corr,false),
    sumsquare_column(num_corr, false);

  //get means of each column
  for (i=0; i<num_corr; i++) {
    absmax_column(i)=0;
    Real sum = 0.0;
    for (j=0; j<num_obs; j++){
      sum += total_data(i,j);
      if (std::fabs(total_data(i,j)) > absmax_column(i))
	absmax_column(i) = std::fabs(total_data(i,j));
    }
    mean_column(i) = sum/((Real)num_obs);
  }

  //subtract means from each value in each column
  for (i=0; i<num_corr; i++)
    for (j=0; j<num_obs; j++){
      total_data(i,j) -= mean_column(i);
      if (std::fabs(total_data(i,j))/absmax_column(i) < 1.e-15)
	total_data(i,j) = 0;
    }

  //calculate sum of squares for each column
  for (i=0; i<num_corr; i++) {
    Real sumsq = 0.0;
    for (j=0; j<num_obs; j++)
      sumsq += total_data(i,j)*total_data(i,j);
    sumsquare_column(i) = std::sqrt(sumsq);
  }
 
  //normalize the column values with the sumsquare term
  for (i=0; i<num_corr; i++)
    for (j=0; j<num_obs; j++)
      total_data(i,j) = (sumsquare_column(i)>1E-30)
                      ? total_data(i,j)/sumsquare_column(i) : 0.;

  //calculate simple correlation coeff, put in matrix
  if (num_corr == num_in) {
    for (i=0; i<num_corr; i++) {
      if (rank_on)
	simpleRankCorr(i,i) = 1.0;
      else
	simpleCorr(i,i) = 1.0;
    }
    for (i=0; i<num_corr; i++) {
      for (k=0; k<i; k++) {
	if (rank_on) {
	  simpleRankCorr(k,i) = 0.0;
	  for (j=0; j<num_obs; j++)
	    simpleRankCorr(i,k) += total_data(i,j)*total_data(k,j);
	}
	else {
	  simpleCorr(k,i) = 0.0;
	  for (j=0; j<num_obs; j++)
	    simpleCorr(i,k) += total_data(i,j)*total_data(k,j);
	}
      }
    }
  }
  else{//input/output case
    for (i=0; i<num_in; i++) {
      for (k=0; k<num_out; k++) {
	if (rank_on) {
	  simpleRankCorr(i,k) = 0.0;
	  for (j=0; j<num_obs; j++)
	    simpleRankCorr(i,k) += total_data(i,j)*total_data(k+num_in,j);
	}
	else {
	  simpleCorr(i,k) = 0.0;
	  for (j=0; j<num_obs; j++)
	    simpleCorr(i,k) += total_data(i,j)*total_data(k+num_in,j);
	}
      }
    }
  }
} 


void SensAnalysisGlobal::
partial_corr(RealMatrix& total_data, bool rank_on, const int& num_in)
{
  //this method calculates partial correlation coefficients from a
  //matrix of data.  Note that it is assumed that total data is
  //normalized (each value is the (original value-column value)/sumsqr col.)

  size_t i,j,k,m;
  int num_obs = total_data.numCols(), num_out = total_data.numRows() - num_in;
  RealMatrix    total_pdata, partial_corr, partial_transpose, temp2_corr;
  RealSymMatrix temp_corr;
  Teuchos::SerialSpdDenseSolver<int, Real> corr_solve1;
  Teuchos::SerialDenseSolver<int, Real>    corr_solve2;
  //temporary ints to take return codes from matrix factorization and solves
  int succ_solve1 = 0, succ_solve2 = 0;
  if (rank_on)
    numericalIssuesRank=false;  
  else
    numericalIssuesRaw=false;

  total_pdata.shape(num_in+1,num_obs);
  temp_corr.shape(num_in+1);
  partial_corr.shape(num_in+1,num_in+1);
  if (rank_on)
    partialRankCorr.shape(num_in, num_out);
  else
    partialCorr.shape(num_in, num_out);
  temp2_corr.shape(num_in+1,num_in+1);
  partial_transpose.shape(num_in+1,num_in+1);

  //start off by populating partial_corr
  //NOTE:  need to calculate partial correlations one response at a time
  for (i=0; i<num_in; i++)
    for (k=0; k<num_obs; k++)
      total_pdata(i,k) = total_data(i,k);

  for (m=0; m<num_out; m++) {
    //first finish populating data by adding one response at a time
    for (j=0; j<num_obs; j++)
      total_pdata(num_in,j) = total_data(num_in + m,j);
    for (i=0; i<num_in+1; i++) {
      for (k=0; k<=i; k++){
	temp_corr(i,k) = 0.0;
	if (k==i)
	  temp_corr(k,i) = 1.0;
	else {
	  for (j=0; j<num_obs; j++)
	    temp_corr(i,k) += total_pdata(i,j)*total_pdata(k,j);
	  temp_corr(k,i) = temp_corr(i,k);
	}
      }
    }
 
    corr_solve1.setMatrix( Teuchos::rcp(&temp_corr, false) );
    succ_solve1 = corr_solve1.factor();
    for (i=0; i<num_in+1; i++)
      for (k=0; k<=i; k++)
	temp2_corr(i,k) = (k <= i) ? temp_corr(i,k): 0.0;
    corr_solve2.setMatrix( Teuchos::rcp(&temp2_corr, false) );
    succ_solve2 = corr_solve2.invert();
    if (rank_on)
      numericalIssuesRank
	= (succ_solve1 != 0) || (succ_solve2 !=0) || (numericalIssuesRank);
    else
      numericalIssuesRaw
	= (succ_solve1 != 0) || (succ_solve2 !=0) || (numericalIssuesRaw);
 
    for (i=0; i<num_in+1; i++)
      for (k=0; k<num_in+1; k++)
	partial_transpose(k,i) = temp2_corr(i,k);
    partial_corr.multiply(Teuchos::NO_TRANS,Teuchos::NO_TRANS,1.0,
                          partial_transpose,temp2_corr,0.0);
    for (i=0; i<num_in; i++) {
      if (rank_on)
	partialRankCorr(i,m) = -partial_corr(i, num_in) /
	  std::sqrt(partial_corr(num_in, num_in) * partial_corr(i, i));
      else
	partialCorr(i,m) = -partial_corr(i, num_in) /
	  std::sqrt(partial_corr(num_in, num_in) * partial_corr(i, i));
    }
  }
}


// TODO: combine archive with print
void SensAnalysisGlobal::
archive_correlations(const StrStrSizet& run_identifier,  
		     ResultsManager& iterator_results,
		     StringMultiArrayConstView cv_labels,
		     StringMultiArrayConstView div_labels,
		     StringMultiArrayConstView drv_labels,
		     const StringArray& resp_labels) const
{
  if (!iterator_results.active())  return;

  int num_in_out = numVars + numFns;
  if (simpleCorr.numRows() == num_in_out &&
      simpleCorr.numCols() == num_in_out) {
    MetaDataType md;
    md["Row labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, resp_labels);
    md["Column labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, resp_labels);
    iterator_results.insert(run_identifier, 
			    iterator_results.results_names.correl_simple_all,
			    simpleCorr, md);
  }
  else if (simpleCorr.numRows() == numVars &&
	   simpleCorr.numCols() == numFns) {
    MetaDataType md;
    md["Row labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, StringArray());
    md["Column labels"] = make_metadatavalue(resp_labels);
    iterator_results.insert(run_identifier, 
			    iterator_results.results_names.correl_simple_io,
			    simpleCorr, md);
  }

  if (partialCorr.numRows() == numVars &&
      partialCorr.numCols() == numFns) {
    MetaDataType md;
    md["Row labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, StringArray());
    md["Column labels"] = make_metadatavalue(resp_labels);
    iterator_results.insert(run_identifier, 
			    iterator_results.results_names.correl_partial_io,
			    partialCorr, md);
  }
  // TODO: metadata
  //  if (numericalIssuesRaw)

  if (simpleRankCorr.numRows() == num_in_out &&
      simpleRankCorr.numCols() == num_in_out) {
    MetaDataType md;
    md["Row labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, resp_labels);
    md["Column labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, resp_labels);
    iterator_results.insert(run_identifier, 
			    iterator_results.results_names.correl_simple_rank_all,
			    simpleRankCorr, md);
  }
  else if (simpleRankCorr.numRows() == numVars &&
	   simpleRankCorr.numCols() == numFns) {
    MetaDataType md;
    md["Row labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, StringArray());
    md["Column labels"] = make_metadatavalue(resp_labels);
    iterator_results.insert(run_identifier, 
			    iterator_results.results_names.correl_simple_rank_io,
			    simpleRankCorr, md);


  }

  if (partialRankCorr.numRows() == numVars &&
      partialRankCorr.numCols() == numFns) {
    MetaDataType md;
    md["Row labels"] = 
      make_metadatavalue(cv_labels, div_labels, drv_labels, StringArray());
    md["Column labels"] = make_metadatavalue(resp_labels);
    iterator_results.insert(run_identifier, 
			    iterator_results.results_names.correl_partial_rank_io,
			    partialRankCorr, md);
  }
}


void SensAnalysisGlobal::
print_correlations(std::ostream& s, StringMultiArrayConstView cv_labels,
		   StringMultiArrayConstView div_labels,
		   StringMultiArrayConstView drv_labels,
		   const StringArray& resp_labels) const
{
  // output correlation matrices

  s << std::scientific << std::setprecision(5);

  if (resp_labels.size() != numFns) { 
    Cerr << "Error: Number of response labels (" << resp_labels.size()
	 << ") passed to print_correlations not equal to number of output "
	 << "functions (" << numFns << ") in compute_correlations()."
	 << std::endl;
    abort_handler(-1);
  }

  size_t i, j, num_cv = cv_labels.size(), num_div = div_labels.size(),
    num_drv = drv_labels.size();
  if (num_cv+num_div+num_drv != numVars) {
    Cerr << "Error: Number of variable labels passed to print_correlations not "
	 << "equal to number of input variables in compute_correlations()."
	 << std::endl;
    abort_handler(-1);
  }

  int num_in_out = numVars + numFns;
  if (simpleCorr.numRows() == num_in_out &&
      simpleCorr.numCols() == num_in_out) {
    s << "\nSimple Correlation Matrix among all inputs and outputs:\n"
      << "             ";
    for (i=0; i<num_cv; ++i)
      s << std::setw(12) << cv_labels[i] << ' ';
    for (i=0; i<num_div; ++i)
      s << std::setw(12) << div_labels[i] << ' ';
    for (i=0; i<num_drv; ++i)
      s << std::setw(12) << drv_labels[i] << ' ';
    for (i=0; i<numFns; ++i)
      s << std::setw(12) << resp_labels[i] << ' ';
    s << '\n';
    for (i=0; i<num_in_out; ++i) {
      if (i<num_cv)
	s << std::setw(12) << cv_labels[i] << ' ';
      else if (i<num_cv+num_div)
	s << std::setw(12) << div_labels[i-num_cv] << ' ';
      else if (i<numVars)
	s << std::setw(12) << drv_labels[i-num_cv-num_div] << ' ';
      else
	s << std::setw(12) << resp_labels[i-numVars] << ' ';
      for (j=0; j<=i; ++j)
	s << std::setw(12) << simpleCorr(i,j) << ' ';
      s << '\n';
    }
  }
  else if (simpleCorr.numRows() == numVars &&
	   simpleCorr.numCols() == numFns) {
    s << "\nSimple Correlation Matrix between input and output:\n"
      << "             ";
    for (j=0; j<numFns; ++j)
      s << std::setw(12) << resp_labels[j] << ' ';
    s << '\n';
    for (i=0; i<numVars; ++i) {
      if (i<num_cv)
	s << std::setw(12) << cv_labels[i] << ' ';
      else if (i<num_cv+num_div)
	s << std::setw(12) << div_labels[i-num_cv] << ' ';
      else
	s << std::setw(12) << drv_labels[i-num_cv-num_div] << ' ';
      for (j=0; j<numFns; ++j)
	s << std::setw(12) << simpleCorr(i,j) << ' ';
      s << '\n';
    }
  }

  if (partialCorr.numRows() == numVars &&
      partialCorr.numCols() == numFns) {
    s << "\nPartial Correlation Matrix between input and output:\n"
      << "             ";
    for (j=0; j<numFns; ++j)
      s << std::setw(12) << resp_labels[j]<<' ';
    s << '\n';
    for (i=0; i<numVars; ++i) {
      if (i<num_cv)
	s << std::setw(12) << cv_labels[i] << ' ';
      else if (i<num_cv+num_div)
	s << std::setw(12) << div_labels[i-num_cv] << ' ';
      else
	s << std::setw(12) << drv_labels[i-num_cv-num_div] << ' ';
      for (j=0; j<numFns; ++j)
	s << std::setw(12) << partialCorr(i,j) << ' ';
      s << '\n';
    }
  }

  if (numericalIssuesRaw)
    s << "\nThere may be some numerical issues associated with the calculation "
      << "of the \npartial correlation coefficients above.  This can be due to "
      << "very small \nnumbers of input samples, or to ill-conditioned matrices"
      << ", \nin situations where the partials are very close to zero, -1, or "
      << "+1.\n";

  if (simpleRankCorr.numRows() == num_in_out &&
      simpleRankCorr.numCols() == num_in_out) {
    s << "\nSimple Rank Correlation Matrix among all inputs and outputs:\n"
      << "             ";
    for (i=0; i<num_cv; ++i)
      s << std::setw(12) << cv_labels[i] << ' ';
    for (i=0; i<num_div; ++i)
      s << std::setw(12) << div_labels[i] << ' ';
    for (i=0; i<num_drv; ++i)
      s << std::setw(12) << drv_labels[i] << ' ';
    for (i=0; i<numFns; ++i)
      s << std::setw(12) << resp_labels[i] << ' ';
    s << '\n';
    for (i=0; i<num_in_out; ++i) {
      if (i<num_cv)
	s << std::setw(12) << cv_labels[i] << ' ';
      else if (i<num_cv+num_div)
	s << std::setw(12) << div_labels[i-num_cv] << ' ';
      else if (i<numVars)
	s << std::setw(12) << drv_labels[i-num_cv-num_div] << ' ';
      else
	s << std::setw(12) << resp_labels[i-numVars] << ' ';
      for (j=0; j<=i; ++j)
	s << std::setw(12) << simpleRankCorr(i,j) << ' ';
      s << '\n';
    }
  }
  else if (simpleRankCorr.numRows() == numVars &&
	   simpleRankCorr.numCols() == numFns) {
    s << "\nSimple Rank Correlation Matrix between input and output:\n"
      << "             ";
    for (j=0; j<numFns; ++j)
      s << std::setw(12) << resp_labels[j] << ' ';
    s << '\n';
    for (i=0; i<numVars; ++i) {
      if (i<num_cv)
	s << std::setw(12) << cv_labels[i] << ' ';
      else if (i<num_cv+num_div)
	s << std::setw(12) << div_labels[i-num_cv] << ' ';
      else
	s << std::setw(12) << drv_labels[i-num_cv-num_div] << ' ';
      for (j=0; j<numFns; ++j)
	s << std::setw(12) << simpleRankCorr(i,j) << ' ';
      s << '\n';
    }
  }

  if (partialRankCorr.numRows() == numVars &&
      partialRankCorr.numCols() == numFns) {
    s << "\nPartial Rank Correlation Matrix between input and output:\n"
      << "             ";
    for (j=0; j<numFns; ++j)
      s << std::setw(12) << resp_labels[j] << ' ';
    s << '\n';
    for (i=0; i<numVars; ++i) {
      if (i<num_cv)
	s << std::setw(12) << cv_labels[i] << ' ';
      else if (i<num_cv+num_div)
	s << std::setw(12) << div_labels[i-num_cv] << ' ';
      else
	s << std::setw(12) << drv_labels[i-num_cv-num_div] << ' ';
      for (j=0; j<numFns; ++j)
	s << std::setw(12) << partialRankCorr(i,j) << ' ';
      s << '\n';
    }
  }

  if (numericalIssuesRank)
    s << "\nThere may be some numerical issues associated with the calculation "
      << "of the \npartial rank correlation coefficients above.  This can be "
      << "due to very small \nnumbers of input samples, or to ill-conditioned "
      << "matrices, \nin situations where the partials are very close to zero, "
      << "-1, or +1.\n";
   
  s << std::setprecision(write_precision)  // return to previous precision
    << std::endl;
}

} // namespace Dakota

