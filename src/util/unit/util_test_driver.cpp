/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <Teuchos_UnitTestHarness.hpp>

int main(int argc, char** argv) {
  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  int ierr = Teuchos::UnitTestRepository::runUnitTestsFromMain(argc, argv);

  return ierr;
}
