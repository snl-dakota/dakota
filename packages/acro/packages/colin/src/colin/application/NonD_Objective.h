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
 * \file application/NonD_Objective.h
 *
 * Defines the colin::Application_NonD_Objective class.
 */

#ifndef colin_Application_NonD_Objective_h
#define colin_Application_NonD_Objective_h

#include <colin/application/Base.h>

namespace colin
{

/**
 * Defines the general elements of an Application that pertain to computing
 * a nondeterministic objectives.
 */
class Application_NonD_Objective : virtual public Application_Base
{
protected:
   utilib::Privileged_Property _nond_objective;

public: // methods

   /// Constructor
   Application_NonD_Objective();

   /// Destructor
   virtual ~Application_NonD_Objective();

public: // data

   /// BitArray indicating if any objectives are non-deterministic
   utilib::ReadOnly_Property nond_objective;

   /// Convenience method for retrieving a single NonD flag
   bool nondeterministicObjective(size_t index) const;

private: // methods

   ///
   void cb_print(std::ostream& os);

   ///
   void cb_initialize(TiXmlElement*);

   ///
   void cb_onChange_numObj(const utilib::ReadOnly_Property &prop);

   ///
   bool cb_validate_nond(const utilib::ReadOnly_Property &prop,
                         const utilib::Any &value);
};


template <bool active>
class Application_Include_NonD_Objective
   : public Application_NonD_Objective
{};

template<>
class Application_Include_NonD_Objective<false>
{};

} // namespace colin
#endif // defined colin_Application_NonD_Objective_h
