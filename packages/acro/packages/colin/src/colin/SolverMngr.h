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
 * \file SolverMngr.h
 *
 * Function wrapper for managing COLIN application objects.
 */

#ifndef colin_SolverMngr_h
#define colin_SolverMngr_h

#include <acro_config.h>

#include <colin/solver/Base.h>

#include <utilib/Any.h>

namespace colin
{

class SolverManager;

/// The Singleton through which the world accesses the global solver manager
SolverManager& SolverMngr();


/// A base class for exceptions thrown by the SolverManager
class solverManager_error : public std::runtime_error
{
public:
   /// Constructor
   solverManager_error(const std::string& msg) 
      : runtime_error(msg)
   {}
};

/// The exception thrown by the solver manager when name registrations fail
class solverManager_registration_error : public solverManager_error
{
public:
   /// Constructor
   solverManager_registration_error(const std::string& msg) 
      : solverManager_error(msg)
   {}
};


class SolverManager
{
public:

   ///
   typedef SolverHandle(*create_solver_fcn)();

   ///
   static const std::string default_solver_name;

public:

   ///
   SolverManager();

   ///
   ~SolverManager();

   ///
   void clear();

   ///
   SolverHandle register_solver( SolverHandle handle, std::string type );

   ///
   void reregister_solver( SolverHandle solver, const std::string &name );

   ///
   void unregister_solver(const std::string& name);

   ///
   void unregister_solver(SolverHandle solver);

   ///
   SolverHandle create_solver( const std::string type );

   ///
   SolverHandle get_solver(const std::string& name) const;

   ///
   std::string get_solver_name(const Solver_Base* solver) const;

   ///
   std::string get_solver_type(const Solver_Base* solver) const;

   ///
   std::string get_newest_solver() const;

   ///
   template<typename TYPE>
   bool declare_solver_type(std::string name, std::string description)
   {
      return declare_solver_type_impl( name, description,
                                       &create_derived_solver<TYPE> );
   }

   ///
   void get_solver_types(std::list<std::pair<std::string, std::string> >
                         &solvers) const;

   ///
   void print_solver_types(std::ostream& os) const;

private:

   ///
   template<typename TYPE>
   static SolverHandle create_derived_solver()
   {
      return SolverHandle::create<TYPE>().first;
   }

   ///
   bool declare_solver_type_impl( std::string name, std::string description,
                                  create_solver_fcn fcn );
   
private:

   ///
   struct Data;

   ///
   Data *data;
};


#define REGISTER_COLIN_SOLVER(CLASS, SOLVER_NAME, DESCRIPTION)          \
namespace StaticInitializers {                                          \
namespace {                                                             \
bool Register ## CLASS()                                                \
{                                                                       \
   return ::colin::SolverMngr().declare_solver_type< CLASS >            \
      ( SOLVER_NAME ,  DESCRIPTION  );                                  \
}                                                                       \
}                                                                       \
extern const volatile bool CLASS ## _bool = Register ## CLASS ();       \
}

#define REGISTER_COLIN_SOLVER_WITH_ALIAS(CLASS, NAME, ALIAS, DESCRIPTION) \
namespace StaticInitializers {                                          \
namespace {                                                             \
bool Register ## CLASS()                                                \
{                                                                       \
   bool ans = true;                                                     \
   ans &= ::colin::SolverMngr().declare_solver_type< CLASS >            \
      ( NAME ,  DESCRIPTION );                                         \
   ans &= ::colin::SolverMngr().declare_solver_type< CLASS >            \
      ( ALIAS , "An alias to " NAME );                                  \
   return ans;                                                          \
}                                                                       \
}                                                                       \
extern const volatile bool CLASS ## _bool = Register ## CLASS();        \
}

} // namespace colin

#endif // defined colin_SolverMngr_h
