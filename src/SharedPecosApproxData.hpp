/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  /// invoke Pecos::SharedPolyApproxData::update_basis_distribution_parameters()
  void update_basis_distribution_parameters(
    const Pecos::MultivariateDistribution& u_dist);

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

  /// get active Pecos::SharedOrthogPolyApproxData::multiIndex
  const UShort2DArray& multi_index() const;
  /// get Pecos::SharedOrthogPolyApproxData::multiIndex
  const std::map<Pecos::ActiveKey, UShort2DArray>& multi_index_map() const;

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

  /// set the expansion configuration options within Pecos::SharedPolyApproxData
  void configuration_options(const Pecos::ExpansionConfigOptions& ec_options);
  /// set the basis configuration options within Pecos::SharedPolyApproxData
  void configuration_options(const Pecos::BasisConfigOptions& bc_options);
  /// set the regression configuration options within 
  /// Pecos::SharedRegressOrthogPolyApproxData
  void configuration_options(const Pecos::RegressionConfigOptions& rc_options);

  /// update ExpansionConfigOptions::refineStatsType
  void refinement_statistics_mode(short stats_mode);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void active_model_key(const Pecos::ActiveKey& key);
  void clear_model_keys();

  void construct_basis(const Pecos::MultivariateDistribution& mv_dist);

  void integration_iterator(const Iterator& iterator);

  short discrepancy_reduction() const;

  void build();
  void rebuild();

  void pop(bool save_surr_data);

  bool push_available();
  size_t push_index(const Pecos::ActiveKey& key);
  void pre_push();
  void post_push();

  size_t finalize_index(size_t i, const Pecos::ActiveKey& key);
  void pre_finalize();
  void post_finalize();

  void pre_combine();
  void post_combine();
  void combined_to_active(bool clear_combined = true);

  bool advancement_available();

  void clear_inactive();

  void increment_order();
  void decrement_order();

private:

  //
  //- Heading: Member functions
  //

  /// return pecosSharedData
  Pecos::SharedBasisApproxData& pecos_shared_data();
  /// return pecosSharedDataRep
  std::shared_ptr<Pecos::SharedPolyApproxData> pecos_shared_data_rep();
  
  /// utility to convert Dakota type string to Pecos type enumeration
  void approx_type_to_basis_type(const String& approx_type, short& basis_type);

  //
  //- Heading: Data
  //

  /// the Pecos shared approximation data
  Pecos::SharedBasisApproxData pecosSharedData;
  /// convenience pointer to derived letter within pecosSharedData
  std::shared_ptr<Pecos::SharedPolyApproxData> pecosSharedDataRep;
};


inline SharedPecosApproxData::SharedPecosApproxData()
{ }


inline SharedPecosApproxData::~SharedPecosApproxData()
{ }


inline void SharedPecosApproxData::active_model_key(const Pecos::ActiveKey& key)
{
  SharedApproxData::active_model_key(key);
  pecosSharedDataRep->active_key(key);
}


inline void SharedPecosApproxData::clear_model_keys()
{
  SharedApproxData::clear_model_keys();
  pecosSharedDataRep->clear_keys();
}


inline short SharedPecosApproxData::discrepancy_reduction() const
{ return pecosSharedDataRep->discrepancy_reduction(); }


inline void SharedPecosApproxData::build()
{ pecosSharedDataRep->allocate_data();  formUpdated[activeKey] = false; }


inline void SharedPecosApproxData::rebuild()
{ pecosSharedDataRep->increment_data(); formUpdated[activeKey] = false; }


inline void SharedPecosApproxData::pop(bool save_surr_data)
{ pecosSharedDataRep->decrement_data(); } // save is implied


inline bool SharedPecosApproxData::push_available()
{ return pecosSharedDataRep->push_available(); }


/** In Pecos, SharedPolyApproxData::push_index() is for internal use as
    it can be either a flattened (ISGDriver) or level-specific index
    (HSGDriver), as convenient for combined or hierarchical data
    restoration.  SharedPolyApproxData::{restore,finalize}_index() are
    intended for external use and will map from push_index() to provide
    a consistent (flattened) representation.  Dakota, however, does not
    make this distinction and uses {push,finalize}_index() semantics
    for consistency with {push,finalize}_data(). */
inline size_t SharedPecosApproxData::push_index(const Pecos::ActiveKey& key)
{ return pecosSharedDataRep->restore_index(key); }


inline void SharedPecosApproxData::pre_push()
{ pecosSharedDataRep->pre_push_data(); }


inline void SharedPecosApproxData::post_push()
{ pecosSharedDataRep->post_push_data(); }


inline size_t SharedPecosApproxData::
finalize_index(size_t i, const Pecos::ActiveKey& key)
{ return pecosSharedDataRep->finalize_index(i, key); }


inline void SharedPecosApproxData::pre_finalize()
{ pecosSharedDataRep->pre_finalize_data(); }


inline void SharedPecosApproxData::post_finalize()
{ pecosSharedDataRep->post_finalize_data(); }


inline void SharedPecosApproxData::clear_inactive()
{ pecosSharedDataRep->clear_inactive_data(); }


inline void SharedPecosApproxData::pre_combine()
{ pecosSharedDataRep->pre_combine_data(); }


inline void SharedPecosApproxData::post_combine()
{ pecosSharedDataRep->post_combine_data(); }


inline void SharedPecosApproxData::combined_to_active(bool clear_combined)
{ pecosSharedDataRep->combined_to_active(clear_combined); }


inline void SharedPecosApproxData::
random_variables_key(const Pecos::BitArray& random_vars_key)
{ pecosSharedDataRep->random_variables_key(random_vars_key); }


inline void SharedPecosApproxData::
construct_basis(const Pecos::MultivariateDistribution& mv_dist)
{ pecosSharedDataRep->construct_basis(mv_dist); }


inline void SharedPecosApproxData::
update_basis_distribution_parameters(
  const Pecos::MultivariateDistribution& u_dist)
{ pecosSharedDataRep->update_basis_distribution_parameters(u_dist); }


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
{ pecosSharedDataRep->polynomial_basis(poly_basis); }


inline const std::vector<Pecos::BasisPolynomial>& SharedPecosApproxData::
polynomial_basis() const
{ return pecosSharedDataRep->polynomial_basis(); }


inline std::vector<Pecos::BasisPolynomial>& SharedPecosApproxData::
polynomial_basis()
{ return pecosSharedDataRep->polynomial_basis(); }


inline void SharedPecosApproxData::allocate(const UShort2DArray& mi)
{ std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->allocate_data(mi); }


inline const UShort2DArray& SharedPecosApproxData::multi_index() const
{
  return std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->multi_index();
}


inline const std::map<Pecos::ActiveKey, UShort2DArray>& SharedPecosApproxData::
multi_index_map() const
{
  return std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->multi_index_map();
}


inline const Pecos::BitArrayULongMap& SharedPecosApproxData::
sobol_index_map() const
{ return pecosSharedDataRep->sobol_index_map(); }


inline void SharedPecosApproxData::coefficients_norms_flag(bool flag)
{
  std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->coefficients_norms_flag(flag);
}


inline size_t SharedPecosApproxData::expansion_terms() const
{
  return std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->expansion_terms();
}


inline const UShortArray& SharedPecosApproxData::expansion_order() const
{
  return std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->expansion_order();
}


inline void SharedPecosApproxData::expansion_order(const UShortArray& order)
{
  std::shared_ptr<Pecos::SharedOrthogPolyApproxData> data_rep =
    std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep);
  if (order != data_rep->expansion_order()) {
    data_rep->expansion_order(order);
    formUpdated[activeKey] = true;    
  }
}


inline bool SharedPecosApproxData::advancement_available()
{
  // delegate this fn rather than use multiple forwards, since also
  // implementing in RegressOrthogPolyApproximation
  return pecosSharedDataRep->advancement_available();
}


inline void SharedPecosApproxData::increment_order()
{
  std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->increment_order();
  formUpdated[activeKey] = true;
}


inline void SharedPecosApproxData::decrement_order()
{
  std::static_pointer_cast<Pecos::SharedOrthogPolyApproxData>
    (pecosSharedDataRep)->decrement_order();
  formUpdated[activeKey] = true;
}


inline void SharedPecosApproxData::
configuration_options(const Pecos::ExpansionConfigOptions& ec_options)
{ pecosSharedDataRep->configuration_options(ec_options); }


inline void SharedPecosApproxData::
configuration_options(const Pecos::BasisConfigOptions& bc_options)
{ pecosSharedDataRep->configuration_options(bc_options); }


inline void SharedPecosApproxData::
configuration_options(const Pecos::RegressionConfigOptions& rc_options)
{
  std::static_pointer_cast<Pecos::SharedRegressOrthogPolyApproxData>
    (pecosSharedDataRep)->configuration_options(rc_options);
}


inline void SharedPecosApproxData::refinement_statistics_mode(short stats_mode)
{ pecosSharedDataRep->refinement_statistics_type(stats_mode); }


inline Pecos::SharedBasisApproxData& SharedPecosApproxData::pecos_shared_data()
{ return pecosSharedData; }


inline std::shared_ptr<Pecos::SharedPolyApproxData>
SharedPecosApproxData::pecos_shared_data_rep()
{ return pecosSharedDataRep; }

} // namespace Dakota

#endif
