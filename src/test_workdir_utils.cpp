/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "WorkdirHelper.hpp"
//#include "dakota_global_defs.hpp"
//#include "dakota_filesystem_utils.hpp"

// Boost.Test
#include <boost/test/minimal.hpp>

//#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>


namespace Dakota {
namespace TestWorkdir {

// ToDo:  Populate a handful of workdir util "unit" tests

//int count_driver_scripts();
int list_driver_scripts(const std::string& glob_string)
{
  bfs::path search_dir;
  std::string wild_card;
  WorkdirHelper::split_wildcard(glob_string, search_dir, wild_card);

  std::cout << "Searching " << search_dir << " for " << wild_card << std::endl;

  MatchesWC wc_predicate(wild_card);
//
  glob_iterator fit(wc_predicate, bfs::directory_iterator(search_dir));
  glob_iterator fitend(wc_predicate, bfs::directory_iterator());
  for ( ; fit != fitend; ++fit)
    std::cout << "ls: " << fit->path() << std::endl;
//
  return boost::exit_success;
}


int test_rmdir(bfs::path& wd)
{
  std::cout << "OK to remove EMPTY dir:  " << wd << std::endl;
  WorkdirHelper::recursive_remove(wd);

  return boost::exit_success;
}


int test_create_and_remove_dir(const std::string& dir_name)
{
  bfs::path wd( WorkdirHelper::system_tmp_name(dir_name) );
  WorkdirHelper::create_directory(wd, DIR_CLEAN);

  if( bfs::exists(wd) && is_directory(wd) ) {
    std::cout << "in newly created dir: " << wd << std::endl;
//
    bfs::directory_iterator it(wd), eod;
    BOOST_FOREACH( bfs::path const &p, std::make_pair(it, eod) )   
    { 
      std::cout << "iterating filesIn created dir.. " << "\tWJB should NOT see this line"  << std::endl;
      if( is_regular_file(p) )
      {
        // no files (yet)
      } 
    }
//
    std::cout << "DONE iterating newly created/EMPTY dir:  " << wd << std::endl;

    int status = test_rmdir(wd);
    return status;
  }
  else
    BOOST_ERROR( "Ouch..." );
}

} // end namespace TestWorkdir
} // end namespace Dakota



// NOTE: Boost.Test framework provides the main progran driver

//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  using namespace Dakota::TestWorkdir;
  //using Dakota::WorkdirHelper;;

  //bfs::path fq_search(argv[1]);
  std::string fq_search(std::getenv("PWD"));
  fq_search += "/../test/d*.sh";
  int run_result = 0;

  run_result = list_driver_scripts(fq_search);
  run_result = test_create_and_remove_dir("dak_wd");

  //BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}
