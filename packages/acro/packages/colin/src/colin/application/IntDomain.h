/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file Application_IntDomain.h
 *
 * Defines the colin::Application_IntDomain class.
 */

#ifndef colin_Application_IntDomain
#define colin_Application_IntDomain

#include <acro_config.h>
#include <colin/application/Domain.h>
#include <colin/BoundTypeArray.h>

namespace colin
{

/**
 *  Defines the elements of an Application that pertain to integer
 *  variables.
 */
class Application_IntDomain : virtual public Application_Domain
{
protected:
   utilib::Privileged_Property _num_int_vars;
   utilib::Privileged_Property _int_lower_bounds;
   utilib::Privileged_Property _int_upper_bounds;
   utilib::Privileged_Property _int_lower_bound_types;
   utilib::Privileged_Property _int_upper_bound_types;
   utilib::Privileged_Property _int_labels;

   utilib::Privileged_Property _num_binary_vars;
   utilib::Privileged_Property _binary_labels;

public: // methods

   /// Constructor
   Application_IntDomain();

   /// Virtual destructor
   virtual ~Application_IntDomain();


   /// Returns the int lower bound on the $i$-th variable
   int intLowerBound(size_t i) const;

   /// Returns the int upper bound on the $i$-th variable
   int intUpperBound(size_t i) const;

   /// Returns the type of the int lower bound on the $i$-th variable
   bound_type_enum intLowerBoundType(size_t i) const;

   /// Returns the type of the int upper bound on the $i$-th variable
   bound_type_enum intUpperBoundType(size_t i) const;

   /// Return the label for a specific int variable
   std::string intLabel(size_t i) const;

   /// Returns true if this problem has a lower bound on the $i$-th variable
   bool hasIntLowerBound(size_t i) const;

   /// Returns true if this problem has an upper bound on the $i$-th variable
   bool hasIntUpperBound(size_t i) const;

   /// Returns true if the bounds on the $i$-th integer variable are periodic
   bool hasPeriodicIntBound(size_t i) const;


   /// Return the label for a specific binary variable
   std::string binaryLabel(size_t i) const;


   ///
   bool finiteBoundConstraints() const;

   /// Returns true if the point is feasible with respect to bound constraints
   bool testBoundsFeasibility(const utilib::Any& point);


   ///
   virtual ApplicationHandle relaxed_application() const;

public: // data

   /// Number of int variables
   utilib::ReadOnly_Property num_int_vars;

   /// Dense vector of int variable lower bounds
   utilib::ReadOnly_Property int_lower_bounds;

   /// Dense vector of int variable upper bounds
   utilib::ReadOnly_Property int_upper_bounds;

   /// Dense vector of int variable lower bound type
   utilib::ReadOnly_Property int_lower_bound_types;

   /// Dense vector of int variable upper bound type
   utilib::ReadOnly_Property int_upper_bound_types;

   /// Bimap of index to int variable string label
   utilib::ReadOnly_Property int_labels;


   /// Number of binary variables
   utilib::ReadOnly_Property num_binary_vars;

   /// Bimap of index to binary variable string label
   utilib::ReadOnly_Property binary_labels;

protected: // methods

   /// Sets the type of an integer lower bound
   void _setIntLowerBoundType(size_t i, bound_type_enum type);

   /// Sets the type of an integer upper bound
   void _setIntUpperBoundType(size_t i, bound_type_enum type);

   /// Indicates that the bounds on the $i$-th integer variable are periodic
   void _setPeriodicIntBound(size_t i);

   /// Set a single label for the integer domain
   void _setIntLabel(const size_t i, const std::string &label);

   /// Set a single label for the binary domain
   void _setBinaryLabel(const size_t i, const std::string &label);


private: // methods
   ///
   void cb_onChange_int( const utilib::ReadOnly_Property &prop );
   
   ///
   void cb_onChange_binary( const utilib::ReadOnly_Property &prop );
   
   ///
   void cb_onChange_bounds( const utilib::ReadOnly_Property &prop );

   ///
   void cb_onChange_bound_types(const utilib::ReadOnly_Property &prop);

   ///
   bool cb_validate_bounds( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

   ///
   bool cb_validate_bound_types( const utilib::ReadOnly_Property &prop,
                                 const utilib::Any &value );

   ///
   bool cb_validate_labels( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

   ///
   void cb_print(std::ostream& os);

   /// Helper class for callback methods & private functions
   //class Implementation;

   void cb_initialize_int(TiXmlElement*);

   void cb_initialize_binary(TiXmlElement*);
};


template <bool active>
class Application_Include_IntDomain
   : public Application_IntDomain
{};

template<>
class Application_Include_IntDomain<false>
{
public:

   ///
   bool finiteBoundConstraints() const
      { return true; }
   ///
   bool testBoundsFeasibility( const utilib::Any& )
      { return true; }
};

} // namespace colin

#endif // defined colin_Application_IntDomain
