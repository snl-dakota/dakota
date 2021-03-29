#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

add_subdirectory(advanced)
if(DAKOTA_ENABLE_TESTS)
  add_subdirectory(users)
else()
  message(WARNING 
    "Users Manual examples will not be built; requires DAKOTA_ENABLE_TESTS")
endif()
add_subdirectory(script_interfaces/generic)
