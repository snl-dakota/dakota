#include(FindBoost)
#include_directories(${Dakota_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIRS})

# TODO: Understand leveraging BOOST_DISABLE_ASSERTS wrt to Unit Testing

option(BOOST_DISABLE_ASSERTS "Toggle BoostAssert, default is AssertDisabled" ON)
if (BOOST_DISABLE_ASSERTS)
  add_definitions("-DBOOST_DISABLE_ASSERTS")
endif(BOOST_DISABLE_ASSERTS)


# Unit test targets

add_executable(min_unit_test min_unit_test.cpp)
add_test(min_unit_test min_unit_test)


# TODO: Next Unit test target

#set(auto_unit_test_SRCS
#  auto_unit_test.cpp
#)

#add_executable(auto_unit_test ${auto_unit_test_SRCS})
#target_link_libraries(auto_unit_test boost_unit_test_framework)
#add_test(auto_unit_test auto_unit_test)

# TODO: MORE Unit test targets

