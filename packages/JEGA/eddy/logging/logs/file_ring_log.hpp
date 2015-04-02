/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class file_ring_log.

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
 * \brief Contains the definition of the file_ring_log class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_BACKUP_LOG_HPP
#define EDDY_LOGGING_BACKUP_LOG_HPP




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
 * \brief A log which writes entries into a continuum or ring of ofstreams.
 *
 * In order for this log to write to a continuum of files, simply supply a
 * value (or use the default) of 0 for the max_files argument of the
 * constructor.  If that is the value, then no end to the sequence of files
 * and the effect is a ring of infinite diameter.
 *
 * In order for this log to write to a ring-of-files, supply a
 * non-zero value for the max_files in the constructor.  If you do so,
 * the backup will begin writing files numbered 0-(max_files-1) and
 * then begin again at 0 (overwriting the original file).
 *
 * The pattern of file names to write into is supplied at construction time.
 * The pattern should contain a # anywhere that the number of the file should
 * appear in a filename.  This requirement is asserted!
 *
 * The target file size in general will not be met precisely (unless the last
 * entry fills it up to exactly the desired size).  Generally, it will be
 * met to within some portion (not all) of the size of the last entry that
 * "takes it over the top".  It will not split an entry in order to
 * enforce the file size.  Presumably the entry size is miniscule next to
 * the overall file size and so this is no problem.
 *
 * Entries are only written if the stream is open and in good shape.
 *
 * The file stream is mutex protected and closed upon destruction of this
 * log object.
 *
 * \param RCBT a call back predicate that must have an overloaded ()
 *        operator taking the parameters (const my_type&, const string&, const
 *        string&).
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup log_types
 */
template <
          typename RCBT,
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS
          >
class EDDY_SL_IEDECL file_ring_log
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

        /// A shorthand for the file stream type for this file_ring_log
        typedef
        std::basic_ofstream<char_type, traits_type>
        ofstream_type;

        /// A shorthand for the string type for this file_ring_log
        typedef
        std::basic_string<char_type, traits_type>
        string_type;

        /// The type of a callback that can be called when a roll occurs.
        /**
         * The arguments passed will be this log, the name of the file
         * that just got closed, and the name of the file that just got
         * opened.
         */
        typedef
        RCBT
        roll_callback;

    protected:

        /// A list of callbacks to call when the file rolls.
        typedef
        std::list<roll_callback>
        roll_callback_list;

    private:

        /// A shorthand for the type of this log.
        typedef
        file_ring_log<char_type, traits_type>
        my_type;

        /**
         * \brief A shorthand for an ostringstream type that is compatible with
         *        our character type.
         */
        typedef
        std::basic_ostringstream<
            char_type,
            traits_type,
            std::allocator<char_type>
            >
        ostringstream_type;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The ostream into which to write the entries as they come.
        ofstream_type _fstream;

        /// The pattern of filenames for this backup log.
        string_type _fpat;

        /// The name of the current file being written to.
        string_type _fname;

        /// The number of file created so far in this ring.
        std::size_t _fcount;

        /// The maximum number of files to create before "ringing"
        std::size_t _max_fcount;

        /// The maximumn size in bytes of each file.
        /**
         * This number will be exceeded in nearly all cases by some portion
         * of the size of the last entry.  Therefore it should be considered
         * a target that will be nearly met for any reasonably sized entries.
         */
        std::size_t _max_bytes;

        /// The list of callbacks to call when the log rolls.
        roll_callback_list _roll_call_list;

        /// A mutex to protect roll callback list.
        EDDY_DECLARE_MUTABLE_MUTEX(_roll_list_mutex)

        /// A mutex to protect the entry stream.
        EDDY_DECLARE_MUTABLE_MUTEX(_fstream_mutex)

        /// A mutex to protect the name of the file.
        EDDY_DECLARE_MUTABLE_MUTEX(_fname_mutex)

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
         * \param e The entry to log to the file of this file ring log.
         */
        template <typename ET>
        void
        log(
            const ET& e
            )
        {
            // lock down the stream.
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_fstream_mutex)

            if(!this->_fstream) EDDY_LOGGING_THROW_EXCEPTION(
                logging_file_error(
                    "Attempt to write to bad stream in a file_ring_log"
                    )
                );

            if(!this->_fstream.is_open()) EDDY_LOGGING_THROW_EXCEPTION(
                logging_file_error(
                    "Attempt to write to closed stream in a file_ring_log"
                    )
                );

            // Write the entry first and then worry about the resulting
            // file size
            this->_fstream << e << '\n';

            // Compute the filesize by checking the end position and
            // multiplying by the number of bytes/position.
            // If our file size meets or exceeds our desired size, roll it.
            if((this->_fstream.tellp()*sizeof(char_type)) >=
                this->_max_bytes) roll_log();

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows access to the current name of the file of this log.
        /**
         * \return The name of the file currently being written to by this file
         *         ring log.
         */
        EDDY_NOTHREAD_INLINE
        const string_type&
        get_current_filename(
            ) const
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_fname_mutex)
            return this->_fname;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Adds a callback to the list of those called when the log rolls.
        /**
         * \param rc A callback to be called whenever this logger moves on to
         *           a new file.
         */
        EDDY_NOTHREAD_INLINE
        void
        add_roll_callback(
            const roll_callback& rc
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_roll_list_mutex)
            this->_roll_call_list.push_back(rc);
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

        /// Closes the current file and creates the next one.
        /**
         * THE STREAM IS EXPECTED TO BE LOCKED DOWN BY THE TIME
         * THIS METHOD GETS CALLED!! This method will not lock the
         * stream mutex but will lock the file name mutex.
         */
        void
        roll_log(
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l1, this->_fname_mutex)

            // prepare to supply the old name to callbacks.
            string_type old(this->_fname);
            this->_fname = this->create_filename(this->_fpat, this->_fcount++);

            // close the current file and open the new one.
            if(this->_fstream.is_open()) this->_fstream.close();
            this->_fstream.open(this->_fname.c_str());
            EDDY_LOGGING_THROW_EXCEPTION_IF(
                !this->_fstream.is_open(),
                logging_file_error(
                    "Unable to open file " + this->_fname +  " for logging."
                    );
                );

            // now close the ring if necessary.
            // note that _fcount can never be 0 here so a 0 value for
            // _max_fcount means ring of infinite diameter.
            // do it inside the name lock as a bit of a cheat to
            // use the mutex for more than one purpose.
            if(this->_fcount == this->_max_fcount) this->_fcount = 0;

            // now make any needed callbacks.
            this->execute_roll_callbacks(old);

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Replaces all #'s in fp with a string version of fnum.
        /**
         * This method simply creates an ostringstream into which it writes
         * the new name replacing all #'s with the file number \a fnum.
         *
         * \param fp The filename pattern in which to replace all #'s.
         * \param fnum The number to put in place of all #'s in \a fp.
         */
        static
        string_type
        create_filename(
            const string_type& fp,
            std::size_t fnum
            )
        {
            ostringstream_type fname;
            for(string_type::const_iterator i(fp.begin()); i!=fp.end(); ++i)
            {
                if(*i == '#') fname << fnum;
                else fname << *i;
            }
            return fname.str();
        }

        /// Calls all roll_callbacks.
        /**
         * The callbacks are issued using this class as the log, _fname as the
         * newly opened file name and the supplied oldname as the name of the
         * file that was just closed.
         *
         * The name of the file should already be locked down prior to
         * calling this function.
         *
         * \param oldname The previous filename being used which is the file
         *                that was just closed.
         */
        void
        execute_roll_callbacks(
            const string_type& oldname
            )
        {
            // The type of the iterator used in the for loop below.
            typedef typename roll_callback_list::const_iterator it_t;

            // lock down the list.  The filename should already be locked
            EDDY_LOGGING_TRY_SCOPEDLOCK(l1, this->_roll_list_mutex)

            // for speed, check for an empty roll call list.
            if(!this->_roll_call_list.empty())
            {
                // iterate the callbacks and issue them.
                for(it_t it(this->_roll_call_list.begin());
                    it!=this->_roll_call_list.end();
                    ++it) (*it)(*this, oldname, this->_fname);
            }

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a file_ring_log using the file fname.
        /**
         * The pattern must include at least 1 # which is the only character
         * respected as special by this class.  It may include as many #'s as
         * you like.  All occurrances will be replaced by the number of the
         * file currently being created.
         *
         * \param pattern The filename pattern on which to base the names of
         *                the files created by this file log.
         * \param max_bytes The maximum size in bytes of any files created.
         *                  Once a file reaches this size, the log rolls.
         * \param max_files The maximum number of files to write.  Once this
         *                  many files have been written, the log returns to
         *                  the beginning and begins overwriting the oldest
         *                  existing file.  A value of 0 means that there is
         *                  no limit to the number of files.
         */
        file_ring_log(
            const char_type* pattern,
            const std::size_t max_bytes,
            const std::size_t max_files = 0
            ) :
                EDDY_INIT_MUTEX(_roll_list_mutex, 1)
                EDDY_INIT_MUTEX(_fstream_mutex, 1)
                EDDY_INIT_MUTEX(_fname_mutex, 1)
                _fpat(pattern),
                _fname(),
                _fstream(),
                _max_bytes(max_bytes),
                _max_fcount(max_files),
                _roll_call_list(),
                _fcount(0)
        {
            assert(this->_fpat.find_first_of("#") != string_type::npos);
            roll_log();
        }

        /// Constructs a file_ring_log using the file fname.
        /**
         * See the documentation for the overload for more details.
         *
         * \param pattern The filename pattern on which to base the names of
         *                the files created by this file log.
         * \param max_bytes The maximum size in bytes of any files created.
         *                  Once a file reaches this size, the log rolls.
         * \param max_files The maximum number of files to write.  Once this
         *                  many files have been written, the log returns to
         *                  the beginning and begins overwriting the oldest
         *                  existing file.  A value of 0 means that there is
         *                  no limit to the number of files.
         */
        file_ring_log(
            const string_type& pattern,
            const std::size_t max_bytes,
            const std::size_t max_files = 0
            ) :
                EDDY_INIT_MUTEX(_roll_list_mutex, 1)
                EDDY_INIT_MUTEX(_fstream_mutex, 1)
                EDDY_INIT_MUTEX(_fname_mutex, 1)
                _fpat(pattern),
                _fname(),
                _fstream(),
                _max_bytes(max_bytes),
                _max_fcount(max_files),
                _roll_call_list(),
                _fcount(0)
        {
            assert(this->_fpat.find_first_of("#") != string_type::npos);
            roll_log();
        }

        /// Copy constructs a file_ring_log.
        /**
         * \param copy An existing file_log to copy properties from into this.
         */
        file_ring_log(
            const my_type& copy
            ) :
                EDDY_INIT_MUTEX(_roll_list_mutex, 1)
                EDDY_INIT_MUTEX(_fstream_mutex, 1)
                EDDY_INIT_MUTEX(_fname_mutex, 1)
                _fpat(copy._fpat),
                _fname(),
                _fstream(),
                _max_bytes(copy._max_bytes),
                _max_fcount(copy._max_fcount),
                _roll_call_list(),
                _fcount()
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, copy._fname_mutex)
            this->_fname = copy._fname;
            this->_fcount = copy._fcount;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

            EDDY_LOGGING_TRY_SCOPEDLOCK(l, copy._roll_list_mutex)
            this->_roll_call_list.assign(
                copy._roll_call_list.begin(), copy._roll_call_list.end()
                );
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

            this->_fstream.open(
                this->_fname.c_str(), ios_base::out|ios_base::app
                );

            EDDY_LOGGING_THROW_EXCEPTION_IF(
                !this->_fstream.is_open(),
                logging_file_error(
                    "Unable to open file " + this->_fname +  " for logging."
                    );
                );
        }

        /// Destructs a file_ring_log which closes the file stream.
        ~file_ring_log(
            )
        {
            if(this->_fstream.is_open()) this->_fstream.close();
        }


}; // class file_ring_log



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
#endif // EDDY_LOGGING_BACKUP_LOG_HPP
