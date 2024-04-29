#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2024
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

include(DakotaUnitTest)

# List of test files to copy into this directory
#set(dakota_unit_test_copied_files)

dakota_add_unit_test(NAME DataScalerTest
  SOURCES DataScalerTest.cpp
  LINK_LIBS dakota_util
  )

dakota_add_unit_test(NAME MathToolsTest
  SOURCES MathToolsTest.cpp
  LINK_LIBS dakota_util
  )

dakota_add_unit_test(NAME MetricsTest
  SOURCES MetricsTest.cpp
  LINK_LIBS dakota_util
  )

dakota_add_unit_test(NAME TeuchosEigenConversionTest
  SOURCES TeuchosEigenConversion.cpp
  LINK_LIBS dakota_util
  )

dakota_add_unit_test(NAME LinearSolverTest
  SOURCES LinearSolverTest.cpp
  LINK_LIBS dakota_util
  )

#target_include_directories(DataScalerTest PRIVATE
#  "${CMAKE_CURRENT_SOURCE_DIR}/.." "${Teuchos_INCLUDE_DIRS}")

# Add a single target for all copied files
#add_custom_target(util_unit_test_files ALL
#  DEPENDS ${util_unit_test_copied_files})

if(DAKOTA_PYTHON AND DAKOTA_PYBIND11)
  pybind11_add_module(pybind11_example pybind11_example.cpp)
  # BMA TODO: couldn't get to work using .py in source tree
  configure_file(pybind11_example.py pybind11_example.py COPYONLY)
  add_test(NAME dakota_util_pybind11_example
    COMMAND ${Python_EXECUTABLE} pybind11_example.py)
  set_tests_properties(dakota_util_pybind11_example PROPERTIES LABELS Unit)
endif()
