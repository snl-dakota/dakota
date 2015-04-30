/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <utilib/Parameter.h>
#include <utilib/stl_auxiliary.h>
#include <sstream>

namespace utilib
{

void Parameter::set_value_with_string(const std::string& value)
{
   try
   {
      if ((action == store_true) || (action == store_false))
      {
         if (value != "")
            EXCEPTION_MNGR(std::runtime_error, "Boolean parameter specified with unexpected value '" << value << "'");
         if ( info.is_type<bool>() )
            anyref_cast<bool>(info) = (action == store_true);
         else
         {
            std::stringstream sbuf;
            sbuf << (action == store_true);
            sbuf >> info;
         }
      }
      else if (action == store_value)
      {
         std::stringstream sbuf;
         sbuf << value;

         if (info.is_type<bool>())
            parse(sbuf,info.expose<bool>());
         else
            sbuf >> info;
      }
      else if (action == append_value)
      {
         std::stringstream sbuf;
         sbuf << value;
         sbuf >> list_info;
         (*list_functor)(info, list_info);
      }
      initialized = true;
   }
   catch (const bad_any_cast &e)
   {
      EXCEPTION_MNGR(runtime_error, "Parameter::set_parameter_with_string - "
                     "bad parameter type for \"" << name << "\":" 
                     << std::endl << e.what());
   }
}

}
