/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedPecosApproxData
//- Description:  Base Class for Pecos polynomial approximations
//-               
//- Owner:        Mike Eldred

#ifndef SHARED_PECOS_APPROX_DATA_H
#define SHARED_PECOS_APPROX_DATA_H

#include "SharedApproxData.hpp"
#include "SharedRegressOrthogPolyApproxData.hpp"

namespace Dakota {


/// Derived approximation class for global basis polynomials.

/** The SharedPecosApproxData class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */

class SharedPecosApproxData: public SharedApproxData
{
  //
  //- Heading: Friends
  //

  friend class PecosApproximation;

public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SharedPecosApproxData();
  /// alternate constructor
  SharedPecosApproxData(const String& approx_type,
			const UShortArray& approx_order, size_t num_vars,
			short data_order, short output_level);
  /// standard ProblemDescDB-driven constructor
  SharedPecosApproxData(ProblemDescDB& problem_db, size_t num_vars);
  /// destructor
  ~SharedPecosApproxData();

  //
  //- Heading: Member functions
  //

  /// set pecosBasisApprox.randomVarsKey
  void random_variables_key(const Pecos::BitArray& random_vars_key);

  /// set pecosBasisApprox.driverRep
  void integration_iterator(const Iterator& iterator);

  /// invoke Pecos::SharedOrthogPolyApproxData::construct_basis()
  void construct_basis(const Pecos::ShortArray& u_types,
		       const Pecos::AleatoryDistParams& adp);

  // set Pecos::SharedOrthogPolyApproxData::basisTypes
  //void basis_types(const Pecos::ShortArray& basis_types);
  // get Pecos::SharedOrthogPolyApproxData::basisTypes
  //const Pecos::ShortArray& basis_types() const;

  /// set Pecos::SharedOrthogPolyApproxData::polynomialBasis
  void polynomial_basis(const std::vector<Pecos::BasisPolynomial>& poly_basis);
  /// get Pecos::SharedOrthogPolyApproxData::polynomialBasis
  const std::vector<Pecos::BasisPolynomial>& polynomial_basis() const;
  /// get Pecos::SharedOrthogPolyApproxData::polynomialBasis
  std::vector<Pecos::BasisPolynomial>& polynomial_basis();

  /// set Pecos::SharedOrthogPolyApproxData::multiIndex and allocate
  /// associated arrays
  void allocate(const UShort2DArray& mi);
  /// get Pecos::SharedOrthogPolyApproxData::multiIndex
  const UShort2DArray& multi_index() const;

  /// return Pecos::SharedPolyApproxData::sobolIndexMap
  const Pecos::BitArrayULongMap& sobol_index_map() const;

  /// invoke Pecos::SharedOrthogPolyApproxData::coefficients_norms_flag()
  void coefficients_norms_flag(bool flag);

  /// return Pecos::SharedOrthogPolyApproxData::expansion_terms()
  size_t expansion_terms() const;
  /// return Pecos::SharedOrthogPolyApproxData::expansion_order()
  const UShortArray& expansion_order() const;
  /// invokes Pecos::SharedOrthogPolyApproxData::expansion_order(UShortArray&)
  void expansion_order(const UShortArray& order);
  /// invokes Pecos::SharedOrthogPolyApproxData::increment_order()
  void increment_order();

  /// set the expansion configuration options within Pecos::SharedPolyApproxData
  void configuration_options(const Pecos::ExpansionConfigOptions& ec_options);
  /// set the basis configuration options within Pecos::SharedPolyApproxData
  void configuration_options(const Pecos::BasisConfigOptions& bc_options);
  /// set the regression configuration options within 
  /// Pecos::SharedRegressOrthogPolyApproxData
  void configuration_options(const Pecos::RegressionConfigOptions& rc_options);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void build();

  void rebuild();
  void pop(bool save_surr_data);
  bool push_available();
  size_t retrieval_index();
  void pre_push();
  void post_push();
  size_t finalization_index(size_t i);
  void pre_finalize();
  void post_finalize();

  void store(size_t index = _NPOS);
  void restore(size_t index = _NPOS);
  void remove_stored(size_t index = _NPOS);
  size_t pre_combine(short corr_type);
  void post_combine(short corr_type);

private:

  //
  //- Heading: Member functions
  //

  /// return pecosSharedData
  Pecos::SharedBasisApproxData& pecos_shared_data();

  /// utility to convert Dakota type string to Pecos type enumeration
  void approx_type_to_basis_type(const String& approx_type, short& basis_type);

  //
  //- Heading: Data
  //

  /// the Pecos shared approximation data
  Pecos::SharedBasisApproxData pecosSharedData;
  /// convenience pointer to derived letter within pecosSharedData
  Pecos::SharedPolyApproxData* pecosSharedDataRep;
};


inline SharedPecosApproxData::SharedPecosApproxData():
  pecosSharedDataRep(NULL)
{ }


inline SharedPecosApproxData::~SharedPecosApproxData()
{ }


inline void SharedPecosApproxData::build()
{ pecosSharedDataRep->allocate_data(); }


inline void SharedPecosApproxData::rebuild()
{ pecosSharedDataRep->increment_data(); }


inline void SharedPecosApproxData::pop(bool save_surr_data)
{ pecosSharedDataRep->decrement_data(); } // save is implied


inline bool SharedPecosApproxData::push_available()
{ return pecosSharedDataRep->push_available(); }


inline size_t SharedPecosApproxData::retrieval_index()
{ return pecosSharedDataRep->retrieval_index(); }


inline void SharedPecosApproxData::pre_push()
{ pecosSharedDataRep->pre_push_data(); }


inline void SharedPecosApproxData::post_push()
{ pecosSharedDataRep->post_push_data(); }


inline size_t SharedPecosApproxData::finalization_index(size_t i)
{ return pecosSharedDataRep->finalization_index(i); }


inline void SharedPecosApproxData::pre_finalize()
{ pecosSharedDataRep->pre_finalize_data(); }


inline void SharedPecosApproxData::post_finalize()
{ pecosSharedDataRep->post_finalize_data(); }


inline void SharedPecosApproxData::store(size_t index)
{ pecosSharedDataRep->store_data(index); }


inline void SharedPecosApproxData::restore(size_t index)
{ pecosSharedDataRep->restore_data(index); }


inline void SharedPecosApproxData::remove_stored(size_t index)
{ pecosSharedDataRep->remove_stored_data(index); }


inline size_t SharedPecosApproxData::pre_combine(short corr_type)
{ return pecosSharedDataRep->pre_combine_data(corr_type); }


inline void SharedPecosApproxData::post_combine(short corr_type)
{ pecosSharedDataRep->post_combine_data(corr_type); }


inline void SharedPecosApproxData::
random_variables_key(const Pecos::BitArray& random_vars_key)
{ pecosSharedDataRep->random_variables_key(random_vars_key); }


inline void SharedPecosApproxData::
construct_basis(const Pecos::ShortArray& u_types,
		const Pecos::AleatoryDistParams& adp)
{
  ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    construct_basis(u_types, adp);
}


/*
inline void SharedPecosApproxData::
basis_types(const Pecos::ShortArray& basis_types)
{
  ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    orthog_poly_basis_types(basis_types);
}


inline const Pecos::ShortArray& SharedPecosApproxData::basis_types() const
{
  return ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    orthog_poly_basis_types();
}
*/


inline void SharedPecosApproxData::
polynomial_basis(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    polynomial_basis(poly_basis);
}


inline const std::vector<Pecos::BasisPolynomial>& SharedPecosApproxData::
polynomial_basis() const
{
  return ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    polynomial_basis();
}


inline std::vector<Pecos::BasisPolynomial>& SharedPecosApproxData::
polynomial_basis()
{
  return ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    polynomial_basis();
}


inline void SharedPecosApproxData::allocate(const UShort2DArray& mi)
{ ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->allocate_data(mi); }


inline const UShort2DArray& SharedPecosApproxData::multi_index() const
{
  return ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    multi_index();
}


inline const Pecos::BitArrayULongMap& SharedPecosApproxData::
sobol_index_map() const
{ return pecosSharedDataRep->sobol_index_map(); }


inline void SharedPecosApproxData::coefficients_norms_flag(bool flag)
{
  ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    coefficients_norms_flag(flag);
}


inline size_t SharedPecosApproxData::expansion_terms() const
{
  return ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    expansion_terms();
}


inline const UShortArray& SharedPecosApproxData::expansion_order() const
{
  return ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    expansion_order();
}


inline void SharedPecosApproxData::expansion_order(const UShortArray& order)
{
  ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->
    expansion_order(order);
}


inline void SharedPecosApproxData::increment_order()
{ ((Pecos::SharedOrthogPolyApproxData*)pecosSharedDataRep)->increment_order(); }


inline void SharedPecosApproxData::
configuration_options(const Pecos::ExpansionConfigOptions& ec_options)
{ pecosSharedDataRep->configuration_options(ec_options); }


inline void SharedPecosApproxData::
configuration_options(const Pecos::BasisConfigOptions& bc_options)
{ pecosSharedDataRep->configuration_options(bc_options); }


inline void SharedPecosApproxData::
configuration_options(const Pecos::RegressionConfigOptions& rc_options)
{
  ((Pecos::SharedRegressOrthogPolyApproxData*)pecosSharedDataRep)->
    configuration_options(rc_options);
}


inline Pecos::SharedBasisApproxData& SharedPecosApproxData::pecos_shared_data()
{ return pecosSharedData; }

} // namespace Dakota

#endif
