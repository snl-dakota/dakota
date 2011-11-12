/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDStochCollocation
//- Description: Iterator for the stochastic collocation approach to UQ
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_STOCH_COLLOCATION_H
#define NOND_STOCH_COLLOCATION_H

#include "NonDExpansion.H"


namespace Dakota {


/// Nonintrusive stochastic collocation approaches to uncertainty
/// quantification

/** The NonDStochCollocation class uses a stochastic collocation (SC)
    approach to approximate the effect of parameter uncertainties on
    response functions of interest.  It utilizes the
    InterpPolyApproximation class to manage multidimensional Lagrange
    polynomial interpolants. */

class NonDStochCollocation: public NonDExpansion
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDStochCollocation(Model& model);    ///< standard constructor
  NonDStochCollocation(Model& model, short exp_coeffs_approach,
		       unsigned short num_int_level, short u_space_type,
		       bool use_derivs); ///< alternate constructor
  ~NonDStochCollocation();               ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  /// initialize the surrogate model
  void initialize_u_space_model();

  //void initialize_expansion();
  //void compute_expansion();

  // print the SC coefficient array using a monomial basis
  //void print_coefficients(std::ostream& s);

private:

  //
  //- Heading: Data
  //
};

} // namespace Dakota

#endif
