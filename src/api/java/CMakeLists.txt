#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Build the Dakota Java API underneath
find_package(SWIG 2.0 REQUIRED)
FIND_PACKAGE(JNI REQUIRED)

include(AddFileCopyCommand)

set(PACKAGE "gov.sandia.dart.dakota.jni")
set(PACKAGEDIR "gov/sandia/dart/dakota/jni")

# Setup link flags
#LDFLAGS=-rdynamic -L${DAKLIB} -Wl,-rpath=${DAKLIB} ${LIBS}
#get_target_properties(DAKOTA_LINK dakota LINK_FLAGS)
#set(LDFLAGS -rdynamic ${DAKOTA_LINK})
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -rdynamic")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

# preprocessor defines are inherited from parent, but only for CMake targets
set(SWIG_CPPFLAGS ${CPPFLAGS} -I${Dakota_SOURCE_DIR}/src)
foreach(def ${Dakota_DEFINES})
  list(APPEND SWIG_CPPFLAGS -D${def})
endforeach()

add_custom_command(
  # TODO: SWIG generates the .java files too
  OUTPUT dakota_swig_wrap.h dakota_swig_wrap.cpp
  # TODO: also depends on Dakota headers
  DEPENDS dakota_swig.i DakotaUtils.hpp DakotaExtras.hpp
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGEDIR}
  COMMAND ${SWIG_EXECUTABLE} -c++ -java -package ${PACKAGE} -outdir ${PACKAGEDIR} ${SWIG_CPPFLAGS} -o dakota_swig_wrap.cpp ${CMAKE_CURRENT_SOURCE_DIR}/dakota_swig.i 
)

# Generate the main dynamic library, inheriting preprocessor defines
# from parent
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${Dakota_SOURCE_DIR}/src
		    ${JNI_INCLUDE_DIRS})
add_library(dakotajni 
  dakota_swig_wrap.cpp
  DakotaExtras.cpp
  DakotaUtils.cpp
  DartSerialDirectApplicInterface.cpp
)
target_link_libraries(dakotajni ${DAKOTA_ALL_LIBS})
set_target_properties(dakotajni PROPERTIES COMPILE_FLAGS "${CPPFLAGS} -fPIC -Wno-deprecated")
# -fPIC shouldn't be necessary on .so platforms:
set_target_properties(dakotajni PROPERTIES LINK_FLAGS "${LDFLAGS} -fPIC -shared")

# TODO: would prefer not to have another target
# Build SWIG-generated .java into .class files hanging the dep off the library
#add_custom_command(TARGET dakotajni POST_BUILD
add_custom_target(dakotajni-class ALL
  # Weak dependency management for now
  DEPENDS dakota_swig_wrap.cpp
  #OUTPUT ${PACKAGEDIR}/DakotaWrapper.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -cp ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGEDIR}/*.java
)


if (DAKOTA_ENABLE_TESTS)

  # Required for test
  add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/dakota_square.in
    ${CMAKE_CURRENT_BINARY_DIR}/dakota_square.in)

  add_custom_target(dakota_test_java ALL
    #OUTPUT TODO
    # for now fake the dependence on the library and input file for the test
    DEPENDS dakotajni ${CMAKE_CURRENT_BINARY_DIR}/dakota_square.in

    # Output class files in build tree
    COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -cp ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/gov/sandia/dart/dakota/tools/*.java
    )

  add_test(dakota_java_hello
    ${Java_JAVA_EXECUTABLE} -Djava.library.path=. gov.sandia.dart.dakota.tools.HelloDakota)

endif()


# Install class files to lib as well, but not the generated .java files
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gov DESTINATION lib
  FILES_MATCHING PATTERN *.class)
install(TARGETS dakotajni EXPORT ${ExportTarget} DESTINATION lib)

# Be a little careful with removals in case build was in a source tree
# (don't remove .java files there)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
  "dakota.rst;OPT_DEFAULT.out;gov/sandia/dart/dakota/jni;gov/sandia/dart/dakota/tools/*.class")
