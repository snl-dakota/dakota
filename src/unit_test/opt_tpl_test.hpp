#ifndef OPT_TPL_TEST_H
#define OPT_TPL_TEST_H

#include "APPSOptimizer.hpp"

#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "LibraryEnvironment.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"
#include "TestDriverInterface.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


namespace Dakota {

namespace Opt_TPL_Test {

  static Dakota::LibraryEnvironment * create_env(const String& in_string);

} // namespace Opt_TPL_Test_Apps
} // namespace Dakota


//----------------------------------------------------------------

static Dakota::LibraryEnvironment * Dakota::Opt_TPL_Test::create_env(const String& in_string)
{
  // No input file set --> no parsing.  Could set other command line
  // options such as restart in opts:
  Dakota::ProgramOptions opts;
  opts.echo_input(false);

  opts.input_string(in_string);

  // delay validation/sync of the Dakota database and iterator
  // construction to allow update after all data is populated
  bool check_bcast_construct = false;

  // set up a Dakota instance
  Dakota::LibraryEnvironment * p_env = new Dakota::LibraryEnvironment(MPI_COMM_WORLD, opts, check_bcast_construct);
  Dakota::LibraryEnvironment & env = *p_env;
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();

  // configure Dakota to throw a std::runtime_error instead of calling exit
  env.exit_mode("throw");

  // once done with changes: check database, broadcast, and construct iterators
  env.done_modifying_db();

  return p_env;
}
 #endif
