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
 * \file application/MultiObjective.h
 *
 * Defines the colin::Application_MultiObjective class.
 */

#ifndef colin_Application_MultiObjective_h
#define colin_Application_MultiObjective_h

#include <colin/application/Base.h>

namespace colin
{

/**
 * Defines the general elements of an Application that pertain to computing
 * a single objective.
 */
class Application_MultiObjective : virtual public Application_Base
{
protected:
   utilib::Privileged_Property _num_objectives;
   utilib::Privileged_Property _sense;

public: // methods

   /// Constructor
   Application_MultiObjective();

   /// Destructor
   virtual ~Application_MultiObjective();

   /// Request the appripriate objective for the application (f or mf)
   virtual
   void Request_objective( AppRequest &request, 
                           utilib::AnyFixedRef result = utilib::Any() )
   { Request_MF(request, result); }

   /// Register function evaluation computations
   DECLARE_REQUEST(MF);

public: // data

   /// The number of objectives
   utilib::ReadOnly_Property num_objectives;

   /// The optimization sense (vector of min/max)
   utilib::ReadOnly_Property sense;

private: // methods

   ///
   void cb_print(std::ostream& os);

   ///
   void cb_initialize(TiXmlElement*);

   ///
   void cb_onChange_numObj(const utilib::ReadOnly_Property &prop);

   ///
   bool cb_validate_sense(const utilib::ReadOnly_Property &prop,
                          const utilib::Any &value);

};



template <bool active>
class Application_Include_MultiObjective
   : public Application_MultiObjective
{};

template<>
class Application_Include_MultiObjective<false>
{};

} // namespace colin

#endif // defined colin_Application_MultiObjective_h
