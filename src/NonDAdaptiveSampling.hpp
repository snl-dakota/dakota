/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDAdaptiveSampling
//- Description: Wrapper class for Morse-Smale Topological Importance Sampling
//- Owner:       Dan Maljovec
//- Checked by:  Brian ADams
//- Version:

#ifndef NOND_ADAPTIVE_SAMPLING_H
#define NOND_ADAPTIVE_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"

// forward declaration so class size doesn't change conditionally
class MS_Complex;

namespace Dakota {

/// Class for testing various Adaptively sampling methods using geometric,
/// statisctical, and topological information of the surrogate

/** NonDAdaptiveSampling implements an adaptive sampling method based
    on the work presented in Adaptive Sampling with Topological Scores
    by Dan Maljovec, Bei Wang, Ana Kupresanin, Gardar Johannesson,
    Valerio Pascucci, and Peer-Timo Bremer presented in IJUQ (insert
    issue). The method computes scores based on the topology of the
    known data and the topology of the surrogate model.  A number of
    alternate adaption strategies are offered as well. */
class NonDAdaptiveSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDAdaptiveSampling(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for sample generation and evaluation "on the fly"
  /// has not been implemented

  ~NonDAdaptiveSampling(); ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void core_run();
  Real final_probability();
  void print_results(std::ostream& s);

private:

  //
  //- Heading: Data
  //

  /// LHS iterator for building the initial GP
  Iterator gpBuild;
  /// LHS iterator for sampling on the GP
  Iterator gpEval;
  /// LHS iterator for sampling on the final GP
  Iterator gpFinalEval;
  /// GP model of response, one approximation per response function
  Model gpModel;
  
  /// the number of rounds of additions of size batchSize to add  to the 
  /// original set of LHS samples
  int numRounds;
  /// the total number of points
  int numPtsTotal;
  /// the number of points evaluated by the GP each iteration
  int numEmulEval;
  /// number of points evaluated on the final GP
  int numFinalEmulEval;
  /// the type of scoring metric to use for sampling
  int scoringMethod;
  /// the final calculated probability (p)
  Real finalProb;
  /// Vector to hold the current values of the current sample inputs on the GP
  RealVectorArray gpCvars;
  /// Vector to hold the current values of the current mean estimates 
  /// for the sample values on the GP
  RealVectorArray gpMeans;
  /// Vector to hold the current values of the current variance estimates 
  /// for the sample values on the GP
  RealVectorArray gpVar;
  /// Vector to hold the scored values for the current GP samples
  RealVector emulEvalScores; 
  /// Vector to hold the RMSE after each round of adaptively fitting the model
  RealVector predictionErrors;

  /// Validation point set used to determine predictionErrors above
  RealVectorArray validationSet;
  /// True function responses at the values corresponding to validationSet
  RealVector yTrue;
  /// Surrogate function responses at the values corresponding to validationSet
  RealVector yModel;

  /// Number of points used in the validationSet
  int validationSetSize;
  /// Number of points to add each round, default = 1
  int batchSize;
  /// String describing the tpye of batch addition to use.  Allowable values are
  /// naive, distance, topology
  String batchStrategy;
  /// Temporary string for dumping validation files used in TopoAS visualization
  String outputDir;
  /// String describing the method for scoring candidate points. Options are:
  /// alm, distance, gradient, highest_persistence, avg_persistence, bottleneck,
  /// alm_topo_hybrid
  /// Note: alm and alm_topo_hybrid will fail when used with surrogates other
  /// than global_kriging as it is based on the variance of the surrogate.
  /// At the time of implementation, global_kriging is the only surrogate
  /// capable of yielding this information 
  String scoringMetric;
  /// enum describing the initial sample design. Options are:
  /// RANDOM_SAMPLING, FSU_CVT, FSU_HALTON, FSU_HAMMERSLEY
  unsigned short sampleDesign;
  /// String describing type of surrogate is used to fit the data. Options are:
  /// global_kriging, global_mars, global_neural_network, global_polynomial,
  /// globabl_moving_least_squares, global_radial_basis
  String approx_type;
                    
  /// Function to compute the ALM scores for the candidate points
  /// ALM score is the variance computed by the surrogate at the point
  void calc_score_alm( );
  /// Function to compute the Distance scores for the candidate points
  /// Distance score is the shortest distance between the candidate and an 
  /// existing training point
  void calc_score_delta_x( );
  /// Function to compute the Gradient scores for the candidate points
  /// Gradient score is the function value difference between a candidate's
  /// surrogate response and its nearest evaluated true response from the
  /// training set
  void calc_score_delta_y( );
  /// Function to compute the Bottleneck scores for the candidate points
  /// Bottleneck score is computed by determining the bottleneck distance
  /// between the persistence diagrams of two approximate Morse-Smale complices.
  /// The complices used include one built from only the training data, and
  /// another built from the training data and the single candidate
  void calc_score_topo_bottleneck( );
  /// Function to compute the Average Change in Persistence scores for the 
  /// candidate points
  /// Avg_Persistence score is computed as the average change in persistence
  /// each point undergoes between two approximate Morse-Smale complices.
  /// The complices used include one built from only the training data, and
  /// another built from the training data and the single candidate
  void calc_score_topo_avg_persistence(int respFnCount);
  /// Function to compute the Highest Persistence scores for the candidate 
  /// points
  /// Highest Persistence score is calculated as a ranking of a set of
  /// candidates by constructing an approximate Morse-Smale complex over the 
  /// entire set of candidates, using their surrogate responses, and the 
  /// training data, using their true responses, and ranking points based on the
  /// most topological significance as measured by their persistence values.
  /// In the case where there are no topologically significant points, the
  /// point will be chosen randomly
  /// TODO: It may be wiser to fall back to a scheme that ranks points based
  /// on proximity to extrema, or the most significant extream?
  void calc_score_topo_highest_persistence(int respFnCount);
  /// Function to comptue the Hybrid scores for the candidate points
  /// Hybrid score is computed the same as Avg_Persistence score except that 
  /// instead of computing one score, three scores are computing not only a mean
  /// surface, but a mean +/- std. dev. surfaces and then averaging the three
  /// separate scores.  The hope is that you strike a balance between selecting 
  /// points in topologically important areas and areas of high uncertainty
  void calc_score_topo_alm_hybrid(int respFnCount);

  /// Same as the otehr function of the same name, only this allows the user to
  /// specify the location of the candidate
  Real calc_score_alm(int respFnCount, RealVector &test_point);
  /// Same as the otehr function of the same name, only this allows the user to
  /// specify the location of the candidate
  Real calc_score_delta_x(int respFnCount, RealVector &test_point);
  /// Same as the otehr function of the same name, only this allows the user to
  /// specify the location of the candidate
  Real calc_score_delta_y(int respFnCount, RealVector &test_point);
  /// Same as the otehr function of the same name, only this allows the user to
  /// specify the location of the candidate
  Real calc_score_topo_bottleneck(int respFnCount, RealVector &test_point);
  /// Same as the otehr function of the same name, only this allows the user to
  /// specify the location of the candidate
  Real calc_score_topo_avg_persistence(int respFnCount, RealVector &test_point);
  /// Same as the otehr function of the same name, only this allows the user to
  /// specify the location of the candidate
  Real calc_score_topo_alm_hybrid(int respFnCount, RealVector &test_point);

  /// Using the validationSet, compute the RMSE over the surface
  Real compute_rmspe();
  /// Using the validationSet, compute the approximate Morse-Smale complices 
  /// of the true model over the validationSet as well as the surrogate model
  /// over the validationSet, and output some topological comparisons
  void compare_complices(int dim, std::ostream& output);
  /// Parse misc_options specified in a user input deck
  void parse_options();

  /// function to pick the next X value to be evaluated by the Iterated model
  RealVectorArray drawNewX(int this_k, int respFnCount=0);

  /// Temporary function for dumping validation data to output files to be 
  /// visualized in TopoAS
  void output_round_data(int round, int respFnCount=0);

  /// Update the approximate Morse-Smale complex based on the training points
  /// and selected candidates.  Uses surrogate function responses
  void update_amsc(int respFnCount=0);

  /// The approximate Morse-Smale complex data structure
  MS_Complex *AMSC;
  /// The number of approximate nearest neighbors to use in computing the AMSC
  int numKneighbors;
  /// Temporary variable for toggling writing of data files to be used by TopoAS
  bool outputValidationData;

  /// Copy of construct_lhs only it allows for the construction of FSU sample
  /// designs.  This can break the fsu_cvt, so it is not used at the moment,
  /// and these designs only affect the initial sample build not the candidate
  /// sets constructed at each round
  void construct_fsu_sampler(Iterator& u_space_sampler, Model& u_model, 
    int num_samples, int seed, unsigned short sample_type);

  /// This function will write an input deck for a multi-start global 
  /// optimization run of DAKOTA by extracting all of the local minima 
  /// off the approximate Morse-Smale complex created from the validation set
  /// of the surrogate model
  void output_for_optimization(int dim);

  /// compute the median of the sorted values passed in
  Real median(const RealVector& sorted_data);

  /// Pick new candidates from Emulator
  void pick_new_candidates( );

  /// Score New candidates based on the chosen metrics
  void score_new_candidates();
};


inline Real NonDAdaptiveSampling::final_probability()
{ return finalProb; }


inline Real NonDAdaptiveSampling::median(const RealVector& sorted_data)
{
  int num_vals = sorted_data.length();
  if (num_vals % 2 == 0) {
    // even length: average of middle values
    int middle_right = num_vals / 2;
    return 0.5*(sorted_data(middle_right-1) + sorted_data(middle_right));
  }
  // odd length: middle value
  return sorted_data((num_vals-1)/2);
}


} // namespace Dakota

#endif  // NOND_ADAPTIVE_SAMPLING_H
