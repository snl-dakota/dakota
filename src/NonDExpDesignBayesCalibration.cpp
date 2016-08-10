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
  for (size_t i=0; i<initSamples; i++)
    Cout << "Exp Data  i " << i << " value = " << exp_data.all_data(i);

  Iterator queso_iterator;
  NonDQUESOBayesCalibration* queso_rep;
  
  // need to initialize this from user input eventually
  size_t num_candidates = 100, num_mcmc_samples=1000; 

  bool stop_metric = false;
  double max_MI = 0; 

  while (!stop_metric) {
    // need to determine if we construct QUESO every time we go through the loop or not
    //queso_rep = new NonDQUESOBayesCalibration(prior, exp_data, options);
    //queso_iterator.assign_rep(queso_rep,false);
    //queso_iterator.run();

    // after QUESO is run, get the posterior values of the samples
    // currently, it looks like acceptedFnVals is a protected data member within NonDBayesCalibration
    // We will need to expose this by adding a function such as post_sample() shown below
    // RealMatrix posterior_theta = queso_iterator->post_sample();  
     
    // go through all the designs and pick the one with maximum mutual information
    for (size_t i=0; i<num_candidates; i++) {
      // Get the lowFidModelPtr that we initialized from the input file in the constructor,
      // initialize the low fidelity model.  
      // Declare a matrix to store the low fidelity responses
      /*
      RealMatrix responses_low(num_responses, num_mcmc_samples);
      RealVector col_vec(num_theta + num_responses);
      RealVector low_fid_response(num_responses);
      */
      for (size_t j=0; j<num_mcmc_samples; j++) {
        // for each posterior sample, get the variable values, and run the model
        // low_fid_model_vars = posterior_theta(j,:); 
        // low_fid_model_vars = Teuchos::getCol(Teuchos::View,posterior_theta,j); 
        // lowFidModel.evaluate();
        // responses_low(j,:)  = lowFidModel.current_responses().function_values();
	/*
	low_fid_response = lowFidModel.current_responses().function_values();
	Teuchos::setCol(low_fid_response, j, responses_low);
	*/
      }
      // now concatenate posterior_theta and responses_low into Xmatrix
      /*
      for (size_t k = 0; k < num_theta; k++){
        col_vec[k] = low_fid_model_vars[k];
      }
      for (k = 0; k < num_responses; k ++){
        col_vec[num_theta+k] = low_fid_response[k];
      }
      Teuchos::setCol(col_vec, j, Xmatrix);
      */
      // calculate the mutual information with posterior_theta and responses_low matrices
      // MI = queso_iterator.knn_mutual_info(Xmatrix, num_theta, num_responses);
      
      // Now track max MI:
      // if ( MI > max_MI) { 
      //   max_MI = MI; 
      //   design_new = design_i;
      //}
    } // end for over the number of candidates

    // evaluate hi fidelity iteratedModel at design_i;
    // Add this data to the expData for the next iteteration of likelihood
    // check stopping metric (may just start with doing this 5 times?
    stop_metric = true;
  } // end while loop

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
