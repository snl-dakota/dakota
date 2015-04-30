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

#include <utilib/stl_auxiliary.h>
#include <utilib/OptionParser.h>

int main(int argc, char** argv)
{
   utilib::OptionParser options;
   //
   // Add general documentation
   //
   options.add_usage("OptionParser1 [options] <arg1> <arg2>");
   options.description = "This is simple example of how to use the OptionParser object to parse the argc/argv data on a C++ command line.";
   options.add_argument("arg1", "This is the first argument after the options");
   options.add_argument("arg1", "This is the second argument after the options");
   options.epilog = "This is additional text that describes this command.  Newlines\nare\nused to format\nthis text, but really long lines like this one are wrapped so a users does not need to do that!";
   options.version("beta");
   options.min_num_required_args = 2;
   //
   // Add options
   //
   bool long_arg = false;
   options.add("long_arg", long_arg, "An option with only a long argument name");
   bool negate = true;
   options.add("negate", negate, "A boolean flag that is positive by default.  Using this flag turns this value to 'false'.", utilib::Parameter::store_false);
   int short_arg = -1;
   options.add('s', short_arg, "An option with only a short argument name");
   std::string mixed = "unknown";
   options.add('m', "mixed", mixed, "An option with both short and long argument names");
   std::list<int> int_vals;
   options.add('i', "int", int_vals, "An option that fills a list of integers. Each with instance of this option adds another value.");

   //
   // Parse the command line
   //
   try
   {
      options.parse_args(argc, argv);
   }
   catch (std::runtime_error& err)
   {
      std::cout << err.what() << std::endl;
      std::cout << "Type 'OptionParser1 --help' for more information." << std::endl;
      return 1;
   }

   //
   // Print help information when the --help option is used
   //
   if (options.help_option())
   {
      options.write(std::cout);
      return 1;
   }
   //
   // Print version information when the --version option is used
   //
   if (options.version_option())
   {
      options.print_version(std::cout);
      return 1;
   }
   //
   // Check the list of arguments that were not consumed when the command line
   // options were parsed.
   //
   if (options.args().size() < 3)
   {
      std::cout << "Type 'OptionParser1 --help' for usage information." << std::endl;
      return 1;
   }

   //
   // Iterate through the arguments, printing their values.
   // Note: the first argument is the name of the command being
   // executed.
   //
   utilib::OptionParser::args_t::iterator curr = options.args().begin();
   utilib::OptionParser::args_t::iterator end  = options.args().end();
   std::cout << "Arguments:" << std::endl;
   while (curr != end)
   {
      std::cout << "  " << *curr << std::endl;
      curr++;
   }
   //
   // Print option values
   //
   std::cout << "long_arg: " << long_arg << std::endl;
   std::cout << "negate:   " << negate << std::endl;
   std::cout << "s:        " << short_arg << std::endl;
   std::cout << "mixed:    " << mixed << std::endl;
   std::cout << "int:      " << int_vals << std::endl;

   return 0;
}
