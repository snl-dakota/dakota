/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_GLOBAL_DEFS_H
#define DAKOTA_GLOBAL_DEFS_H

#include "dakota_system_defs.hpp"
#include <stdexcept>
#include <limits>

namespace Dakota {

// --------------
// Special values
// --------------
/// special value returned by index() when entry not found
//const size_t _NPOS  = ~(size_t)0; // one's complement
const size_t SZ_MAX = std::numeric_limits<size_t>::max();
const size_t _NPOS  = SZ_MAX; // alias old definition
/// bound beyond which constraints are considered inactive
const double BIG_REAL_BOUND = 1.0e+30;
/// constant pi
extern double PI;
/// constant log(2*pi)/2.0
extern double HALF_LOG_2PI;

// --------------
// Global objects
// --------------

// define Cout/Cerr, use them to dereference dakota_cout/dakota_cerr
#define Cout (*Dakota::dakota_cout)
#define Cerr (*Dakota::dakota_cerr)

// externs
// Note: Dakota class externs are declared elsewhere in order to maintain a
// clear hierarchy of dependencies.
//extern PRPCache data_pairs;
//extern ParallelLibrary dummy_lib;

/// enum for Dakota abort reasons; using negative numbers to
/// distinguish Dakota exit states from signals / uncaught signals.
/// These need to be in range [-63, -1], so exit code (256+enum) is in
/// [193, 255].  See RATIONALE in dakota_global_defs.cpp.
enum {
  VARS_ERROR      = -11, // error with Variables
  CONS_ERROR      = -10, // error with Constraints
  RESP_ERROR      = -9,  // error with Response
  APPROX_ERROR    = -8,  // error with Approximation
  METHOD_ERROR    = -7,  // error with Iterator/MetaIterator
  MODEL_ERROR     = -6,  // error within Model
  IO_ERROR        = -5,  // error with core file I/O
  INTERFACE_ERROR = -4,  // error with map invocation (user workflow/driver): 
                         // analysis driver, if/of
  CONSTRUCT_ERROR = -3,
  PARSE_ERROR     = -2,
  OTHER_ERROR     = -1   // the historical Dakota default error
};

/// enum for selecting the models that store evaluations
enum { 
      MODEL_EVAL_STORE_TOP_METHOD = 0,
      MODEL_EVAL_STORE_NONE,
      MODEL_EVAL_STORE_ALL,
      MODEL_EVAL_STORE_ALL_METHODS};

/// enum for selecting the interfaces that store evaluations
enum {
      INTERF_EVAL_STORE_SIMULATION = 0,
      INTERF_EVAL_STORE_NONE,
      INTERF_EVAL_STORE_ALL};


/// enum for dakota abort behaviors
enum {ABORT_EXITS, ABORT_THROWS};

/// enum for active subspace cross validation identification
enum {CV_ID_DEFAULT = 0, MINIMUM_METRIC, RELATIVE_TOLERANCE, DECREASE_TOLERANCE};

/// whether dakota exits/aborts or throws on errors
extern short abort_mode;

/// throw or exit depending on abort_mode
void abort_throw_or_exit(int dakota_code);


/// base class for Dakota file read exceptions (to allow catching both
/// tabular and general file truncation issues)
class FileReadException: public std::runtime_error
{
public:
  FileReadException(const std::string& msg): std::runtime_error(msg)
  { /* empty ctor */ }
};

/// exception thrown when data read truncated 
class TabularDataTruncated: public FileReadException
{ 
public:
  TabularDataTruncated(const std::string& msg): FileReadException(msg)
  { /* empty ctor */ }
};

/// exception throw for other results file read error
class ResultsFileError: public FileReadException 
{
public:
  ResultsFileError(const std::string& msg): FileReadException(msg)
  { /* empty ctor */ }			    
};


/// exception class for function evaluation failures
class FunctionEvalFailure: public std::runtime_error
{
public:
  FunctionEvalFailure(const std::string& msg): std::runtime_error(msg)
  { /* empty ctor */ }
};



extern std::ostream* dakota_cout;
extern std::ostream* dakota_cerr;
extern int write_precision;

/// options for tabular columns
enum { TABULAR_NONE = 0, TABULAR_HEADER = 1, 
       TABULAR_EVAL_ID = 2, TABULAR_IFACE_ID = 4,
       // experiment data annotated has header and exp_id
       TABULAR_EXPER_ANNOT = TABULAR_HEADER | TABULAR_EVAL_ID,
       // default for tabular files is fully annotated as of Dakota 6.1
       TABULAR_ANNOTATED = TABULAR_HEADER | TABULAR_EVAL_ID | TABULAR_IFACE_ID };

/// Results output format
enum { RESULTS_OUTPUT_TEXT = 1, RESULTS_OUTPUT_HDF5 = 2};

/// options for results file format
enum {FLEXIBLE_RESULTS, LABELED_RESULTS};

/// Parameters file format
enum { PARAMETERS_FILE_STANDARD = 0, PARAMETERS_FILE_APREPRO, PARAMETERS_FILE_JSON };

/// Results file format
enum { RESULTS_FILE_STANDARD = 0, RESULTS_FILE_JSON };

/// define special values for surrogateExportFormats
enum { NO_MODEL_FORMAT=0, TEXT_ARCHIVE=1, BINARY_ARCHIVE=2, ALGEBRAIC_FILE=4,
       ALGEBRAIC_CONSOLE=8 };


#ifdef DAKOTA_MODELCENTER
extern int mc_ptr_int;
extern int dc_ptr_int;
#endif // DAKOTA_MODELCENTER

/// Dummy struct for overloading letter-envelope constructors.
/** BaseConstructor is used to overload the constructor for the base class
    portion of letter objects.  It avoids infinite recursion (Coplien p.139)
    in the letter-envelope idiom by preventing the letter from instantiating
    another envelope.  Putting this struct here avoids circular dependencies. */
struct BaseConstructor {
  BaseConstructor(int = 0) {} ///< C++ structs can have constructors
};

/// Dummy struct for overloading constructors used in on-the-fly
/// instantiations without ProblemDescDB support.
/** NoDBBaseConstructor is used to overload the constructor used for
    on-the-fly instantiations in which ProblemDescDB queries cannot be
    used.  Putting this struct here avoids circular dependencies. */
struct NoDBBaseConstructor {
  NoDBBaseConstructor(int = 0) {} ///< C++ structs can have constructors
};

/// Dummy struct for overloading constructors used in on-the-fly Model 
/// instantiations.
/** LightWtBaseConstructor is used to overload the constructor used for
    on-the-fly Model instantiations.  Putting this struct here avoids
    circular dependencies. */
struct LightWtBaseConstructor {
  LightWtBaseConstructor(int = 0) {} ///< C++ structs can have constructors
};

// ----------------
// Global functions
// ----------------

/// global function which handles serial or parallel aborts
void abort_handler(int code);

/** Templatized abort_handler_t method that allows for convenient return from 
    methods that otherwise have no sensible return from error clauses.  Usage:
    MyType& method() { return abort_handler<MyType&>(-1); } */
template <typename T>
T abort_handler_t(int code)
{
  abort_handler(code);
  throw code;
}

/// Global function to register signal handlers at top-level
void register_signal_handlers();

/// Global function to hold Dakota processes to help with MPI debugging.
void mpi_debug_hold();

} // namespace Dakota

/// Return type for EvaluationStore::model_allocate and interface_allocate
/// UNITIALIZED: Model constructor has this as its default
/// ACTIVE: The database is open and this model or interface should write to it
/// INACTIVE: The database is either closed, or this model or interface should not
///           write to it.
enum class EvaluationsDBState {UNINITIALIZED, ACTIVE, INACTIVE};

#endif // DAKOTA_GLOBAL_DEFS_H
