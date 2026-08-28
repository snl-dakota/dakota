/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "StudyConfig.hpp"

namespace Dakota::python {

void bind_study_config(py::module_& m)
{
  py::class_<StudyOutputConfig>(m, "StudyOutputConfig")
    .def(py::init<>())
    .def_readwrite("output_file", &StudyOutputConfig::outputFile)
    .def_readwrite("error_file", &StudyOutputConfig::errorFile)
    .def_readwrite("read_restart", &StudyOutputConfig::readRestart)
    .def_readwrite("stop_restart", &StudyOutputConfig::stopRestart)
    .def_readwrite("write_restart", &StudyOutputConfig::writeRestart)
    .def_readwrite("graphics", &StudyOutputConfig::graphics)
    .def_readwrite("tabular_graphics_data",
                   &StudyOutputConfig::tabularGraphicsData)
    .def_readwrite("tabular_graphics_file",
                   &StudyOutputConfig::tabularGraphicsFile)
    .def_readwrite("tabular_format", &StudyOutputConfig::tabularFormat)
    .def_readwrite("precision", &StudyOutputConfig::precision)
    .def_readwrite("results_output", &StudyOutputConfig::resultsOutput)
    .def_readwrite("results_output_file",
                   &StudyOutputConfig::resultsOutputFile)
    .def_readwrite("results_output_format",
                   &StudyOutputConfig::resultsOutputFormat)
    .def_readwrite("model_evals_selection",
                   &StudyOutputConfig::modelEvalsSelection)
    .def_readwrite("interface_evals_selection",
                   &StudyOutputConfig::interfaceEvalsSelection);

  py::class_<StudyRunConfig>(m, "StudyRunConfig")
    .def(py::init<>())
    .def_readwrite("pre_run", &StudyRunConfig::preRun)
    .def_readwrite("run", &StudyRunConfig::run)
    .def_readwrite("post_run", &StudyRunConfig::postRun)
    .def_readwrite("pre_run_input", &StudyRunConfig::preRunInput)
    .def_readwrite("pre_run_output", &StudyRunConfig::preRunOutput)
    .def_readwrite("run_input", &StudyRunConfig::runInput)
    .def_readwrite("run_output", &StudyRunConfig::runOutput)
    .def_readwrite("post_run_input", &StudyRunConfig::postRunInput)
    .def_readwrite("post_run_output", &StudyRunConfig::postRunOutput)
    .def_readwrite("pre_run_output_format",
                   &StudyRunConfig::preRunOutputFormat)
    .def_readwrite("post_run_input_format",
                   &StudyRunConfig::postRunInputFormat);

  py::class_<StudyConfig>(m, "StudyConfig")
    .def(py::init<>())
    .def_readwrite("output", &StudyConfig::output)
    .def_readwrite("run", &StudyConfig::run);
}

} // namespace Dakota::python
