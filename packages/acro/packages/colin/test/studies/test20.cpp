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

#include <iostream>
#include <colin/ColinDriver.h>
#include <colin/ColinGlobals.h>
#include <colin/ApplicationMngr.h>

int test_test20a(int , char**)
{
   std::cout << std::endl;
   std::cout << "Test20a - Running COLIN from test20a.xml" << std::endl;

   colin::ColinGlobals::output_level = "quiet";
   char* argv[] = { "test", "test20a.xml" };
   int status = colin::colin_driver(2, argv);
   colin::ApplicationMngr().clear();
   return status;
}

int test_test20b(int , char**)
{
   std::cout << std::endl;
   std::cout << "Test20b - Running COLIN from test20b.xml" << std::endl;

   colin::ColinGlobals::output_level = "quiet";
   char* argv[] = { "test", "test20b.xml" };
   int status = colin::colin_driver(2, argv);
   colin::ApplicationMngr().clear();
   return status;
}

int test_test20c(int , char**)
{
   std::cout << std::endl;
   std::cout << "Test20c - Running COLIN from test20c.xml" << std::endl;

   colin::ColinGlobals::output_level = "quiet";
   char* argv[] = { "test", "test20c.xml" };
   int status = colin::colin_driver(2, argv);
   colin::ApplicationMngr().clear();
   return status;
}

int test_test20d(int , char**)
{
   std::cout << std::endl;
   std::cout << "Test20d - Running COLIN from test20d.xml" << std::endl;

   colin::ColinGlobals::output_level = "quiet";
   char* argv[] = { "test", "test20d.xml" };
   int status = colin::colin_driver(2, argv);
   colin::ApplicationMngr().clear();
   return status;
}

int test_test20e(int , char**)
{
   std::cout << std::endl;
   std::cout << "Test20e - Running COLIN from test20e.xml" << std::endl;

   colin::ColinGlobals::output_level = "quiet";
   char* argv[] = { "test", "test20e.xml" };
   int status = colin::colin_driver(2, argv);
   colin::ApplicationMngr().clear();
   return status;
}

int test_test20f(int , char**)
{
   std::cout << std::endl;
   std::cout << "Test20f - Running COLIN from test20f.xml" << std::endl;

   colin::ColinGlobals::output_level = "verbose";
   char* argv[] = { "test", "test20f.xml" };
   int status = colin::colin_driver(2, argv);
   colin::ApplicationMngr().clear();
   return status;
}

