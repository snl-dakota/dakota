/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       PStudyDACE
//- Description: Base class for ParamStudy, DDACEDesignCompExp, and
//-              FSUDesignCompExp
//- Owner:       Mike Eldred
//- Version: $Id: DakotaPStudyDACE.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef DAKOTA_PSTUDY_DACE_H
#define DAKOTA_PSTUDY_DACE_H

#include "DakotaAnalyzer.hpp"
#include "SensAnalysisGlobal.hpp"

namespace Dakota {

/// Base class for managing common aspects of parameter studies and
/// design of experiments methods.

/** The PStudyDACE base class manages common data and functions,
    such as those involving the best solutions located during the
    parameter set evaluations or the printing of final results. */

class PStudyDACE: public Analyzer
{
public:

protected:

  //
  //- Heading: Constructors and destructors
  //

  /// constructor
  PStudyDACE(Model& model);
  /// alternate constructor for instantiations "on the fly"
  PStudyDACE(NoDBBaseConstructor, Model& model);
  /// destructor
  ~PStudyDACE();
    
  //
  //- Heading: Virtual member function redefinitions
  //

  void run();
  void print_results(std::ostream& s);

  //
  //- Heading: New virtual member functions
  //

  /// Redefines the run_iterator virtual function for the PStudy/DACE branch.
  virtual void extract_trends() = 0;

  //
  //- Heading: Member functions
  //

  /// Calculation of volumetric quality measures
  void volumetric_quality(int ndim, int num_samples, double* sample_points);

  //
  //- Heading: Data
  //

  /// initialize statistical post processing
  SensAnalysisGlobal pStudyDACESensGlobal;

  /// flag which specifies evaluation of volumetric quality measures
  bool volQualityFlag;
  /// flag which specifies calculating variance based decomposition 
  /// sensitivity analysis metrics
  bool varBasedDecompFlag;

private:

  //
  //- Heading: Data
  //

  /// quality measure
  double chiMeas;
  /// quality measure
  double dMeas;
  /// quality measure
  double hMeas;
  /// quality measure
  double tauMeas;
};


inline void PStudyDACE::run()
{ bestVarsRespMap.clear(); extract_trends(); }

} // namespace Dakota

#endif
