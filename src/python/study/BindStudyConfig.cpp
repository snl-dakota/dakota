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
  py::class_<StudyOutputConfig>(
    m, "StudyOutputConfig",
    "Output, restart, tabular-data, and results settings for a Study.")
    .def(py::init<>())
    .def_readwrite("output_file", &StudyOutputConfig::outputFile,
                   "File receiving Dakota standard output; empty uses the active stream.")
    .def_readwrite("error_file", &StudyOutputConfig::errorFile,
                   "File receiving Dakota error output; empty uses the active stream.")
    .def_readwrite("read_restart", &StudyOutputConfig::readRestart,
                   "Restart file to read before execution.")
    .def_readwrite("stop_restart", &StudyOutputConfig::stopRestart,
                   "Maximum evaluation number to read from the restart file; zero is unlimited.")
    .def_readwrite("write_restart", &StudyOutputConfig::writeRestart,
                   "Restart file to write during execution.")
    .def_readwrite("graphics", &StudyOutputConfig::graphics,
                   "Enable Dakota graphics output when supported.")
    .def_readwrite("tabular_graphics_data",
                   &StudyOutputConfig::tabularGraphicsData,
                   "Enable tabular graphics-data output.")
    .def_readwrite("tabular_graphics_file",
                   &StudyOutputConfig::tabularGraphicsFile,
                   "Destination for tabular graphics data.")
    .def_readwrite("tabular_format", &StudyOutputConfig::tabularFormat,
                   "Dakota tabular-format enumeration value.")
    .def_readwrite("precision", &StudyOutputConfig::precision,
                   "Numeric output precision; zero retains Dakota's default.")
    .def_readwrite("results_output", &StudyOutputConfig::resultsOutput,
                   "Enable results-database output.")
    .def_readwrite("results_output_file",
                   &StudyOutputConfig::resultsOutputFile,
                   "Base name for results-database output.")
    .def_readwrite("results_output_format",
                   &StudyOutputConfig::resultsOutputFormat,
                   "Dakota results-output format enumeration value.")
    .def_readwrite("model_evals_selection",
                   &StudyOutputConfig::modelEvalsSelection,
                   "Selection policy for model evaluations stored in results output.")
    .def_readwrite("interface_evals_selection",
                   &StudyOutputConfig::interfaceEvalsSelection,
                   "Selection policy for interface evaluations stored in results output.");

  py::class_<StudyRunConfig>(
    m, "StudyRunConfig",
    "Controls the pre-run, run, and post-run execution phases of a Study.")
    .def(py::init<>())
    .def_readwrite("pre_run", &StudyRunConfig::preRun,
                   "Enable the pre-run phase.")
    .def_readwrite("run", &StudyRunConfig::run,
                   "Enable the main run phase.")
    .def_readwrite("post_run", &StudyRunConfig::postRun,
                   "Enable the post-run phase.")
    .def_readwrite("pre_run_input", &StudyRunConfig::preRunInput,
                   "Input file for the pre-run phase.")
    .def_readwrite("pre_run_output", &StudyRunConfig::preRunOutput,
                   "Output file for the pre-run phase.")
    .def_readwrite("run_input", &StudyRunConfig::runInput,
                   "Input file for the main run phase.")
    .def_readwrite("run_output", &StudyRunConfig::runOutput,
                   "Output file for the main run phase.")
    .def_readwrite("post_run_input", &StudyRunConfig::postRunInput,
                   "Input file for the post-run phase.")
    .def_readwrite("post_run_output", &StudyRunConfig::postRunOutput,
                   "Output file for the post-run phase.")
    .def_readwrite("pre_run_output_format",
                   &StudyRunConfig::preRunOutputFormat,
                   "Dakota format enumeration for pre-run output.")
    .def_readwrite("post_run_input_format",
                   &StudyRunConfig::postRunInputFormat,
                   "Dakota format enumeration for post-run input.");

  py::class_<StudyConfig>(
    m, "StudyConfig",
    "Top-level typed configuration for a dependency-injection Study.")
    .def(py::init<>())
    .def_readwrite("output", &StudyConfig::output,
                   "Output and restart configuration.")
    .def_readwrite("run", &StudyConfig::run,
                   "Execution-phase configuration.");
}

} // namespace Dakota::python
