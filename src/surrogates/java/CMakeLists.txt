#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

find_package(Java COMPONENTS Development REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

add_library(jni_dakota_surrogates SHARED
  gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates.cpp)

target_include_directories(jni_dakota_surrogates PUBLIC ${JNI_INCLUDE_DIRS})
target_link_libraries(jni_dakota_surrogates PUBLIC dakota_surrogates)

install(TARGETS jni_dakota_surrogates EXPORT ${ExportTarget}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# For Windows testing, map DLLs into test directory
if (WIN32)
  add_custom_command(TARGET jni_dakota_surrogates
                     POST_BUILD
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:teuchoscomm>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:teuchoscore>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:teuchosnumerics>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:teuchosparameterlist>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:teuchosparser>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:teuchosremainder>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:dakota_util>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:dakota_surrogates>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:blas>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:lapack>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     )
endif()

if (APPLE)
  add_custom_command(TARGET jni_dakota_surrogates
                     POST_BUILD
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:Boost::serialization>" "$<TARGET_FILE_DIR:jni_dakota_surrogates>"
                     )
endif()

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

add_test(NAME dakota_surrogates_jni
  COMMAND ${Java_JAVA_EXECUTABLE} -classpath jni_test.jar
  gov.sandia.dakota.jni.test.DakotaJniTest
  )
# This test uses the polynomial surrogates exported by surrogates_polynomial_regression
# ../unit/poly_test.surr.*
set_tests_properties(dakota_surrogates_jni PROPERTIES
  DEPENDS surrogates_polynomial_regression
  LABELS Unit
  )
