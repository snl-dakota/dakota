/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DDACEDesignCompExp
//- Description: This class uses Design and Analysis of Computer Experiments
//-              (DACE) methods to sample the design space spanned by the
//-              Model.  It returns all generated samples and their
//-              corresponding responses as well as the best sample found.
//- Owner:       Tony Giunta, Sandia National Laboratories


#ifndef DDACE_DESIGN_COMP_EXP_H
#define DDACE_DESIGN_COMP_EXP_H

#include "DakotaPStudyDACE.hpp"

class DDaceSamplerBase;
class DDaceSamplePoint;

namespace Dakota {

/// Wrapper class for the DDACE design of experiments library.

/** The DDACEDesignCompExp class provides a wrapper for DDACE, a C++
    design of experiments library from the Computational Sciences and
    Mathematics Research (CSMR) department at Sandia's Livermore CA
    site.  This class uses design and analysis of computer experiments
    (DACE) methods to sample the design space spanned by the bounds of
    a Model.  It returns all generated samples and their corresponding
    responses as well as the best sample found. */

class DDACEDesignCompExp: public PStudyDACE
{
public:

  //
  //- Heading: Constructors and destructors
  //
    
  /// primary constructor for building a standard DACE iterator
  DDACEDesignCompExp(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor used for building approximations
  DDACEDesignCompExp(Model& model, int samples, int symbols, int seed, 
		     unsigned short sampling_method);
  /// destructor
  ~DDACEDesignCompExp();
  
  //
  //- Heading: Virtual member function redefinitions
  //

  bool resize();
    
protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  void pre_run();
  void core_run();
  void post_input();
  void post_run(std::ostream& s);
  int num_samples() const;
  void sampling_reset(int min_samples, bool all_data_flag, bool stats_flag);
  unsigned short sampling_scheme() const;
  void vary_pattern(bool pattern_flag);
  void get_parameter_sets(Model& model);
  void get_parameter_sets(Model& model, const int num_samples, 
			  RealMatrix& design_matrix);
private:

  //
  //- Heading: Member functions
  //

  /// builds a DDaceMainEffects::OneWayANOVA if mainEffectsFlag is set
  void compute_main_effects();

  /// create a DDACE sampler
  boost::shared_ptr<DDaceSamplerBase> create_sampler(Model& model);

  /// convenience function for resolving number of samples and
  /// number of symbols from input.
  void resolve_samples_symbols();


  //
  //- Heading: Data
  //

  /// oas, lhs, oa_lhs, random, box_behnken, central_composite, or grid
  unsigned short daceMethod;
  /// initial specification of number of samples
  int samplesSpec;
  /// initial specification of number of symbols
  int symbolsSpec;
  /// current number of samples to be evaluated
  int numSamples;
  /// current number of symbols to be used in generating the sample set
  /// (inversely related to number of replications)
  int numSymbols;
  /// the user seed specification for the random number generator
  /// (allows repeatable results)
  const int seedSpec;
  /// current seed for the random number generator
  int randomSeed;
  /// flag which triggers the update of allVars/allResponses for use by
  /// Iterator::all_variables() and Iterator::all_responses()
  bool allDataFlag;
  /// counter for number of executions for this object
  size_t numDACERuns;
  /// flag for continuing the random number sequence from a previous
  /// execution (e.g., for surrogate-based optimization) so that
  /// multiple executions are repeatable but not correlated.
  bool varyPattern;
  /// flag which specifies main effects
  bool mainEffectsFlag;
  /// mapping of symbols for main effects calculations 
  std::vector<std::vector<int> > symbolMapping;
};


inline int DDACEDesignCompExp::num_samples() const
{ return numSamples; }


inline void DDACEDesignCompExp::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  // allow sample reduction relative to previous sampling_reset() calls
  // (that is, numSamples may be increased or decreased to match min_samples),
  // but not relative to the original user specification (samplesSpec is a hard
  // lower bound).  maxEvalConcurrency must not be updated since parallel config
  // management depends on having the same value at ctor/run/dtor times.
  if (min_samples > samplesSpec) {
    // set numSymbols=numSamples or resolve_samples_symbols() can require more
    // than the min number of samples (e.g., in resolving settings for LHS).
    // At this point, numSamples is a user/surrogate requirement, which may be
    // updated in resolve_samples_symbols().
    numSymbols = numSamples = min_samples;
  }
  else { // reset (may have been updated previously)
    numSamples = samplesSpec;
    numSymbols = symbolsSpec;
  }

  allDataFlag = all_data_flag;
  //statsFlag = stats_flag; // currently no statsFlag in DDACEDesignCompExp
}


inline unsigned short DDACEDesignCompExp::sampling_scheme() const
{ return daceMethod; }


inline void DDACEDesignCompExp::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }

} // namespace Dakota

#endif
