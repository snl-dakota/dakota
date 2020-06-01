#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

add_subdirectory(advanced)
add_subdirectory(cantilever)
if(DAKOTA_ENABLE_TESTS)
  add_subdirectory(users)
else()
  message(WARNING 
    "Users Manual examples will not be built; requires DAKOTA_ENABLE_TESTS")
endif()
add_subdirectory(script_interfaces/generic)
if(DAKOTA_ENABLE_TESTS)
  if(WIN32)
    message(WARNING "Training examples solutions cannot be built on Windows")
  else()
    add_subdirectory(training/solutions)
  endif()
else()
  message(WARNING 
    "Training examples solutions will not be built; requires DAKOTA_ENABLE_TESTS")
endif()

#add_subdirectory(training/201508/solutions/calibration/cantilever)
#add_subdirectory(training/201508/solutions/sens_analysis/1)
#add_subdirectory(training/201508/solutions/sens_analysis/2)
#add_subdirectory(training/201508/solutions/sens_analysis/3)
