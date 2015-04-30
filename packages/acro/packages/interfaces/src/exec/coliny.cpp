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

//
// coliny.cpp
//

#include <acro_config.h>
#include <colin/ColinDriver.h>
#include <utilib/exception_mngr.h>

#ifdef ACRO_USING_SCOLIB
#include <scolib/SCORegistrations.h>
#endif

#include <interfaces/InterfacesRegistrations.h>

int main(int argc, char* argv[])
{
   std::stringstream registrations_ok;

   // bring in (register) each of the interfaces from this package
   if ( interfaces::StaticInitializers::static_interfaces_registrations )
      registrations_ok
         << "\tinterfaces (" 
         << interfaces::StaticInitializers::static_interfaces_registrations 
         << ")" << std::endl;

#ifdef ACRO_USING_SCOLIB
   // bring in (register) the scolib package solvers, if they is present
   if ( scolib::StaticInitializers::static_scolib_registrations )
      registrations_ok 
         << "\tscolib (" 
         << scolib::StaticInitializers::static_scolib_registrations
         << ")" << std::endl;
#endif

   if ( ! registrations_ok.str().empty() )
      EXCEPTION_MNGR(std::runtime_error, "Coliny: main(): The following Acro "
                     "solver plugin library registrations failed:"
                     << std::endl << registrations_ok.str());
   
   return colin::colin_driver(argc, argv, "coliny");
}
