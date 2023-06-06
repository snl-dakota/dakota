/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       PStudyDACE
//- Description: Implementation code for the PStudyDACE class
//- Owner:       Mike Eldred

#include <random>

#include "dakota_system_defs.hpp"
#include "DakotaPStudyDACE.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"
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
  // Historically this used a bare call to rand(), so using
  // random_device.  However, may have relied on rand() without
  // srand() ==> srand(1), so may prefer seeding with a fixed value.
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<> unif_int(1, std::numeric_limits<int>::max());
  int seed_init  = unif_int(rng);

#ifdef HAVE_FSUDACE
  chiMeas = chi_measure(ndim, num_samples, sample_points, num_trials,seed_init);
  dMeas   = d_measure(ndim, num_samples, sample_points, num_trials, seed_init);
  hMeas   = h_measure(ndim, num_samples, sample_points, num_trials, seed_init);
  tauMeas = tau_measure(ndim, num_samples, sample_points, num_trials,seed_init);
#endif
}


void PStudyDACE::print_results(std::ostream& s, short results_state)
{
  if (volQualityFlag)
    s << "\nVolumetric uniformity measures (smaller values are better):"
      << "\n  Chi measure is: " << chiMeas << "\n    D measure is: " << dMeas
      << "\n    H measure is: " << hMeas   << "\n  Tau measure is: " << tauMeas
      << "\n\n";

  if (numObjFns || numLSqTerms) // DACE usage
    Analyzer::print_results(s, results_state);

  if (varBasedDecompFlag)
    pStudyDACESensGlobal.print_sobol_indices(s,
                                             iteratedModel.ordered_labels(),
                                             iteratedModel.response_labels(),
                                             vbdDropTol);

  if (pStudyDACESensGlobal.correlations_computed()) {
    if (compactMode) { // FSU, DDACE, PSUADE ignore active discrete vars
      StringArray cv_labels;
      copy_data(iteratedModel.continuous_variable_labels(), cv_labels);
      pStudyDACESensGlobal.print_correlations(s, cv_labels, iteratedModel.response_labels());
    }
    else // ParamStudy includes active discrete vars
      pStudyDACESensGlobal.print_correlations(s,
        iteratedModel.ordered_labels(),
        iteratedModel.response_labels());
  }
}


} // namespace Dakota
