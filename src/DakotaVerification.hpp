/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Verification
//- Description: Base class for RichExtrapVerification, ...
//- Owner:       Mike Eldred
//- Version: $Id: DakotaVerification.hpp 6884 2010-07-31 02:47:28Z mseldre $

#ifndef DAKOTA_VERIFICATION_H
#define DAKOTA_VERIFICATION_H

#include "DakotaAnalyzer.hpp"

namespace Dakota {


/// Base class for managing common aspects of verification studies.

/** The Verification base class manages common data and functions,
    such as those involving ... */

class Verification: public Analyzer
{
public:
    
  //
  //- Heading: Virtual member function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors and destructors
  //

  /// constructor
  Verification(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for instantiations "on the fly"
  Verification(unsigned short method_name, Model& model);
  /// destructor
  ~Verification();
    
  //
  //- Heading: Virtual member function redefinitions
  //

  void print_results(std::ostream& s);

  //
  //- Heading: Member functions
  //

private:

  //
  //- Heading: Data
  //

};


inline Verification::~Verification() { }

} // namespace Dakota

#endif
