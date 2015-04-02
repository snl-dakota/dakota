/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class ostream_log.

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

        Sat Jan 29 15:55:21 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ostream_log class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_OSTREAM_LOG_HPP
#define EDDY_LOGGING_OSTREAM_LOG_HPP





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
#include <ostream>
#include "../include/config.hpp"
#include "../detail/threadsafe.hpp"
#include "../include/exceptions.hpp"
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
 * \brief A log to write to a single ostream.
 *
 * This log writes to an ostream object.  It mutex protects the stream if
 * EDDY_LOGGING_THREADSAFE is defined.
 *
 * In addition, it allows access to the stream via a couple of get methods.
 *
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup log_types
 */
template <
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS
         >
class EDDY_SL_IEDECL ostream_log
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A synonym for the character type of this ostream log.
        typedef
        CharT
        char_type;

        /// A synonym for the character traits type of this ostream log.
        typedef
        Traits
        traits_type;

        /// A shorthand for the basic output stream type.
        typedef
        std::basic_ostream<char_type, traits_type>
        ostream_type;

    protected:


    private:

        /// A shorthand for the type of this log.
        typedef
        ostream_log<char_type, traits_type>
        my_type;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:


        /// The ostream into which to write the entries as they come.
        ostream_type* _stream;

        /// A mutex to protect the entry stream.
        EDDY_DECLARE_MUTABLE_MUTEX(_stream_mutex)


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Prints e to the stream of this log.
        /**
         * ET must support stream insertion.
         *
         * \param e The entry to insert into the stream of this log.
         */
        template <typename ET>
        void
        log(
            const ET& e
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_stream_mutex)

            if(this->_stream == 0x0) EDDY_LOGGING_THROW_EXCEPTION(
                logging_ostream_error(
                    "Attempt to write to null stream in an ostream_log"
                    )
                );

            if(!(*this->_stream)) EDDY_LOGGING_THROW_EXCEPTION(
                logging_ostream_error(
                    "Attempt to write " + static_cast<std::string>(e) +
                    " to bad stream in an ostream_log"
                    )
                );

            (*this->_stream) << e << '\n';

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows mutable access to the stream of this logger.
        /**
         * \return The stream currently being used by this log.
         */
        EDDY_NOTHREAD_INLINE
        ostream_type*
        get_stream(
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_stream_mutex)
            return this->_stream;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows immutable access to the stream of this logger.
        /**
         * \return The stream currently being used by this log.
         */
        EDDY_NOTHREAD_INLINE
        const ostream_type*
        get_stream(
            ) const
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_stream_mutex)
            return this->_stream;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows modification of the ostream used by this log.
        /**
         * \param stream The new stream for this log to write into.
         */
        inline
        void
        set_stream(
            ostream_type& stream
            )
        {
            this->set_stream(&stream);
        }

        /// Allows the stream to be nullified.
        inline
        void
        nullify_stream(
            )
        {
            this->set_stream(0x0);
        }

        inline
        void
        flush_stream(
            )
        {
            this->_stream->flush();
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


        /// Allows modification of the ostream used by this log.
        /**
         * \param stream The new stream for this log to write into.
         */
        EDDY_NOTHREAD_INLINE
        void
        set_stream(
            ostream_type* stream
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_stream_mutex)
            this->_stream = stream;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs an ostream_log using stream.
        ostream_log(
            ) :
                _stream(0x0) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_stream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /// Constructs an ostream_log using stream.
        /**
         * \param stream The stream for this log to write into.
         */
        ostream_log(
            ostream_type& stream
            ) :
                _stream(&stream) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_stream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /// Copy constructs an ostream_log.
        /**
         * \param copy An existing ostream_log to copy properties from into
         *             this.
         */
        ostream_log(
            const my_type& copy
            ) :
                _stream(copy.get_stream()) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_stream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }


}; // class ostream_log



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
#endif // EDDY_LOGGING_ON

/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_OSTREAM_LOG_HPP
