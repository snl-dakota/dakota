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

#include <acro_config.h>
#include <colin/AmplDriver.h>
#include <colin/ApplicationMngr.h>
#include <colin/Utilities.h>
#include <utilib/seconds.h>
#include <utilib/PropertyDict_YamlPrinter.h>
#include <fstream>

//===========================================================================
#if !defined(ACRO_USING_AMPL ) || defined(TFLOPS)

namespace colin
{

int AMPL_driver(int argc, char* argv[], const char* codename)
{
   EXCEPTION_MNGR(std::runtime_error,
                  "AMPL_driver(): Configured withough AMPL support.");
}

} // namespace colin


//===========================================================================
#else // We are building in AMPL support


#include <colin/SolverMngr.h>
#include <colin/solver/Base.h>
#include <colin/AmplApplication.h>

#include <utilib/string_ops.h>
#include <utilib/BasicArray.h>
#include <utilib/CharString.h>

using colin::SolverHandle;

using utilib::Any;
using utilib::BasicArray;
using utilib::CharString;
using utilib::PropertyDict;

#include <utilib/ParameterList.h>
using utilib::ParameterList;

using std::endl;
using std::string;
using std::istringstream;

using std::list;
using std::pair;

#undef NO
extern "C"
{
#include "getstub.h"
#include "asl.h"
};
#undef real
#undef getenv

/** AMPL "emulates" object-oriented programming by defining a state
*  structure (ASL* asl) and then a slew of #defines to wrap what should
*  be methods and properties of a class with the common asl state
*  pointer.  This is problemmatic for real programs where there may be
*  more than one active AMPL problem (think outer and inner problems).
*
*  The workaround for this is yet another #define: by #defining the
*  common asl pointer to be something else, when the preprocessor
*  expands AMPL's internal #defines, we can get AMPL to reference a
*  properly-scoped (and contained) ASL* member of *our* class.
*/
#define asl ASL_ptr

//
// Global data
//
#define DRIVER_DATE 20081031
#define _QUOTE(x) #x

namespace
{

void
process_options(CharString& envstr, char* s,
                list< pair<string, string> > & plist, CharString& solverstr, int verbosity)
{
   if (verbosity >= 1)
      ucout << endl << endl << "Processing AMPL Options: " << envstr << endl;
   //// Eliminate white space before and after '=' characters
   int slen = strlen(s);
   int offset = 0;
   for (int i = 0; i < slen; i++)
   {
      if (s[i] == '=')
      {
         int j = i - 1;
         while ((j >= 0) && (s[j] == ' ')) { offset++; j--; }
         s[i-offset] = s[i];
         i++;
         while ((i < slen) && (s[i] == ' ')) { offset++; i++; }
         s[i-offset] = s[i];
      }
      else
      {
         s[i-offset] = s[i];
      }
   }
   s[slen-offset] = '\000';
   //// Process the options
   istringstream isstr(s);
   string str;
   while (isstr >> str)
   {
      const char* param = str.c_str();
      char* tmp = const_cast<char*>(param);
      unsigned int i = 0;
      while ((i < str.size()) && (*tmp != '=')) {tmp++; i++;}
      if (i == str.size())
      {
         if (verbosity)
            ucout << "  OPTION: " << param << " true" << endl;
         plist.push_back(std::pair<std::string, std::string>(str, "true"));
      }
      else
      {
         *tmp = '\000';
         tmp++;
         if (verbosity)
            ucout << "  OPTION: " << param << " " << tmp << endl;
         if (strcmp(param, "solver") == 0)
         {
            solverstr = tmp;
         }
         else
         {
            plist.push_back(std::pair<std::string, std::string>(param, tmp));
         }
      }
   }
   ucout << endl;
}

} // namespace (local)


namespace colin
{

int AMPL_driver(int argc, char* argv[], const char* codename)
{
   InitializeTiming();

   if ((argc != 3) || (argv == NULL))
      EXCEPTION_MNGR(std::runtime_error, "AMPL_driver(): "
                     "passed an invalid number of arguments (is " << argc <<
                     ", should be 3)");
   if ((argv[2] == NULL) || (strncmp(argv[2], "-AMPL", 5) != 0))
      EXCEPTION_MNGR(std::runtime_error, "AMPL_driver(): "
                     "'-AMPL' not found. Are we really being driven by AMPL?");
   if ((argv[1] == NULL) || (strcmp(argv[1], "") == 0))
      EXCEPTION_MNGR(std::runtime_error, "AMPL_driver(): "
                     "given empty NL stub file name");
   if ((codename == NULL) || strcmp(codename, "") == 0)
      EXCEPTION_MNGR(std::runtime_error, "AMPL_driver(): "
                     "given empty executable code name");

   CharString envstr = codename;
   envstr += "_options";

   char DriverVersion[] =
      "AMPL/Colin\0\nAMPL/Colin 3.0 Driver Version " _QUOTE(DRIVER_DATE) "\n";

   // gymnastics to cast away constness of literal; following existing pattern
   CharString driver_usage_msg("  TODO ");
   char* DriverUsageMsg[] =
      {
	const_cast<char*>(driver_usage_msg.c_str())
      };

   // gymnastics to cast away constness of literal; following existing pattern
   CharString client_name("Colin Solver Library 3.0");
   Option_Info ASL_options_info =
      {
 	 const_cast<char*>(codename),
	 const_cast<char*>(client_name.c_str()),
         envstr.data(), 0, 0, 0, DriverVersion,
         DriverUsageMsg, 0, 0, 0, 0, DRIVER_DATE,
         0, 0, 0, 0, 0, 0, 0, 0, 0
      };


   //
   // Get the command_verbosity environment
   //
   // 0 - quiet
   // 1 - normal
   // 2 - verbose
   //
   int verbosity = 1;
   {
      CharString envstr = codename;
      envstr += "_verbosity";
      char* s = getenv(envstr.data());
      if (!s)
      {
         utilib::toUpper(envstr);
         s = getenv(envstr.data());
      }
      if (s)
      {
         std::string tmp = s;
         if (tmp == "quiet")
            verbosity = 0;
         else if (tmp == "verbose")
            verbosity = 2;
         else
            verbosity = 1;
      }
   }

   //
   // Process solver options
   //
   ASL* ASL_ptr = ASL_alloc(ASL_read_fgh);
   std::string fname;
   fname = argv[1];
   string endOfName(fname, fname.length() - 3, 3);
   if (endOfName == ".nl")
      fname.erase(fname.length() - 3, 3);
   getstub(&argv, &ASL_options_info);
   ucout << std::endl << utilib::Flush;

   list< pair<string, string> > plist;
   BasicArray< list< pair<string, string> > > plist_subsolvers;
   char* s = getenv(envstr.data());
   if (!s)
   {
      utilib::toUpper(envstr);
      s = getenv(envstr.data());
   }
   CharString solverstr;
   if (s)
   {
      process_options(envstr, s, plist, solverstr, verbosity);
      if (solverstr != "")
      {
         CharString tmp;
         BasicArray<CharString> strarray = split(solverstr, '-');
         strarray[0] += "_options";
         s = getenv(strarray[0].data());
         if (s)
            process_options(strarray[0], s, plist, tmp, verbosity);
         if (strarray.size() > 1)
         {
            plist_subsolvers.resize(strarray.size() - 1);
            for (size_t i = 1; i < strarray.size(); i++)
            {
               strarray[i] += "_options";
               s = getenv(strarray[i].data());
               if (!s)
               {
                  strarray[i] = upper_case(strarray[i]);
                  s = getenv(strarray[i].data());
               }
               if (s)
                  process_options(strarray[i], s, plist_subsolvers[i-1], tmp, verbosity);
            }
         }
      }
   }
   else
      EXCEPTION_MNGR(std::runtime_error, "Missing " << envstr
                     << " environmental variable!");

   //
   // Create the solver handle
   //
   SolverHandle solver = SolverMngr().create_solver(solverstr.data());
   if (solver.empty())
      EXCEPTION_MNGR(std::runtime_error, "Ampl_Driver():"
                     " Solver \"" << solverstr << "\" not found");
   list< pair<string, string> >::iterator curr = plist.begin();
   list< pair<string, string> >::iterator end  = plist.end();
   while (curr != end)
   {
      (*solver)[curr->first] = curr->second;
      curr++;
   }

   //
   // Setup the problem
   //
   std::pair<ApplicationHandle, AmplApplication*> problem
      = ApplicationHandle::create<AmplApplication>();
   problem.second->keep_nl_file();
   Any initial_point = problem.second->set_nl_file(fname);

   //
   // Give the problem to the solver, set the initial point, and
   // configure the options
   //
   // Special case: if the problem is strictly continuous, then pass in
   // the relaxed application.
   if ( problem.second->num_int_vars.as<size_t>() + 
        problem.second->num_binary_vars.as<size_t>() > 0)
      solver->set_problem(problem.first);
   else
      solver->set_problem(problem.second->relaxed_application());

   //Application_Base* a = solver->get_problem_handle().operator->();
   //while ( a != NULL )
   //{
   //   std::cerr << "Problem: " << utilib::demangledName(typeid(*a))
   //             << endl;
   //   a = a->get_reformulated_application();
   //}
   //std::cerr << "initial point = " << initial_point << endl;

   string name = ApplicationMngr().get_application_name(problem.second);
   if (verbosity >= 2)
   {
      ucout << "\n------------------------" << endl;
      ucout << " AMPL Application Summary" << endl;
      ucout << "------------------------" << endl;
      //ucout << "Problem Name: " << name << endl;
      ucout << "Problem Type: " << problem.first->problem_type_name() << endl;
      problem.first->print_summary(ucout);
      ucout << endl;
      ucout.flush();
   }

   if (!initial_point.empty())
   {
      // Because the initial point is in the context of the
      // AmplApplication, but the solver may (or may not) be operating
      // directly on the AmplApplication, we need to explicitly convert
      // the initial_point domain into an AppResponse in the
      // AmplApplication context to guarantee consistency within the cache.
      AppResponse init_response = solver->eval_mngr().perform_evaluation
         (problem.second->set_domain(initial_point));
      solver->add_initial_point(init_response);
   }

   //
   // Reset the optimizer and print debugging info
   //
   solver->reset();
   if (verbosity >= 1)
   {
      list<Any> point_list;
      solver->get_initial_points().get_points(problem.second, point_list);
      ucout << "Num Initial Points " << point_list.size() << endl;
      for (int i=1; ! point_list.empty(); ++i, point_list.pop_front() )
         ucout << "  Point #" << i << ": " << point_list.front() << endl;
      ucout << utilib::Flush;
   }

   if (verbosity >= 2)
   {
      ucout << "\n------------------------" << endl;
      ucout << " Solver Options";
      ucout << "\n------------------------" << endl;
      solver->Properties().write(PropertyDict::DescriptionWriter(ucout));
      ucout << "\n------------------------" << endl;
      ucout << " Solver Option Values";
      ucout << "\n------------------------" << endl;
      solver->Properties().write(PropertyDict::ValueWriter(ucout));
      ucout << endl;
      ucout << utilib::Flush;
   }
   //
   // Solve!!!
   //
   double user_start = UserSeconds();
   double system_start = SystemSeconds();
   double wallclock_start = WallClockSeconds();
   solver->optimize();
   solver->postsolve();
   double user_end = UserSeconds();
   double system_end = SystemSeconds();
   double wallclock_end = WallClockSeconds();
   solver->set_statistic("User time", user_end - user_start);
   solver->set_statistic("System time", system_end - system_start);
   solver->set_statistic("Wallclock time", wallclock_end - wallclock_start);
   //
   // Print final status
   //
   std::stringstream msg;
   PointSet ps = solver->get_final_points();
   if (verbosity >= 1)
   {
      ucout << endl;
      utilib::PropertyDict_YamlPrinter printer;
      printer.print(ucout, solver->results(), "AMPL Solver Results");

      std::list<Any > points;
      ps.get_points(problem.second, points);

      if (points.size() == 1)
      {
#if 0
         ucout << "Final Point: " << points.front() << endl;

         std::vector<double> obj;
         problem.second
            ->EvalMF(problem.second->eval_mngr(), points.front(), obj);
         if (obj.size() == 1)
            ucout << "Final Value: " << obj[0] << endl;
         else
            ucout << "Final Value: " << obj << endl;
         std::vector<double> cfviol;
         problem.second->EvalCFViol(problem.second->eval_mngr(), points.front(), cfviol);
         double tmp = sqrt(static_cast<double>(l2_norm_sq(cfviol)));
         ucout << "Constraint Violation: " << tmp << endl;
#endif
      }
      else if (points.size() > 1)
      {
         std::string ofile;
         ofile = name;
         ofile += ".sol.txt";
         std::ofstream ofstr(ofile.c_str());
         std::list<Any >::iterator curr = points.begin();
         std::list<Any >::iterator end  = points.end();
         ucout << "Final Points" << endl;
         while (curr != end)
         {
            ofstr << *curr << endl;
            std::vector<double> obj;
            problem.second->EvalMF(problem.second->eval_mngr(), *curr, obj);
            ofstr << obj << endl;
            curr++;
         }
         ofstr.close();
         msg << "Writing solutions in file: " << ofile << ".\n";
      }
      ucout << utilib::Flush;
   }
   ucout << std::endl;
   //
   // Publish the result back to AMPL
   //
   msg << "Results from COLIN Solver: " << solver->type() << ".\n";
   problem.second->write_sol_file(ps, msg);
   //
   // Cleanup AMPL data
   //
   ASL_free(&ASL_ptr);
   return 0;
}

} // namespace colin

#endif // defined(ACRO_USING_AMPL ) && !defined(TFLOPS)

