#pragma once
#include <memory>


namespace Dakota {
    class Iterator;
    class Model;
    class ProblemDescDB;
    namespace IteratorUtils {
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Iterator>
        get_iterator(ProblemDescDB& problem_db);
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Iterator>
        get_iterator(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
        /// Used by the envelope to instantiate the correct letter class
        std::shared_ptr<Iterator>
        get_iterator(const std::string& method_string, std::shared_ptr<Model> model);
    }
}
