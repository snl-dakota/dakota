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
 * \file Application_SingleObjective.cpp
 *
 * Defines the colin::Application_SingleObjective class members.
 */

#include <colin/application/SingleObjective.h>
#include <utilib/TinyXML_helper.h>

#include <boost/algorithm/string.hpp>    

using std::cerr;
using std::endl;
using std::vector;

using boost::bind;

using utilib::Property;
using utilib::Privileged_Property;
using utilib::ReadOnly_Property;

namespace colin {


//--------------------------------------------------------------------
// Application_SingleObjective Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_SingleObjective::Application_SingleObjective()
   : _sense(Property::Bind<optimizationSense>(minimization)),
     num_objectives(),
     sense(_sense.set_readonly())
{
   Privileged_Property _num_objectives(Property::Bind<size_t>(1));
   num_objectives = _num_objectives.set_readonly();

   ObjectType type = ObjectType::get(this);
   register_application_component(type);
   properties.declare("num_objectives", _num_objectives, type);
   properties.declare("sense", _sense, type);

   print_signal.connect
      (11, boost::bind(&Application_SingleObjective::cb_print, this, _1));

   initializer("Objective").connect
      (bind(&Application_SingleObjective::cb_initialize, this, _1));
}


/// Virtual destructor
Application_SingleObjective::~Application_SingleObjective()
{}


/// Register a function evaluation computation
DEFINE_REQUEST( Application_SingleObjective, F, f_info, objective function );


void 
Application_SingleObjective::
cb_print(std::ostream& os)
{
   os << "Number of objectives: " << num_objectives 
      << "  {" << ( sense == minimization ? " MIN" 
                    : ( sense == maximization ? " MAX" : " ???") )
      << " }" << endl;
}

/// callback for "Objective" element
void 
Application_SingleObjective::cb_initialize(TiXmlElement* elt)
{
   std::string txt = "";
   utilib::get_string_attribute(elt, "sense", txt, "");
   boost::algorithm::to_lower(txt);

   if ( txt.find("min") == 0 )
      _sense = minimization;
   else if ( txt.find("max") == 0 )
      _sense = maximization;
   else if ( txt.size() > 0 )
      EXCEPTION_MNGR(std::runtime_error, "Application_SingleObjective::"
                     "cb_initialize(): invalid value for optimization sense "
                     << utilib::get_element_info(elt));
}

} // namespace colin
