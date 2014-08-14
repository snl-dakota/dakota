/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "WorkdirHelper.hpp"
#include "dakota_data_util.hpp"  // for strcontains
#include "dakota_global_defs.hpp"
//#include "dakota_filesystem_utils.hpp"

// Boost.Test
#include <boost/test/minimal.hpp>

//#include <boost/assign/std/vector.hpp>
//#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>

int test_main( int argc, char* argv[] );  // prototype for users test_main()

//using namespace Dakota;


namespace Dakota {
namespace TestWorkdir {

// ToDo:  Populate a handful of workdir util "unit" tests
class caller {
public:
    // constructor
    caller( int argc, char** argv )
    : m_argc( argc ), m_argv( argv ) {}

    // execution monitor hook implementation
    int operator()()
    {
      //bfs::path fq_search(argv[1]);
      std::string fq_search(std::getenv("PWD"));
      fq_search += "/../test/d*.sh";

      bfs::path search_dir;
      std::string wild_card;
      WorkdirHelper::split_wildcard(fq_search, search_dir, wild_card);

      std::cout << "Searching " << search_dir << " for " << wild_card << std::endl;

      MatchesWC wc_predicate(wild_card);
      glob_iterator fit(wc_predicate, bfs::directory_iterator(search_dir));
      glob_iterator fitend(wc_predicate, bfs::directory_iterator());

      for ( ; fit != fitend; ++fit)
        std::cout << "in wildcard: " << fit->path() << std::endl;

      //BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );
      return 0;
    }

private:
    // Data members
    int         m_argc;
    char**      m_argv;
}; // caller

} // end namespace TestWorkdir

} // namespace Dakota



//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  //using namespace boost::minimal_test;
  using namespace Dakota::TestWorkdir;

  try {
    ::boost::execution_monitor ex_mon;
    int run_result = ex_mon.execute( caller( argc, argv ) );
  }
  catch( boost::execution_exception const& exex ) {
    if( exex.code() != boost::execution_exception::no_error )
      BOOST_ERROR( (std::string( "exception \"" ).
                            append( exex.what().begin(), exex.what().end() ).
                            append( "\" caught" ) ).c_str() );
    std::cerr << "\n**** Testing aborted.";
  }

/*
  if( boost::minimal_test::errors_counter() != 0 ) {
    std::cerr << "\n**** " << boost::minimal_test::errors_counter()
              << " error" << (boost::minimal_test::errors_counter() > 1 ? "s" : "" )
              << " detected\n";

    return boost::exit_test_failure;
  }
*/

  return boost::exit_success;
}
