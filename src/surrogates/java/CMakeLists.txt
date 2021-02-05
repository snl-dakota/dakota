#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

find_package(Java REQUIRED)
find_package(JNI REQUIRED)

add_library(jni_dakota_surrogates SHARED
  gov_sandia_dakota_jni_surrogates_java_DakotaSurrogates.cpp)

target_include_directories(jni_dakota_surrogates PUBLIC ${JNI_INCLUDE_DIRS})
target_link_libraries(jni_dakota_surrogates PUBLIC dakota_surrogates)

install(TARGETS jni_dakota_surrogates EXPORT ${ExportTarget} DESTINATION lib)
