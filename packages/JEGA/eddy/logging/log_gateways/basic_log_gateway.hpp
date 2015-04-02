/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class basic_log_gateway.

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

        Mon Jan 24 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the basic_log_gateway class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_BASIC_LOG_MANAGER_HPP
#define EDDY_LOGGING_BASIC_LOG_MANAGER_HPP


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
#include <string>
#include "../include/config.hpp"
#include "../detail/threadsafe.hpp"
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
 * \brief A log gateway that passes everything on to a log.
 *
 * This class logs entries to a known log without any restrictions.  The
 * purpose of this class is to provide thread protection to a known log
 * and otherwise serve as something of a completely transparent log gateway.
 *
 * This gateway accepts the type of the underlying log as a template parameter.
 * It keeps a reference to a log of that type, the instance for which must
 * be supplied at construction time.  In that way, many gateways may be
 * attached to the same log at the same time but this gateway can never
 * be attached to another log.
 *
 * \param LogT The type of the log to which this gateway forwards entries.
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup log_gateway_types
 */
template <
          typename LogT,
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS
         >
class EDDY_SL_IEDECL basic_log_gateway
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the log to which log entries must be sent.
        typedef
        LogT
        log_type;

        /// A synonym for the character type of this log gateway.
        typedef
        CharT
        char_type;

        /// A synonym for the character traits type.
        typedef
        Traits
        traits_type;

    private:

        /// A shorthand for this type
        typedef
        basic_log_gateway<log_type, char_type, traits_type>
        my_type;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The log to which entries are sent.
        log_type& _log;

        /// A mutex to protect the log.
        EDDY_DECLARE_MUTABLE_MUTEX(_log_mutex)


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Logs e to the known log no matter what and returns true.
        /**
         * \param e The entry to be logged to the known log.
         * \return Always true indicating that the entry was passed to the log.
         */
        template <typename ET>
        inline
        bool
        log(
            const ET& e
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_log_mutex)
            this->_log.log(e);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
            return true;
        }

        /// Allows mutable access to the underlying log for this logger.
        /**
         * \return A reference to the log known by this gateway.
         */
        inline
        log_type&
        get_log(
            )
        {
            return this->_log;
        }

        /// Allows immutable access to the underlying log for this logger.
        /**
         * \return A constant reference to the log known by this gateway.
         */
        inline
        const log_type&
        get_log(
            ) const
        {
            return this->_log;
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

        /**
         * \brief Constructs a log gateway to manage log using the supplied
         *        level as the default.
         *
         * \param log The log to associate with this gateway.
         */
        basic_log_gateway(
            log_type& log
            ) :
                _log(log) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_log_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /// Copy constructs a basic_log_gateway.
        /**
         * \param copy The gateway to copy the properties from in this new
         *             gateway object.
         */
        basic_log_gateway(
            const my_type& copy
            ) :
                _log(copy._log) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_log_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }

}; // class basic_log_gateway





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace eddy
} // namespace logging






/*
================================================================================
Include Inlined Functions File
================================================================================
*/




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
#endif // EDDY_LOGGING_BASIC_LOG_MANAGER_HPP
