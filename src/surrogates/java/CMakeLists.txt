#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

find_package(Java COMPONENTS Development REQUIRED)
find_package(JNI REQUIRED)

add_library(jni_dakota_surrogates SHARED
  gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates.cpp)

target_include_directories(jni_dakota_surrogates PUBLIC ${JNI_INCLUDE_DIRS})
target_link_libraries(jni_dakota_surrogates PUBLIC dakota_surrogates)

install(TARGETS jni_dakota_surrogates EXPORT ${ExportTarget} DESTINATION lib)

# Java JNI bridge to C++ test

set(jni_test_surrogates_src "${CMAKE_CURRENT_SOURCE_DIR}/gov.sandia.dakota.jni.test/src/gov/sandia/dakota/jni/surrogates/java")
set(jni_test_test_src "${CMAKE_CURRENT_SOURCE_DIR}/gov.sandia.dakota.jni.test/src/gov/sandia/dakota/jni/test")

add_jar(jni_test
  SOURCES
  "${jni_test_surrogates_src}/DakotaSurrogates.java"
  "${jni_test_test_src}/DakotaJniTest.java"
  "${jni_test_test_src}/DakotaSurrogatesException.java"
  "${jni_test_test_src}/DakotaSystemPathUtil.java"
  "${jni_test_test_src}/OsUtil.java"
  )

# Until we generate the file as part of the test:
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/exp_poly.s12.response_fn_1.txt"
  "${CMAKE_CURRENT_BINARY_DIR}/exp_poly.s12.response_fn_1.txt")

add_test(NAME jni_dakota_surrogates
  COMMAND ${Java_JAVA_EXECUTABLE} -classpath jni_test.jar
  gov.sandia.dakota.jni.test.DakotaJniTest
  )
