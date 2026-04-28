#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Build-tree assembly and install rules for the Dakota Python package.

if(DAKOTA_PYTHON_PATH)
  set(DAKOTA_PYTHON_BUILD_DIR "${DAKOTA_PYTHON_PATH}")
else()
  set(DAKOTA_PYTHON_BUILD_DIR "${Dakota_BINARY_DIR}/python")
endif()
set(DAKOTA_PYTHON_PATH "${DAKOTA_PYTHON_BUILD_DIR}" CACHE INTERNAL
  "Dakota build-tree Python package root" FORCE)
set(DAKOTA_PYTHON_PATH "${DAKOTA_PYTHON_BUILD_DIR}" PARENT_SCOPE)

file(GLOB_RECURSE dakota_python_package_files CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/dakota/*.py")

set(dakota_python_package_stamp
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_python_package.stamp")

add_custom_command(
  OUTPUT "${dakota_python_package_stamp}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${DAKOTA_PYTHON_BUILD_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/dakota"
          "${DAKOTA_PYTHON_BUILD_DIR}/dakota"
  COMMAND ${CMAKE_COMMAND} -E touch "${dakota_python_package_stamp}"
  DEPENDS ${dakota_python_package_files}
  COMMENT "Assembling Dakota Python package in the build tree"
  VERBATIM
)

add_custom_target(dakota_python_package ALL
  DEPENDS "${dakota_python_package_stamp}")

if(DAKOTA_ENABLE_TESTS AND Python_Interpreter_FOUND AND
    Python_VERSION VERSION_GREATER 2.6)
  add_test(NAME dakota_interfacing_test
    COMMAND ${Python3_EXECUTABLE} -B
            "${Dakota_SOURCE_DIR}/interfaces/Python/dakota_interfacing_test.py")
  set_property(TEST dakota_interfacing_test PROPERTY LABELS Unit Python)
  if(WIN32)
    set_property(TEST dakota_interfacing_test PROPERTY
      ENVIRONMENT "PYTHONPATH=${DAKOTA_PYTHON_BUILD_DIR};$ENV{PYTHONPATH}")
  else()
    set_property(TEST dakota_interfacing_test PROPERTY
      ENVIRONMENT "PYTHONPATH=${DAKOTA_PYTHON_BUILD_DIR}:$ENV{PYTHONPATH}")
  endif()
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dakota/"
  DESTINATION "share/dakota/Python/dakota"
  PATTERN "__pycache__" EXCLUDE
  PATTERN "*.py[cdo]" EXCLUDE
  PATTERN "environment/__init__.py" EXCLUDE
)
