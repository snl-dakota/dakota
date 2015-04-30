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
 * \file DowncastApplication.h
 *
 * Defines the colin::DowncastApplication class
 */

#ifndef colin_DowncastApplication_h
#define colin_DowncastApplication_h

#include <acro_config.h>
#include <colin/Application.h>
#include <colin/reformulation/Base.h>

namespace colin
{

/// Cast an application with "more" functionality into a simpler one
/** This class casts an application with "more" functionality into an
 *  application with "less".  For example, an NLP2_problem to an
 *  NLP1_problem.  This is the simplest of all reformulation
 *  applications and does not actually need to \em do anything (the
 *  defaults in the underlying Application_Base transformation
 *  mechanisms will handle the mapping and error checking).
 */
template<class ToProblemT>
class DowncastApplication 
   : public Application<ToProblemT>,
     public ReformulationApplication
{
public: // methods

   /// Constructor
   DowncastApplication()
   { }

   /// Constructor
   DowncastApplication(ApplicationHandle src)
   { 
      reformulate_application(src);
   }

   /// Destructor
   virtual ~DowncastApplication()
   { }

private:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      // verify this is actually a *downcast* (or at least, not an upcast)
      if (( ToProblemT::value & handle->problem_type() ) ^ ToProblemT::value 
          || ToProblemT::value == handle->problem_type() )
         EXCEPTION_MNGR(std::runtime_error, "DowncastApplication::"
                        "validate_reformulated_application(): The downcast "
                        "problem type, " << ProblemType<ToProblemT>::name() << 
                        ", is not a subset of the original problem type, "
                        << handle->problem_type_name());
   }

   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info (including the
      // objective... the properties aren't changing!)
      this->reference_reformulated_application_properties();
   }
};


} // namespace colin

#endif // defined colin_DowncastApplication_h
