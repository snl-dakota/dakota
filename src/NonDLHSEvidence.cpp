/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonLHSDEvidence
//- Description: Implementation code for NonDEvidence class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDLHSEvidence.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"

//#define DEBUG

namespace Dakota {


NonDLHSEvidence::NonDLHSEvidence(ProblemDescDB& problem_db, Model& model):
  NonDLHSInterval(problem_db, model)
{ }


NonDLHSEvidence::~NonDLHSEvidence()
{ }


void NonDLHSEvidence::initialize()
{ calculate_cells_and_bpas(); }


void NonDLHSEvidence::post_process_samples()
{
  // Construct a surrogate based on a set of LHS samples, and evaluate that 
  // sample a large number of times (default: 1 million) to sufficiently 
  // populate the belief intervals
  // Use the sample set generated above to determine the maximum and minimum 
  // of each function within each input interval combination

  const RealMatrix&     all_samples = lhsSampler.all_samples();
  const IntResponseMap& all_responses = lhsSampler.all_responses();

  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {
    // Use the max and mins to determine the cumulative distributions
    // of plausibility and belief
    RealVector& cell_fn_l_bnds = cellFnLowerBounds[respFnCntr];
    RealVector& cell_fn_u_bnds = cellFnUpperBounds[respFnCntr];
    for (size_t i=0; i <numCells; i++) {
      cell_fn_l_bnds[i] =  DBL_MAX;
      cell_fn_u_bnds[i] = -DBL_MAX; 
    }
    Cout << ">>>>> Identifying minimum and maximum samples for response "
	 << "function " << respFnCntr+1 << " within cells 1 through "
	 << numCells << '\n';
    size_t i, j; IntRespMCIter it;
    for (i=0, it=all_responses.begin(); i<numSamples; i++, ++it) {

      const Real& fn_val = it->second.function_value(respFnCntr);
      Variables all_vars = iteratedModel.current_variables().copy();
      sample_to_variables(all_samples[i],all_vars);

      const RealVector&  c_vars = all_vars.continuous_variables();
      const IntVector&  di_vars = all_vars.discrete_int_variables();
      const RealVector& dr_vars = all_vars.discrete_real_variables();

      for (cellCntr=0; cellCntr<numCells; ++cellCntr) {
        bool in_bounds = true;
        for (j=0; j<numContIntervalVars; ++j)
          if (c_vars[j] < cellContLowerBounds[cellCntr][j] ||
              c_vars[j] > cellContUpperBounds[cellCntr][j])
            { in_bounds = false; break; }
        if (in_bounds)
          for (j=0; j<numDiscIntervalVars; ++j)
            if (di_vars[j] < cellIntRangeLowerBounds[cellCntr][j] ||
                di_vars[j] > cellIntRangeUpperBounds[cellCntr][j]) // TO DO
              { in_bounds = false; break; }
        if (in_bounds)
          for (j=0; j<numDiscSetIntUncVars; ++j)
            if (di_vars[j+numDiscIntervalVars] != cellIntSetBounds[cellCntr][j]) // TO DO
              { in_bounds = false; break; }
        if (in_bounds)
          for (j=0; j<numDiscSetRealUncVars; ++j)
            if (dr_vars[j] != cellRealSetBounds[cellCntr][j]) // TO DO
              { in_bounds = false; break; }

	if (in_bounds) {
	  if (fn_val < cell_fn_l_bnds[cellCntr]) 
	    cell_fn_l_bnds[cellCntr] = fn_val;
	  if (fn_val > cell_fn_u_bnds[cellCntr])
	    cell_fn_u_bnds[cellCntr] = fn_val;
	}
      }
    }
#ifdef DEBUG
    for (i=0; i<numCells; i++) {
      Cout << "CMAX " <<i<< " is " << cell_fn_u_bnds[i] << '\n' ;
      Cout << "CMIN " <<i<< " is " << cell_fn_l_bnds[i] << '\n' ;
    }
#endif //DEBUG

    // replace with fortran free function to calculate CBF, CPF
    calculate_cbf_cpf();
  }

  compute_evidence_statistics();
}

} // namespace Dakota
