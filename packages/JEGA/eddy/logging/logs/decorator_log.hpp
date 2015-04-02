/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class decorator_log.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Feb 06 15:27:27 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the decorator_log class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_DECORATOR_LOG_HPP
#define EDDY_LOGGING_DECORATOR_LOG_HPP




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
#include "../include/config.hpp"
#include "../include/default_types.hpp"







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
Class Definition
================================================================================
*/
/**
 * \brief A log to wrap two other pre-existing logs and serve as a decorator.
 *
 * This log will log to 2 other underlying logs.  The underlyers may
 * themselves be decorators.
 *
 * The underlyers must be pre-existing and will be known to this decorator by
 * reference.
 *
 * \param LogT1 The type of the first decorated log.
 * \param LogT2 The type of the second decorated log.
 *
 * \ingroup log_types
 */
template <
          typename LogT1,
          typename LogT2
         >
class EDDY_SL_IEDECL decorator_log
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the first decorated log.
        typedef
        LogT1
        first_log_type;

        /// The type of the second decorated log.
        typedef
        LogT2
        second_log_type;

    private:

        /// A shorthand for the type of this log.
        typedef
        decorator_log<first_log_type, second_log_type>
        my_type;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The first underlying log that this decorator uses.
        first_log_type& _log1;

        /// The second underlying log that this decorator uses.
        second_log_type& _log2;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Logs e to the two decorated logs.
         *
         * \param e the entry to send to each of the two decorated logs.
         */
        template <typename ET>
        void
        log(
            const ET& e
            )
        {
            // just pass the logging on to the underlyers.
            this->_log1.log(e);
            this->_log2.log(e);
        }

        /// Allows mutable access to the first decorated log.
        /**
         * \return A mutable reference to the log known as the first decorated
         *         log.
         */
        inline
        first_log_type&
        get_first_log(
            )
        {
            return this->_log1;
        }

        /// Allows immutable access to the first decorated log.
        /**
         * \return An immutable reference to the log known as the first
         *         decorated log.
         */
        inline
        const first_log_type&
        get_first_log(
            ) const
        {
            return this->_log1;
        }

        /// Allows mutable access to the second decorated log.
        /**
         * \return A mutable reference to the log known as the second decorated
         *         log.
         */
        inline
        second_log_type&
        get_second_log(
            )
        {
            return this->_log2;
        }

        /// Allows immutable access to the second decorated log.
        /**
         * \return An immutable reference to the log known as the second
         *         decorated log.
         */
        inline
        const second_log_type&
        get_second_log(
            ) const
        {
            return this->_log2;
        }

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:




    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a decorator log to decorate log1 and log2.
        /**
         * \param log1 The instance of the first log type to which to log.
         * \param log2 The instance of the second log type to which to log.
         */
        decorator_log(
            first_log_type& log1,
            second_log_type& log2
            ) :
                _log1(log1),
                _log2(log2)
        {
        }

        /// Copy constructs a decorator log.
        /**
         * \param copy An existing decorator_log to copy properties from into
         *             this.
         */
        decorator_log(
            const my_type& copy
            ) :
                _log1(copy._log1),
                _log2(copy._log2)
        {
        }

}; // class decorator_log



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace logging
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.


/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif




/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_DECORATOR_LOG_HPP
