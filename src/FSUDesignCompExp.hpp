/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifndef FSU_DESIGN_COMP_EXP_H
#define FSU_DESIGN_COMP_EXP_H

#include "DakotaPStudyDACE.hpp"
#include "dakota_mersenne_twister.hpp"

namespace Dakota {

/// Wrapper class for the FSUDace QMC/CVT library.

/** The FSUDesignCompExp class provides a wrapper for FSUDace, a C++
    design of experiments library from Florida State University.  This
    class uses quasi Monte Carlo (QMC) and Centroidal Voronoi
    Tesselation (CVT) methods to uniformly sample the parameter space
    spanned by the active bounds of the current Model.  It returns all
    generated samples and their corresponding responses as well as the
    best sample found. */

class FSUDesignCompExp: public PStudyDACE
{
public:

  //
  //- Heading: Constructors and destructors
  //
    
  /// primary constructor for building a standard DACE iterator
  FSUDesignCompExp(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
  /// alternate constructor for building a DACE iterator on-the-fly
  FSUDesignCompExp(std::shared_ptr<Model> model, int samples, int seed,
		   unsigned short sampling_method);
  /// destructor
  ~FSUDesignCompExp() override;

  //
  //- Heading: Virtual function redefinitions
  //
  
  bool resize() override;
    
protected:

  //
  //- Heading: Virtual function redefinitions
  //
  
  void pre_run() override;
  void core_run() override;
  void post_input() override;
  void post_run(std::ostream& s) override;
  size_t num_samples() const override;
  void sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag) override;
  unsigned short sampling_scheme() const override;
  void vary_pattern(bool pattern_flag) override;
  void get_parameter_sets(std::shared_ptr<Model> model) override;
  void get_parameter_sets(std::shared_ptr<Model> model, const size_t num_samples, 
			  RealMatrix& design_matrix) override;

private:

  //
  //- Heading: Convenience functions
  //

  /// enforce sanity checks/modifications for the user input specification
  void enforce_input_rules();

  //
  //- Heading: Data
  //

  /// initial specification of number of samples
  int samplesSpec;
  /// current number of samples to be evaluated
  size_t numSamples;
  /// flag which triggers the update of allVars/allResponses for use by
  /// Iterator::all_variables() and Iterator::all_responses()
  bool allDataFlag;
  /// counter for number of executions for this object
  size_t numDACERuns;
  /// flag which specifies latinization of QMC or CVT sample sets
  bool latinizeFlag;

  // QMC data

  /// Integer vector defining a starting index into the sequence for random
  /// variable sampled.  Default is 0 0 0 (e.g. for three random variables).
  IntVector sequenceStart;
  /// Integer vector defining the leap number for each sequence being
  /// generated.  Default is 1 1 1 (e.g. for three random vars.)
  IntVector sequenceLeap;
  /// Integer vector defining the prime base for each sequence being
  /// generated.  Default is 2 3 5 (e.g., for three random vars.)
  IntVector primeBase;

  // CVT data

  // Using Boost MT since need it anyway for unif int dist
  /// RNG to persist for varying pattern
  boost::mt19937 rng;

  /// the user seed specification for the random number generator
  /// (allows repeatable results)
  int seedSpec;
  /// current seed for the random number generator
  int randomSeed;
  /// flag for continuing the random number or QMC sequence from a previous
  /// execution (e.g., for surrogate-based optimization) so that
  /// multiple executions are repeatable but not identical.
  bool varyPattern;
  /// specifies the number of sample points taken at internal CVT iteration
  int numCVTTrials;
  // initialization type in CVT.  Specifies where the initial points
  // are placed for consideration.  One of init_grid (2), init_halton (1),
  // init_uniform (1), or init_random (-1).  Default is init_random.
  //int initType;
  /// Trial type in CVT.  Specifies where the points are placed for
  /// consideration relative to the centroids.  Choices are grid (2),
  /// halton (1), uniform (0), or random (-1).  Default is random.
  int trialType;
  /// initialize statistical post processing
};


inline size_t FSUDesignCompExp::num_samples() const
{ return numSamples; }


inline void FSUDesignCompExp::
sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag)
{
  // allow sample reduction relative to previous sampling_reset() calls
  // (that is, numSamples may be increased or decreased by min_samples), but
  // not relative to the original specification (samplesSpec is a hard lower
  // bound).  maxEvalConcurrency must not be updated since parallel config
  // management depends on having the same value at ctor/run/dtor times.
  numSamples = (min_samples > samplesSpec) ? min_samples : samplesSpec;
  // note that previous value of numSamples is irrelevant: may increase or
  // decrease relative to previous value

  allDataFlag = all_data_flag;
  //statsFlag   = stats_flag; // currently no statsFlag in FSUDesignCompExp
}


inline unsigned short FSUDesignCompExp::sampling_scheme() const
{ return methodName; }


inline void FSUDesignCompExp::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }

} // namespace Dakota

#endif
