/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        global_defs
//- Description:  Global object initializations and global function
//-               implementations
//- Owner:        Mike Eldred

#include "global_defs.h"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "DakotaBinStream.H"
#include "DakotaGraphics.H"
#include "AnalysisCode.H"
#include "DakotaInterface.H"
#include "ParallelLibrary.H"
#include "ProblemDescDB.H"
#include "ResultsManager.H"

static const char rcsId[]="@(#) $Id: global_defs.C 6716 2010-04-03 18:35:08Z wjbohnh $";


namespace Dakota {

// --------------------------
// Instantiate global objects
// --------------------------
std::ostream* dakota_cout = &std::cout; ///< DAKOTA stdout initially points to
  ///< std::cout, but may be redirected to a tagged ofstream if there are
  ///< concurrent iterators.
std::ostream* dakota_cerr = &std::cerr; ///< DAKOTA stderr initially points to
  ///< std::cerr, but may be redirected to a tagged ofstream if there are
  ///< concurrent iterators.
PRPCache data_pairs;          ///< contains all parameter/response pairs.
BoStream write_restart;       ///< the restart binary output stream (doesn't
                              ///< really need to be global anymore except for
                              ///< abort_handler()).

/// Global results database for iterator results
ResultsManager iterator_results_db;


Graphics dakota_graphics;     ///< the global Dakota::Graphics object used by
                              ///< strategies, models, and approximations
int write_precision = 10;     ///< used in ostream data output functions
                              ///< (restart_util.C overrides this default value)
ParallelLibrary dummy_lib("dummy"); ///< dummy ParallelLibrary object used for
                                    ///< mandatory reference initialization when
                                    ///< a real ParallelLibrary instance is 
                                    ///< unavailable
ProblemDescDB dummy_db;       ///< dummy ProblemDescDB object used for
                              ///< mandatory reference initialization when a
                              ///< real ProblemDescDB instance is unavailable
#ifdef DAKOTA_MODELCENTER
int mc_ptr_int = 0;           ///< global pointer for ModelCenter API
int dc_ptr_int = 0;           ///< global pointer for ModelCenter eval DB
#endif // DAKOTA_MODELCENTER

ProblemDescDB *Dak_pddb;      ///< set by ProblemDescDB, for use in parsing
ParallelLibrary *Dak_pl;      ///< set by ParallelLibrary, for use in CLH


// -----------------------
// Define global functions
// -----------------------
void abort_handler(int code)
{
  // WEH - uncomment this code if you want signals to generate a corefile.
  //       This is handy for debugging infinite loops ... which can be
  //       difficult to debug within GDB.
  //abort();

  if (code > 1) // code = 2 (Cntl-C signal), 0 (normal), & -1/1 (abnormal)
    Cout << "Signal Caught!" << std::endl;

  // Clean up
  Cout << std::flush; // flush cout or ofstream redirection
  Cerr << std::flush; // flush cerr or ofstream redirection
  write_restart.close();

  if (Dak_pddb) {
    // cleanup parameters/results files
    AnalysisCode const *ac;
    InterfaceList &ifaces = Dak_pddb->interface_list();
    InterfaceList::iterator It = ifaces.begin(), Ie = ifaces.end();
    for(; It != Ie; ++It) {
      if ((ac = It->analysis_code()))
	ac->file_cleanup();
    }
    // properly terminate in parallel
    Dak_pddb->parallel_library().abort_helper(code);
  }
  else
    std::exit(code);
}

} // namespace Dakota
