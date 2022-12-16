/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGenACVSampling
//- Description: Class for generalized approximate control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_GEN_ACV_SAMPLING_H
#define NOND_GEN_ACV_SAMPLING_H

#include "NonDACVSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {


/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Generalized versions of Approximate Control Variate (ACV) that
    enumerate different model graphs (CV inter-relationships) and
    different model memberships. (Bomarito et al., 2022, JCP) */

class NonDGenACVSampling: public NonDACVSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDGenACVSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDGenACVSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  //void print_variance_reduction(std::ostream& s);

  //
  //- Heading: member functions
  //

  void generalized_acv_online_pilot();
  void generalized_acv_offline_pilot();
  void generalized_acv_pilot_projection();

private:

  //
  //- Heading: Helper functions
  //

  void generate_dags(UShortArraySet& model_graphs);

  void compute_parameterized_G_g(const RealVector& N_vec,
				 const UShortArray& dag, RealMatrix& G,
				 RealVector& g);

  //
  //- Heading: Data
  //

  /// type of tunable recursion for defining set of DAGs: KL, SR, or MR
  short dagRecursionType;
};

} // namespace Dakota

#endif
