/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file exception_mngr.h
 *
 * Defines routines for managing exceptions in various ways: throwing the
 * exception, exiting the algorithm, or aborting.
 *
 * Note that you can disable the throwing of exceptions by adding the
 * following line in your code:
 \verbatim

  utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
 \endverbatim
 * This code forces an abort whenever an exception occurs, which creates a
 * core file for debugging on most systems.
 *
 * \author William E. Hart
 */

#ifndef utilib_exception_mngr_h
#define utilib_exception_mngr_h

#include <utilib/std_headers.h>


namespace utilib {

using std::runtime_error;

void default_exitfn();

/// Key utility to get the mangled type name from the type_info structure
/** This provides a simple workaround for Microsoft's non-standard
 *  implementation of the std::type_info structure.
 */
inline const char * mangledName(const std::type_info & ti)
{
#ifdef _MSC_VER
   return ti.raw_name();
#else
   return ti.name();
#endif
}
/// Key utility to get the mangled type name from the type_info structure
/** This provides a simple workaround for Microsoft's non-standard
 *  implementation of the std::type_info structure.
 */
inline const char * mangledName(const std::type_info * ti)
{
   return ( ti == NULL ? "" : mangledName(*ti) );
}


/// Key utility to get the demangled name for a C++ symbol
std::string demangledName(const char* mangled);

/// Key utility to get the demangled name for a C++ symbol
inline std::string demangledName(const std::type_info & ti)
{
#ifdef _MSC_VER
   return ti.name();
#else
   return demangledName(ti.name());
#endif
}

/// Key utility to get the demangled name for a C++ symbol
inline std::string demangledName(const std::type_info * ti)
{
   return ( ti == NULL ? "" : demangledName(*ti) );
}


/**
 * A namespace that encapsulates the enum declaration and global data
 * used for managing exceptions.
 */
namespace exception_mngr {


/** Different modes of operation for the exception manager. */
enum handle_t
  {
  /// Simply throw exceptions
  Standard,
  /// Print an error (using CommonIO) and abort
  Abort,
  /// Print an error (using CommonIO) and exit
  Exit
  };


/** The global mode of the exception manager. */
handle_t mode();


/** Set the mode of the exception manager. */
void set_mode(handle_t);


/** A typedef for functions that are called when aborting or exiting on
  * exceptions. */
typedef void (*null_fn_type)(void);


/** The global exit function of the exception manager. */
void exit_function();


/** Setup a function that is called by \c handle_exception
  * when aborting or exiting.
  */
void set_exit_function(null_fn_type fn);


/** If true, EXCEPTION_MNGR() will append a full stack trace to the
 *  exception message.
 *
 *  stack_trace() follows a similar behavior to assert().  It defaults
 *  to false if exception_mngr.cpp is compiled with NDEBUG, otherwise,
 *  it defaults to true.
 *
 *  NB: to get symbolic stack traces, you will most likely have to
 *  compile and link with extra options.  For example, GCC requires
 *  linking with \c -rdynamic
 */
bool stack_trace();

/** Set the global value for stack_trace()
 */
void set_stack_trace(bool stack);

/// Generate a stack trace and print results into os
void generate_stack_trace(std::ostringstream &os);



/// a static buffer to assist in generating an exception message
extern std::string exception_message_buffer;

/// A base class that can throw an exception; used by handle_exception()
class ExceptionGenerator_base
{
public:
   virtual ~ExceptionGenerator_base() {}
   virtual void throw_it(const std::string& msg) const = 0;
};

/// A specialized derived class for throwing a specific exception
template<class ExceptionT>
class ExceptionGenerator : public ExceptionGenerator_base
{
public:
   virtual ~ExceptionGenerator() {}
   virtual void throw_it (const std::string& msg) const
   { throw ExceptionT(msg); }
};


/// Process an exception based on the exception manager mode.
/** Note: this function has side effects (the passed msg is written to
 *  and cleared).  As almost all use cases call for using the
 *  EXCEPTION_TEST and EXCEPTION_MNGR macros (which declare local
 *  ostringstream buffers), this is usually not an issue.  However, if
 *  you call this function directly... beware!
 */
void handle_exception( const ExceptionGenerator_base &exception, 
                       std::ostringstream& msg );


/// Helper class for "new" (gcc > 4.3) EXCEPTION_MNGR macro
class ExceptionMngr
{
protected: 
  ExceptionMngr(const char* file, int line)
     : msg("")
   { msg << file << ":" << line << ": "; }

   std::ostringstream  msg;

public: 
   template<typename RHS>
   std::ostream&
   operator<<(const RHS& rhs)
   { 
      msg << rhs;
      return msg;
   }

   std::ostream&
   operator<<(const char* rhs)
   {
      msg << rhs;
      return msg;
   }
};

template<typename TYPE>
class ExceptionMngr_instance : public ExceptionMngr
{
public:
   ExceptionMngr_instance(const char* file, int line)
      : ExceptionMngr(file, line)
   {}

   ~ExceptionMngr_instance()
   { handle_exception(ExceptionGenerator<TYPE>(), msg); }
};
 

} // namespace exception_mngr

} // namespace utilib



/** This macro is designed to test and handle an exception.
 *
 * @param  exception_test
 *               [in] Test for when an exception has occured.  This can and
 *               should be an expression that may mean something to the user.
 *               The text verbatim of this expression is included in the
 *               formed error string.
 * @param  Exception
 *               [in] This should be the name of an exception class.  The
 *               only requirement for this class is that it have a constructor
 *               that accepts a null terminated C string 
 *               (i.e. <tt>const char*</tt>).
 * @param  msg   [in] This is any expression that can be included in an
 *               output stream operation.  This is useful when building
 *               error messages on the fly.  Note that the code in this
 *               argument only gets evaluated if <tt>exception_test</tt>
 *               evaluates to <tt>true</tt> when an exception must be handled.
 *
 * This macro should be called within source code like a function, 
 * except that a semicolon should not be added after the macro.  For example,
 * suppose that in a piece of code in the file <tt>my_source_file.cpp</tt>
 * that the exception <tt>std::out_of_range</tt> is thrown if <tt>n > 100</tt>.
 * To use the macro, the source code would contain (at line 225
 * for instance):
 \verbatim

  EXCEPTION_TEST( n>100, std::out_of_range , "Error, n = " << n << is bad" );
 \endverbatim
 * When the program runs and with <tt>n = 125 > 100</tt> for instance,
 * the <tt>std::out_of_range</tt> exception would be thrown with the
 * error message:
 \verbatim

 /home/bob/project/src/my_source_file.cpp:225: n > 100: Error, n = 125 is bad
 \endverbatim
 *
 * Note that this macro calls the exception_mngr::handle_exception function,
 * so you can simply break in this function when using a debugger.  See
 * the \c set_mode function for a discussion of how to force exit or aborts
 * when an exception occurs (instead of throwing an exception).
 *
 * \note This macro is adapted from a similar technique developed by Roscoe
 * Bartlett for the rSQP++ software.
 */
#define EXCEPTION_TEST(TEST_, EXCEPTION_, MSG_)                         \
   if ( true ) {                                                        \
      const bool _test_val = (TEST_);                                   \
      if (_test_val) {                                                  \
         std::ostringstream _msg;                                       \
         _msg << __FILE__ << ":" << __LINE__ << ": "                    \
               << #TEST_ << ": " << MSG_;                               \
         utilib::exception_mngr::handle_exception                       \
            (utilib::exception_mngr::ExceptionGenerator<EXCEPTION_>(), _msg); \
      }                                                                 \
   } else static_cast<void>(0)
// NB: If compilers gripe about static_cast<void>(0), we can switch back
// to the "do { } while ( false )" form (which, while portable, may
// defeat some compiler's in-lining rules, see: 
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.5

/** This macro is designed to handle an exception.
 *
 * @param  Exception
 *               [in] This should be the name of an exception class.  The
 *               only requirement for this class is that it have a constructor
 *               that accepts a null terminated C string 
 *               (i.e. <tt>const char*</tt>).
 * @param  msg   [in] This is any expression that can be included in an
 *               output stream operation.  This is useful when building
 *               error messages on the fly.  Note that the code in this
 *               argument only gets evaluated if <tt>throw_exception_test</tt>
 *               evaluates to <tt>true</tt> when an exception is throw.
 *
 * This macro should be called within source code like a function, 
 * except that a semicolon should not be added after the macro.  For example,
 * suppose that in a piece of code in the file <tt>my_source_file.cpp</tt>
 * that the exception <tt>std::out_of_range</tt> is thrown if <tt>n > 100</tt>.
 * To use the macro, the source code would contain (at line 225
 * for instance):
 \verbatim

  if (n > 100)
    EXCEPTION_MNGR( std::out_of_range , "Error, n = " << n << is bad" );
 \endverbatim
 * When the program runs and with <tt>n = 125 > 100</tt> for instance,
 * the <tt>std::out_of_range</tt> exception would be thrown with the
 * error message:
 \verbatim

 /home/bob/project/src/my_source_file.cpp:225: Error, n = 125 is bad
 \endverbatim
 *
 * Note that this macro calls the exception_mngr::handle_exception function,
 * so you can simply break in this function when using a debugger.  See
 * the \c set_mode function for a discussion of how to force exit or aborts
 * when an exception occurs (instead of throwing an exception).
 */
#ifdef LEGACY_EXCEPTION_MANAGER
/** Deprecated form of the EXCEPTION_MNGR() macro used up until gcc
 *  4.3, where the compiler started complaining about ambiguous else
 *  statements.
 * 
 * WEH - this deprication is not in effect until we can get the prototype
 *  exception manager working on all platforms.
 */
#define EXCEPTION_MNGR(EXCEPTION_, MSG_)                                \
   if ( true ) {                                                        \
      std::ostringstream _omsg;                                         \
      _omsg << __FILE__ << ":" << __LINE__ << ": " << MSG_;             \
      utilib::exception_mngr::handle_exception                          \
         (utilib::exception_mngr::ExceptionGenerator<EXCEPTION_>(), _omsg); \
   } else static_cast<void>(0)
// NB: If compilers gripe about static_cast<void>(0), we can switch back
// to the "do { } while ( false )" form (which, while portable, may
// defeat some compiler's in-lining rules, see: 
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.5
#else
#define EXCEPTION_MNGR(TYPE_, MSG_)                           \
   utilib::exception_mngr::ExceptionMngr_instance<TYPE_>(__FILE__, __LINE__) \
      << MSG_
#endif

/// A macro that performs a catch for all standard exception types
#define STD_CATCH(cmd)\
  catch (const char* str) { std::cerr << "ERROR! Caught exception string: " << str << std::endl << std::flush; cmd ; }\
\
  catch (const std::bad_alloc& err) { std::cerr << "ERROR! Caught 'bad_alloc' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::bad_exception& err) { std::cerr << "ERROR! Caught 'bad_exception' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::bad_cast& err) { std::cerr << "ERROR! Caught 'bad_cast' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::bad_typeid& err) { std::cerr << "ERROR! Caught 'bad_typeid' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::ios_base::failure& err) { std::cerr << "ERROR! Caught 'ios_base::failure' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::length_error& err) { std::cerr << "ERROR! Caught 'length_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::domain_error& err) { std::cerr << "ERROR! Caught 'domain_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::out_of_range& err) { std::cerr << "ERROR! Caught 'out_of_range' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::invalid_argument& err) { std::cerr << "ERROR! Caught 'invalid_argument' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::logic_error& err) { std::cerr << "ERROR! Caught 'logic_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::range_error& err) { std::cerr << "ERROR! Caught 'range_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::overflow_error& err) { std::cerr << "ERROR! Caught 'overflow_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::underflow_error& err) { std::cerr << "ERROR! Caught 'underflow_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::runtime_error& err) { std::cerr << "ERROR! Caught 'runtime_error' exception: " << err.what() << std::endl << std::flush; cmd ; }\
\
  catch (const std::exception& str) { std::cerr << "ERROR! Caught other exception object: " << str.what() << std::endl << std::flush; cmd ; }


#endif
