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
 * \file Application_SingleObjective.h
 *
 * Defines the colin::Application_SingleObjective class.
 */

#ifndef colin_Application_SingleObjective_h
#define colin_Application_SingleObjective_h

#include <colin/application/Base.h>

namespace colin {


/**
 * Defines the general elements of an Application that pertain to computing
 * a single objective.
 */
class Application_SingleObjective : virtual public Application_Base
{
protected:
   utilib::Privileged_Property _sense;

public: // methods

   /// Constructor
   Application_SingleObjective();

   /// Virtual destructor
   virtual ~Application_SingleObjective();

   /// Request the appripriate objective for the application (f or mf)
   virtual
   void Request_objective( AppRequest &request, 
                           utilib::AnyFixedRef result = utilib::Any() )
   { Request_F(request, result); }

   /// Register a function evaluation computation
   DECLARE_REQUEST(F);

public: // data

   /// The number of objectives (always 1)
   utilib::ReadOnly_Property num_objectives;

   /// The optimization sense (min/max)
   utilib::ReadOnly_Property sense;

private: // methods

   ///
   void cb_print(std::ostream& os);

   ///
   void cb_initialize(TiXmlElement*);
};   



template <bool active>
class Application_Include_SingleObjective
   : public Application_SingleObjective
{};

template<>
class Application_Include_SingleObjective<false>
{};

} // namespace colin

#endif // defined colin_Application_SingleObjective_h
