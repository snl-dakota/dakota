/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "generic_factory.hpp"
#include <gtest/gtest.h>

TEST(GenericFactoryTest_tests, factory_create) {
  using factory_type = Dakota::Util::GenericFactory<int, int(int)>;

  factory_type fac;

  EXPECT_FALSE(fac.key_is_registered(0));
  EXPECT_TRUE(fac.register_key(0, [](int x){ return 1 + x; }));
  EXPECT_TRUE(fac.key_is_registered(0));
  EXPECT_FALSE(fac.register_key(0, [](int x) { return 2 + x; }));
  EXPECT_EQ(3, fac.create(0, 2));

  EXPECT_FALSE(fac.key_is_registered(1));
  EXPECT_TRUE(fac.register_key(1, [](int x) { return 2 + x;}));
  EXPECT_TRUE(fac.key_is_registered(1));
  EXPECT_EQ(5, fac.create(1, 3));
}

struct test_error_policy
{
  int on_unknown_key(int, int) const
  {
    return -1;
  }
};

TEST(GenericFactoryTest_tests, factory_error) {
  using factory_type = Dakota::Util::GenericFactory<int, int(int)>;

  factory_type fac;

  EXPECT_FALSE(fac.key_is_registered(0));
  EXPECT_EQ(0, fac.create(0, 2));  // Default error policy will default construct return

  EXPECT_TRUE(fac.register_key(0, [](int x){ return 1 + x; }));
  EXPECT_TRUE(fac.key_is_registered(0));
  EXPECT_EQ(3, fac.create(0, 2));

  EXPECT_FALSE(fac.key_is_registered(1));
  EXPECT_EQ(0, fac.create(1, 2));

  using factory_type2 = Dakota::Util::GenericFactory<int, int(int), test_error_policy>;

  factory_type2 fac2;

  EXPECT_FALSE(fac2.key_is_registered(0));
  EXPECT_EQ(-1, fac2.create(0, 2));

  EXPECT_TRUE(fac2.register_key(0, [](int x){ return 1 + x; }));
  EXPECT_TRUE(fac2.key_is_registered(0));
  EXPECT_EQ(3, fac2.create(0, 2));

  EXPECT_FALSE(fac2.key_is_registered(1));
  EXPECT_EQ(-1, fac2.create(1, 2));
}
