/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedC3ApproxData
//- Description:  Base Class for C3 aprpoximation interface
//-               
//- Owner:        Alex Gorodetsky

#ifndef SHARED_C3_APPROX_DATA_H
#define SHARED_C3_APPROX_DATA_H

#include "SharedApproxData.hpp"
#include "SharedRegressOrthogPolyApproxData.hpp"
#include "dakota_c3_include.hpp"

namespace Dakota {


/// Derived approximation class for global basis polynomials.

/** The SharedC3ApproxData class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */

class SharedC3ApproxData: public SharedApproxData
{
  //
  //- Heading: Friends
  //

  friend class C3Approximation;

public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SharedC3ApproxData();
  /// standard ProblemDescDB-driven constructor
  SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars);
  /// on-the-fly constructor (no problem DB)
  SharedC3ApproxData(const String& approx_type,
                     const UShortArray& approx_order, size_t num_vars,
                     short data_order, short output_level);
  /// destructor
  ~SharedC3ApproxData();

  //
  //- Heading: Member functions
  //

  void store(size_t);
  void restore(size_t);
  void remove_stored(size_t);
  size_t pre_combine(short);
  void post_combine(short);

  void construct_basis(const Pecos::MultivariateDistribution& u_dist);

  void set_parameter(String var, void * val);

  // set SharedOrthogPolyApproxData::basisTypes
  //void basis_types(const ShortArray& basis_types);
  // get SharedOrthogPolyApproxData::basisTypes
  //const ShortArray& basis_types() const;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // Activate as additional features (and basis types) come online
  
  //void update_basis_distribution_parameters(        // beyond STD_{NORMAL,
  //  const Pecos::MultivariateDistribution& u_dist); //             UNIFORM}

  //void configuration_options(const Pecos::ExpansionConfigOptions& ec_options);
  //void configuration_options(const Pecos::BasisConfigOptions&     bc_options);

  //void random_variables_key(const BitArray& random_vars_key);
  //void refinement_statistics_type(short stats_type);
  //const Pecos::BitArrayULongMap& SharedApproxData::sobol_index_map();

  //
  //- Heading: Data
  //

  struct OneApproxOpts ** oneApproxOpts;
  struct MultiApproxOpts * approxOpts;

  std::vector<struct OneApproxOpts **> storeOne;
  std::vector<struct MultiApproxOpts *> storeMulti;

  size_t startOrder;
  size_t maxOrder;

  size_t startRank;
  size_t kickRank;
  size_t maxRank;
  bool adaptRank; // converted to size_t (0 no, 1 yes) for C3

  double roundingTol;
  double solverTol;
  size_t maxIterations;
  size_t crossMaxIter;
  size_t verbose;

  //void build();
  //void rebuild();

private:

  //
  //- Heading: Convenience functions
  //

  //
  //- Heading: Data
  //

  int constructed;
};


// inline void SharedC3ApproxData::
// random_variables_key(const Pecos::BitArray& random_vars_key)
// { pecosSharedDataRep->random_variables_key(random_vars_key); }

} // namespace Dakota

#endif
