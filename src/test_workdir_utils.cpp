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
  std::string fq_search(std::getenv("HOME"));
  fq_search += "/d*";

  bfs::path search_dir;
  std::string wild_card;
  WorkdirHelper::split_wildcard(fq_search, search_dir, wild_card);

  std::cout << "Searching " << search_dir << " for " << wild_card << std::endl;

  MatchesWC wc_predicate(wild_card);
  glob_iterator fit(wc_predicate, bfs::directory_iterator(search_dir));
  glob_iterator fitend(wc_predicate, bfs::directory_iterator());

  for ( ; fit != fitend; ++fit)
    std::cout << "in wildcard: " << fit->path() << std::endl;

  //return status;
  return 0;
}
