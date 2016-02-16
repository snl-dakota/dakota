/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       PSUADEDesignCompExp
//- Description: This class wraps the PSUADE DACE library.
//- Owner:       Brian M. Adams, Sandia National Laboratories


#ifndef PSUADE_DESIGN_COMP_EXP_H
#define PSUADE_DESIGN_COMP_EXP_H

#include "DakotaPStudyDACE.hpp"

namespace Dakota {

/// Wrapper class for the PSUADE library.

/** The PSUADEDesignCompExp class provides a wrapper for PSUADE, a C++
    design of experiments library from Lawrence Livermore National Laboratory.
    Currently this class only includes the PSUADE Morris One-at-a-time (MOAT)
    method to uniformly sample the parameter space spanned by the active bounds
    of the current Model.  It returns all generated samples and their 
    corresponding responses as well as the best sample found. */

class PSUADEDesignCompExp: public PStudyDACE
{
public:

  //
  //- Heading: Constructors and destructors
  //
    
  /// primary constructor for building a standard DACE iterator
  PSUADEDesignCompExp(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~PSUADEDesignCompExp();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();
    
protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void post_input();
  void core_run();
  void post_run(std::ostream& s);

  int num_samples() const;
  void sampling_reset(int min_samples, bool all_data_flag, bool stats_flag);
  unsigned short sampling_scheme() const;
  void vary_pattern(bool pattern_flag);
  void get_parameter_sets(Model& model);

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
  int numSamples;
  /// number of partitions in each variable direction
  const UShortArray& varPartitionsSpec;
  /// number of partitions to pass to PSUADE (levels = partitions + 1)
  int numPartitions;
  /// flag which triggers the update of allVars/allResponses for use by
  /// Iterator::all_variables() and Iterator::all_responses()
  bool allDataFlag;
  /// counter for number of executions for this object
  size_t numDACERuns;

  /// flag for generating a sequence of seed values within multiple
  /// get_parameter_sets() calls so that the sample sets are not repeated,
  /// but are still repeatable
  bool varyPattern;
  /// the user seed specification for the random number generator
  /// (allows repeatable results)
  const int seedSpec;
  /// current seed for the random number generator
  int randomSeed;

  // MOATSampling *psuadeSampler;
  // MOATAnalyzer *psuadeAnalyzer;

};


inline int PSUADEDesignCompExp::num_samples() const
{ return numSamples; }


inline void PSUADEDesignCompExp::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
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
  //statsFlag   = stats_flag; // currently no statsFlag in PSUADEDesignCompExp
}


inline unsigned short PSUADEDesignCompExp::sampling_scheme() const
{ return methodName; }


inline void PSUADEDesignCompExp::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }



} // namespace Dakota

#endif
