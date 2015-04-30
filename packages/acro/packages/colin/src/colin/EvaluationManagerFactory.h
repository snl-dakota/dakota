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
 * \file EvaluationManagerFactory.h
 *
 * Defines the colin::EvaluationManagerFactory class.
 */

#ifndef colin_EvaluationManagerFactory_h
#define colin_EvaluationManagerFactory_h

#include <acro_config.h>
#include <utilib/std_headers.h>

namespace colin
{

class EvaluationManager_Base;
class EvaluationManagerFactory;

EvaluationManagerFactory& EvalManagerFactory();


class EvaluationManagerFactory
{
public:
   ///
   typedef EvaluationManager_Base*(*create_fcn_t)(size_t);

   ///
   EvaluationManagerFactory();

   ///
   ~EvaluationManagerFactory();

   ///
   void set_default_manager(std::string name)
   { m_default_manager_name = name; }

   ///
   bool register_manager(std::string name, create_fcn_t fcn);

   ///
   EvaluationManager_Base* default_manager();

   ///
   EvaluationManager_Base* create(std::string name);

private:
   ///
   std::map<std::string, create_fcn_t>  m_registry;
   ///
   std::map<size_t, EvaluationManager_Base*>  m_managers;
   ///
   size_t  m_last_mgr_id;
   ///
   std::string  m_default_manager_name;
   ///
   EvaluationManager_Base*  m_default_manager;
};


} // namespace colin

#endif // defined colin_EvaluationManagerFactory_h
