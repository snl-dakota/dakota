/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EfficientSubspaceMethod
//- Description: Interface to Iterator for Abdel-Khalik's ESM
//- Owner:       Brian M. Adams

#ifndef EFFICIENT_SUBSPACE_METHOD_H
#define EFFICIENT_SUBSPACE_METHOD_H

#include "DakotaNonD.hpp"

namespace Dakota {

/// Efficient Subspace Method (ESM), as proposed by Hany S. Abdel-Khalik

/** ESM uses random sampling to construct a low-dimensional subspace
    of the full dimensional parameter space, then performs UQ in the
    reduced space */
class EfficientSubspaceMethod: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Standard, model-based constructor
  EfficientSubspaceMethod(ProblemDescDB& problem_db, Model& model);
  /// Destructor
  ~EfficientSubspaceMethod();

  //
  //- Heading: Virtual function redefinitions
  //

  /// specialization of init comms due to varied use of the original model
  void derived_init_communicators(ParLevLIter pl_iter);
  /// specialization of init comms due to varied use of the original model
  void derived_set_communicators(ParLevLIter pl_iter);
  /// specialization of free comms due to varied use of the original model
  void derived_free_communicators(ParLevLIter pl_iter);

  /// ESM re-implementation of the virtual UQ iterator function
  void quantify_uncertainty();

private:

  // Initialization methods 
  
  /// validate user-supplied input values, setting defaults, aborting on error
  void validate_inputs();

  /// initialize the native problem space Monte Carlo sampler
  void init_fullspace_sampler();


  // Methods used in the rank-revealing build phase

  /// generate fullspace samples, append to matrix, and factor,
  /// returning whether tolerance met
  void expand_basis(bool& svtol_met);

  /// determine the number of full space samples for next iteration,
  /// based on batchSize, limiting by remaining function evaluation
  /// budget
  unsigned int calculate_fullspace_samples();

  /// sample the derivative at diff_samples points and leave temporary
  /// in dace_iterator
  void generate_fullspace_samples(unsigned int diff_samples); 

  /// append the fullSpaceSampler samples to the derivative and vars matrices
  void append_sample_matrices(unsigned int diff_samples);

  /// factor the derivative matrix and analyze singular values,
  /// assessing convergence and rank, returning whether tolerance met
  void compute_svd(bool& svtol_met);

  /// print inner iteration stats after SVD
  void print_svd_stats();

  /// determine if the reduced basis yields acceptable reconstruction
  /// error, based on sampling in the orthogonal complement of the
  /// reduced basis
  void assess_reconstruction(bool& recon_tol_met);


  /// experimental method to demonstrate creating a RecastModel and
  /// perform sampling-based UQ in the reduced space
  void reduced_space_uq();


  // Helper methods

  /// translate the characterization of uncertain variables in the
  /// native_model to the reduced space of the transformed model
  void uncertain_vars_to_subspace(Model& native_model,
				  Model& vars_transform_model);

  /// map the active continuous recast variables to the active
  /// submodel variables (linear transformation)
  static void map_xi_to_x(const Variables& recast_xi_vars, 
			  Variables& sub_model_x_vars);
  
  
  // Data controlling iteration and status

  /// seed controlling all samplers
  int seedSpec;

  /// initial number of samples at which to query the truth model
  int initialSamples;

  /// number of points to add at each iteration
  int batchSize;

  /// number of UQ samples to perform in the reduced space
  int subspaceSamples;

  /// current iteration
  unsigned int currIter;

  /// total construction samples evaluated so far
  unsigned int totalSamples;

  /// total evaluations of model (accounting for UQ phase)
  unsigned int totalEvals;

  /// tolerance on singular value ratio, max of user-specified and macheps
  double SVTol;

  /// boolean flag to determine if reconstruction assessment is performed
  bool performAssessment;

  /// user-specified tolerance on nullspace 
  double nullspaceTol;

  /// current singular value ratio (sigma_k/sigma_0)
  double svRatio;

  /// Number of bootstrap samples for Bing Li criterion
  size_t numReplicates;


  // Data for numerical representation

  /// current approximation of system rank
  unsigned int reducedRank;

  /// basis for the reduced subspace
  RealMatrix reducedBasis;

  /// matrix of derivative data with numFunctions columns per fullspace sample;
  /// each column contains the gradient of one function at one sample point, 
  /// so total matrix size is numContinuousVars * (numFunctions * numSamples)
  /// [ D1 | D2 | ... | Dnum_samples]
  /// [ dy1/dx(k=1) | dy2/dx(k=1) | ... | dyM/dx(k=1) | k=2 | ... | k=n_s ]
  RealMatrix derivativeMatrix;

  /// matrix of fullspace variable points samples
  /// size numContinuousVars * (numSamples)
  RealMatrix varsMatrix;

  /// index for the active ParallelLevel within ParallelConfiguration::miPLIters
  size_t miPLIndex;

  // Helper members

  /// Monte Carlo sampler for the full parameter space
  Iterator fullSpaceSampler;

  /// instance of this class for use in static member functions
  static EfficientSubspaceMethod* esmInstance;

};

}

#endif  // EFFICIENT_SUBSPACE_METHOD_H
