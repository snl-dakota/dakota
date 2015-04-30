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
 * \file Application_Domain.h
 *
 * Defines the colin::Application_Domain class.
 */

#ifndef colin_Application_Domain_h
#define colin_Application_Domain_h

#include <acro_config.h>
#include <colin/application/Base.h>
#include <colin/real.h>
#include <colin/BoostExtras.h>

namespace colin
{

/**
 *  Defines the general elements of an Application that pertain to bounds.
 */
class Application_Domain : virtual public Application_Base
{
protected:
   utilib::Privileged_Property _enforcing_domain_bounds;

public:

   /// Constructor.
   Application_Domain();

   /// Destructor.
   virtual ~Application_Domain();

public: // data

   /// The total number of variables in the problem domain
   utilib::ReadOnly_Property domain_size;

   /// True if the application is penalizing domain bound violations
   utilib::ReadOnly_Property enforcing_domain_bounds;

protected: // methods

   /// A callback for collecting the domain size from derived classes
   boost::signal<size_t(), boost_extras::sum<size_t> > get_domain_size_signal;

   /// Register derived class callback function for initialize()
   cb_initializer_t& domain_initializer(std::string element)
   { return initializer("Domain::" + element); }


private:  // methods

   void cb_get_domain_size( const utilib::Any&, utilib::Any& ans)
   { ans = get_domain_size_signal(); }

   void cb_initialize(TiXmlElement*);
};

namespace Domain {

template <typename T>
class Component
{
public:
   std::vector<T>       data;
   std::vector<size_t>  source_index;

   size_t size() { return data.size(); }
   T& operator[](size_t i) { return data[i]; }
};

class RealComponent           : public Component<real> {};
class IntegerComponent        : public Component<int>  {};
class BinaryComponent         : public Component<bool> {};
class DiscreteComponent       : public Component<int>  {};
class DifferentiableComponent : public Component<real> {};

} // namespace colin::Domain
} // namespace colin

#endif // colin_Application_Domain_h
