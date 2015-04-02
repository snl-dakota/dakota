/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of logging project exceptions.

    NOTES:



    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sat Jan 29 16:59:28 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains definitions of logging project exceptions.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_EXCEPTIONS_HPP
#define EDDY_LOGGING_EXCEPTIONS_HPP



/*
================================================================================
Conditional Inclusion
================================================================================
*/
#ifdef EDDY_LOGGING_ON


/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"

#ifdef EDDY_LOGGING_NO_EXCEPTIONS
#   include <cassert>
#else
#   include <stdexcept>


/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/








/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace logging {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/




/*
================================================================================
In-Namespace Exceptions
================================================================================
*/
/// A basic error class for logging exceptions.
class EDDY_SL_IEDECL logging_exception :
    public std::runtime_error
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a logging_exception exception.
        logging_exception(
            const std::string& msg = "no message supplied."
            ) :
                std::runtime_error(msg)
        { }

        /// Copy constructs a logging_exception exception.
        /**
         * \param copy The exception to copy.
         */
        logging_exception(
            const logging_exception& copy
            ) :
                std::runtime_error(copy.what())
        { }

        /// Destructs a logging_exception exception.
        virtual
        ~logging_exception(
            ) throw()
        { }

}; // class logging_exception

/// An exception to indicate that a file handling error has occurred.
class EDDY_SL_IEDECL logging_file_error :
    public logging_exception
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a logging_file_error exception.
        logging_file_error(
            const std::string& msg = "no message supplied."
            ) :
                logging_exception(msg)
        { }

        /// Copy constructs a logging_file_error exception.
        /**
         * \param copy The exception to copy.
         */
        logging_file_error(
            const logging_file_error& copy
            ) :
                logging_exception(copy)
        { }

        /// Destructs a logging_file_error exception.
        virtual
        ~logging_file_error(
            ) throw()
        { }

}; // class logging_file_error


/// An exception to indicate that an error has occurred while using an ostream.
class EDDY_SL_IEDECL logging_ostream_error :
    public logging_exception
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a logging_ostream_error exception.
        logging_ostream_error(
            const std::string& msg = "no message supplied."
            ) :
                logging_exception(msg)
        { }

        /// Copy constructs a logging_ostream_error exception.
        /**
         * \param copy The exception to copy.
         */
        logging_ostream_error(
            const logging_ostream_error& copy
            ) :
                logging_exception(copy)
        { }

        /// Destructs a logging_ostream_error exception.
        virtual
        ~logging_ostream_error(
            ) throw()
        { }

}; // class logging_ostream_error

/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace logging
} // namespace eddy


#endif // #else of #ifdef EDDY_LOGGING_NO_EXCEPTIONS

/*
================================================================================
Macros
================================================================================
*/

#ifdef EDDY_LOGGING_NO_EXCEPTIONS

/**
 * \brief A macro to either throw an exception if allowed or cause an assertion
 *        failure.
 *
 * If EDDY_LOGGING_NO_EXCEPTIONS is defined, this macro will expand to
 * \code
    assert(false)
   \endcode
 * Otherwise, it will expand to:
 * \code
    throw e
   \endcode
 *
 * \param e The exception declaration statement to be thrown.
 *
 * \ingroup dev_macros
 */
#define EDDY_LOGGING_THROW_EXCEPTION(e) assert(false)

/// A macro to either throw an exception if allowed or assert a condition.
/**
 * If EDDY_LOGGING_NO_EXCEPTIONS is defined, this macro will expand to
 * \code
    assert(!(cond))
   \endcode
 * Otherwise, it will expand to:
 * \code
    if(cond) throw e
   \endcode
 *
 * \param cond The condition on which to base the throwing of the exception.
 * \param e The exception declaration statement to be thrown.
 *
 * \ingroup dev_macros
 */
#define EDDY_LOGGING_THROW_EXCEPTION_IF(cond, e) assert(!(cond))

#else // #ifndef EDDY_LOGGING_NO_EXCEPTIONS

/**
 * \brief A macro to either throw an exception if allowed or cause an assertion
 *        failure.
 *
 * If EDDY_LOGGING_NO_EXCEPTIONS is defined, this macro will expand to
 * \code
    assert(false)
   \endcode
 * Otherwise, it will expand to:
 * \code
    throw e
   \endcode
 *
 * \param e The exception declaration statement to be thrown.
 *
 * \ingroup dev_macros
 */
#define EDDY_LOGGING_THROW_EXCEPTION(e) throw e;

/// A macro to either throw an exception if allowed or assert a condition.
/**
 * If EDDY_LOGGING_NO_EXCEPTIONS is defined, this macro will expand to
 * \code
    assert(!(cond))
   \endcode
 * Otherwise, it will expand to:
 * \code
    if(cond) throw e
   \endcode
 *
 * \param cond The condition on which to base the throwing of the exception.
 * \param e The exception declaration statement to be thrown.
 *
 * \ingroup dev_macros
 */
#define EDDY_LOGGING_THROW_EXCEPTION_IF(cond, e)                            \
    { if(cond) EDDY_LOGGING_THROW_EXCEPTION(e); }

#endif

/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif // EDDY_LOGGING_ON



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_EXCEPTIONS_HPP
