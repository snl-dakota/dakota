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
 * \file Application_RealDomain.h
 *
 * Defines the colin::Application_RealDomain class.
 */

#ifndef colin_Application_RealDomain_h
#define colin_Application_RealDomain_h

#include <acro_config.h>

#include <colin/application/Domain.h>
#include <colin/BoundTypeArray.h>
#include <colin/real.h>

namespace colin
{

/**
 *  Defines the elements of an Application that pertain to real variables.
 */
class Application_RealDomain : virtual public Application_Domain
{
protected:
   utilib::Privileged_Property _num_real_vars;
   utilib::Privileged_Property _real_lower_bounds;
   utilib::Privileged_Property _real_upper_bounds;
   utilib::Privileged_Property _real_lower_bound_types;
   utilib::Privileged_Property _real_upper_bound_types;
   utilib::Privileged_Property _real_labels;

public: // methods

   /// Constructor
   Application_RealDomain();

   /// Virtual destructor
   virtual ~Application_RealDomain();

   /// Returns the lower bound on the $i$-th variable
   colin::real realLowerBound(size_t i) const;

   /// Returns the upper bound on the $i$-th variable
   colin::real realUpperBound(size_t i) const;

   /// Returns the type of the real lower bound on the $i$-th variable
   bound_type_enum realLowerBoundType(size_t i) const;

   /// Returns the type of the real upper bound on the $i$-th variable
   bound_type_enum realUpperBoundType(size_t i) const;

   /// Return the label for a specific real variable
   std::string realLabel(size_t i) const;

   /// Returns true if this problem has a lower bound on the $i$-th variable
   bool hasRealLowerBound(size_t i) const;

   /// Returns true if this problem has an upper bound on the $i$-th variable
   bool hasRealUpperBound(size_t i) const;

   /// Returns true if the bounds on the $i$-th real variable are periodic
   bool hasPeriodicRealBound(size_t i) const;

   ///
   bool finiteBoundConstraints() const;

   /// Returns true if the point is feasible with respect to bound constraints
   bool testBoundsFeasibility(const utilib::Any& point);

public: // data

   /// Number of real variables
   utilib::ReadOnly_Property num_real_vars;

   /// Dense vector of real variable lower bounds
   utilib::ReadOnly_Property real_lower_bounds;

   /// Dense vector of real variable upper bounds
   utilib::ReadOnly_Property real_upper_bounds;

   /// Dense vector of real variable lower bound type
   utilib::ReadOnly_Property real_lower_bound_types;

   /// Dense vector of real variable upper bound type
   utilib::ReadOnly_Property real_upper_bound_types;

   /// Bimap of index to real variable string label
   utilib::ReadOnly_Property real_labels;

protected: // methods

   /// Sets the type of a single real lower bound
   void _setRealLowerBoundType(size_t i, bound_type_enum type);

   /// Sets the type of a single real upper bound
   void _setRealUpperBoundType(size_t i, bound_type_enum type);

   /// Indicates that the bounds on the $i$-th real variable are periodic
   void _setPeriodicRealBound(size_t i);

   /// Set a single label for the real domain
   void _setRealLabel(const size_t i, const std::string &label);


private: // methods
   ///
   void cb_onChange_nvars( const utilib::ReadOnly_Property &prop );
   
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

   void cb_initialize(TiXmlElement*);

};



template <bool active>
class Application_Include_RealDomain
         : public Application_RealDomain
   {};

template<>
class Application_Include_RealDomain<false>
{
public:
   ///
   bool finiteBoundConstraints() const
      {return true;}
   ///
   bool testBoundsFeasibility( const utilib::Any& )
      { return true; }
};


} // namespace colin

#endif // defined colin_Application_RealDomain_h
