/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDExpDesignBayesCalibration
//- Description: Derived class for Adaptive Experimental Design in Bayesian inference
//- Owner:       Laura Swiler and Kathryn Maupin
//- Checked by:
//- Version:

#include "NonDExpDesignBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "ProbabilityTransformation.hpp"
#include "NonDLHSSampling.hpp"
#include "ExperimentData.hpp"

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDExpDesignBayesCalibration::
NonDExpDesignBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  initSamples(problem_db.get_int("method.samples"))
{ 
  const String& lowFidModelPtr = problem_db.get_string("method.low_fidelity_model_pointer");
  if (lowFidModelPtr.empty()){
    Cerr << "Error: Bayesian Experimental Design requires a low and high " 
         << "fidelity model pointer." << std::endl;
    abort_handler(METHOD_ERROR);
   } 
}



NonDExpDesignBayesCalibration::~NonDExpDesignBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDExpDesignBayesCalibration::core_run()
{
  //NOTE:  Iterated Model is the high fidelity model which is considered the truth model

  Iterator lhs_iterator;
  NonDLHSSampling* lhs_sampler_rep;

  // The following uses on the fly derived ctor:
  unsigned short sample_type = SUBMETHOD_LHS;
  bool vary_pattern = true;
  String rng("mt19937");
  lhs_sampler_rep = new NonDLHSSampling(iteratedModel, sample_type,
             initSamples, randomSeed, rng, vary_pattern, ACTIVE_UNIFORM);
  lhs_iterator.assign_rep(lhs_sampler_rep, false);
  
  lhs_iterator.run(); 
  const IntResponseMap& all_responses = lhs_iterator.all_responses();

  ExperimentData exp_data(initSamples,all_responses);
 
  // now go into loop with QUESO, generate posterior on theta, and 
  // calculate mutual information 

}

void NonDExpDesignBayesCalibration::
extract_selected_posterior_samples(const std::vector<int> &points_to_keep,
				   const RealMatrix &samples_for_posterior_eval,
				   const RealVector &posterior_density,
				   RealMatrix &posterior_data ) const {
}

void NonDExpDesignBayesCalibration::
export_posterior_samples_to_file( const std::string filename, 
				  const RealMatrix &posterior_data ) const{
}

void NonDExpDesignBayesCalibration::print_results(std::ostream& s)
{
  NonDBayesCalibration::print_results(s);

}


} // namespace Dakota
