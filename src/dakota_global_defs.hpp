/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        global_defs
//- Description:  Encapsulates global objects and functions.
//-
//- Owner:        Mike Eldred
//- Version: $Id: global_defs.h 6492 2009-12-19 00:04:28Z briadam $

#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#include "dakota_system_defs.hpp"


namespace Dakota {

// --------------
// Special values
// --------------
/// special value returned by index() when entry not found
const size_t _NPOS = ~(size_t)0; // one's complement

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

/// enum for Dakota abort reasons; using negative numbers to avoid
/// clash with signal codes 1--64 in signum.h
enum {
  MODEL_ERROR     = -6, // error with Model recursion
  IO_ERROR        = -5, // error with core file I/O
  INTERFACE_ERROR = -4, // error with map invocation (user workflow/driver): 
                        // analysis driver, if/of
  CONSTRUCT_ERROR = -3,
  PARSE_ERROR     = -2,
  OTHER_ERROR     = -1  // the historical Dakota default error
};

/// enum for dakota abort behaviors
enum {ABORT_EXITS, ABORT_THROWS};

/// whether dakota exits/aborts or throws on errors
extern short abort_mode;

/// throw or exit depending on abort_mode
void abort_throw_or_exit(int code);

extern std::ostream* dakota_cout;
extern std::ostream* dakota_cerr;
extern int write_precision;
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

/// Dummy struct for overloading constructors used in on-the-fly instantiations.
/** NoDBBaseConstructor is used to overload the constructor used for
    on-the-fly instantiations in which ProblemDescDB queries cannot be
    used.  Putting this struct here avoids circular dependencies. */
struct NoDBBaseConstructor {
  NoDBBaseConstructor(int = 0) {} ///< C++ structs can have constructors
};

/// Dummy struct for overloading constructors used in on-the-fly Model 
/// instantiations.
/** RecastBaseConstructor is used to overload the constructor used for
    on-the-fly Model instantiations.  Putting this struct here avoids
    circular dependencies. */
struct RecastBaseConstructor {
  RecastBaseConstructor(int = 0) {} ///< C++ structs can have constructors
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

#endif // GLOBAL_DEFS_H
