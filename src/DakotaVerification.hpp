/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

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
