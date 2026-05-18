#pragma once

namespace Dakota {

// Models selected for evaluation storage.
enum {
  MODEL_EVAL_STORE_TOP_METHOD = 0,
  MODEL_EVAL_STORE_NONE,
  MODEL_EVAL_STORE_ALL,
  MODEL_EVAL_STORE_ALL_METHODS
};

// Interfaces selected for evaluation storage.
enum {
  INTERF_EVAL_STORE_SIMULATION = 0,
  INTERF_EVAL_STORE_NONE,
  INTERF_EVAL_STORE_ALL
};

// Active subspace cross-validation identification.
enum { CV_ID_DEFAULT = 0, MINIMUM_METRIC, RELATIVE_TOLERANCE, DECREASE_TOLERANCE };

// Options for tabular columns.
enum {
  TABULAR_NONE = 0,
  TABULAR_HEADER = 1,
  TABULAR_EVAL_ID = 2,
  TABULAR_IFACE_ID = 4,
  TABULAR_EXPER_ANNOT = TABULAR_HEADER | TABULAR_EVAL_ID,
  TABULAR_ANNOTATED = TABULAR_HEADER | TABULAR_EVAL_ID | TABULAR_IFACE_ID
};

// Results output format.
enum { RESULTS_OUTPUT_TEXT = 1, RESULTS_OUTPUT_HDF5 = 2 };

// Parameters/results file formats.
enum { PARAMETERS_FILE_STANDARD = 0, PARAMETERS_FILE_APREPRO, PARAMETERS_FILE_JSON };
enum { RESULTS_FILE_STANDARD = 0, RESULTS_FILE_JSON };

// Surrogate export formats.
enum { NO_MODEL_FORMAT = 0, TEXT_ARCHIVE = 1, BINARY_ARCHIVE = 2, ALGEBRAIC_FILE = 4, ALGEBRAIC_CONSOLE = 8 };

} // namespace Dakota

