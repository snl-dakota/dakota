/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "interface_utils.hpp"

#include "DakotaInterface.hpp"
#include "DakotaVariables.hpp"
#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "SysCallApplicInterface.hpp"

#if defined(HAVE_SYS_WAIT_H) && defined(HAVE_UNISTD_H)
#include "ForkApplicInterface.hpp"
#elif defined(_WIN32)  // or _MSC_VER (native MSVS compilers)
#include "SpawnApplicInterface.hpp"
#endif  // HAVE_SYS_WAIT_H, HAVE_UNISTD_H

// Direct interfaces
#ifdef DAKOTA_GRID
#include "GridApplicInterface.hpp"
#endif  // DAKOTA_GRID
#ifdef DAKOTA_MATLAB
#include "MatlabInterface.hpp"
#endif  // DAKOTA_MATLAB
#ifdef DAKOTA_PYBIND11
#include "Pybind11Interface.hpp"
#endif  // DAKOTA_PYBIND11
#ifdef DAKOTA_SCILAB
#include "ScilabInterface.hpp"
#endif  // DAKOTA_SCILAB
#include "ApproximationInterface.hpp"
#include "PluginInterface.hpp"
#include "TestDriverInterface.hpp"

#ifdef HAVE_AMPL
#undef NO  // avoid name collision from UTILIB
#include "external/ampl/asl.h"
#endif  // HAVE_AMPL

namespace Dakota {
namespace InterfaceUtils {

// Derived class factory
std::shared_ptr<Interface> get_interface(ProblemDescDB& problem_db,
                                         ParallelLibrary& parallel_lib) {
  const unsigned short interface_type = problem_db.get_ushort("interface.type");

  // In the case where a derived interface type has been selected for managing
  // analysis_drivers, then this determines the instantiation and any
  // algebraic mappings are overlayed by ApplicationInterface.
  const String& algebraic_map_file =
      problem_db.get_string("interface.algebraic_mappings");
  if (interface_type == SYSTEM_INTERFACE)
    return std::make_shared<SysCallApplicInterface>(problem_db, parallel_lib);
  else if (interface_type == FORK_INTERFACE) {
#if defined(HAVE_SYS_WAIT_H) && defined(HAVE_UNISTD_H)  // includes CYGWIN/MINGW
    return std::make_shared<ForkApplicInterface>(problem_db, parallel_lib);
#elif defined(_WIN32)  // or _MSC_VER (native MSVS compilers)
    return std::make_shared<SpawnApplicInterface>(problem_db, parallel_lib);
#else
    Cerr << "Fork interface requested, but not enabled in this Dakota "
         << "executable." << std::endl;
    return std::shared_ptr<Interface>();
#endif
  }

  else if (interface_type == TEST_INTERFACE)
    return std::make_shared<TestDriverInterface>(problem_db, parallel_lib);
  // Note: in the case of a plug-in direct interface, this object gets replaced
  // using Interface::assign_rep().  Error checking in DirectApplicInterface::
  // derived_map_ac() should catch if this replacement fails to occur properly.

  else if (interface_type == PLUGIN_INTERFACE)
    return std::make_shared<PluginInterface>(problem_db, parallel_lib);

#ifdef DAKOTA_GRID
  else if (interface_type == GRID_INTERFACE)
    return std::make_shared<GridApplicInterface>(problem_db, parallel_lib);
#endif

  else if (interface_type == MATLAB_INTERFACE) {
#ifdef DAKOTA_MATLAB
    return std::make_shared<MatlabInterface>(problem_db, parallel_lib);
#else
    Cerr << "Direct Matlab interface requested, but not enabled in this "
         << "Dakota executable." << std::endl;
    return std::shared_ptr<Interface>();
#endif
  }

  else if (interface_type == PYTHON_INTERFACE) {
#ifdef DAKOTA_PYBIND11
    return std::make_shared<Pybind11Interface>(problem_db, parallel_lib);
#else
    Cerr << "Python interface requested, but not enabled in this "
         << "Dakota executable." << std::endl;
    return std::shared_ptr<Interface>();
#endif
  }

  else if (interface_type == SCILAB_INTERFACE) {
#ifdef DAKOTA_SCILAB
    return std::make_shared<ScilabInterface>(problem_db, parallel_lib);
#else
    Cerr << "Direct Scilab interface requested, but not enabled in this "
         << "Dakota executable." << std::endl;
    return std::shared_ptr<Interface>();
#endif
  }

  // Should not be needed since ApproximationInterface is plugged-in from
  // DataFitSurrModel using Interface::assign_rep().
  // else if (interface_type == APPROX_INTERFACE)
  //  return std::make_shared<ApproximationInterface>(problem_db, num_acv,
  //  num_fns);

  // In the case where only algebraic mappings are used, then no derived map
  // functionality is needed and ApplicationInterface is used
  else if (!algebraic_map_file.empty()) {
#ifdef DEBUG
    Cout << ">>>>> new ApplicationInterface: " << algebraic_map_file
         << std::endl;
#endif  // DEBUG
    return std::make_shared<ApplicationInterface>(problem_db, parallel_lib);
  }

  // If the interface type is empty (e.g., from default DataInterface creation
  // in ProblemDescDB::check_input()), then ApplicationInterface used
  else if (interface_type == DEFAULT_INTERFACE) {
    Cerr << "Warning: empty interface type in Interface::get_interface()."
         << std::endl;
    return std::make_shared<ApplicationInterface>(problem_db, parallel_lib);
  }

  else {
    Cerr << "Invalid interface: " << interface_enum_to_string(interface_type)
         << std::endl;
  }

  return std::shared_ptr<Interface>();
}

}  // namespace InterfaceUtils
}  // namespace Dakota
