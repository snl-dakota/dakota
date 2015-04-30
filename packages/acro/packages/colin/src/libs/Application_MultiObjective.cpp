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
 * \file Application_MultiObjective.cpp
 *
 * Defines the colin::Application_MultiObjective class members.
 */

#include <colin/application/MultiObjective.h>
#include <utilib/TinyXML_helper.h>

#include <boost/algorithm/string.hpp>    

using std::cerr;
using std::endl;
using std::vector;

using boost::bind;

using utilib::Any;
using utilib::Property;
using utilib::ReadOnly_Property;

namespace colin {

namespace {

bool register_conversion_fcns()
{
   utilib::TypeManager()->register_lexical_cast
      ( typeid(optimizationSense),  typeid(vector<optimizationSense>),
        & utilib::LexicalCasts::cast_val2stl
          <optimizationSense, vector<optimizationSense> > );

   utilib::TypeManager()->register_lexical_cast
      ( typeid(vector<optimizationSense>),  typeid(optimizationSense),
        & utilib::LexicalCasts::cast_stl2val
          <optimizationSense, vector<optimizationSense> >,
        10 );

   return true;
}

static const volatile bool registrations_complete = register_conversion_fcns();

} // namespace colin::(local)

//--------------------------------------------------------------------
// Application_MultiObjective Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_MultiObjective::Application_MultiObjective()
   : _num_objectives(Property::Bind<size_t>()),
     _sense(Property::Bind<vector<optimizationSense> >()),
     num_objectives(_num_objectives.set_readonly()),
     sense(_sense.set_readonly())
{
   ObjectType type = ObjectType::get(this);
   register_application_component(type);
   properties.declare("num_objectives", _num_objectives, type);
   properties.declare("sense", _sense, type);

   _num_objectives.onChange().connect
      (bind(&Application_MultiObjective::cb_onChange_numObj, this, _1));
   _sense.validate().connect
      (bind(&Application_MultiObjective::cb_validate_sense, this, _1, _2));

   // we set the number of objectives here so the callback syncs the
   // sense dimensionality
   _num_objectives = 0;

   print_signal.connect
      (11, boost::bind(&Application_MultiObjective::cb_print, this, _1));

   initializer("Objectives").connect
      (bind(&Application_MultiObjective::cb_initialize, this, _1));
}


/// Destructor
Application_MultiObjective::~Application_MultiObjective()
{}


/// Register function evaluation computations
DEFINE_REQUEST( Application_MultiObjective, MF, mf_info, 
                multiobjective function );


//--------------------------------------------------------------------
// Application_MultiObjective Private member functions
//--------------------------------------------------------------------

///
void
Application_MultiObjective::cb_print(std::ostream& os)
{
   os << "Number of objectives: " << num_objectives;

   vector<optimizationSense> tmp = sense;
   if ( ! tmp.empty() )
   {
      os << "  {";
      for (size_t i = 0; i < tmp.size(); ++i )
         os << ( tmp[i] == minimization ? " MIN" 
                 : ( tmp[i] == maximization ? " MAX" : " ???") );
      os << " }";
   }
   os << endl;
}

/// callback for "Objectives" element
void 
Application_MultiObjective::cb_initialize(TiXmlElement* elt)
{
   size_t obj = 0;
   utilib::get_num_attribute(elt, "num", obj);
   if ( obj < 1 )
      EXCEPTION_MNGR(std::runtime_error, "Application_MultiObjective::"
                     "cb_initialize(): invalid number of objectives at "
                     << utilib::get_element_info(elt));
   
   _num_objectives = obj;

   int id = -1;
   elt = elt->FirstChildElement();
   std::vector<optimizationSense> tmp_sense = _sense;
   while ( elt )
   {
      if ( elt->ValueStr().compare("Objective") != 0 )
         EXCEPTION_MNGR(std::runtime_error, "Application_MultiObjective::"
                        "cb_initialize(): invalid element at "
                        << utilib::get_element_info(elt));

      utilib::get_num_attribute(elt, "id", id, id+1);
      if ( id < 0 || id >= obj )
         EXCEPTION_MNGR(std::runtime_error, "Application_MultiObjective::"
                        "cb_initialize(): invalid objective index at "
                        << utilib::get_element_info(elt));

      std::string txt = "";
      utilib::get_string_attribute(elt, "sense", txt, "");
      boost::algorithm::to_lower(txt);
      
      if ( txt.find("min") == 0 )
         tmp_sense[id] = minimization;
      else if ( txt.find("max") == 0 )
         tmp_sense[id] = maximization;
      else if ( txt.size() > 0 )
         EXCEPTION_MNGR(std::runtime_error, "Application_MultiObjective::"
                        "cb_initialize(): invalid value for optimization "
                        "sense at " << utilib::get_element_info(elt));

      elt = elt->NextSiblingElement();
   }
   if ( id != -1 )
      _sense = tmp_sense;
}

void
Application_MultiObjective::cb_onChange_numObj(const ReadOnly_Property &prop)
{
   static_cast<void>(prop);

   vector<optimizationSense> tmp = sense;
   tmp.resize(_num_objectives, minimization);
   _sense = tmp;
}

/// The optimization sense for a specific objective
bool
Application_MultiObjective::cb_validate_sense(const ReadOnly_Property &prop,
                                              const Any &value)
{
   static_cast<void>(prop);
   bool ok = 
      ( num_objectives == value.expose<vector<optimizationSense> >().size() );
   if ( ! ok )
   {
      EXCEPTION_MNGR(std::runtime_error, "Application_MultiObjective::"
                     "sense(): vector length (" << 
                     value.expose<vector<optimizationSense> >().size() << 
                     ") does not match num_objectives (" << num_objectives
                     << ")");
   }
   return ok;
}

} // namespace colin
