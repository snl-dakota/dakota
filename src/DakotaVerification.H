/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Verification
//- Description: Base class for RichExtrapVerification, ...
//- Owner:       Mike Eldred
//- Version: $Id: DakotaVerification.H 6884 2010-07-31 02:47:28Z mseldre $

#ifndef DAKOTA_VERIFICATION_H
#define DAKOTA_VERIFICATION_H

#include "DakotaAnalyzer.H"

namespace Dakota {


/// Base class for managing common aspects of verification studies.

/** The Verification base class manages common data and functions,
    such as those involving ... */

class Verification: public Analyzer
{
public:

protected:

  //
  //- Heading: Constructors and destructors
  //

  /// constructor
  Verification(Model& model);
  /// alternate constructor for instantiations "on the fly"
  Verification(NoDBBaseConstructor, Model& model);
  /// destructor
  ~Verification();
    
  //
  //- Heading: Virtual member function redefinitions
  //

  void run();
  void print_results(std::ostream& s);

  //
  //- Heading: New virtual member functions
  //

  /// Redefines the run_iterator virtual function for the PStudy/DACE branch.
  virtual void perform_verification() = 0;

  //
  //- Heading: Member functions
  //

protected:

  //
  //- Heading: Data
  //

private:

  //
  //- Heading: Data
  //

};


inline Verification::~Verification() { }


inline void Verification::run()
{ perform_verification(); }

} // namespace Dakota

#endif
