/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonLHSDEvidence
//- Description: Implementation code for NonDEvidence class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDLHSEvidence.H"
#include "data_types.h"
#include "system_defs.h"

//#define DEBUG

namespace Dakota {


NonDLHSEvidence::NonDLHSEvidence(Model& model): NonDLHSInterval(model)
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

  const RealMatrix&     all_samples   = lhsSampler.all_samples();
  const IntResponseMap& all_responses = lhsSampler.all_responses();
  
  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {
    // Use the max and mins to determine the cumulative distributions
    // of plausibility and belief
    RealArray& cell_fn_l_bnds = cellFnLowerBounds[respFnCntr];
    RealArray& cell_fn_u_bnds = cellFnUpperBounds[respFnCntr];
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
      const Real* c_vars = all_samples[i]; // column vector

      int num_total_vars = numContIntervalVars+numDiscIntervalVars+
                           numDiscSetIntUncVars+numDiscSetRealUncVars;
      RealVector in_cell(num_total_vars);
      Real total_incell;
      //for (j=0; j< num_total_vars; j++) 
      //  Cout << "cvars " << c_vars[j] << "\n";
        
      for (cellCntr=0; cellCntr<numCells; ++cellCntr) {
	total_incell = 1;
	j=0;
	const RealVector& cell_l_bnds = cellLowerBounds[cellCntr];
	const RealVector& cell_u_bnds = cellUpperBounds[cellCntr];
        //Cout << "cell l bounds " << cell_l_bnds << "\n";
        //Cout << "cell u bounds " << cell_u_bnds << "\n";
        // for now, treat ContIntervalVars and DiscInterval vars as the same (real bounds
        // but check for equality of the discrete set types when calculating 
        // number in a cell.  Note we need to redo getting all the samples in a RealMatrix 
        while (total_incell && j<num_total_vars) {
	  in_cell[j]=0;
	  if (cell_l_bnds[j] <= c_vars[j] && c_vars[j] <= cell_u_bnds[j]) 
	      in_cell[j] = 1;
	  total_incell = in_cell[j]*total_incell;
	  ++j;
	}

	if (total_incell == 1) {
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
