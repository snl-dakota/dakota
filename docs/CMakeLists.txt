# _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
# _______________________________________________________________________

# DakotaDocs can be built as a separate project, though requires full
# dakota source tree and for packaging, Dakota_FINAL_PACKAGE_LABEL and
# Dakota_VERSION_STRING
project(DakotaDocs LANGUAGES NONE)
cmake_minimum_required(VERSION 3.17)

# TODO: make documentation build cross-platform

find_package(Python REQUIRED)

# Required for generated included input files
find_package(Perl REQUIRED)

# create Latex header with Dakota version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DakotaDefs.tex.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/DakotaDefs.tex)

find_program(_dakota_sphinx_exe NAMES sphinx-build sphinx-build.exe)
if(NOT _dakota_sphinx_exe)
  message(FATAL_ERROR "Dakota docs require sphinx-build executable")
endif()

# Duplicate build of java utils
#set(Dakota_SOURCE_DIR "..")
#file(REAL_PATH ".." Dakota_SOURCE_DIR)
if( NOT Dakota_SOURCE_DIR)
  get_filename_component(Dakota_SOURCE_DIR ".." ABSOLUTE) 
endif()

set(CMAKE_MODULE_PATH 
  "${Dakota_SOURCE_DIR}"
  "${Dakota_SOURCE_DIR}/cmake"
  "${Dakota_SOURCE_DIR}/cmake/semsCMake" ${CMAKE_MODULE_PATH})
include(DakotaVersion)
include(DakotaVersionUtils)
include(DakotaCPack)
include(ListFilterEmacsBackups)
include(DakotaTestHelpers)
include(DakotaUsersExamples)

dakota_version_set_helper_vars()

# Keyword reference dependencies
file(GLOB ref_kw_md "keywords/*")
list_filter_emacs_backups(ref_kw_md)

# TODO: Generate all reference manual to build tree

# Generate the ultimate RST files for ref man keywords, specifying
# only required dependencies. The Python generator takes dakota.json
# together with reference metadata files to create reference manual
# keyword pages.
add_custom_command( 
  OUTPUT ${DakotaDocs_SOURCE_DIR}/user/usingdakota/reference/environment.rst
  DEPENDS ${Dakota_SOURCE_DIR}/src/dakota.json
          ${DakotaDocs_SOURCE_DIR}/generate_keyword_reference.py
	  ${ref_kw_md}
  COMMAND ${CMAKE_COMMAND}
  ARGS -E make_directory ${DakotaDocs_SOURCE_DIR}/user/usingdakota/reference
  COMMAND ${Python_EXECUTABLE}
  ARGS ${DakotaDocs_SOURCE_DIR}/generate_keyword_reference.py
    ${Dakota_SOURCE_DIR}/src/dakota.json
    ${DakotaDocs_SOURCE_DIR}/keywords
    ${DakotaDocs_SOURCE_DIR}/user/usingdakota/reference
  COMMENT "Generating keyword reference RST files to ${DakotaDocs_SOURCE_DIR}/user/usingdakota/reference/"
  )

add_custom_target(docs-keywords
  DEPENDS ${DakotaDocs_SOURCE_DIR}/user/usingdakota/reference/environment.rst
)

# Generated sample input files (duplicate of process in test/ for now
# so docs can build on their own...)
dakota_generate_test_properties("${Dakota_SOURCE_DIR}/test"
  "${CMAKE_CURRENT_BINARY_DIR}" test_props)
# Each line is "generated file source file test number"
file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/dakota_usersexamples.props"
  users_test_inputs)
file(MAKE_DIRECTORY "${DakotaDocs_SOURCE_DIR}/user/usingdakota/samples")
generate_users_examples_interfacing(
  "${DakotaDocs_SOURCE_DIR}/user/usingdakota/samples" _gen_interfacing_var_out)
generate_users_examples("${users_test_inputs}"
  "${DakotaDocs_SOURCE_DIR}/user/usingdakota/samples" _gen_inputs_var_out)

add_custom_target(docs-user-samples DEPENDS ${_gen_inputs_var_out} ${_gen_interfacing_var_out})


function(insert_dakota_version _conf_py_filename)
  file(READ "${_conf_py_filename}" _conf_data)
  string(REGEX REPLACE "version = '[0-9]+\\.[0-9]+'"
    "version = '${Dakota_VERSION_PAIR}'" _conf_data "${_conf_data}")
  string(REGEX REPLACE "release = '[0-9]+\\.[0-9]+\\.[0-9]+\\+?'"
    "release = '${Dakota_VERSION_STRING}'" _conf_data "${_conf_data}")
  file(WRITE "${_conf_py_filename}" "${_conf_data}")
endfunction()

# NOTE: This only happens on re-configure
#
# TODO: Need build time dep on this and ideally generate to build tree
# and perhaps then use sphinx-build -c ${DakotaDocs_BINARY_DIR}/user
insert_dakota_version("${DakotaDocs_SOURCE_DIR}/user/conf.py")

# Serial build assuming all files are stale
add_custom_target(docs-user
  COMMAND "${_dakota_sphinx_exe}" -a -E -b html ${DAKOTA_SPHINX_DEFS} "${DakotaDocs_SOURCE_DIR}/user" "${CMAKE_CURRENT_BINARY_DIR}/user-html"
  COMMENT "Generating user-html with Sphinx:
  ${_dakota_sphinx_exe} -a -E -b html ${DAKOTA_SPHINX_DEFS} ${DakotaDocs_SOURCE_DIR}/user ${CMAKE_CURRENT_BINARY_DIR}/user-html"
)
add_dependencies(docs-user docs-keywords docs-user-samples)

# Parallel build assuming all files are stale
add_custom_target(docs-user-fast
  COMMAND "${_dakota_sphinx_exe}" -a -E -b html -j auto ${DAKOTA_SPHINX_DEFS} "${DakotaDocs_SOURCE_DIR}/user" "${CMAKE_CURRENT_BINARY_DIR}/user-html"
  COMMENT "Generating user-html with Sphinx (full rebuild):
  ${_dakota_sphinx_exe} -a -E -b html -j auto ${DAKOTA_SPHINX_DEFS} ${DakotaDocs_SOURCE_DIR}/user ${CMAKE_CURRENT_BINARY_DIR}/user-html"
)
add_dependencies(docs-user-fast docs-keywords docs-user-samples)

# Serial build of only changed files
add_custom_target(docs-user-increm
  COMMAND "${_dakota_sphinx_exe}" -b html ${DAKOTA_SPHINX_DEFS} "${DakotaDocs_SOURCE_DIR}/user" "${CMAKE_CURRENT_BINARY_DIR}/user-html"
  COMMENT "Generating user-html with Sphinx:
  ${_dakota_sphinx_exe} -b html ${DAKOTA_SPHINX_DEFS} ${DakotaDocs_SOURCE_DIR}/user ${CMAKE_CURRENT_BINARY_DIR}/user-html"
)
add_dependencies(docs-user-increm docs-keywords docs-user-samples)


add_custom_target(docs-user-clean
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_BINARY_DIR}/user-html"
  COMMENT "Cleaning docs/user-html"
  )

option(DAKOTA_DOCS_DEV "Enable Dakota C++ developer docs" ON)
if(DAKOTA_DOCS_DEV)
  add_subdirectory(dev)
endif()

# Generate HTML Dev
add_custom_target(docs)
add_dependencies(docs docs-user)
if(DAKOTA_DOCS_DEV)
  add_dependencies(docs docs-dev-html)
endif()

# Generate PDF Dev
# TODO: depend on sphinx users
#add_custom_target(docs-pdf)
#if(DAKOTA_DOCS_DEV)
#  add_dependencies(docs-pdf docs-dev-pdf)
#endif()

# Package docs for nightly builds (HTML and PDF for dev)
# TODO: Rework this into custom command(s) and bind into target
# TODO: Better handling of html-dev instead of symlink
# Define convenience variables used here and in subdirs, such as
#   Dakota_VERSION_{PAIR, TRIPLE, STRING, SRC}
dakota_docs_package_label("${DAKOTA_PACKAGE_LABEL}" Dakota_FINAL_PACKAGE_LABEL)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/label/label.txt" "${Dakota_FINAL_PACKAGE_LABEL}")
add_custom_target(package_docs
  COMMAND ${CMAKE_COMMAND} -E
    tar czf user-html-${Dakota_FINAL_PACKAGE_LABEL}.tar.gz user-html
  COMMAND ${CMAKE_COMMAND} -E create_symlink dev/html-dev html-dev
  COMMAND ${CMAKE_COMMAND} -E
    tar czf html-dev-${Dakota_FINAL_PACKAGE_LABEL}.tar.gz "html-dev/*"
#  COMMAND ${CMAKE_COMMAND} -E copy
#    dev/latex-dev/refman.pdf Developers-${Dakota_FINAL_PACKAGE_LABEL}.pdf
  COMMAND ${CMAKE_COMMAND} -E
    tar czf dakota-${Dakota_FINAL_PACKAGE_LABEL}-docs.tar.gz
      user-html
      user-html-${Dakota_FINAL_PACKAGE_LABEL}.tar.gz
      "html-dev/*"
      html-dev-${Dakota_FINAL_PACKAGE_LABEL}.tar.gz
#      Developers-${Dakota_FINAL_PACKAGE_LABEL}.pdf
  COMMENT "Packaging docs"
  )
add_dependencies(package_docs docs ) #docs-pdf


if(DAKOTA_DOCS_LEGACY)
  add_subdirectory(inactive)
endif()

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES 
  "dakota-${Dakota_FINAL_PACKAGE_LABEL}-docs.tar.gz;html-dev;html-dev-${Dakota_VERSION_STRING}.tar.gz;user-html;user-html-${Dakota_VERSION_STRING}.tar.gz"
  #"dakota-${Dakota_FINAL_PACKAGE_LABEL}-docs.tar.gz;Developers-${Dakota_VERSION_STRING}.pdf;html-dev;html-dev-${Dakota_VERSION_STRING}.tar.gz;user-html;user-html-${Dakota_VERSION_STRING}.tar.gz"
  )
