/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class ts_list.

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

        Tue Mar 02 20:42:09 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ts_list class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_TS_LIST_HPP
#define EDDY_THREADS_TS_LIST_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <list>
#include "mutex_lock.hpp"
#include "scoped_lock_concept.hpp"





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
    namespace threads {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
template<
            typename ValueType,
            typename AllocType
        >
class ts_list;





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
/// A list class implementation with mutex protection.
/**
 * All methods lock for their duration.  To lock over multiple method
 * calls, use the lock and unlock methods or the scoped_lock.
 *
 * A note on iterating this container.  To do so, you should lock and unlock
 * or use the scoped_lock.  There is no iterator level mutex protection.
 * The begin and end methods will lock for their duration but to be sure
 * that it is not modified by another thread for the entire time you are
 * iterating it, lock, and when you are done, unlock.
 *
 * Certain operations, such as assignment, are defined for both a
 * right-hand-side of this type as well as for one of type
 * std::list<ValueType, AllocType>
 *
 * \param ValueType The type of the items stored in this list.
 * \param AllocType The type of the allocator used to create space for new
 *                  items in this list.
 */
template<
            typename ValueType,
            typename AllocType = std::allocator<ValueType>
        >
class EDDY_SL_IEDECL ts_list
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /// The type of the allocator this list is using.
        typedef
        typename std::list<ValueType, AllocType>::allocator_type
        allocator_type;

        /// A constant iterator type for this list.
        typedef
        typename std::list<ValueType, AllocType>::const_iterator
        const_iterator;

        /// A type to serve as a constant pointer to an element in the list.
        typedef
        typename std::list<ValueType, AllocType>::const_pointer
        const_pointer;

        /// A type to serve as a constant-iterator for the list.
        typedef
        typename std::list<ValueType, AllocType>::const_reference
        const_reference;

        /// A type to serve as a constant reverse iterator for the list.
        typedef
        typename std::list<ValueType, AllocType>::const_reverse_iterator
        const_reverse_iterator;

        /// A shorthand for the underlying list type.
        typedef
        std::list<ValueType, AllocType>
        container_type;

        /**
         * \brief A type to represent the difference between the address of two
         *        elements.
         */
        typedef
        typename std::list<ValueType, AllocType>::difference_type
        difference_type;

        /// An iterator type for this list.
        typedef
        typename std::list<ValueType, AllocType>::iterator
        iterator;

        /// A shorthand for this type.
        typedef
        ts_list<ValueType, AllocType>
        my_type;

        /// The type of the scoped lock that can be used with this container.
        typedef
        scoped_lock_concept<my_type>
        scoped_lock;

        /// A type to serve as a non-const pointer to data.
        typedef
        typename std::list<ValueType, AllocType>::pointer
        pointer;

        /// A type to serve as a reference to an element in the list.
        typedef
        typename std::list<ValueType, AllocType>::reference
        reference;

        /// A type to serve as a non-constant reverse iterator for the list.
        typedef
        typename std::list<ValueType, AllocType>::reverse_iterator
        reverse_iterator;

        /// A type for keeping the size of this list.
        typedef
        typename std::list<ValueType, AllocType>::size_type
        size_type;

        /// The type being stored in this list.
        typedef
        typename std::list<ValueType, AllocType>::value_type
        value_type;


    protected:

    private:

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The underlying un-protected list for this list.
        container_type _container;

        /// The mutex used to protect this list.
        mutable mutex _mutex;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Locks this list until an unlock is issued.
        /**
         * All methods lock the list for their duration. Use this method if you
         * need to lock the list over multiple actions.
         */
        void
        lock(
            ) const;

        /// Unlocks this list.
        /**
         * Only use this method to counter a lock call.
         */
        void
        unlock(
            ) const;

        /// Assignment operator.  Assigns this to \a right.
        /**
         * \param right The existing ts_list to assign items from into this.
         * \return This thread safe list after assignment is complete.
         */
        const my_type&
        operator =(
            const my_type& right
            );

        /// Assigns this's container to \a right's container.
        /**
         * \param right The existing non-threadsafe list to assign items from
         *               into this.
         * \return This thread safe list after assignment is complete.
         */
        const my_type&
        operator =(
            const container_type& right
            );

        /**
         * \brief Returns an iterator to the first element in this list
         *        or end() if empty().
         *
         * \return Iterator to the first element in the container.
         */
        inline
        iterator
        begin(
            );

        /**
         * \brief Returns a const_iterator to the first element in this list
         *        or end() if empty().
         *
         * \return Iterator to the first element in the container.
         */
        inline
        const_iterator
        begin(
            ) const;

        /// Returns an iterator to one past the last element in this list.
        /**
         * \return Iterator to 1 past the last element in this list.
         */
        inline
        iterator
        end(
            );

        /// Returns a const_iterator to one past the last element in this list.
        /**
         * \return Iterator to 1 past the last element in this list.
         */
        inline
        const_iterator
        end(
            ) const;

        /**
         * \brief Returns a reverse_iterator to the first element in this list
         *        reversed or rend() if empty().
         *
         * \return Iterator to the first element in the reverse sequence of
         *         this container.
         */
        inline
        reverse_iterator
        rbegin(
            );

        /**
         * \brief Returns a const_reverse_iterator to the first element in this
         *        list reversed or rend() if empty().
         *
         * \return Iterator to the first element in the reverse sequence of
         *         this container.
         */
        inline
        const_reverse_iterator
        rbegin(
            ) const;

        /**
         * \brief Returns a reverse_iterator to one past the last element in
         *        this list reversed.
         *
         * \return Iterator to 1 past the last element in the reverse sequence
         *         of this container.
         */
        inline
        reverse_iterator
        rend(
            );

        /**
         * \brief Returns a const_reverse_iterator to one past the last element
         *        in this list reversed.
         *
         * \return Iterator to 1 past the last element in the reverse sequence
         *         of this container.
         */
        inline
        const_reverse_iterator
        rend(
            ) const;

        /// Makes the size of this list exactly \a newsze.
        /**
         * If elements are added, they have default values.  Elements may be
         * erased.
         *
         * \param newsze the new size for the list.
         */
        inline
        void
        resize(
            size_type newsze
            );

        /// Makes the size of this list exactly \a newsze.
        /**
         * If elements are added, they have the value \a val.  Elements
         * may be erased.
         *
         * \param newsze the new size for the list.
         * \param val The values to use for any new items added to this list.
         */
        inline
        void
        resize(
            size_type newsze,
            value_type val
            );

        /// Returns the current number of elements in this list.
        /**
         * \return The number of elements currently in this list.
         */
        inline
        size_type
        size(
            ) const;

        /// Returns the maximum possible size of this list.
        /**
         * \return The maximum allowable size of this list.
         */
        inline
        size_type
        max_size(
            ) const;

        /// Returns true if the size of this list is 0 and false otherwise.
        /**
         * \return true if the list has no elements and false otherwise.
         */
        inline
        bool
        empty(
            ) const;

        /// Returns a duplicate of the allocator being used by this list.
        /**
         * \return A copy of the allocator being used by this list.
         */
        inline
        allocator_type
        get_allocator(
            ) const;

        /// Returns a reference to the first element in the list.
        /**
         * \return A reference to the first element in the list.
         */
        inline
        reference
        front(
            );

        /// Returns a constant reference to the first element in the list.
        /**
         * \return A const-reference to the first element in the list.
         */
        inline
        const_reference
        front(
            ) const;

        /// Returns a reference to the last element in the list.
        /**
         * \return A reference to the last element in the list.
         */
        inline
        reference
        back(
            );

        /// Returns a constant reference to the last element in the list.
        /**
         * \return A const-reference to the last element in the list.
         */
        inline
        const_reference
        back(
            ) const;

        /// Adds val to the end of the list.
        /**
         * \param val The value to be added to the end of this list.
         */
        inline
        void
        push_back(
            const value_type& val
            );

        /// Adds val to the beginning of the list.
        /**
         * \param val The value to be added to the front of this list.
         */
        inline
        void
        push_front(
            const value_type& val
            );

        /// Removes the last element from the list.
        inline
        void
        pop_back(
            );

        /// Removes the first element from the list.
        inline
        void
        pop_front(
            );

        /**
         * \brief Assigns the contents of this list to the elements in
         *        [first, last).
         *
         * \param first The iterator that points to the first element to be
         *              included in this list.
         * \param last The iterator to one past the last element to be included
         *             in this list.
         */
        template<typename Iter>
        inline
        void
        assign(
            Iter first,
            Iter last
            )
        {
            mutex_lock lock(_mutex);
            _container.assign(first, last);
        }

        /// Assigns the contents of this list to \a cnt copies of \a val.
        /**
         * \param cnt The number of \a val instances to put into this list.
         * \param val The object instance to copy \a cnt times into this list.
         */
        inline
        void
        assign(
            size_type cnt,
            const value_type& val
            );

        /// Inserts the value \a val before \a where in the list.
        /**
         * \param where The location before which to insert the supplied value.
         * \param val The value to insert into this list before \a where.
         */
        inline
        iterator
        insert(
            iterator where,
            const value_type& val
            );

        /**
         * \brief Inserts \a cnt copies of the value \a val before
         *        \a where in the list.
         *
         * \param where The location before which to insert the supplied value.
         * \param cnt The number of \a val instances to insert into this list.
         * \param val The value to insert into this list before \a where.
         */
        inline
        void
        insert(
            iterator where,
            size_type cnt,
            const value_type& val
            );

        /// Inserts [first, last) before \a where in the list.
        template<typename Iter>
        inline
        void insert(
            iterator where,
            Iter first,
            Iter last
            )
        {
            mutex_lock lock(_mutex);
            _container.insert(where, first, last);
        }

        /// Removes the element pointed to by \a where from the list.
        inline
        iterator
        erase(
            iterator where
            );

        /// Removes the elements [first, last) from the list.
        inline
        iterator
        erase(
            iterator first,
            iterator last
            );

        /// Removes all elements from the list.
        inline
        void
        clear(
            );

        /// Erases all elements in this list that match \a val.
        inline
        void
        remove(
            const value_type& val
            );

        /**
         * \brief Erases elements from this list for which the supplied
         *        predicate is satisfied.
         */
        template<typename Predicate>
        inline
        void
        remove_if(
            Predicate pred
            )
        {
            mutex_lock lock(_mutex);
            _container.remove_if(pred);
        }

        /// Removes the elements of \a right and puts them into this list.
        /**
         * The elements are then put in ascending order.
         */
        inline
        void
        merge(
            my_type& right
            );

        /// Removes the elements of \a right and puts them into this list.
        /**
         * The elements are then put in ascending order.
         */
        inline
        void
        merge(
            container_type& right
            );

        /// Removes the elements of \a right and puts them into this list.
        /**
         * The elements are then put in order according to the supplied
         * comparitor.
         */
        template<typename Traits>
        void
        merge(
            my_type& right,
            Traits comp
            )
        {
            mutex_lock lock(_mutex);
            _container.merge(right._container, comp);
        }

        /// Removes the elements of \a right and puts them into this list.
        /**
         * The elements are then put in order according to the supplied
         * comparitor.
         */
        template<typename Traits>
        void
        merge(
            container_type& right,
            Traits comp
            )
        {
            mutex_lock lock(_mutex);
            _container.merge(right, comp);
        }

        /// Trades contents with \a right.
        inline
        void
        swap(
            container_type& right
            );

        /// Trades contents with \a right.
        inline
        void
        swap(
            my_type& right
            );

        /// Sorts the elements into ascending order.
        inline
        void
        sort(
            );

        /**
         * \brief Sorts the elements into order determined by the comparator
         *        \a comp.
         */
        template<typename Traits>
        void
        sort(
            Traits comp
            )
        {
            mutex_lock lock(_mutex);
            _container.sort(comp);
        }

        /// Trades contents between \a left and \a right.
        friend
        inline
        void
        swap(
            my_type& left,
            my_type& right
            )
        {
            left.swap(right);
        }

        /// Trades the contents of \a left and \a right.
        friend
        inline
        void
        swap(
            container_type& left,
            my_type& right
            )
        {
            mutex_lock lock(right._mutex);
            left.swap(right._container);
        }

        /// Trades the contents of \a left and \a right.
        friend
        inline
        void
        swap(
            my_type& left,
            container_type& right
            )
        {
            mutex_lock lock(left._mutex);
            left._container.swap(right);
        }

        /// Removes all elements of \a right and places them before \a where
        /**
         * \a where must be from this list.
         */
        inline
        void
        splice(
            iterator where,
            my_type& right
            );

        /**
         * \brief Removes the element at \a first from \a right and places it
         *        before \a where
         *
         * \a first must be from \a right and \a where must be from this
         * list.
         */
        inline
        void
        splice(
            iterator where,
            my_type& right,
            iterator first
            );

        /**
         * \brief Removes [\a first, \a last) from \a right and places them
         *        before \a where
         *
         * \a first and \a last: must be from \a right and \a where must be
         * from this list.
         */
        inline
        void
        splice(
            iterator where,
            my_type& right,
            iterator first,
            iterator last
            );

        /// Removes all elements of \a right and places them before \a where
        /**
         * \a where must be from this list.
         */
        inline
        void
        splice(
            iterator where,
            container_type& right
            );

        /**
         * \brief Removes the element at \a first from \a right and places it
         *        before \a where
         *
         * \a first must be from \a right and \a where must be from this list.
         */
        inline
        void
        splice(
            iterator where,
            container_type& right,
            iterator first
            );

        /**
         * \brief Removes [\a first, \a last) from \a right and places them
         *        before \a where
         *
         * \a first and \a last: must be from \a right and \a where must be
         * from this list.
         */
        inline
        void
        splice(
            iterator where,
            container_type& right,
            iterator first,
            iterator last
            );

        /// Removes every element that compares equal to its preceding element.
        inline
        void
        unique(
            );

        /**
         * \brief Removes every element that satisfies the predicate function
         *        \a pred when compared with its preceding element.
         */
        template<typename BinaryPredicate>
        inline
        void
        unique(
            BinaryPredicate pred
            )
        {
            mutex_lock lock(_mutex);
            _container.unique(pred);
        }

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:





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

        /// Default constructs an empty ts_list.
        ts_list(
            );

        /// Copy constructs a ts_list from a base class object.
        ts_list(
            const container_type& held
            );

        /// Constructs a default ts_list using allocator \a alloc
        explicit
        ts_list(
            const allocator_type& alloc
            );

        /// Constructs a ts_list containing \a cnt default values.
        explicit
        ts_list(
            size_type cnt
            );

        /// Constructs a ts_list containing \a cnt copies of \a val.
        ts_list(
            size_type cnt,
            const value_type& val
            );

        /**
         * \brief Constructs a ts_list containing \a cnt copies of \a val
         *        using \a alloc.
         */
        ts_list(
            size_type cnt,
            const value_type& val,
            const allocator_type& alloc
            );

        /// Copy Constructs a ts_list.
        ts_list(
            const my_type& right
            );

        /**
         * \brief Constructs a ts_list containing the elements in
         *        [\a first, \a last).
         */
        template<typename Iter>
        ts_list(
            Iter first,
            Iter last
            ) :
                _container(first, last),
                _mutex(PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /**
         * \brief Constructs a ts_list containing the elements in
         *        [\a first, \a last) using \a alloc.
         */
        template<typename Iter>
        ts_list(
            Iter first,
            Iter last,
            const allocator_type& alloc
            ) :
                _container(first, last, alloc),
                _mutex(PTHREAD_MUTEX_RECURSIVE)
        {
        }



}; // class ts_list



/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "./inline/ts_list.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_TS_LIST_HPP
