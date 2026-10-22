#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Script for install-python / uninstall-python targets.
# Invoked with cmake -P after running the normal install.
#
# Required inputs:
#   -DPython3_EXECUTABLE=<path>       interpreter Dakota was configured with
#   -DDAKOTA_PYTHON_ROOT=<path>       <prefix>/share/dakota/Python
#   -DACTION=install|uninstall

cmake_minimum_required(VERSION 3.17)

if(NOT Python3_EXECUTABLE)
  message(FATAL_ERROR "Python3_EXECUTABLE must be set")
endif()
if(NOT DAKOTA_PYTHON_ROOT)
  message(FATAL_ERROR "DAKOTA_PYTHON_ROOT must be set")
endif()
if(NOT ACTION MATCHES "^(install|uninstall)$")
  message(FATAL_ERROR "ACTION must be install or uninstall")
endif()

# Skip staged installs — the .pth must go into the live interpreter env.
if(DEFINED ENV{DESTDIR} AND NOT "$ENV{DESTDIR}" STREQUAL "")
  message(STATUS "DESTDIR is set; skipping dakota.pth registration for staged install.")
  return()
endif()

if(ACTION STREQUAL "install")
  # Sanity-check that the package was actually installed.
  if(NOT EXISTS "${DAKOTA_PYTHON_ROOT}/dakota/__init__.py")
    message(FATAL_ERROR
      "Expected ${DAKOTA_PYTHON_ROOT}/dakota/__init__.py to exist after install. "
      "Run 'make install' before 'make install-python', or use 'make install-python' "
      "which runs install automatically.")
  endif()
endif()

# Ask the interpreter where to put the .pth file.
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -c
    "import os, site, sysconfig, sys\n\
in_venv   = (sys.prefix != sys.base_prefix)\n\
in_conda  = os.path.isdir(os.path.join(sys.prefix, 'conda-meta'))\n\
if in_venv or in_conda:\n\
    target = sysconfig.get_paths()['purelib']\n\
else:\n\
    if not site.ENABLE_USER_SITE:\n\
        raise SystemExit('User site-packages is disabled (ENABLE_USER_SITE=False). '\n\
                         'Activate a venv or conda env before running make install-python.')\n\
    target = site.getusersitepackages()\n\
print(target)"
  OUTPUT_VARIABLE _pth_dir
  ERROR_VARIABLE  _pth_err
  RESULT_VARIABLE _pth_rc
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT _pth_rc EQUAL 0)
  message(FATAL_ERROR "Could not determine site-packages directory:\n${_pth_err}")
endif()

# Warn if the active env differs from the configured interpreter.
if(DEFINED ENV{VIRTUAL_ENV} AND NOT "$ENV{VIRTUAL_ENV}" STREQUAL "")
  execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import sys; print(sys.prefix)"
    OUTPUT_VARIABLE _interp_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT "$ENV{VIRTUAL_ENV}" STREQUAL "${_interp_prefix}")
    message(WARNING
      "VIRTUAL_ENV=$ENV{VIRTUAL_ENV} differs from the interpreter's sys.prefix "
      "(${_interp_prefix}). dakota.pth will be written for ${Python3_EXECUTABLE}, "
      "not for the active venv.")
  endif()
elseif(DEFINED ENV{CONDA_PREFIX} AND NOT "$ENV{CONDA_PREFIX}" STREQUAL "")
  execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import sys; print(sys.prefix)"
    OUTPUT_VARIABLE _interp_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT "$ENV{CONDA_PREFIX}" STREQUAL "${_interp_prefix}")
    message(WARNING
      "CONDA_PREFIX=$ENV{CONDA_PREFIX} differs from the interpreter's sys.prefix "
      "(${_interp_prefix}). dakota.pth will be written for ${Python3_EXECUTABLE}, "
      "not for the active conda env.")
  endif()
endif()

set(_pth_file "${_pth_dir}/dakota.pth")

if(ACTION STREQUAL "install")
  file(MAKE_DIRECTORY "${_pth_dir}")
  file(WRITE "${_pth_file}" "${DAKOTA_PYTHON_ROOT}\n")
  message(STATUS "Wrote ${_pth_file}")
  message(STATUS "Verify with: ${Python3_EXECUTABLE} -c \"import dakota\"")
else()
  if(EXISTS "${_pth_file}")
    file(REMOVE "${_pth_file}")
    message(STATUS "Removed ${_pth_file}")
  else()
    message(STATUS "Nothing to remove: ${_pth_file} does not exist")
  endif()
endif()
