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

void count_driver_scripts(const std::string& glob_string)
{
  bfs::path search_dir;
  bfs::path wild_card;
  WorkdirHelper::split_wildcard(glob_string, search_dir, wild_card);

#if defined(DEBUG)
  std::cout << "Listing driver scripts in " << search_dir
            << " by wildcard " << wild_card << std::endl;
#endif

  MatchesWC wc_predicate(wild_card);
  glob_iterator fit(wc_predicate, bfs::directory_iterator(search_dir));
  glob_iterator fitend(wc_predicate, bfs::directory_iterator());
  size_t file_count = 0;
  for ( ; fit != fitend; ++fit) {
#if defined(DEBUG)
    std::cout << "ls: " << fit->path() << std::endl;
#endif
    BOOST_CHECK( is_regular_file( fit->path() ) );
    ++file_count;
  }

  BOOST_CHECK( file_count >= 7 );
}


void test_cp_template_files_into_wd(bfs::path& wd)
{
  //std::cout << "OK to COPY template files into EMPTY dir:  " << wd << std::endl;
  std::string template_path_str( std::getenv("PWD") );
  template_path_str += "/../test/dakota_workdir.templatedir/*";

  StringArray template_items(1, template_path_str);
  WorkdirHelper::copy_items(template_items, wd, true);

  // double-check contents of expected items in the wd
  bfs::directory_iterator it(wd), eod;
  size_t file_count = 0; size_t link_count = 0; size_t dir_count = 0;
  BOOST_FOREACH( bfs::path const &p, std::make_pair(it, eod) ) { 
    if( is_regular_file(p) ) {
      ++file_count;
      //std::cout << "counting files poplated wdir.. currentFCount is: " << file_count << std::endl;
    } 
    //else if( is_link(p) ) {
      //++link_count;
      //std::cout << "counting links poplated wdir.. currentLCount is: " << link_count << std::endl;
    //} 
    else if( is_directory(p) ) {
      ++dir_count;
      //std::cout << "counting dirs poplated wdir.. currentDCount is: " << dir_count << std::endl;
    } 
  }

  BOOST_CHECK( !wd.empty() );
  BOOST_CHECK( file_count+link_count+dir_count >= 3 );
  //std::cout << "DONE iterating dir:  " << wd << std::endl;
}


void test_ln_template_files_into_wd(bfs::path& wd)
{
  //std::cout << "OK to LINK template files into EMPTY dir:  " << wd << std::endl;
  std::string template_path_str( std::getenv("PWD") );
  template_path_str += "/../test/dakota_workdir.templatedir/*";

  StringArray template_items(1, template_path_str);
  WorkdirHelper::link_items(template_items, wd, true);

  // double-check contents of expected items in the wd
  bfs::directory_iterator it(wd), eod;
  size_t file_count = 0; size_t link_count = 0; size_t dir_count = 0;
  BOOST_FOREACH( bfs::path const &p, std::make_pair(it, eod) ) { 
    if( is_regular_file(p) ) {
      ++file_count;
      //std::cout << "counting files poplated wdir.. currentFCount is: " << file_count << std::endl;
    } 
    //else if( is_link(p) ) {
      //++link_count;
      //std::cout << "counting links poplated wdir.. currentLCount is: " << link_count << std::endl;
    //} 
    else if( is_directory(p) ) {
      ++dir_count;
      //std::cout << "counting dirs poplated wdir.. currentDCount is: " << dir_count << std::endl;
    } 
  }

  BOOST_CHECK( !wd.empty() );
  BOOST_CHECK( file_count+link_count+dir_count >= 3 );
  //std::cout << "DONE iterating dir:  " << wd << std::endl;

  //BOOST_CHECK( bfs::exists(wd += "/dakota_workdir.templatedir.dat") );
}

void test_rmdir(bfs::path& wd)
{
  WorkdirHelper::recursive_remove(wd);
  BOOST_CHECK( !bfs::exists(wd) );
}


void test_create_and_remove_tmpdir(const std::string& dir_name, bool copy=false)
{
  bfs::path wd( WorkdirHelper::system_tmp_name(dir_name) );
  WorkdirHelper::create_directory(wd, DIR_CLEAN);

  if( bfs::exists(wd) && is_directory(wd) ) {
    (copy) ? test_cp_template_files_into_wd(wd)
           : test_ln_template_files_into_wd(wd);

    //std::cout << "OK to remove dir (with contained items): " << wd << std::endl;
    test_rmdir(wd); // WJB: comment-out for manual inspection after a testrun
  }
  else
    BOOST_ERROR( "Ouch..." );
}


void test_create_and_remove_wd_in_rundir(const std::string& dir_name,
  bool copy=false)
{
  //bfs::path wd( WorkdirHelper::system_tmp_name(dir_name) );
  bfs::path wd( dir_name );
  WorkdirHelper::create_directory(wd, DIR_CLEAN);

  if( bfs::exists(wd) && is_directory(wd) ) {
    (copy) ? test_cp_template_files_into_wd(wd)
           : test_ln_template_files_into_wd(wd);

    //std::cout << "OK to remove dir (with contained items): " << wd << std::endl;
#if 1
    test_rmdir(wd); // WJB: comment-out for manual inspection after a testrun
#endif
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

  count_driver_scripts(fq_search);
  test_create_and_remove_tmpdir("dak_wd");        // SYM LINKS used by default
  test_create_and_remove_wd_in_rundir("workdir"); // SYM LINKS used by default

  bool do_copy = true;
  test_create_and_remove_tmpdir("dak_wd", do_copy);
  test_create_and_remove_wd_in_rundir("workdir", do_copy);

  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}
