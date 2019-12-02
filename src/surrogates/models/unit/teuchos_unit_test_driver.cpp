/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
    _______________________________________________________________________ */

#include <Teuchos_UnitTestHarness.hpp> 

int main(int argc,char**argv)
{
  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  int ierr = Teuchos::UnitTestRepository::runUnitTestsFromMain(argc, argv);

  return ierr;
}
