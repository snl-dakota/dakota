#include(FindBoost)
include_directories(${Dakota_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIRS})

# TODO: Understand leveraging BOOST_DISABLE_ASSERTS wrt to Unit Testing

option(BOOST_DISABLE_ASSERTS "Toggle BoostAssert, default is AssertDisabled" ON)
if (BOOST_DISABLE_ASSERTS)
  add_definitions("-DBOOST_DISABLE_ASSERTS")
endif(BOOST_DISABLE_ASSERTS)


# Unit test targets

add_executable(min_unit_test min_unit_test.cpp)
# It is desirable to split-up dakota into packages of re-useable components
# but that is a significant re-factoring effort in its own right, so
# link with the whole dakota library (for now)
target_link_libraries( min_unit_test ${Dakota_LIBRARIES}
  ${Dakota_TPL_LIBRARIES} )
add_test(min_unit_test min_unit_test)


add_executable(test_workdir_utils test_workdir_utils.cpp)
target_link_libraries( test_workdir_utils ${Dakota_LIBRARIES}
  ${Dakota_TPL_LIBRARIES} )
add_test(test_workdir_utils test_workdir_utils)


# Next Unit test target (try BOOST_AUTO_TEST_CASE)

#set(auto_unit_test_SRCS
#  auto_unit_test.cpp
#)
#add_executable(auto_unit_test ${auto_unit_test_SRCS})
add_executable(auto_unit_test auto_unit_test.cpp)
target_link_libraries( min_unit_test ${Dakota_LIBRARIES}
  ${Dakota_TPL_LIBRARIES} )
#target_link_libraries(auto_unit_test boost_unit_test_framework) # need team buy-in
add_test(auto_unit_test auto_unit_test)


# TODO: MORE Unit test targets


# RWH - Introducing another unit testing option which may or may not be embraced
#       but serves as a demo
remove_definitions("-DHAVE_CONFIG_H")

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions("-DHAVE_UNISTD_H")
endif(HAVE_UNISTD_H)

include_directories(
  ${Teuchos_INCLUDE_DIRS}
)

option(DAKOTA_ENABLE_SAMPLE_UNIT_TESTS "Enable Sample unit tests driver" ON)
if(DAKOTA_ENABLE_SAMPLE_UNIT_TESTS)
  set(dakota_sample_unit_tests
    covariance_reader.cpp
    expt_data.cpp
    expt_data_reader.cpp
    file_reader.cpp
    data_conversions.cpp
    )
  if(HAVE_PECOS)
    list(APPEND dakota_sample_unit_tests expt_data.cpp)
    include_directories(${Pecos_SOURCE_DIR}/src)
  endif() # HAVE_PECOS
  add_executable(sample_unit_tests sample_unit_test_driver.cpp ${dakota_sample_unit_tests})
  target_link_libraries(sample_unit_tests ${Dakota_LIBRARIES} ${Dakota_TPL_LIBRARIES})
  add_test(sample_unit_tests sample_unit_tests)
  file( COPY expt_data_test_files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif() # DAKOTA_ENABLE_SAMPLE_UNIT_TESTS

