cmake_minimum_required(VERSION 2.8)
project(Acro CXX C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_MODULE_PATH})

set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING 
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

#assume any modern compiler has STD
add_definitions(-DHAVE_STD)

function(AcroCheckDirExists dirPath)
  get_filename_component(dirName ${dirPath} NAME)
  if(EXISTS ${dirPath})
    string(TOUPPER "${dirName}" DIRNAME)
    option(HAVE_${DIRNAME} "Build with ${dirName} enabled?" ON)
  endif()
endfunction()


option(ACRO_HAVE_DLOPEN "Enable dlopen in Acro" ON)
if (ACRO_HAVE_DLOPEN)
  find_package(DL)
  if (DL_FOUND)
    # Use the same definition as autotools build
    add_definitions("-DHAVE_DLOPEN")
    # TODO: add include path in relevant subdirs?
  else()
    message(WARNING "dlopen requested, but not found")
  endif()
endif()

option(ENABLE_TESTS "Enable testing in Acro" OFF)

if(ENABLE_TESTS)

  # Install Python virtual environment for testing
  find_package(PythonInterp)
  if (NOT PYTHONINTERP_FOUND)
    message(SEND_ERROR "Python interpreter required for ENABLE_TESTS")
  endif()

  file(GLOB acro_python_packages packages/*/python)
  foreach(x ${acro_python_packages})
    list(APPEND PYTHON_PACKAGES " -p %s/${x}")
  endforeach()

  # TODO: OUTPUT python binary  OUTPUT python/bin/python
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/python
           ${CMAKE_CURRENT_BINARY_DIR}/python.log
	   ${CMAKE_CURRENT_BINARY_DIR}/python/bin/cxxtestgen
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/admin/vpy/python.zip
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tpl/vpykit/bin/install_python
            ${CMAKE_CURRENT_SOURCE_DIR}/admin/vpy/dev.ini
    COMMAND ${CMAKE_COMMAND}
    ARGS -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/python
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/tpl/vpykit/bin/install_python
         --src ${CMAKE_CURRENT_SOURCE_DIR}
	 --build ${CMAKE_CURRENT_BINARY_DIR}
	 --logfile ${CMAKE_CURRENT_BINARY_DIR}/python.log
	 -c ${CMAKE_CURRENT_SOURCE_DIR}/admin/vpy/dev.ini
	 -z ${CMAKE_CURRENT_SOURCE_DIR}/admin/vpy/python.zip
	 -p %s/admin/acro-admin
	 -p %s/tpl/cxxtest/python
	 ${PYTHON_PACKAGES}
  )

  add_custom_target(acro-python-env DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/python)
  add_custom_target(acro-cxxtestgen
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/python/bin/cxxtestgen)

endif() # ENABLE_TESTS

# In Acro, presence of a local tecuhos/optpp means Acro needs it,
# but it might be provided externally by the user, e.g., in Trilinos.
# Take care in case HAVE_TEUCHOS set by a parent instead of locally.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tpl/teuchos)
  option(HAVE_TEUCHOS "Build with Teuchos enabled?" ON)
  # when building inside Trilinos, the path to Teuchos will already be set
  if (NOT BUILD_IN_TRILINOS)
    # first probe for system-installed Trilinos
    # this will respect Trilinos_DIR if already set
    find_package(Trilinos QUIET)
    if (NOT Trilinos_DIR)
      # if no one has configured a local src Teuchos, do so
      # this will respect Teuchos_DIR if already set
      if(NOT Teuchos_DIR)
        set(Teuchos_DIR ${CMAKE_CURRENT_BINARY_DIR}/tpl/teuchos)
        message("Setting Teuchos_DIR to ${Teuchos_DIR}")
        add_subdirectory(tpl/teuchos)
      else()
        message("in ${CMAKE_CURRENT_BINARY_DIR} Teuchos_DIR already set to ${Teuchos_DIR}")
      endif()
      find_package(Teuchos NO_MODULE REQUIRED)
    else()
      message("Using system trilinos in ${Trilinos_DIR}")
    endif()
  endif()
endif()

add_subdirectory(tpl)

set(Coin_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpl/coin-cbc/cbc)
set(TINYXML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpl/tinyxml)
set(COBYLA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tpl/3po/cobyla2c)

add_subdirectory(packages)
