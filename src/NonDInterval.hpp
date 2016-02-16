/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDInterval
//- Description: Class for the Interval theory methods within DAKOTA/UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_INTERVAL_H
#define NOND_INTERVAL_H

#include "dakota_data_types.hpp"
#include "DakotaNonD.hpp"

namespace Dakota {


/// Base class for interval-based methods within DAKOTA/UQ

/** The NonDInterval class implements the propagation of epistemic
    uncertainty using either pure interval propagation or
    Dempster-Shafer theory of evidence. In the latter approach, one
    assigns a set of basic probability assignments (BPA) to intervals
    defined for the uncertain variables.  Input interval combinations
    are calculated, along with their BPA.  Currently, the response
    function is evaluated at a set of sample points, then a response
    surface is constructed which is sampled extensively to find the
    minimum and maximum within each input interval cell, corresponding
    to the belief and plausibility within that cell, respectively.
    This data is then aggregated to calculate cumulative distribution
    functions for belief and plausibility. */

class NonDInterval: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDInterval(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDInterval();                                       ///< destructor

  //
  //- Heading: Virtual member function redefinitions
  //

  // performs an epistemic uncertainty propagation using interval propagation
  //void core_run();

  /// print the cumulative distribution functions for belief and plausibility
  void print_results(std::ostream& s);

  //
  //- Heading: Virtual member function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  /// initialize finalStatistics for belief/plausibility results sets
  void initialize_final_statistics();

  /// method for computing belief and plausibility values for response levels 
  /// or vice-versa
  void compute_evidence_statistics();

  //
  //- Heading: Convenience functions
  //

  /// computes the interval combinations (cells) and their bpas
  /// replaces CBPIIC_F77 from wrapper calculate_basic_prob_intervals()
  void calculate_cells_and_bpas();

  /// function to compute (complementary) distribution functions on belief and
  /// plausibility replaces CCBFPF_F77 from wrapper calculate_cum_belief_plaus()
  void calculate_cbf_cpf(bool complementary = true);
  
  //
  //- Heading: Data
  //

  /// flag for SingleInterval derived class
  bool singleIntervalFlag;

  /// Storage array to hold CCBF values
  RealVectorArray ccBelFn;
  /// Storage array to hold CCPF values
  RealVectorArray ccPlausFn;
  /// Storage array to hold CCB response values
  RealVectorArray ccBelVal;
  /// Storage array to hold CCP response values
  RealVectorArray ccPlausVal;

  /// Storage array to hold cell lower bounds for continuous variables
  RealVectorArray cellContLowerBounds;
  /// Storage array to hold cell upper bounds for continuous variables
  RealVectorArray cellContUpperBounds;
  /// Storage array to hold cell lower bounds for discrete int range variables
  IntVectorArray cellIntRangeLowerBounds;
  /// Storage array to hold cell upper bounds for discrete int range variables
  IntVectorArray cellIntRangeUpperBounds;
  /// Storage array to hold cell values for discrete integer set variables
  IntVectorArray cellIntSetBounds;
  /// Storage array to hold cell value for discrete real set variables
  IntVectorArray cellRealSetBounds;
  /// Storage array to hold cell min
  RealVectorArray cellFnLowerBounds;
  /// Storage array to hold cell max
  RealVectorArray cellFnUpperBounds;
  /// Storage array to hold cell bpa
  RealVector cellBPA;

  /// response function counter
  size_t respFnCntr;
  /// cell counter
  size_t cellCntr;
  /// total number of interval combinations
  size_t numCells;	
};

} // namespace Dakota

#endif
