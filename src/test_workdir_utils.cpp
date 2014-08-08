/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    2opyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "WorkdirHelper.hpp"
#include "dakota_data_util.hpp"  // for strcontains
#include "dakota_global_defs.hpp"
//#include "dakota_filesystem_utils.hpp"

//#include <boost/assign/std/vector.hpp>
//#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>

using namespace Dakota;

namespace Dakota {
}


namespace TestWorkdir {

// ToDo:  Populate a handful of workdir util "unit" tests

} // end namespace TestWorkdir


int main()
{
  // argc/argv??

  //bfs::path fq_search(argv[1]);
  bfs::path fq_search( std::getenv("HOME") );

  // TODO: better way to do this (trailing slash yields filename of .)
  // want to allow matching /tmp/foo/ as /tmp with entry foo so foo
  // gets copied or linked

  // BUT if fq_search is a symlink, perhaps want to resolve it...
  // In Posix, trailing slash on symlink means follow it

  if ( boost::equals( (--fq_search.end())->native(), ".") )
    ;//fq_search.remove_filename();

  bfs::path search_dir = fq_search.parent_path();

  // could instead consider the Dakota runtime PWD
    //search_dir = std::getenv(PWD);
  if (search_dir.empty())
    search_dir = ".";

  std::string wild_card = fq_search.filename().native();

  std::cout << "Searching " << search_dir << " for " << wild_card << std::endl;

  bfs::directory_iterator dir_it(search_dir);
  bfs::directory_iterator dir_end;
  for ( ; dir_it != dir_end; ++dir_it) {
    ;//if (matches_wildcard(dir_it->path().filename(), wild_card))
      std::cout << "in wildcard: " << dir_it->path() << std::endl;
    //else
    //  std::cout << "NOT in wildcard: " << dir_it->path() << std::endl;
  }

  //return status;
return 0;
}

