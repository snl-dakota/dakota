/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class file_log.

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

        Sat Jan 29 16:55:52 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the file_log class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_FILE_LOG_HPP
#define EDDY_LOGGING_FILE_LOG_HPP




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
#include <fstream>
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
 * \brief A log which writes entries into an ofstream.
 *
 * The name of the file to write into is supplied at construction time.
 * The file is opened at that point and if it fails to open, a
 * logging_exception is thrown.
 *
 * Entries are only written if the stream is open and in good shape.
 *
 * The file stream is mutex protected and closed upon destruction of this
 * log object.
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
class EDDY_SL_IEDECL file_log
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A synonym for the character type of this file log.
        typedef
        CharT
        char_type;

        /// A synonym for the character traits type.
        typedef
        Traits
        traits_type;

        /// A shorthand for the file stream type for this file_log
        typedef
        std::basic_ofstream<char_type, traits_type>
        ofstream_type;

        /// A shorthand for the string type for this file_log
        typedef
        std::basic_string<char_type, traits_type>
        string_type;

    private:

        /// A shorthand for the type of this log.
        typedef
        file_log<char_type, traits_type>
        my_type;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The ofstream into which to write the entries as they come.
        ofstream_type _fstream;

        /// The name of the file created.
        /**
         * This is used for copy construction and for file size checking.
         */
        string_type _fname;

        /// A mutex to protect the entry stream.
        EDDY_DECLARE_MUTABLE_MUTEX(_fstream_mutex)

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Streams e into the current fstream of this log.
        /**
         * If the file stream is not in good condition, the logging does
         * not occur and an exception is thrown or an assertion fails
         * depending on whether or not exceptions are enabled.
         *
         * \param e The entry to log to the file of this file log.
         */
        template <typename ET>
        void
        log(
            const ET& e
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_fstream_mutex)

            if(!this->_fstream) EDDY_LOGGING_THROW_EXCEPTION(
                logging_file_error(
                    "Attempt to write to bad stream in a file_log"
                    )
                );

            if(!this->_fstream.is_open()) EDDY_LOGGING_THROW_EXCEPTION(
                logging_file_error(
                    "Attempt to write to closed stream in a file_log"
                    )
                );

            this->_fstream << e << std::endl;

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows immutable access to the name of the file of this logger
        /**
         * \return The name of the file currently being written to.
         */
        inline
        const string_type&
        get_filename(
            ) const
        {
            return this->_fname;
        }

        /// Allows modification of the name of the file being logged to.
        /**
         * If the fstream is currently open whether using the supplied name
         * or not, it is closed and re-opened using the supplied name.  The
         * supplied mode is used so that files may be appended, overwritten,
         * etc.
         *
         * \param fname The name of the new file to begin logging to.
         * \param mode The mode in which to open the new file.
         */
        void
        set_filename(
            const char_type* fname,
            std::ios_base::openmode mode=std::ios_base::out|std::ios_base::app
            )
        {
            if(this->_fstream.is_open()) this->_fstream.close();
            this->_fstream.open(fname, mode);
            this->_fname = fname;
            EDDY_LOGGING_THROW_EXCEPTION_IF(
                !this->_fstream.is_open(),
                logging_file_error(
                    "Unable to open file " + this->_fname +  " for logging."
                    );
                )
        }

        /// Allows modification of the name of the file being logged to.
        /**
         * See the documentation for the overload for more details.
         *
         * \param fname The name of the new file to begin logging to.
         * \param mode The mode in which to open the new file.
         */
        inline
        void
        set_filename(
            const string_type& fname,
            std::ios_base::openmode mode=std::ios_base::out|std::ios_base::app
            )
        {
            this->set_filename(fname.c_str(), mode);
        }

        inline
        void
        flush_stream(
            )
        {
            this->_fstream.flush();
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

        /// Constructs a file_log with no file initially open.
        file_log(
            ) :
                _fstream(),
                _fname() EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_fstream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /// Constructs a file_log using the file fname.
        /**
         * \param fname The name of the file to log into.
         * \param mode The mode in which to open the new file.
         */
        file_log(
            const string_type& fname,
            std::ios_base::openmode mode=std::ios_base::out|std::ios_base::app
            ) :
                _fstream(fname.c_str(), mode),
                _fname(fname) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_fstream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
            EDDY_LOGGING_THROW_EXCEPTION_IF(
                !this->_fstream.is_open(),
                logging_file_error(
                    "Unable to open file " + this->_fname +  " for logging."
                    );
                )
        }

        /// Constructs a file_log using the file fname.
        /**
         * \param fname The name of the file to log into.
         * \param mode The mode in which to open the new file.
         */
        file_log(
            const char_type* fname,
            std::ios_base::openmode mode=std::ios_base::out|std::ios_base::app
            ) :
                _fstream(fname, mode),
                _fname(fname) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_fstream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
            EDDY_LOGGING_THROW_EXCEPTION_IF(
                !this->_fstream.is_open(),
                logging_file_error(
                    "Unable to open file " + this->_fname + " for logging."
                    );
                )
        }

        /// Copy constructs a file_log.
        /**
         * \param copy An existing file_log to copy properties from into this.
         * \param mode The mode in which to open the new file.
         */
        file_log(
            const my_type& copy,
            std::ios_base::openmode mode=std::ios_base::out|std::ios_base::app
            ) :
                _fstream(copy._fname.c_str(), mode),
                _fname(copy._fname) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_fstream_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
            EDDY_LOGGING_THROW_EXCEPTION_IF(
                !this->_fstream.is_open(),
                logging_file_error(
                    "Unable to open file " + this->_fname + " for logging."
                    );
                )
        }

        /// Destructs a file_log which closes the file stream.
        ~file_log(
            )
        {
            if(this->_fstream.is_open()) this->_fstream.close();
        }


}; // class file_log



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
#endif // EDDY_LOGGING_FILE_LOG_HPP
