/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once
#include <memory>
#include <string>

namespace Dakota {
    class Iterator;
    class Model;
    class ProblemDescDB;
    class ParallelLibrary;

    namespace IteratorUtils {
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Iterator>
        get_iterator(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib);
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Iterator>
        get_iterator(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model);
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Iterator>
        get_iterator(const std::string& method_string, std::shared_ptr<Model> model);
    }
}
