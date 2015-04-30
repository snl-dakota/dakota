/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <utilib_config.h>
#include <utilib/stl_auxiliary.h>
#include <cxxtest/TestSuite.h>
#include <fstream>

namespace utilib {

namespace unittest { class Test_stl_auxiliary; }
class utilib::unittest::Test_stl_auxiliary : public CxxTest::TestSuite
{
public:

    void test_toUpper()
    {
    TS_TRACE("Testing toUpper");
    std::string tmp;
    tmp = "abCD12_#";
    toUpper(tmp);
    TS_ASSERT_EQUALS(tmp,"ABCD12_#");
    }

    void test_toLower()
    {
    TS_TRACE("Testing toLower");
    std::string tmp;
    tmp = "abCD12_#";
    toLower(tmp);
    TS_ASSERT_EQUALS(tmp,"abcd12_#");
    }

};

} // namespace utilib
