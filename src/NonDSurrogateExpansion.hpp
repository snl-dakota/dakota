/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_SURROGATE_EXPANSION_H
#define NOND_SURROGATE_EXPANSION_H

#include "NonDExpansion.hpp"

namespace Dakota {


/// Generic uncertainty quantification with Model-based stochastic expansions

/** The NonDSurrogateExpansion class leverages a Model specification for
    stochastic expansions (PCE, SC, FT) to build a stochastic emulator and
    then queries the emulator to generate the set of requested statistics. */

class NonDSurrogateExpansion: public NonDExpansion
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDSurrogateExpansion(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDSurrogateExpansion();

  //
  //- Heading: Virtual function redefinitions
  //

  // perform a forward uncertainty propagation using PCE/SC/FT DataFitSurrModel
  //void core_run();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void print_results(std::ostream&);

  //
  //- Heading: Member function definitions
  //

private:

  //
  //- Heading: Data
  //

};
    
} // namespace Dakota

#endif
