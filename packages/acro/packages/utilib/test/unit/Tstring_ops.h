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
#include <utilib/string_ops.h>
#include <cxxtest/TestSuite.h>
#include <fstream>

namespace utilib {

namespace unittest { class Test_string_ops; }
class utilib::unittest::Test_string_ops : public CxxTest::TestSuite
{
public:

    void test_toUpper()
    {
    TS_TRACE("Testing toUpper");
    CharString tmp;
    tmp = "abCD12_#";
    toUpper(tmp);
    TS_ASSERT_EQUALS(tmp,"ABCD12_#");
    }

    void test_toLower()
    {
    TS_TRACE("Testing toLower");
    CharString tmp;
    tmp = "abCD12_#";
    toLower(tmp);
    TS_ASSERT_EQUALS(tmp,"abcd12_#");
    }

    void test_split1()
    {
    TS_TRACE("Splitting returns a list of strings");
    CharString tmp;
    tmp = "a.b.c.d";
    BasicArray<CharString> words = split(tmp,'.');
    TS_ASSERT_EQUALS(words.size(),(size_t)4);
    TS_ASSERT_EQUALS(words[0],"a");
    TS_ASSERT_EQUALS(words[1],"b");
    TS_ASSERT_EQUALS(words[2],"c");
    TS_ASSERT_EQUALS(words[3],"d");
    }

    void test_split2()
    {
    TS_TRACE("Splitting returns empty strings when the split character is at the beginning or end");
    CharString tmp;
    tmp = ".a.b.c.d..";
    BasicArray<CharString> words = split(tmp,'.');
    TS_ASSERT_EQUALS(words.size(),(size_t)7);
    TS_ASSERT_EQUALS(words[0],"");
    TS_ASSERT_EQUALS(words[1],"a");
    TS_ASSERT_EQUALS(words[2],"b");
    TS_ASSERT_EQUALS(words[3],"c");
    TS_ASSERT_EQUALS(words[4],"d");
    TS_ASSERT_EQUALS(words[5],"");
    TS_ASSERT_EQUALS(words[6],"");
    }

    void test_split3()
    {
    TS_TRACE("Splitting a string with a character that does not exist, returns a list with just the original string");
    CharString tmp;
    tmp = "a.b.c.d";
    BasicArray<CharString> words = split(tmp,'x');
    TS_ASSERT_EQUALS(words.size(),(size_t)1);
    TS_ASSERT_EQUALS(words[0],tmp);
    }

    void test_join1()
    {
    TS_TRACE("Joining a non-empty list.");
    CharString tmp;
    tmp = "a.b.c.d";
    BasicArray<CharString> words = split(tmp,'.');
    tmp = join(words,'-');
    TS_ASSERT_EQUALS(tmp,"a-b-c-d");
    }

    void test_join2()
    {
    TS_TRACE("Joining an empty list returns an empty string.");
    CharString tmp;
    tmp = "a.b.c.d";
    BasicArray<CharString> words;
    tmp = join(words,'-');
    TS_ASSERT_EQUALS(tmp,"");
    }

    void test_wordwrap_printline()
    {
    TS_TRACE("Testing that we get the expected output from the wordwrap_printline method");
    std::string indent = "                                ";
    std::ofstream os("Tstring_ops_wordwrap.out");
    wordwrap_printline(os, "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",indent);
    wordwrap_printline(os,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore          veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",indent);
    wordwrap_printline(os,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae             \nab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",indent);
    wordwrap_printline(os,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae             \n - ab illo\n - inventore \n - veritatis et quasi architecto \n - beatae vitae dicta\n sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",indent);
    wordwrap_printline(os,"a bb ccc dddd ------------------------------------------------------------------------------------- **********************************************************************\n xx y",indent);

    TS_ASSERT_SAME_FILES( "Tstring_ops_wordwrap.out", 
                          "Tstring_ops_wordwrap.txt" );
    unlink("Tstring_ops_wordwrap.out");
    }

};

} // namespace utilib
