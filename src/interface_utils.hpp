/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once
#include "dakota_global_defs.hpp"
#include <memory>
#include <optional>
#include <string>

namespace Dakota {
    class Interface;
    class ProblemDescDB;
    namespace InterfaceUtils {
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Interface>
        get_interface(ProblemDescDB& problem_db);

        template<typename T>
          // Use these when we update to c++17
          //inline std::optional<T>
          inline T
          no_derived_method_error() {
            Cerr << "Error: No derived " << __func__ << " method.\n"
                 << "No default at Interface base class." << std::endl;
            abort_handler(-1);
            return T();
            //return std::nullopt;
          }

        inline void no_derived_method_error() {
          Cerr << "Error: No derived " << __func__ << " method.\n"
               << "No default at Interface base class." << std::endl;
          abort_handler(-1);
        }
    }
}
