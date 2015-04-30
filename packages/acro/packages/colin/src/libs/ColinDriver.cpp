/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */


#include <colin/XMLProcessor.h>
#include <colin/ApplicationMngr.h>
#include <colin/Solver.h>
#include <colin/SolverMngr.h>
#include <colin/AmplDriver.h>

#include <utilib/OptionParser.h>
#include <utilib/exception_mngr.h>
#include <utilib/seconds.h>
#include <utilib/CommonIO.h>
#include <utilib/MPIStream.h>

#include <tinyxml/tinyxml.h>

using utilib::PropertyDict;

using std::cerr;
using std::endl;

using std::string;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::list;
using std::pair;


namespace colin {

namespace {

int done(int return_code = 0)
{
   ucout << utilib::Flush;
   ucerr << utilib::Flush;
   utilib::CommonIO::end();

#if defined(ACRO_HAVE_MPI)
   utilib::uMPI::done();
#endif
   
   colin::ApplicationMngr().clear();

   return return_code;
}

}


int colin_driver( int argc, char* argv[], 
                  const std::string codename, const std::string version )
{
   InitializeTiming();
   //double start_time = CPUSeconds();
   
#if defined(ACRO_HAVE_MPI)
   utilib::uMPI::init(&argc, &argv, MPI_COMM_WORLD);
#endif
   
   utilib::CommonIO::begin();
   
#if defined(ACRO_HAVE_MPI)
   int rank = utilib::uMPI::rank;
   if (utilib::uMPI::rank == 0)
      utilib::CommonIO::end_tagging();
#else
   int rank = 0;
#endif

   utilib::OptionParser parser;
   string usage = codename;
   usage += " [options] <arg1> <arg2>";
   parser.add_usage(usage);
   usage = codename;
   usage += " <*.nl> -AMPL";
   parser.add_usage(usage);
   usage = codename;
   usage += " [options] <colin-xml-input>";
   parser.add_usage(usage);
   parser.description = "This is a COLIN optimizer executable.";
   parser.epilog = "Additional optimizer options can be specified within "
      "AMPL or the COLIN XML input.";
   if (version != "")
      parser.version(version);

   bool verbose = false;
   parser.add("verbose", verbose, "Verbose printing during optimization.");

   bool debug_solver_params = false;
   parser.add("debug-solver-params", debug_solver_params,
              "Print solver option values before running it.");

   string help_options = "";
   parser.add("help-options", help_options,
              "Print options for specified solver.");

   bool help_solvers = false;
   parser.add('s', "help-solvers", help_solvers,
              "Print the list of solvers supported by this executable.");

   bool help_solvers_xml = false;
   parser.add("help-solvers-xml", help_solvers_xml,
              "Print the list of solvers supported by this executable to "
              "a file using an XML format.");

   int repetitions = 0;
   parser.add("repetitions", repetitions,
              "The number of times that the solver is rerun (to check that "
              "solver reset() methods work).");

   bool use_abort = false;
   parser.add("use-abort", use_abort,
              "Force an abort when an error occurs.");

   //
   // Parse the command line
   //
   utilib::OptionParser::args_t args;
   try
   {
      args = parser.parse_args(argc, argv);
   }
   catch (std::runtime_error& err)
   {
      ucout << err.what() << endl
            << "Type '" << codename << " --help' for more information." 
            << endl;
      return done(1);
   }
   if ( parser.help_option() )
   {
      parser.write(ucout);
      return done();
   }
   if ( parser.version_option() )
   {
      parser.print_version(ucout);
      return done();
   }

   if ( help_solvers_xml ) 
   {
      list<pair<string, string> > info;
      SolverMngr().get_solver_types(info);

      ucout << "<COLINSolvers interface=\"" << codename 
            << "\" num=\"" << info.size() << "\">" << endl;
      list<pair<string, string> >::iterator curr = info.begin();
      list<pair<string, string> >::iterator end  = info.end();
      while (curr != end) 
      {
         ucout << "<Solver name=\"" << curr->first << "\">" << endl;
         SolverHandle handle = SolverMngr().create_solver( curr->first );
         //handle->options().write_parameters_xml(ucout);
         ucout << "</Solver>" << endl;
         curr++;
      }
      ucout << "</COLINSolvers>" << endl;
      return done();
   }

   //
   // Help - options for a solver
   //
   if ( help_options == "true" )
      EXCEPTION_MNGR(std::runtime_error, "ERROR: using --help-options "
                     "but no solver name is specified.");

   if ( ! help_options.empty() )
   {
      colin::SolverHandle handle;
      try 
      {
         handle = SolverMngr().create_solver(help_options);
      }
      catch ( colin::solverManager_error& e )
      {
         string what = e.what();
         size_t p = what.find("\nStack trace:");
         if ( p != string::npos )
            what.resize(p);
         ucerr << "ERROR interrogating solver \"" << help_options << "\": "
               << endl << "   " << what << endl
               << "Run '" << codename << " -s' to list the available solvers."
               << endl;
         return done(1);
      }
      if ( !handle.empty() )
      {
         ucout << "Options for solver \"" << help_options << "\"" << endl;
         handle->Properties().write(PropertyDict::DescriptionWriter(ucout));
         return done();
      }
   }
   //
   // Help - list solvers 
   //
   if (help_solvers)
   {
      ucout << string(77, '*') << endl;
      ucout << "Solvers defined in '" << codename << "'" << endl;
      ucout << string(77, '*') << endl;

      list<pair<string, string> > info;
      SolverMngr().get_solver_types(info);
      if (info.size() == 0)
         ucout << "    None" << endl;
      else
      {
         ucout << endl;
         list<pair<string, string> >::iterator curr = info.begin();
         list<pair<string, string> >::iterator end  = info.end();
         while (curr != end)
         {
            {
               ucout << "    " << curr->first << endl;
               ucout << "         " << curr->second << endl;
               curr++;
            }
         }
         return done();
      }
   }

   try
   {
      if (parser.args().size() == 2) 
      {
         // 2) Load the XML file (root rank only)
         TiXmlDocument doc;
         if ( rank == 0 )
         {
            if ( !doc.LoadFile(args[1]) )
            {
               cerr << "TinyXML Error : " << doc.ErrorDesc() << endl;
               cerr << "TinyXML Error : Row " << doc.ErrorRow() 
                    << "  Column " << doc.ErrorCol() << endl;
               EXCEPTION_MNGR(std::runtime_error, 
                              "colin_driver - Error processing filename \"" 
                              << args[1] << "\"");
            }
            if ( doc.RootElement()->ValueStr().compare("ColinInput") != 0 )
               EXCEPTION_MNGR(std::runtime_error, 
                              "colin_driver - Error processing data from \"" 
                              << args[1] << 
                              "\". The root element must be ColinInput.");
         }

#if defined (ACRO_HAVE_MPI)
         // 3) Distribute the XML input to all other ranks
         if ( utilib::uMPI::size > 1 )
         {
            utilib::MPIBroadcastStream bcast(0);
            if ( rank == 0 )
            {
               ostringstream input_buf;
               input_buf << doc;
               bcast << input_buf.str();
            }
            else
            {
               string input_str;
               bcast >> input_str;
               istringstream input_buf(input_str);
               input_buf >> doc;
            }
         }
#endif

         // 4) Process the XML input
         XMLProcessor().process(doc.RootElement());

         // 5) 5 is right out.
      }
      else if ((parser.args().size() == 3)  && (parser.args()[2] ==  "-AMPL"))
      {
         AMPL_driver(argc, argv, codename.c_str());
      }
      else
      {
         //ucerr << "ERROR: expected exactly one argument: the xml spec "
            //"file; instead got " << (argc-1) << ":" << endl;
         //for(int i = 0; i < argc; ++i )
            //ucerr << "          \"" << argv[i] << "\"" << endl;
         ucerr << "Type '" << codename << " --help' for usage information." << endl;
         return done(1);
      }

   }
   catch (const std::exception& err)
   {
      ucerr << "Caught STL exception: " << err.what() << endl;
      return done(1);
   }
   catch (const std::string& err)
   {
      ucerr << "Caught string exception: " << err << endl;
      return done(1);
   }
   catch (const char* err)
   {
      ucerr << "Caught string exception: " << err << endl;
      return done(1);
   }

   return done();
}

} // namespace colin
