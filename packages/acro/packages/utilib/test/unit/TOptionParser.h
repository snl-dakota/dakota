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

#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/OptionParser.h>
#include <utilib/Property.h>
#include <utilib/mpiUtil.h>
#include <cxxtest/TestSuite.h>

namespace utilib {

namespace unittest { class Test_OptionParser; }
class utilib::unittest::Test_OptionParser : public CxxTest::TestSuite
{
public:

   void test1()
   {
      TS_TRACE("");
      bool a = false;
      int b = -1;
      double c = -1.0;
      unsigned int d = 1;

      utilib::OptionParser options;
      options.add_usage("command options arguments");
      options.description = "Description here.";
      options.version("beta");
      options.add("AA", a, "A option");
      options.add("BB", b, "B option");
      options.add("CC", c, "C option");
      options.add("DD", d, "D option");
      options.add_argument("arg1", "This is an arg description.");

      std::ofstream os("test1.out");
      options.write(os);

      os << a << " " << b << " " << c << " " << d << std::endl;

      int argc = 8;
      char* argv[] = { "command", "--AA", "--BB", "0", "--CC", "2.0", "--DD", "2", 0};
      options.parse_args(argc, argv);

      os << a << " " << b << " " << c << " " << d << std::endl;
      os.close();

      TS_ASSERT_SAME_FILES("test1.out", "TOptionParser_test1.txt");
      //unlink("test1.out");
   }



   void test2()
   {
      TS_TRACE("");
      bool a = false;
      int b = -1;
      double c = -1.0;
      unsigned int d = 1;

      utilib::OptionParser options;
      options.add_usage("command options arguments");
      options.description = "Description here.";
      options.version("beta");
      options.add('A', a, "A option");
      options.add('B', b, "B option");
      options.add('C', c, "C option");
      options.add('D', d, "D option");
      options.add_argument("arg1", "This is an arg description.");

      std::ofstream os("test2.out");
      options.write(os);

      os << a << " " << b << " " << c << " " << d << std::endl;

      int argc = 8;
      char* argv[] = { "command", "-A", "-B", "0", "-C", "2.0", "-D", "2", 0};
      options.parse_args(argc, argv);

      os << a << " " << b << " " << c << " " << d << std::endl;
      os.close();

      TS_ASSERT_SAME_FILES("test2.out", "TOptionParser_test2.txt");
      //unlink("test2.out");
   }


   void test3()
   {
      TS_TRACE("");
      bool a = false;
      int b = -1;
      double c = -1.0;
      unsigned int d = 1;

      utilib::OptionParser options;
      options.add_usage("command options arguments");
      options.description = "Description here.";
      options.version("beta");
      options.add('A', a, "A option");
      options.add('B', "my-b", b, "B option");
      options.add("my-c", c, "C option");
      options.add('D', d, "D option");
      options.add_argument("arg1", "This is an arg description.");

      std::ofstream os("test3.out");
      options.write(os);

      os << a << " " << b << " " << c << " " << d << std::endl;

      int argc = 8;
      char* argv[] = { "command", "-A", "--my-b", "0", "--my-c", "2.0", "-D", "2", 0};
      options.parse_args(argc, argv);

      os << a << " " << b << " " << c << " " << d << std::endl;
      os.close();

      TS_ASSERT_SAME_FILES("test3.out", "TOptionParser_test3.txt");
      //unlink("test3.out");
   }


   void test4()
   {
      TS_TRACE("Test that line-wrapping works correctly for long option descriptions.");
      bool a = false;
      int b = -1;
      double c = -1.0;
      unsigned int d = 1;

      utilib::OptionParser options;
      options.add_usage("command options arguments");
      options.description = "Description here.";
      options.add('A', a, "A option");
      options.add('B', "my-b", b, "B option");
      options.add("my-ccccccccccccccccccc", c, "C option");
      options.add("my-dddddddddddddddddddd", c, "D option");
      options.add("my-eeeeeeeeeeeeeeeeee", c, "E option");
      options.add("my-feeeeeeeeeeeeeeeee", c, "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
      options.add("my-geeeeeeeeeeeeeeeee", c, "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae            ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
      options.add("my-heeeeeeeeeeeeeeeee", c, "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae             \nab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
      options.add("my-ieeeeeeeeeeeeeeeee", c, "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae             \n - ab illo\n - inventore \n - veritatis et quasi architecto \n - beatae vitae dicta\n sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
      options.add('D', d, "D option");
      options.add_argument("arg1", "This is an arg description.");

      std::ofstream os("test4.out");
      options.write(os);
      os.close();

      TS_ASSERT_SAME_FILES("test4.out", "TOptionParser_test4.txt");
      //unlink("test4.out");
   }

   void test_boolean1()
   {
      TS_TRACE("Test that options like '-ab' raise an error.");
      bool a = false;
      bool b = false;
      utilib::OptionParser options;
      options.add('a', "AAa", a, "a option");
      options.add('b', "BB", b, "b option");

      char* argv[2] = { "cmd", "-ab" };
      try
      {
         options.parse_args(2, argv);
         TS_FAIL("Expecting error because multiple posix options cannot be set together");
      }
      catch (std::runtime_error& err)
      {}}

   void test_values1()
   {
      TS_TRACE("Test that values are set, and that the write_values() method correctly prints out changed values.");
      utilib::OptionParser options;
      bool a = false;
      bool A = false;
      bool b = false;
      bool B = false;
      int c = 0;
      unsigned d = 1;
      float e = 1e-12;
      float f = 0.0;
      double g = 0.0;
      std::vector<double> z;
      std::string str1;
      std::string str2 = "string-val";
      std::string str3 = "string val";
      utilib::CharString str4 = "string-val";
      utilib::CharString str5 = "string val";

      options.add('z', z, "z option");
      options.add("s1", str1, "s1 option");
      options.add("s2", str2, "s2 option");
      options.add("s3", str3, "s3 option");
      options.add("s4", str4, "s4 option");
      options.add("s5", str5, "s5 option");
      options.add('a', "aaa", a, "a option");
      options.add('A', "AAA", A, "A option");
      options.add('b', b, "b option");
      options.add("cc", c, "cc option");
      options.add("ddd", d, "ddd option");
      options.add('B', B, "B option");
      options.add("ee", e, "ee option");
      options.add("ff", f, "ff option");
      options.add("gg", g, "gg option");

      std::ofstream os("values1.out");
      options.write(os);

      os << "\nValues before option processing" << std::endl;
      options.write_values(os);

      char* argv[5] = { "cmd", "-a", "--ff=1.0", "--gg", "-1.0"};
      options.parse_args(5, argv);
      os << "\nValues after option processing" << std::endl;
      options.write_values(os);
      os.close();

      TS_ASSERT_SAME_FILES("values1.out", "TOptionParser_values1.txt");
      //unlink("values1.out");
   }

   void test_cat1()
   {
      TS_TRACE("Test that categorized options are appropriately printed out.");
      utilib::OptionParser options;
      bool a = false;
      bool A = false;
      bool b = false;
      bool B = false;
      int c = 0;
      unsigned d = 1;
      float e = 1e-12;
      float f = 0.0;
      double g = 0.0;
      std::vector<double> z;
      std::string str1;
      std::string str2 = "string-val";
      std::string str3 = "string val";
      utilib::CharString str4 = "string-val";
      utilib::CharString str5 = "string val";

      options.add('z', z, "z option");
      options.add("s1", str1, "s1 option");
      options.add("s2", str2, "s2 option");
      options.add("s3", str3, "s3 option");
      options.add("s4", str4, "s4 option");
      options.add("s5", str5, "s5 option");
      options.add('a', "aaa", a, "a option");
      options.add('A', "AAA", A, "A option");
      options.add('b', b, "b option");
      options.add("cc", c, "c option");
      options.add("ddd", d, "d option");
      options.add('B', B, "B option");
      options.add("ee", e, "e option");
      options.add("ff", f, "f option");
      options.add("gg", g, "g option");

      options.categorize("s1", "string");
      options.categorize("s2", "string");
      options.categorize("s3", "string");
      options.categorize("s4", "string");
      options.categorize("s5", "string");
      options.categorize("aaa", "bool");
      options.categorize("AAA", "bool");
      options.categorize("b", "bool");
      options.categorize("cc", "int");
      options.categorize("ddd", "unsigned int");
      options.categorize("ddd", "int");
      options.categorize('B', "bool");
      options.categorize("ee", "float");
      options.categorize("ff", "float");
      options.categorize("gg", "double");

      std::ofstream os("cat1.out");
      options.write(os);

      TS_ASSERT_SAME_FILES("cat1.out", "TOptionParser_cat1.txt");
      //unlink("cat1.out");
   }

   void test_cat2()
   {
      TS_TRACE("Test that select categorized options are appropriately printed out.");
      utilib::OptionParser options;
      bool a = false;
      bool A = false;
      bool b = false;
      bool B = false;
      int c = 0;
      unsigned d = 1;
      float e = 1e-12;
      float f = 0.0;
      double g = 0.0;
      std::vector<double> z;
      std::string str1;
      std::string str2 = "string-val";
      std::string str3 = "string val";
      utilib::CharString str4 = "string-val";
      utilib::CharString str5 = "string val";

      options.add('z', z, "z option");
      options.add("s1", str1, "s1 option");
      options.add("s2", str2, "s2 option");
      options.add("s3", str3, "s3 option");
      options.add("s4", str4, "s4 option");
      options.add("s5", str5, "s5 option");
      options.add('a', "aaa", a, "a option");
      options.add('A', "AAA", A, "A option");
      options.add('b', b, "b option");
      options.add("cc", c, "cc option");
      options.add("ddd", d, "ddd option");
      options.add('B', B, "B option");
      options.add("ee", e, "ee option");
      options.add("ff", f, "ff option");
      options.add("gg", g, "gg option");

      options.categorize("s1", "string");
      options.categorize("s2", "string");
      options.categorize("s3", "string");
      options.categorize("s4", "string");
      options.categorize("s5", "string");
      options.categorize("aaa", "bool");
      options.categorize("AAA", "bool");
      options.categorize("b", "bool");
      options.categorize("cc", "int");
      options.categorize("ddd", "unsigned int");
      options.categorize('B', "bool");
      options.categorize("ee", "float");
      options.categorize("ff", "float");
      options.categorize("gg", "double");

      options.alias("s2", "alias_s2");
      options.alias("a", "alias_a");
      options.alias("aaa", "alias_aaa");
      options.alias("b", "alias_b");
      options.alias("b", "z");

      std::set<std::string> req;
      req.insert("bool");
      req.insert("unsigned int");
      std::ofstream os("cat2.out");
      options.write(os, req);
      options.write(os, false);

      TS_ASSERT_SAME_FILES("cat2.out", "TOptionParser_cat2.txt");
      //unlink("cat2.out");
   }

   void test_merge1()
   {
      TS_TRACE("Test that options are merged correctly.");
      utilib::OptionParser options;
      bool a = false;
      bool A = false;
      bool b = false;
      bool B = false;
      int c = 0;
      unsigned d = 1;
      float e = 1e-12;
      float f = 0.0;
      double g = 0.0;
      std::vector<double> z;
      std::string str1;
      std::string str2 = "string-val";
      std::string str3 = "string val";
      utilib::CharString str4 = "string-val";
      utilib::CharString str5 = "string val";

      options.add('z', z, "z option");
      options.add("s1", str1, "s1 option");
      options.add("s2", str2, "s2 option");
      options.add("s3", str3, "s3 option");
      options.add("s4", str4, "s4 option");
      options.add("s5", str5, "s5 option");
      options.add('a', "aaa", a, "a option");
      options.add('A', "AAA", A, "A option");
      options.add('b', b, "b option");
      options.add("cc", c, "cc option");
      options.add("ddd", d, "ddd option");
      options.add('B', B, "B option");
      options.add("ee", e, "ee option");
      options.add("ff", f, "ff option");
      options.add("gg", g, "gg option");

      options.categorize("s1", "string");
      options.categorize("s2", "string");
      options.categorize("s3", "string");
      options.categorize("s4", "string");
      options.categorize("s5", "string");
      options.categorize("aaa", "bool");
      options.categorize("AAA", "bool");
      options.categorize("b", "bool");
      options.categorize("cc", "int");
      options.categorize("ddd", "unsigned int");
      options.categorize('B', "bool");
      options.categorize("ee", "float");
      options.categorize("ff", "float");
      options.categorize("gg", "double");

      options.alias("s2", "alias_s2");
      options.alias("a", "alias_a");
      options.alias("aaa", "alias_aaa");
      options.alias("b", "alias_b");
      options.alias("b", "z");

      std::set<std::string> req;
      req.insert("bool");
      req.insert("unsigned int");
      std::ofstream os("merge1.out");

      utilib::OptionParser tmp;
      tmp.merge_options(options);
      tmp.write(os, req);
      tmp.write(os, false);

      TS_ASSERT_SAME_FILES("merge1.out", "TOptionParser_cat2.txt");
      //unlink("merge1.out");
   }

   void test_process_xml1()
   {
      TS_TRACE("Test that an OptionParser object can be initialized with an XML block");
#if defined(UTILIB_HAVE_TINYXML)
      utilib::OptionParser options;
      bool a = false;
      bool A = false;
      bool b = false;
      bool B = false;
      int c = 0;
      unsigned d = 1;
      float e = 1e-12;
      float f = 0.0;
      double g = 0.0;
      std::vector<double> z;
      std::string str1;
      std::string str2 = "string-val";
      std::string str3 = "string val";
      utilib::CharString str4 = "string-val";
      utilib::CharString str5 = "string val";

      options.add('z', z, "z option");
      options.add("s1", str1, "s1 option");
      options.add("s2", str2, "s2 option");
      options.add("s3", str3, "s3 option");
      options.add("s4", str4, "s4 option");
      options.add("s5", str5, "s5 option");
      options.add('a', "aaa", a, "a option");
      options.add('A', "AAA", A, "A option");
      options.add('b', b, "b option");
      options.add("cc", c, "cc option");
      options.add("ddd", d, "ddd option");
      options.add('B', B, "B option");
      options.add("ee", e, "ee option");
      options.add("ff", f, "ff option");
      options.add("gg", g, "gg option");

      std::ofstream os("xml1.out");
      options.write(os);

      os << "\nValues before option processing" << std::endl;
      options.write_values(os);

      // WEH - is there a simpler way to setup an XML parse tree?
      TiXmlElement* root = new TiXmlElement("Options");
      {
         TiXmlElement* option = new TiXmlElement("Option");
         option->SetAttribute("name", "a");
         root->LinkEndChild(option);
      }
      {
         TiXmlElement* option = new TiXmlElement("Option");
         option->SetAttribute("name", "ff");
         option->LinkEndChild(new TiXmlText("1.0"));
         root->LinkEndChild(option);
      }
      {
         TiXmlElement* option = new TiXmlElement("Option");
         option->SetAttribute("name", "gg");
         option->LinkEndChild(new TiXmlText("-1.0"));
         root->LinkEndChild(option);
      }

      options.process_xml(root);
      os << "\nValues after option processing" << std::endl;
      options.write_values(os);
      os.close();

      TS_ASSERT_SAME_FILES("xml1.out", "TOptionParser_values1.txt");

      // clean up...
      delete root;
      //unlink("xml1.out");
#endif
   }

   void test_process_xml1_property()
   {
      TS_TRACE("Test that an OptionParser object populated with Properties can be initialized with an XML block");
#if defined(UTILIB_HAVE_TINYXML)
      utilib::OptionParser options;
      utilib::Property a(utilib::Property::Bind<bool>(false));
      utilib::Property A(utilib::Property::Bind<bool>(false));
      utilib::Property b(utilib::Property::Bind<bool>(false));
      utilib::Property B(utilib::Property::Bind<bool>(false));

      utilib::Property c(utilib::Property::Bind<int>(0));
      utilib::Property d(utilib::Property::Bind<unsigned>(1));
      utilib::Property e(utilib::Property::Bind<float>(1e-12));
      utilib::Property f(utilib::Property::Bind<float>(0));
      utilib::Property g(utilib::Property::Bind<double>(0));

      utilib::Property z(utilib::Property::Bind<std::vector<double> >());

      utilib::Property str1(utilib::Property::Bind<std::string>());
      utilib::Property str2(utilib::Property::Bind<std::string>("string-val"));
      utilib::Property str3(utilib::Property::Bind<std::string>("string val"));
      utilib::Property 
         str4(utilib::Property::Bind<utilib::CharString>("string-val"));
      utilib::Property 
         str5(utilib::Property::Bind<utilib::CharString>("string val"));

      options.add<utilib::Property>('z', z, "z option");
      options.add<utilib::Property>("s1", str1, "s1 option");
      options.add<utilib::Property>("s2", str2, "s2 option");
      options.add<utilib::Property>("s3", str3, "s3 option");
      options.add<utilib::Property>("s4", str4, "s4 option");
      options.add<utilib::Property>("s5", str5, "s5 option");
      options.add<utilib::Property>('a', "aaa", a, "a option", 
                                    utilib::Parameter::store_true);
      options.add<utilib::Property>('A', "AAA", A, "A option", 
                                    utilib::Parameter::store_true);
      options.add<utilib::Property>('b', b, "b option", 
                                    utilib::Parameter::store_true);
      options.add<utilib::Property>("cc", c, "cc option");
      options.add<utilib::Property>("ddd", d, "ddd option");
      options.add<utilib::Property>('B', B, "B option", 
                                    utilib::Parameter::store_true);
      options.add<utilib::Property>("ee", e, "ee option");
      options.add<utilib::Property>("ff", f, "ff option");
      options.add<utilib::Property>("gg", g, "gg option");

      std::ofstream os("xml1_property.out");
      options.write(os);

      os << "\nValues before option processing" << std::endl;
      options.write_values(os);

      // WEH - is there a simpler way to setup an XML parse tree?
      TiXmlElement* root = new TiXmlElement("Options");
      {
         TiXmlElement* option = new TiXmlElement("Option");
         option->SetAttribute("name", "a");
         root->LinkEndChild(option);
      }
      {
         TiXmlElement* option = new TiXmlElement("Option");
         option->SetAttribute("name", "ff");
         option->LinkEndChild(new TiXmlText("1.0"));
         root->LinkEndChild(option);
      }
      {
         TiXmlElement* option = new TiXmlElement("Option");
         option->SetAttribute("name", "gg");
         option->LinkEndChild(new TiXmlText("-1.0"));
         root->LinkEndChild(option);
      }

      options.process_xml(root);
      os << "\nValues after option processing" << std::endl;
      options.write_values(os);
      os.close();

      TS_ASSERT_SAME_FILES("xml1_property.out", "TOptionParser_values1.txt");

      // clean up...
      delete root;
      //unlink("xml1.out");
#endif
   }

   void test_xml1()
   {
      TS_TRACE("Test that option values can be written to XML.");
#if defined(UTILIB_HAVE_TINYXML)
      utilib::OptionParser options;
      bool a = false;
      bool A = false;
      bool b = false;
      bool B = false;
      int c = 0;
      unsigned d = 1;
      float e = 1e-12;
      float f = 0.0;
      double g = 0.0;
      std::vector<double> z;
      std::string str1;
      std::string str2 = "string-val";
      std::string str3 = "string val";
      utilib::CharString str4 = "string-val";
      utilib::CharString str5 = "string val";

      options.add('z', z, "z option");
      options.add("s1", str1, "s1 option");
      options.add("s2", str2, "s2 option");
      options.add("s3", str3, "s3 option");
      options.add("s4", str4, "s4 option");
      options.add("s5", str5, "s5 option");
      options.add('a', "aaa", a, "a option");
      options.add('A', "AAA", A, "A option");
      options.add('b', b, "b option");
      options.add("cc", c, "cc option");
      options.add("ddd", d, "ddd option");
      options.add('B', B, "B option");
      options.add("ee", e, "ee option");
      options.add("ff", f, "ff option");
      options.add("gg", g, "gg option");

      options.categorize("s1", "string");
      options.categorize("s2", "string");
      options.categorize("s3", "string");
      options.categorize("s4", "string");
      options.categorize("s5", "string");
      options.categorize("aaa", "bool");
      options.categorize("AAA", "bool");
      options.categorize("b", "bool");
      options.categorize("cc", "int");
      options.categorize("ddd", "unsigned int");
      options.categorize("ddd", "int");
      options.categorize('B', "bool");
      options.categorize("ee", "float");
      options.categorize("ff", "float");
      options.categorize("gg", "double");

      char* argv[5] = { "cmd", "-a", "--ff=1.0", "--gg", "-1.0"};
      options.parse_args(5, argv);
      std::ofstream os("xml2.out");
      options.write_values_xml(os);
      os.close();

      TS_ASSERT_SAME_FILES("xml2.out", "TOptionParser_xml1.txt");
      //unlink("xml2.out");
#endif
   }

};

} // namespace utilib
