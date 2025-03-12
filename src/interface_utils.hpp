#pragma once
#include <memory>
#include <string>

namespace Dakota {
    class Interface;
    class ProblemDescDB;
    namespace InterfaceUtils {
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Interface>
        get_interface(ProblemDescDB& problem_db);
    }
}
