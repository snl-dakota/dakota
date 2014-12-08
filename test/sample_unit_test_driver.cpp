#include <Teuchos_UnitTestHarness.hpp> 

int main(int argc,char**argv)
{
  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  int ierr = Teuchos::UnitTestRepository::runUnitTestsFromMain(argc, argv);

  return ierr;
}
