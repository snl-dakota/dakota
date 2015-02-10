
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
