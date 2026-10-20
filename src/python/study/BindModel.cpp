/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "DakotaModel.hpp"
#include "DataFitSurrModel.hpp"
#include "EnsembleSurrModel.hpp"
#include "NestedModel.hpp"
#include "SimulationModel.hpp"

#include <memory>

namespace Dakota::python {

void bind_models(py::module_& m)
{
  py::class_<Model, std::shared_ptr<Model>>(
    m, "Model", py::module_local(),
    "Base model handle returned by ModelFactory.");
  py::class_<SimulationModel, Model, std::shared_ptr<SimulationModel>>(
    m, "SimulationModel", py::module_local(),
    "Model that evaluates a simulation interface directly.");
  py::class_<DataFitSurrModel, Model, std::shared_ptr<DataFitSurrModel>>(
    m, "DataFitSurrModel", py::module_local(),
    "Data-fit surrogate model constructed by ModelFactory.");
  py::class_<EnsembleSurrModel, Model, std::shared_ptr<EnsembleSurrModel>>(
    m, "EnsembleSurrModel", py::module_local(),
    "Ensemble surrogate model constructed by ModelFactory.");
  py::class_<NestedModel, Model, std::shared_ptr<NestedModel>>(
    m, "NestedModel", py::module_local(),
    "Nested model constructed from an injected sub-iterator.");
}

} // namespace Dakota::python
