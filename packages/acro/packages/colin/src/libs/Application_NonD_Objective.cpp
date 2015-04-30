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
 * \file Application_NonD_Objective.cpp
 *
 * Defines the colin::Application_NonD_Objective class members.
 */

#include <colin/application/NonD_Objective.h>
#include <utilib/TinyXML_helper.h>
#include <utilib/BitArray.h>

using std::cerr;
using std::endl;
using std::vector;

using boost::bind;

using utilib::BitArray;
using utilib::Any;
using utilib::Property;
using utilib::ReadOnly_Property;

namespace colin {

//--------------------------------------------------------------------
// Application_NonD_Objective Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_NonD_Objective::Application_NonD_Objective()
   : _nond_objective(Property::Bind<BitArray>()),
     nond_objective(_nond_objective.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare( "nond_objective", _nond_objective, t);

   properties["num_objectives"].onChange().connect
      (bind(&Application_NonD_Objective::cb_onChange_numObj, this, _1));
   _nond_objective.validate().connect
      (bind(&Application_NonD_Objective::cb_validate_nond, this, _1, _2));

   print_signal.connect
      (11, boost::bind(&Application_NonD_Objective::cb_print, this, _1));

   // NB: this relies on the fact that the signal will call the parent's
   // initializer first.
   initializer("Objectives").connect
      (boost::bind(&Application_NonD_Objective::cb_initialize, this, _1));

   // We need to explicitly call the onChange_numObj callback (because
   // the base class has already set its value)
   cb_onChange_numObj(properties["num_objectives"]);
}


/// Destructor
Application_NonD_Objective::~Application_NonD_Objective()
{ }


/// Return flag indicating whether the objective is nondeterministic
bool 
Application_NonD_Objective::
nondeterministicObjective(size_t index) const
{
   if ( index >= properties["num_objectives"].as<size_t>() )
   {
      EXCEPTION_MNGR(std::runtime_error, "Application_NonD_Objective::"
                     "nondeterministicObjective(): specified objective "
                     "out of range");
   }

   BitArray tmp = _nond_objective;
   return tmp(index);
}


//--------------------------------------------------------------------
// Application_NonD_Objective Private member functions
//--------------------------------------------------------------------


void 
Application_NonD_Objective::cb_print(std::ostream& os)
{
   os << "Stochastic objectives: " << nond_objective << endl;
}


void
Application_NonD_Objective::cb_initialize(TiXmlElement* elt)
{
   //
   // Look for nondeterministic flags...
   //
   bool update = false;
   BitArray tmp = nond_objective;
   tmp.reset();

   TiXmlElement* node = elt->FirstChildElement();
   while ( node != NULL )
   {
      if ( node->ValueStr().compare("StochasticObjective") != 0 )
         EXCEPTION_MNGR(std::runtime_error, 
                        "Application_NonD_Objective::"
                        "Implementation::init_objectives(): "
                        "invalid " << utilib::get_element_info(node));

      size_t id;
      if ( utilib::get_num_attribute(elt, "id", id, 0) )
         tmp.set(id);
      else
         tmp << true;
      node = node->NextSiblingElement();
   }

   if ( update )
      _nond_objective = tmp;
}


void
Application_NonD_Objective::
cb_onChange_numObj(const ReadOnly_Property &prop)
{
   static_cast<void>(prop);

   BitArray tmp = _nond_objective;
   size_t numObj = properties["num_objectives"];
   if ( ! ( numObj == tmp.size() ) )
   {
      tmp.resize(numObj);
      _nond_objective = tmp;
   }
}

/// The optimization sense for a specific objective
bool
Application_NonD_Objective::
cb_validate_nond( const ReadOnly_Property &prop, const Any &value )
{
   static_cast<void>(prop);
   Any tmp;
   utilib::TypeManager()->lexical_cast(value, tmp, typeid(BitArray));

   bool ok = properties["num_objectives"] == tmp.expose<BitArray>().size();
   if ( ! ok )
   {
      EXCEPTION_MNGR(std::runtime_error, "Application_NonD_Objective::"
                     "cb_validate_nond(): vector length (" 
                     << tmp.expose<BitArray>().size() << 
                     ") does not match num_objectives (" 
                     << properties["num_objectives"] << ")");
   }
   return ok;
}

} // namespace colin
