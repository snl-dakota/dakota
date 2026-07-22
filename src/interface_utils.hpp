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
#include "util/generic_factory.hpp"

namespace Dakota {
    class Interface;
    class ProblemDescDB;
    class ParallelLibrary;
    namespace InterfaceUtils {
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Interface>
        get_interface(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib);

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

    struct InterfaceRegistryErrorPolicy
    {
      std::unique_ptr<Interface> on_unknown_key(unsigned short key, ProblemDescDB &problem_db, ParallelLibrary &parallel_lib) const;
    };

    class InterfaceRegistry {
    public:
      using registry_fun = std::unique_ptr<Interface>(ProblemDescDB&,
                                                      ParallelLibrary&);

      InterfaceRegistry();

      std::shared_ptr<Interface> get_interface(ProblemDescDB& db,
                                               ParallelLibrary& par);

      void file_cleanup();

      const std::unordered_map<std::string, std::shared_ptr<Interface>> &cache() const noexcept { return m_cache; }

     private:
      template <typename T>
      static std::unique_ptr<Interface> default_factory_fun(ProblemDescDB& db,
                                                            ParallelLibrary& par) {
        return std::make_unique<T>(db, par);
      }

      Util::GenericFactory<unsigned short, registry_fun,
                          InterfaceRegistryErrorPolicy>
          m_factory;

      std::unordered_map<std::string, std::shared_ptr<Interface>> m_cache;
    };
}
