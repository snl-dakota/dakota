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
 * \file UpcastApplication.h
 *
 * Defines the colin::UpcastApplication class
 */

#ifndef colin_UpcastApplication_h
#define colin_UpcastApplication_h

#include <acro_config.h>
#include <colin/Application.h>
#include <colin/reformulation/Base.h>

namespace colin
{

/// Cast an application with "less" functionality into a more complex one
/** This class casts an application with "less" functionality into an
 *  application with "more".  For example, an NLP_problem to an
 *  MINLP_problem (with no integer component).  This is one of the
 *  simplest of all reformulation applications and does not actually
 *  need to \em do anything (the defaults in the underlying
 *  Application_Base transformation mechanisms will handle the
 *  mapping and error checking).
 */
template<class ProblemT>
class UpcastApplication
   : public Application<ProblemT>,
     public ReformulationApplication
{
public: // methods

   /// Constructor
   UpcastApplication()
   { }

   /// Constructor
   UpcastApplication(ApplicationHandle src)
   {
      reformulate_application(src);
   }

   /// Destructor
   virtual ~UpcastApplication()
   { }


protected:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      // verify this is actually as *upcast* (or at least, not a downcast)
      if (( ProblemT::value & handle->problem_type() )
          ^ handle->problem_type() )
         EXCEPTION_MNGR(std::runtime_error, "UpcastApplication::"
                        "set_base_application(): The upcast problem type, "
                        << ProblemType<ProblemT>::name() << 
                        ", is not a superset of the original problem type, "
                        << handle->problem_type_name());
   }
   
   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info (including the
      // objective... the properties aren't changing, only the way we
      // calculate it.)

      // NB: this requires the silent omission of properties in this
      // application that do not exist in the base application. (While
      // this behavior is currently the default, it wasn't before the
      // move from shared data objects to Properties)
      this->reference_reformulated_application_properties();
   }
};


} // namespace colin

#endif // defined colin_UpcastApplication_h
