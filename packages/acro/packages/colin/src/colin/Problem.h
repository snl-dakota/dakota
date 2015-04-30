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
 * \file Problem.h
 *
 * Defines the colin::Problem class.
 */

#ifndef colin_Problem_h
#define colin_Problem_h

#include <acro_config.h>

#include <colin/application/Base.h>
#include <colin/ProblemTraits.h>
#include <colin/ContextMngr.h>

namespace colin
{

template<typename ProblemT> class Application;

//============================================================================
//============================================================================
// Class Problem
//============================================================================
//============================================================================

/**
 *  The class \c Problem provides a class for managing the interface to all
 *  optimization problems.   This class defines a letter-envelope idiom that
 *  leaves the specific details of how the problem is represented to
 *  an \c Application object.
 *
 *  The methods for \c Problem define envelope operations.
 *  The \c app method provides access to the application object.
 *
 *  NOTE: this letter/envelope idiom is somewhat different from a canonical
 *  design in that the ApplicationManager class is responsible for
 *  managing applications, and for translating applications between different
 *  problem types.  Further, this class manages the memory for these objects.
 */
template <class ProblemT>
class Problem
{
public:

   /// Generic constructor
   Problem()
      : app(NULL),
        app_handle()
   {}

   /// Generic constructor
   Problem( Application<ProblemT>* app_ )
      : app(app_),
        app_handle()
   {
      if ( app != NULL )
         app_handle = app_->get_handle();
   }

   /// Copy constructor
   template <class LProblemT>
   Problem(const Problem<LProblemT>& rhs)
   {
      ( *this ) = rhs;
   }


   /// Returns true if there is no application handle.
   bool empty() const
   { return app == NULL; }

   ///
   operator ApplicationHandle() const
   {
      return app_handle;
   }

   ///
   operator Application_Base*() const
   {
      return app;
   }

   /// Returns the application handle
   Application<ProblemT>* application()
   { 
      assert( app_handle.object() == app );
      return app; 
   }

   /// Returns the application handle
   const Application<ProblemT>* application() const
   { 
      assert( app_handle.object() == app );
      return app; 
   }

   /// Returns the embedded application pointer
   Application<ProblemT>* operator->()
   { return application(); }

   /// Returns the embedded const application pointer
   const Application<ProblemT>* operator->() const
   { return application(); }

   /// Returns the embedded application pointer
   Application<ProblemT>& operator*()
   { return *application(); }

   /// Returns the embedded const application pointer
   const Application<ProblemT>& operator*() const
   { return *application(); }

   /// Set the application object
   template<typename T>
   T* set_application( std::pair< ApplicationHandle, T* > new_app )
   { 
      app = new_app.second; 
      app_handle = new_app.first;
      return new_app.second;
   }

   /// Set the application object
   void set_application(Application<ProblemT>* new_app)
   { 
      app = new_app; 
      if ( app == NULL )
         app_handle = ApplicationHandle();
      else
         app_handle = app->get_handle();
   }

   /// Set the application object
   void set_application(Application<ProblemT>& new_app)
   { 
      app = &new_app;
      app_handle = new_app.get_handle();
   }

   template<typename LProblemT>
   Problem<ProblemT>& operator=(const Problem<LProblemT>& rhs)
   {
      if ( rhs.empty() )
      {
         app = NULL;
         app_handle = ApplicationHandle();
      }
      else
      {
         ProblemMngr().lexical_cast(rhs, *this);
      }

      return *this;
   }

   Problem<ProblemT>& operator=(const Problem<ProblemT>& rhs)
   {
      app = rhs.app;
      app_handle = rhs.app_handle;
      return *this;
   }


private:

   /// The (derived) optimization application
   Application<ProblemT>* app;

   /// A handle on the original application object (to help keep it in scope)
   ApplicationHandle  app_handle;
};

} // namespace colin


// This is a *very* unusual place to include a header file; however, it
// is intentional.  There is a tight linkage between Application and
// Problem: both cause the other to be instantiated.  Anyone who
// includes one header MUST also include the other.  However, the order
// in which they are included definitely matters (Problem must be
// above Application) -- and the error tossed by the compiler for
// getting it wrong is not at all obvious.  By having both header files
// include the other (and relying on the #ifdef guards), we can
// guarantee that things will compile correctly.
#include <colin/Application.h>

#endif
