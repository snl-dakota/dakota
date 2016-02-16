/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       PStudyDACE
//- Description: Implementation code for the PStudyDACE class
//- Owner:       Mike Eldred

#include "dakota_system_defs.hpp"
#include "DakotaPStudyDACE.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#ifdef HAVE_FSUDACE
#include "fsu.H"
#endif

static const char rcsId[]="@(#) $Id: DakotaPStudyDACE.cpp 6492 2009-12-19 00:04:28Z briadam $";


namespace Dakota {

PStudyDACE::PStudyDACE(ProblemDescDB& problem_db, Model& model):
  Analyzer(problem_db, model),
  volQualityFlag(probDescDB.get_bool("method.quality_metrics")),
  varBasedDecompFlag(probDescDB.get_bool("method.variance_based_decomp"))
{
  // Check for discrete variable types
  if ( (numDiscreteIntVars || numDiscreteRealVars) &&
       methodName > VECTOR_PARAMETER_STUDY)
    Cerr << "\nWarning: discrete variables are ignored by "
	 << method_enum_to_string(methodName) << std::endl;

  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (iteratedModel.gradient_type() == "numerical" &&
      iteratedModel.method_source() == "vendor") {
    Cerr << "\nError: ParamStudy/DACE do not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
	 << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}


PStudyDACE::PStudyDACE(unsigned short method_name, Model& model):
  Analyzer(method_name, model), volQualityFlag(false), varBasedDecompFlag(false)
{
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (iteratedModel.gradient_type() == "numerical" &&
      iteratedModel.method_source() == "vendor") {
    Cerr << "\nError: ParamStudy/DACE do not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
	 << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}


PStudyDACE::~PStudyDACE() { }


bool PStudyDACE::resize()
{
  bool parent_reinit_comms = Analyzer::resize();

  // Current nothing to be done here -> no-op
  return parent_reinit_comms;
}


/** Calculation of volumetric quality measures developed by FSU. */
void PStudyDACE::
volumetric_quality(int ndim, int num_samples, double* sample_points)
{
  int num_trials = 100000;
  int seed_init  = 1 + std::rand();

#ifdef HAVE_FSUDACE
  chiMeas = chi_measure(ndim, num_samples, sample_points, num_trials,seed_init);
  dMeas   = d_measure(ndim, num_samples, sample_points, num_trials, seed_init);
  hMeas   = h_measure(ndim, num_samples, sample_points, num_trials, seed_init);
  tauMeas = tau_measure(ndim, num_samples, sample_points, num_trials,seed_init);
#endif
}


void PStudyDACE::print_results(std::ostream& s)
{
  if (volQualityFlag)
    s << "\nVolumetric uniformity measures (smaller values are better):"
      << "\n  Chi measure is: " << chiMeas << "\n    D measure is: " << dMeas
      << "\n    H measure is: " << hMeas   << "\n  Tau measure is: " << tauMeas
      << "\n\n";

  if (numObjFns || numLSqTerms) // DACE usage
    Analyzer::print_results(s);

  if (varBasedDecompFlag)
    print_sobol_indices(s);

  if (pStudyDACESensGlobal.correlations_computed()) {
    if (compactMode) { // FSU, DDACE, PSUADE ignore active discrete vars
      StringMultiArray empty;
      StringMultiArrayConstView empty_view
	= empty[boost::indices[idx_range(0, 0)]];
      pStudyDACESensGlobal.print_correlations(s,
	iteratedModel.continuous_variable_labels(), empty_view, empty_view,
        empty_view, iteratedModel.response_labels());
    }
    else // ParamStudy includes active discrete vars
      pStudyDACESensGlobal.print_correlations(s,
        iteratedModel.continuous_variable_labels(),
        iteratedModel.discrete_int_variable_labels(),
        iteratedModel.discrete_string_variable_labels(),
        iteratedModel.discrete_real_variable_labels(),
        iteratedModel.response_labels());
  }
}


} // namespace Dakota
