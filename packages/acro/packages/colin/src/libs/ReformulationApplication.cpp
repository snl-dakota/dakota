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

#include <colin/reformulation/Base.h>
#include <colin/ApplicationMngr.h>
#include <utilib/TinyXML_helper.h>

using utilib::Any;
using utilib::Property;
using utilib::PropertyDict;

using std::cerr;
using std::endl;

namespace colin {

BasicReformulationApplication::~BasicReformulationApplication()
{
   // clear any pre-existing remote application callbacks
   while ( ! _reformulation_cb_list.empty() )
   {
      _reformulation_cb_list.front().disconnect();
      _reformulation_cb_list.pop_front();
   }
}


void 
BasicReformulationApplication::
reformulate_application(ApplicationHandle handle)
{
   if ( handle.empty() )
      EXCEPTION_MNGR(std::runtime_error, 
                     "BasicReformulation::reformulate_application(): "
                     "cannot set NULL wrapped application.");

   validate_reformulated_application(handle);

   // clear any pre-existing remote application callbacks
   while ( ! _reformulation_cb_list.empty() )
   {
      _reformulation_cb_list.front().disconnect();
      _reformulation_cb_list.pop_front();
   }

   remote_app = handle;
   configure_reformulated_application();
}

void 
BasicReformulationApplication::
cb_initialize(TiXmlElement* elt)
{
   std::string name;
   utilib::get_string_attribute
      ( elt, "id", name, ApplicationManager::default_application_name );
   ApplicationHandle app = ApplicationMngr().get_application(name);
   if ( app.empty() )
      EXCEPTION_MNGR(std::runtime_error, "BasicReformulationApplication::"
                     "cb_initialize(): unknown application, \""
                     << name << "\"");
   reformulate_application(app);
}

/** NB: This method has side-effects: any properties that are set to
 *  reference the external application will have their value reset to
 *  that of the external application (therefore calling any
 *  locally-registered onChange() callbacks!)
 */
void
BasicReformulationApplication::
reference_reformulated_application_properties
    ( std::set<ObjectType> exclude, std::set<std::string> block_promotion )
{
   // clear out any remote refereces from a previous call to this method
   properties.dereference_all();

   std::set<Any> tmp;
   std::set<ObjectType>::iterator it = exclude.begin();
   for( ; it != exclude.end(); ++it)
      tmp.insert(*it);

   
   EXCEPTION_TEST(remote_app.empty(), std::runtime_error, 
                  "I cannot reference a NULL reformulated application handle");

   referencePropertiesFrom( remote_app.object(), tmp, block_promotion );
}

} // namespace colin
