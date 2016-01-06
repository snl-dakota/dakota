/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       RichExtrapVerification
//- Description: Class for Richardson extrapolation for code and solution
//-              verification.   
//- Owner:       Mike Eldred/Ben Pax
//- Version: $Id: RichExtrapVerification.hpp 6972 2010-09-17 22:18:50Z briadam $

#ifndef RICH_EXTRAP_VERIFICATION_H
#define RICH_EXTRAP_VERIFICATION_H

#include "DakotaVerification.hpp"


namespace Dakota {


/// Class for Richardson extrapolation for code and solution verification

/** The RichExtrapVerification class contains several algorithms for
    performing Richardson extrapolation. */

class RichExtrapVerification: public Verification
{
public:

  //
  //- Heading: Constructors and destructors
  //

  /// constructor
  RichExtrapVerification(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~RichExtrapVerification();
    
  //
  //- Heading: Virtual member function redefinitions
  //

  //void initialize_run();
  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void finalize_run();
  void print_results(std::ostream& s);

private:

  //
  //- Heading: Convenience/internal member functions
  //

  /// perform a single estimation of convOrder using extrapolation()
  void estimate_order();
  /// iterate using extrapolation() until convOrder stabilizes
  void converge_order();
  /// iterate using extrapolation() until QOIs stabilize
  void converge_qoi();
  /// estimate convOrder from refinement and quantity of interest (QOI) triples
  void extrapolation(const RealVector& refine_triple, RealMatrix& qoi_triples);
  /// predict the converged value based on the convergence rate and
  /// the value of Phi
  void extrapolate_result(const RealVector& refine_triple,
			  const RealMatrix& qoi_triples);

  //
  //- Heading: Data
  //

  /// internal code for extrapolation study type:
  /// SUBMETHOD_{CONVERGE_ORDER,CONVERGE_QOI,ESTIMATE_ORDER}
  unsigned short studyType;

  /// number of refinement factors defined from active state variables
  size_t numFactors;
  /// initial reference values for refinement factors
  RealVector initialCVars;
  /// the index of the active factor
  size_t factorIndex;
  // total number of extrapolation study evaluations
  //size_t numEvals;
  /// rate of mesh refinement (default = 2.)
  Real refinementRate;

  /// the orders of convergence of the QOIs (numFunctions by numFactors)
  RealMatrix convOrder;
  /// the extrapolated value of the QOI (numFunctions by numFactors)
  RealMatrix extrapQOI;
  /// the numerical uncertainty associated with level of refinement
  /// (numFunctions by numFactors)
  RealMatrix numErrorQOI;
  /// This is a reference point reported for the converged extrapQOI
  /// and numErrorQOI.  It currently corresponds to the coarsest mesh
  /// in the final refinement triple.
  RealVector refinementRefPt;
};


inline RichExtrapVerification::~RichExtrapVerification()
{}

} // namespace Dakota

#endif
