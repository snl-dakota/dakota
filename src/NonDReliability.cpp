/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "NonDReliability.hpp"

static const char rcsId[] = "@(#) $Id: NonDReliability.cpp 6793 2010-05-21 00:07:25Z mseldre $";

namespace Dakota {


NonDReliability::NonDReliability(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  mppSearchType(probDescDB.get_ushort("method.sub_method")),
  integrationRefinement(
    probDescDB.get_ushort("method.nond.integration_refinement")),
  numRelAnalyses(0)
  //refinementSamples(probDescDB.get_int("method.samples")),
  //refinementSeed(probDescDB.get_int("method.random_seed"))
{
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "Error: discrete random variables are not supported in reliability "
	 << "methods." << std::endl;
    abort_handler(-1);
  }

  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);
}


NonDReliability::~NonDReliability()
{ }


bool NonDReliability::resize()
{
  bool parent_reinit_comms = NonD::resize();

  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);

  return parent_reinit_comms;
}


void NonDReliability::post_run(std::ostream& s)
{
  ++numRelAnalyses;

  if (!mppModel.is_null() && mppModel.mapping_initialized()) {
    /*bool var_size_changed =*/ mppModel.finalize_mapping();
    //if (var_size_changed) resize();
  }

  Analyzer::post_run(s);
}

} // namespace Dakota
