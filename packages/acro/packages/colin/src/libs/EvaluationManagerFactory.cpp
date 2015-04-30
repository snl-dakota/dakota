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

#include <colin/EvaluationManagerFactory.h>
#include <colin/EvaluationManager.h>

using std::string;
using std::pair;
using std::map;

namespace colin {
namespace {

static EvaluationManagerFactory* factory = NULL;

} // namespace colin::(local)


/** We use a static pointer here (and not a static object) to guarantee
 *  that the EvalManagerFactory() singleton is NEVER destroyed before
 *  any other singleton.
 *
 *  This is an intentional memory "leak".
 */
EvaluationManagerFactory& EvalManagerFactory()
{
   if ( factory == NULL )
      factory = new EvaluationManagerFactory();
   return *factory;
}



EvaluationManagerFactory::EvaluationManagerFactory()
   : m_last_mgr_id(0),
     m_default_manager_name("Serial"),
     m_default_manager(NULL)
{}


EvaluationManagerFactory::~EvaluationManagerFactory()
{
   std::map<size_t, EvaluationManager_Base*>::iterator it = m_managers.begin();
   while (it != m_managers.end())
   {
      delete it->second;
      ++it;
   }
   m_managers.clear();
}


bool
EvaluationManagerFactory::register_manager(std::string name, create_fcn_t fcn)
{
   if (! m_registry.insert(pair<string, create_fcn_t>(name, fcn)).second)
   {
      EXCEPTION_MNGR(std::runtime_error, "EvaluationManagerFactory::"
                     "register_manager(): duplicate manager name, '"
                     << name << "')");
      return false;
   }
   return true;
}


EvaluationManager_Base*
EvaluationManagerFactory::default_manager()
{
   if (m_default_manager == NULL)
   { m_default_manager = create(m_default_manager_name); }

   return m_default_manager;
}


EvaluationManager_Base*
EvaluationManagerFactory::create(std::string name)
{
   map<string, create_fcn_t>::iterator it = m_registry.find(name);
   if (it == m_registry.end())
   {
      EXCEPTION_MNGR(std::runtime_error, "EvaluationManagerFactory::create(): "
                     "EvaluationManager name " << name << " DNE.");
   }

   ++m_last_mgr_id;
   EvaluationManager_Base* ans = it->second(m_last_mgr_id);
   m_managers[m_last_mgr_id] = ans;
   return ans;
}

} // namespace colin
