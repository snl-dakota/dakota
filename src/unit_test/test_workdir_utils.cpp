/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "WorkdirHelper.hpp"
#include "CommandShell.hpp"
#include "dakota_global_defs.hpp"

// Boost.Test
#include <boost/test/minimal.hpp>

//#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>


namespace Dakota {
namespace TestWorkdir {

void test_save_current_path(const std::string& pwd_str,
                            const std::string& env_path_str)
{
  // NOTE: current path is passed in as a argument

  bfs::path tmp_dir( WorkdirHelper::system_tmp_path() );
  bfs::path temp_name = bfs::unique_path("daktst_%%%%%%%%");
  bfs::path wd( tmp_dir/temp_name );

  WorkdirHelper::create_directory(wd, DIR_CLEAN);

  if( bfs::exists(wd) && is_directory(wd) ) {
    // Change directory into newly created dir (verify current path is as expected)
    WorkdirHelper::change_directory(wd);
    std::string new_pwd_str = boost::filesystem::current_path().string();
    BOOST_CHECK( pwd_str != new_pwd_str );

    WorkdirHelper::set_preferred_path();

    // verify that PATH has been updated; i.e. '.' is prepended
    std::string new_env_path_str( std::getenv("PATH") );
    BOOST_CHECK( env_path_str != new_env_path_str );
    BOOST_CHECK( new_env_path_str[0] == '.' );

    // Change to startup PWD (aka rundir) (verify current path is as expected)
    // change back to original rundir
    bfs::path rundir(pwd_str);
    WorkdirHelper::change_directory(rundir);
    new_pwd_str = boost::filesystem::current_path().string();
    BOOST_CHECK( pwd_str == new_pwd_str );

    // verify that PATH no longer has '.' prepended
    WorkdirHelper::prepend_preferred_env_path( pwd_str );

    std::string newest_env_path_str( std::getenv("PATH") );
    BOOST_CHECK( newest_env_path_str[0] != '.' );
  }
  else
    BOOST_ERROR( "Issue with newly created workdir in test_save_current_path()" );
}


void count_driver_scripts(const std::string& glob_string)
{
  try {
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
  catch (const std::runtime_error&) {
    // WJB - ToDo: improve try/catch blocks
    return;
  }
}


void test_which_driver(bfs::path& wd, const std::string& driver_name)
{
  BOOST_CHECK( bfs::exists(wd) );
  BOOST_CHECK( wd.is_absolute() );

  // BFS issue?  mkdir/change_dir seems problematic if wd is relative dir
  WorkdirHelper::change_directory(wd);
  WorkdirHelper::set_preferred_path();

  // verify that a driver can be found in newly populated wdir
  bfs::path driver = WorkdirHelper::which(driver_name);

  BOOST_CHECK( !driver.is_absolute() );
  BOOST_CHECK( bfs::is_regular_file(driver) );
  BOOST_CHECK( bfs::equivalent(driver, bfs::path(driver_name)) );
}


void test_cp_template_files_into_wd(bfs::path& wd)
{
  //std::cout << "OK to COPY template files into EMPTY dir:  " << wd << std::endl;
  std::string template_path_str = WorkdirHelper::startup_pwd();
  template_path_str += "/../test/dakota_workdir.templatedir/*";

  StringArray template_items(1, template_path_str);

  try {
    WorkdirHelper::copy_items(template_items, wd, true);
  }
  catch (const std::runtime_error&) {
#if defined(DEBUG)
    std::cout << "\nWarning: unable to locate BFS::path items relative.."
              << " to directory:  " << template_path_str << std::endl;
#endif
    // Early return undesirable, but this is just some simple test code
    // (TODO perhaps?) come back and improve testing logic later
    return;
  }

  // double-check contents of expected items in the wd
  bfs::directory_iterator it(wd), eod;
  size_t file_count = 0; size_t link_count = 0; size_t dir_count = 0;
  BOOST_FOREACH( bfs::path const &p, std::make_pair(it, eod) ) { 
    if( is_regular_file(p) ) {
      ++file_count;
      //std::cout << "counting files poplated wdir.. currentFCount is: " << file_count << std::endl;
    } 
    else if( is_symlink(p) ) {
      ++link_count;
      //std::cout << "counting links poplated wdir.. currentLCount is: " << link_count << std::endl;
    } 
    else if( is_directory(p) ) {
      ++dir_count;
      //std::cout << "counting dirs poplated wdir.. currentDCount is: " << dir_count << std::endl;
    } 
  }

  BOOST_CHECK( !wd.empty() );
  BOOST_CHECK( file_count+link_count+dir_count >= 2 );
  //std::cout << "DONE iterating dir:  " << wd << std::endl;

  // now determine if PATH environment is suitable for script in workdir
  // i.e. test the behavior of "which" after cd into newly populated dir

  test_which_driver(wd, "templatedir_rosenbrock.py"); // consider argv[1] here?
}


void test_ln_template_files_into_wd(bfs::path& wd)
{
  //std::cout << "OK to LINK template files into EMPTY dir:  " << wd << std::endl;
  std::string template_path_str = WorkdirHelper::startup_pwd();
  template_path_str += "/../test/dakota_workdir.templatedir/*";

  StringArray template_items(1, template_path_str);

  try {
    WorkdirHelper::link_items(template_items, wd, true);
  }
  catch (const std::runtime_error&) {
#if defined(DEBUG)
    std::cout << "\nWarning: unable to locate BFS::path items relative.."
              << " to directory:  " << template_path_str << std::endl;
#endif
    // Early return undesirable, but this is just some simple test code
    // (TODO perhaps?) come back and improve testing logic later
    return;
  }

  // double-check contents of expected items in the wd
  bfs::directory_iterator it(wd), eod;
  size_t file_count = 0; size_t link_count = 0; size_t dir_count = 0;
  BOOST_FOREACH( bfs::path const &p, std::make_pair(it, eod) ) { 
    if( is_regular_file(p) ) {
      ++file_count;
      //std::cout << "counting files poplated wdir.. currentFCount is: " << file_count << std::endl;
    } 
    else if( is_symlink(p) ) {
      ++link_count;
      //std::cout << "counting links poplated wdir.. currentLCount is: " << link_count << std::endl;
    } 
    else if( is_directory(p) ) {
      ++dir_count;
      //std::cout << "counting dirs poplated wdir.. currentDCount is: " << dir_count << std::endl;
    } 
  }

  BOOST_CHECK( !wd.empty() );
  BOOST_CHECK( file_count+link_count+dir_count >= 2 );
  //std::cout << "DONE iterating dir:  " << wd << std::endl;

  // now determine if PATH environment is suitable for script in workdir
  // i.e. test the behavior of "which" after cd into newly populated dir

  test_which_driver(wd, "templatedir_rosenbrock.py"); // consider argv[1] here?

  //BOOST_CHECK( bfs::exists(wd += "/dakota_workdir.templatedir.dat") );
}


void test_rmdir(bfs::path& wd)
{
  // 7.	Remove the directory (verify its gone)
  WorkdirHelper::recursive_remove(wd, 0); // 0 indicates silent (appropriate for unit test)
  BOOST_CHECK( !bfs::exists(wd) );
}


void test_create_and_remove_tmpdir(bool copy=false)
{
// 2.	Use system tmp name (verify non-empty)
// 3.	Make a directory (verify it exists);
//		ToDo: does this work right for pre-existing dir when it’s supposed to tolerate vs. error?
// 5.	ToDo:  Make a subdir and a contained file so we can verify rm –rf (verify subdir is there)

  bfs::path tmp_dir( WorkdirHelper::system_tmp_path() );
  bfs::path temp_name = bfs::unique_path("daktst_%%%%%%%%");
  bfs::path wd( tmp_dir/temp_name );

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
// 3.	Make a directory (verify it exists);
//		ToDo: does this work right for pre-existing dir when it’s supposed to tolerate vs. error?
// 5.	ToDo:  Make a subdir and a contained file so we can verify rm –rf (verify subdir is there)

  bfs::path wd( WorkdirHelper::rel_to_abs(dir_name) );
  WorkdirHelper::create_directory(wd, DIR_CLEAN);

  if( bfs::exists(wd) && is_directory(wd) ) {
    (copy) ? test_cp_template_files_into_wd(wd)
           : test_ln_template_files_into_wd(wd);

    //std::cout << "OK to remove dir (with contained items): " << wd << std::endl;
#if 0
    test_rmdir(wd); // WJB: comment-out for manual inspection after a testrun
#endif
  }
  else
    BOOST_ERROR( "Ouch..." );
}


void test_driver_relative_path(const bfs::path& rel_driver_path)
{
  std::string rundir_str = Dakota::WorkdirHelper::startup_pwd();
  Dakota::WorkdirHelper::change_directory(rundir_str);

  BOOST_CHECK( !rel_driver_path.is_absolute() );
#if !defined(_WIN32)
  BOOST_CHECK( bfs::is_regular_file(rel_driver_path) );
#endif

  //try {
  Dakota::CommandShell sys_call_sh;
  sys_call_sh << rel_driver_path.string();
  sys_call_sh << Dakota::flush;
}

} // end namespace TestWorkdir
} // end namespace Dakota



// NOTE: Boost.Test framework provides the main progran driver

//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  using namespace Dakota::TestWorkdir;

  Dakota::WorkdirHelper::initialize();

  std::string rundir_str = Dakota::WorkdirHelper::startup_pwd();
  std::string env_path_str( std::getenv("PATH") );
#if !defined(_WIN32)
  test_save_current_path(rundir_str, env_path_str);
#endif

  test_create_and_remove_tmpdir();                // SYM LINKS used by default
  test_create_and_remove_wd_in_rundir("workdir"); // SYM LINKS used by default

  // Repeat create_and_remove, but override the default "link" case by copying
  // files instead
  bool do_copy = true;
  test_create_and_remove_tmpdir(do_copy);
  test_create_and_remove_wd_in_rundir("workdir", do_copy);

  /* WJB: consider refactor count_driver_scripts test -- bfs::path fq_search(argv[1]);
  std::string fq_search(rundir_str);
  fq_search += "/../test/d*.sh";
  int run_result = 0;

  count_driver_scripts(fq_search);
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success ); */

#if defined(_WIN32)
  #define PATH_SEP_SLASH '\\'
#else
  #define PATH_SEP_SLASH '/'
#endif
  std::string relative_driver_path("..");
  relative_driver_path += PATH_SEP_SLASH;
  relative_driver_path += "uthelper";
  test_driver_relative_path(relative_driver_path);

  return boost::exit_success;
}
