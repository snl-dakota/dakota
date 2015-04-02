/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class list_log.

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

        Sat Jan 29 15:32:16 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the list_log class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_LIST_LOG_HPP
#define EDDY_LOGGING_LIST_LOG_HPP




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
 * \brief A log that stores all entries in a std::list.
 *
 * The entries are stored in a simple list which is available for inspection
 * using a get method.
 *
 * All entries must be derivative of the ostream_entry class and will be
 * stored as such.
 *
 * \param LET The type of the entries that can be pushed into the list.
 * \param ListT The type of the container used to store entries.  This type
 *              should be a 1st class container and must support forward
 *              iteration via begin and end methods, insert(iterator, value),
 *              and clear.  Examples of suitable containers include std::list
 *              (default), std::vector, std::set, std::map (if you're clever),
 *              and others.
 *
 * \ingroup log_types
 */
template <
    typename LET = std::string<
        EDDY_LOGGING_DEF_CHAR_TYPE,
        EDDY_LOGGING_DEF_CHAR_TRAITS
        >,
    typename ListT = std::list<const LET>
    >
class EDDY_SL_IEDECL list_log
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A shorthand for the entry type stored by this list log.
        typedef
        LET
        entry_type;

        /// A list for entry's.
        typedef
        ListT
        entry_list;

    private:

        /// A shorthand for the type of this log.
        typedef
        list_log<entry_type, entry_list>
        my_type;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The list of all entries logged so far in this logger.
        entry_list _entries;

        /// A mutex to protect the _entries list.
        EDDY_DECLARE_MUTABLE_MUTEX(_entries_mutex)


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Adds e to the list of entries stored by this log.
        /**
         * \param e The entry to log into this log.
         */
        template <typename ET>
        EDDY_NOTHREAD_INLINE
        void
        log(
            const ET& e
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_entries_mutex)
            this->_entries.insert(this->_entries.end(), e);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows immutable access to the list of all entries in this log.
        /**
         * \return The list of all entries logged so far.
         */
        inline
        const entry_list&
        get_all_entries(
            ) const
        {
            return this->_entries;
        }

        /// Allows a user to lock the log to prevent concurrent modification.
        /**
         * This method is intended for use for example when one wishes to use
         * the \a get_all_entries method to inspect the log contents.  If that
         * is the case (and in a multi-threaded environment), it is necessary to
         * prevent others from modifying the contents concurrently.
         *
         * Use of this method should be subsequently paired with use of the
         * \a unlock method.
         */
        inline
        void
        lock(
            )
        {
            EDDY_IF_THREADSAFE(this->_entries_mutex.lock();)
        }

        /// Allows a user to unlock the log to allow other users to gain access.
        /**
         * This method is intended for use for example when one wishes to use
         * the \a get_all_entries method to inspect the log contents.  If that
         * is the case (and in a multi-threaded environment), it is necessary to
         * prevent others from modifying the contents concurrently.
         *
         * Use of this method should be paired with a previous use of the
         * \a lock method.
         */
        inline
        void
        unlock(
            )
        {
            EDDY_IF_THREADSAFE(this->_entries_mutex.unlock();)
        }

        /**
         * \brief Returns a new list composed of copies of the entries that
         *        satisfy the supplied predicate.
         *
         * \param pred A predicate against which to test the entries of this
         *             log.
         * \return A newly created list containing all entries that satisfy the
         *         supplied predicate.
         */
        template <typename Pred>
        entry_list
        get_entries(
            Pred pred
            ) const
        {
            // prepare a list to return.
            entry_list ret;

            // iterate the entries and copy all those that pass the
            // predicate test.  We must lock the list before we do this.
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_entries_mutex)

            for(typename entry_list::const_iterator i(this->_entries.begin());
                i!=this->_entries.end(); ++i)
                    if(pred(*i)) ret.insert(ret.end(), *i);

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

            // now return our resulting list.
            return ret;
        }

        /**
         * \brief Returns a count of the stored entries that match pred.
         *
         * \param pred A predicate against which to test the entries of this
         *             log.
         * \return A count of the number of entries that satisfy the supplied
         *         predicate.
         */
        template <typename Pred>
        EDDY_NOTHREAD_INLINE
        std::size_t
        count_entries(
            Pred pred
            ) const
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(l, this->_entries_mutex)
            return static_cast<std::size_t>(
                std::count_if(
                    this->_entries.begin(), this->_entries.end(), pred
                    )
                );
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Adds all entries of other into this log.
        /**
         * If other is this, nothing happens.
         *
         * \param other An existing list_log from which to copy entries into
         *              this.
         */
        EDDY_NOTHREAD_INLINE
        void
        copy_in(
            const my_type& other
            )
        {
            // lock down the other list.
            EDDY_LOGGING_TRY_SCOPEDLOCK(l1, other._entries_mutex)
            this->copy_in(other._entries);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Adds all entries of the supplied list into this log.
        /**
         * If the supplied list is the list of this log, nothing happens.
         *
         * This method is useful if you wish for example to copy only those
         * entries of another log that match a certain prediate into this log.
         * Consider the following line of code.
         * \code
            llog1.copy_in(llog2.get_entries(some_pred()));
           \endcode
         *
         * \param olist A list of entries to copy into this log.
         */
        inline
        void
        copy_in(
            const entry_list& olist
            )
        {
            // don't duplicate this log into itself.
            if(&olist == &this->_entries) return;
            this->copy_in(olist.begin(), olist.end());
        }

        /// Adds all entries of the supplied container into this log.
        /**
         * This method is useful if you wish for to copy all entries of some
         * other container into this.  The supplied container type (ContT) must
         * support forward iteration and have the methods begin and end that
         * return such iterators.
         *
         * \param other A collection of entries to copy into this log.
         */
        template <typename ContT>
        inline
        void
        copy_in(
            const ContT& other
            )
        {
            this->copy_in(other.begin(), other.end());
        }

        /// Adds all entries of the range [begin, end) into this log.
        /**
         * This method is useful if you wish for to copy all entries in some
         * range into this.  The supplied iterator type (FwdIt) must
         * support forward iteration via preincrement (operator ++()).
         *
         * \param begin An iterator to the first entry in the range to add into
         *              this container.
         * \param end An iterator to the entry 1 past the last entry in the
         *            range to add into this container.
         */
        template <typename FwdIt>
        void
        copy_in(
            FwdIt begin,
            const FwdIt& end
            )
        {
            // lock down this list.
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_entries_mutex)

            // now insert all entries from the other into this.
            for(; begin != end; ++begin)
                this->_entries.insert(this->_entries.end(), *begin);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows a user to clear the current contents of this log.
        /**
         * After this call, the log list will be empty.
         */
        inline
        void
        clear(
            )
        {
            // lock down this list.
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_entries_mutex)
            this->_entries.clear();
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

        /// Default constructs a list_log of entries.
        list_log(
            ) :
                _entries()
        {
        }

        /// Default constructs a list_log of entries.
        /**
         * \param copy An existing appending_log to copy properties from into
         *             this.
         */
        list_log(
            const my_type& copy
            ) :
                _entries()
        {
            this->copy_in(copy);
        }



}; // class list_log


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
#endif // EDDY_LOGGING_LIST_LOG_HPP
