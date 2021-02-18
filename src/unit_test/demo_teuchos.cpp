/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include <ctype.h>
#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(demo_teuchos, hello_world)
{
  const std::string greet("Hello World");

  std::string greetlow = greet;
  for(unsigned int i = 0; i < greetlow.length(); i++)
    greetlow[i] = tolower(greetlow[i]);

  TEST_EQUALITY( "hello world", greetlow );
}

//----------------------------------------------------------------
