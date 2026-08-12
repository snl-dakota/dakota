/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "ConcurrentMetaIterator.hpp"
#include "EffGlobalMinimizer.hpp"
#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif
#include "DakotaIterator.hpp"
#include "DakotaResponse.hpp"
#include "NL2SOLLeastSq.hpp"
#include "NPSOLOptimizer.hpp"
#include "NonDGlobalSingleInterval.hpp"
#include "NonDLHSSingleInterval.hpp"
#include "NonDLocalSingleInterval.hpp"
#include "ParamStudy.hpp"
#include "RichExtrapVerification.hpp"
#include "NonDLHSSampling.hpp"

#include <memory>
#include <stdexcept>

namespace Dakota::python {

void bind_iterators(py::module_& m)
{
  py::class_<Iterator, std::shared_ptr<Iterator>>(
    m, "Iterator", py::module_local());
  py::class_<NonDLHSSampling, Iterator, std::shared_ptr<NonDLHSSampling>>(
    m, "NonDLHSSampling", py::module_local())
    .def("num_responses",
         [](const NonDLHSSampling& sampling) {
           return sampling.all_responses().size();
         })
    .def("first_response_value",
         [](const NonDLHSSampling& sampling) {
           const auto& responses = sampling.all_responses();
           if (responses.empty())
             throw std::runtime_error("No responses are available.");

           const auto& first_response = responses.begin()->second;
           if (first_response.num_functions() == 0)
             throw std::runtime_error("First response has no functions.");

           return first_response.function_value(0);
         });
  py::class_<ParamStudy, Iterator, std::shared_ptr<ParamStudy>>(
    m, "ParamStudy", py::module_local());
  py::class_<RichExtrapVerification, Iterator, std::shared_ptr<RichExtrapVerification>>(
    m, "RichExtrapVerification", py::module_local());
  py::class_<NonDLocalSingleInterval, Iterator, std::shared_ptr<NonDLocalSingleInterval>>(
    m, "NonDLocalSingleInterval", py::module_local());
  py::class_<NonDGlobalSingleInterval, Iterator, std::shared_ptr<NonDGlobalSingleInterval>>(
    m, "NonDGlobalSingleInterval", py::module_local());
  py::class_<NonDLHSSingleInterval, Iterator, std::shared_ptr<NonDLHSSingleInterval>>(
    m, "NonDLHSSingleInterval", py::module_local());
#ifdef HAVE_NCSU
  py::class_<EffGlobalMinimizer, Iterator, std::shared_ptr<EffGlobalMinimizer>>(
    m, "EffGlobalMinimizer", py::module_local());
#endif
#ifdef HAVE_NPSOL
  py::class_<NPSOLOptimizer, Iterator, std::shared_ptr<NPSOLOptimizer>>(
    m, "NPSOLOptimizer", py::module_local());
#endif
#ifdef HAVE_NL2SOL
  py::class_<NL2SOLLeastSq, Iterator, std::shared_ptr<NL2SOLLeastSq>>(
    m, "NL2SOLLeastSq", py::module_local());
#endif
#ifdef HAVE_DOT
  py::class_<DOTOptimizer, Iterator, std::shared_ptr<DOTOptimizer>>(
    m, "DOTOptimizer", py::module_local());
#endif
  py::class_<ConcurrentMetaIterator, Iterator,
             std::shared_ptr<ConcurrentMetaIterator>>(
    m, "ConcurrentMetaIterator", py::module_local());
}

} // namespace Dakota::python
