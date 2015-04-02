/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class appending_log.

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
 * \brief Contains the definition of the appending_log class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_APPENDING_LOG_HPP
#define EDDY_LOGGING_APPENDING_LOG_HPP



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
#include <list>
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
 * \brief A log to wrap another log and add appenders.
 *
 * This log will log to an underlying log created and supplied to it at
 * construction time and forward any compliant entries on to any supplied
 * appenders.
 *
 * \param LogT The type of log that underlies this appending log.
 * \param CBArgT The type of arguments that may be passed to appenders.
 *
 * \ingroup log_types
 */
template <
          typename LogT,
          typename APPArgT = std::basic_string<
                                            EDDY_LOGGING_DEF_CHAR_TYPE,
                                            EDDY_LOGGING_DEF_CHAR_TRAITS
                                            >
         >
class EDDY_SL_IEDECL appending_log
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the underlying log to which this logs.
        typedef
        LogT
        log_type;

        /// A shorthand for the callbacker type of this class.
        typedef
        APPArgT
        appender_arg_type;

    private:

        /// A shorthand for the type of this class.
        typedef
        appending_log<log_type, appender_arg_type>
        my_type;

    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    public:

        /**
         * \brief A required base class for any class that wishes to receive
         *        the information sent to this log.
         *
         * All subclasses must override the single operator() overload of this
         * base class.
         */
        class appender
        {
            /*
            ====================================================================
            Subclass Overridable Methods
            ====================================================================
            */
            public:

                /// The activation method of this appender.
                /**
                 * All derived classes must implement this method.
                 *
                 * \param arg The argument that will be passed when this
                 *            appender is invoked.
                 */
                virtual
                void
                operator () (
                    const appender_arg_type& arg
                    ) = 0;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Destructs this appender object.
                virtual
                ~appender(
                    ) {}

        }; // class appender


    /*
    ============================================================================
    Class Scope Typedefs Continued
    ============================================================================
    */
    protected:

        /// A shorthand for the type of the list of appenders.
        typedef
        std::list<appender*>
        appender_list;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The underlying log that this appending log uses.
        log_type& _log;

        /// The list of all appenders for entries.
        appender_list _appenders;

        /// A mutex to protect the entry stream.
        EDDY_DECLARE_MUTABLE_MUTEX(_appender_mutex)


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Logs e with the underlying log type and then forwards it to
         *        all appenders.
         *
         * \param e The entry to log and send to all appenders.
         */
        template <typename ET>
        void
        log(
            const ET& e
            )
        {
            // create a shorthand for the iterator of an appender list.
            typedef typename appender_list::iterator AL_IT;

            // First, log it with the underlyer.
            this->_log.log(e);

            // now append it to all appenders if appropriate.
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_appender_mutex)
            if(!this->_appenders.empty())
                for(AL_IT it(this->_appenders.begin());
                    it!=this->_appenders.end(); ++it)
                        (*it)->operator()(e);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Adds an appender to the list of appenders.
        /**
         * Appender pointers are copied into the appender list.  This class
         * does not assume ownership of these objects, just usage.  Therefore
         * it will not destroy them...ever.  The user is responsible for
         * controlling the lifespan of these objects and if one is to be
         * destroyed before this gateway, the user should call the
         * remove_appender method to ensure that this gateway does not
         * attempt to use it in the future.
         *
         * In order to accomplish this, a reference to this log could
         * be stored in your appender and the appender could thus remove
         * itself in its destructor.
         *
         * Appenders can be added multiple times and will receive as many
         * calls to their operator() as there are entries for them.
         *
         * \param app The appender to add to the list of appenders.
         */
        EDDY_NOTHREAD_INLINE
        void
        add_appender(
            appender* app
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_appender_mutex)
            this->_appenders.push_back(app);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Removes app from the list of appenders.
        /**
         * This will remove all occurances of the appender.
         *
         * \param app The appender to remove from the list of appenders.
         */
        EDDY_NOTHREAD_INLINE
        void
        remove_appender(
            appender* app
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_appender_mutex)
            this->_appenders.remove(app);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
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

        /// Constructs an appending log to log to log.
        /**
         * \param log An instance of the wrapped log to which to log entries.
         */
        appending_log(
            log_type& log
            ) :
                _log(log),
                EDDY_INIT_MUTEX(_appender_mutex, 1)
                _appenders()
        {
        }

        /// Copy constructs an appending log.
        /**
         * \param copy An existing appending_log to copy properties from into
         *             this.
         */
        appending_log(
            const my_type& copy
            ) :
                _log(copy._log),
                EDDY_INIT_MUTEX(_appender_mutex, 1)
                _appenders(copy._appenders)
        {
        }




}; // class appending_log



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
#endif // EDDY_LOGGING_APPENDING_LOG_HPP
