/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class ts_vector.

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

        Sat Sep 13 14:00:51 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ts_vector class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_TS_VECTOR_HPP
#define EDDY_THREADS_TS_VECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <vector>
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
class ts_vector;







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
/// A vector class implementation with mutex protection.
/**
 * All methods lock for their duration.  To lock over multiple method
 * calls, use the lock and unlock methods or the scoped_lock concept.
 *
 * A note on iterating this container.  To do so, you should lock and unlock
 * or use the scoped_lock.  There is no iterator level mutex protection.
 * The begin and end methods will lock for their duration but to be sure
 * that it is not modified by another thread for the entire time you are
 * iterating it, lock, and when you are done, unlock.
 *
 * Certain operations, such as assignment, are defined for both a
 * right-hand-side of this type as well as for one of type
 * std::vector<ValueType, AllocType>
 */
template<
            typename ValueType,
            typename AllocType = std::allocator<ValueType>
        >
class EDDY_SL_IEDECL ts_vector
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /// A shorthand for this type.
        typedef
        ts_vector<ValueType, AllocType>
        my_type;

        /// The type of the scoped lock that can be used with this container.
        typedef
        scoped_lock_concept<my_type>
        scoped_lock;

        /// A shorthand for the underlying vector type.
        typedef
        std::vector<ValueType, AllocType>
        container_type;

        /// An type that represents the allocator class for this vector.
        typedef
        typename container_type::allocator_type
        allocator_type;

        /// The type being stored in this vector.
        typedef
        typename container_type::value_type
        value_type;

        /// An iterator type for this vector.
        typedef
        typename container_type::iterator
        iterator;

        /// A constant iterator type for this vector.
        typedef
        typename container_type::const_iterator
        const_iterator;

        /// A type for keeping the size of this vector.
        typedef
        typename container_type::size_type
        size_type;

        /**
         * \brief A type to represent the difference between the address of
         *        two elements.
         */
        typedef
        typename container_type::difference_type
        difference_type;

        /// A type to serve as a non-const pointer to data.
        typedef
        typename container_type::pointer
        pointer;

        /// A type to serve as a constant pointer to an element in the vector.
        typedef
        typename container_type::const_pointer
        const_pointer;

        /// A type to serve as a reference to an element in the vector.
        typedef
        typename container_type::reference
        reference;

        /// A type to serve as a constant-iterator for the vector.
        typedef
        typename container_type::const_reference
        const_reference;

        /// A type to serve as a non-constant reverse iterator for the vector.
        typedef
        typename container_type::reverse_iterator
        reverse_iterator;

        /// A type to serve as a constant reverse iterator for the vector.
        typedef
        typename container_type::const_reverse_iterator
        const_reverse_iterator;


    protected:

    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The underlying un-protected vector for this vector.
        container_type _container;

        /// The mutex used to protect this vector.
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


        /// Locks this vector until an unlock is issued.
        /**
         * All methods lock the vector for their duration.
         * Use this method if you need to lock the vector over
         * multiple actions.
         *
         * The vector may be locked recursively.
         */
        void
        lock(
            ) const;

        /// Unlocks this vector.
        /**
         * Only use this method to counter a lock call.
         */
        void
        unlock(
            ) const;

        /// Assignment operator.  Assigns this to \a _Right.
        const my_type&
        operator =(
            const my_type& _Right
            );

        /**
         * \brief Assignment operator.  Assigns this' container to \a _Right's
         *        container.
         */
        const my_type&
        operator =(
            const container_type& _Right
            );

        /// Makes sure that the capacity is at least \a _Count.
        inline
        void
        reserve(
            size_type _Count
            );

        /// Returns the capacity of this.
        /**
         * The capacity is the number of elements it could contain before it
         * has to reallocate.
         */
        inline
        size_type
        capacity(
            ) const;

        /// Returns an iterator to the first element.
        inline
        iterator
        begin(
            );

        /// Returns a constant iterator to the first element
        inline
        const_iterator
        begin(
            ) const;

        /// Returns an iterator to one past the last element.
        inline
        iterator
        end(
            );

        /// Returns a constant iterator to one past the last element.
        inline
        const_iterator
        end(
            ) const;

        /// Returns an iterator to the beginning of the reverse sequence.
        inline
        reverse_iterator
        rbegin(
            );

        /**
         * \brief Returns a constant iterator to the beginning of the reverse
         *        sequence.
         */
        inline
        const_reverse_iterator
        rbegin(
            ) const;

        /// Returns an iterator to one past the end of the reverse sequence.
        inline
        reverse_iterator
        rend(
            );

        /**
         * \brief Returns a constant iterator to one past the end of the
         *        reverse sequence.
         */
        inline
        const_reverse_iterator
        rend(
            ) const;

        /// Makes the size of this vector exactly \a _Newsize.
        /**
         * If elements are added, they have default values.  Elements
         * may be erased.
         */
        inline
        void
        resize(
            size_type _Newsize
            );

        /// Makes the size of this vector exactly \a _Newsize.
        /**
         * If elements are added, they have the value \a _Val.  Elements
         * may be erased.
         */
         inline
        void
        resize(
            size_type _Newsize,
            value_type _Val
            );

        /// Returns the number of elements currently in the container.
        inline
        size_type
        size(
            ) const;

        /// Returns the maximum length of the container.
        inline
        size_type
        max_size(
            ) const;

        /// Returns true if the vector has no elements.  False otherwise.
        inline
        bool
        empty(
            ) const;

        /// Returns a duplicate the allocator being used by this.
        inline
        allocator_type
        get_allocator(
            ) const;

        /// Returns a constant reference to the element at \a _Off.
        /**
         * This method does bound checking.
         */
        inline
        const_reference
        at(
            size_type _Off
            ) const;

        /// Returns a reference to the element at \a _Off.
        /**
         * This method does bound checking.
         */
        inline
        reference
        at(
            size_type _Off
            );

        /// Returns a reference to the element at \a _Off.
        /**
         * This method does not do bound checking.
         */
        inline
        reference
        operator[](
            size_type _Off
            );

        /// Returns a constant reference to the element at \a _Off.
        /**
         * This method does not do bound checking.
         */
        inline
        const_reference
        operator[](
            size_type _Off
            ) const;

        /// Returns a reference to the first element in the vector.
        inline
        reference
        front(
            );

        /// Returns a constant reference to the first element in the vector.
        inline
        const_reference
        front(
            ) const;

        /// Returns a reference to the last element in the vector.
        inline
        reference
        back(
            );

        /// Returns a constant reference to the last element in the vector.
        inline
        const_reference
        back(
            ) const;

        /// Adds _Val to the end of the vector.
        inline
        void
        push_back(
            const value_type& _Val
            );

        /// Removes the last element from the vector.
        inline
        void
        pop_back(
            );

        /**
         * \brief Assigns the contents of this vector to the elements in
         *        [_First, _Last).
         */
        template<typename _Iter>
        inline
        void
        assign(
            _Iter _First,
            _Iter _Last
            )
        {
            mutex_lock lock(_mutex);
            _container.assign(_First, _Last);
        }

        /// Assigns the contents of this vector to \a _Count copies of \a _Val.
        inline
        void
        assign(
            size_type _Count,
            const value_type& _Val
            );

        /// Inserts the value \a _Val before \a _Where in the vector.
        inline
        iterator
        insert(
            iterator _Where,
            const value_type& _Val
            );

        /**
         * \brief Inserts \a _Count copies of the value \a _Val before
         *        \a _Where in the vector.
         */
        inline
        void
        insert(
            iterator _Where,
            size_type _Count,
            const value_type& _Val
            );

        /// Inserts [_First, _Last) before \a _Where in the vector.
        template<typename _Iter>
        inline
        void
        insert(
            iterator _Where,
            _Iter _First,
            _Iter _Last
            )
        {
            mutex_lock lock(_mutex);
            _container.insert(_Where, _First, _Last);
        }

        /// Removes the element pointed to by \a _Where from the vector.
        inline
        iterator
        erase(
            iterator _Where
            );

        /// Removes the elements [_First, _Last) from the vector.
        inline
        iterator
        erase(
            iterator _First,
            iterator _Last
            );

        /// Removes all elements from the vector.
        inline
        void
        clear(
            );

        /// Trades contents with \a _Right.
        inline
        void
        swap(
            container_type& _Right
            );

        /// Trades contents with \a _Right.
        inline
        void
        swap(
            my_type& _Right
            );

        /// Trades contents between \a _Left and \a _Right.
        friend
        inline
        void
        swap(
            my_type& _Left,
            my_type& _Right
            )
        {
            _Left.swap(_Right);
        }

        /// Trades the contents of \a _Left and \a _Right.
        friend
        inline
        void
        swap(
            container_type& _Left,
            my_type& _Right
            )
        {
            mutex_lock lock(_Right._mutex);
            _Left.swap(_Right._container);
        }

        /// Trades the contents of \a _Left and \a _Right.
        friend
        inline
        void
        swap(
            my_type& _Left,
            container_type& _Right
            )
        {
            mutex_lock lock(_Left._mutex);
            _Left._container.swap(_Right);
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


        /// Default constructs an empty ts_vector.
        ts_vector(
            );

        /// Copy constructs a ts_vector from a base class object.
        ts_vector(
            const container_type& held
            );

        /// Constructs a default ts_vector using allocator \a _Al
        explicit
        ts_vector(
            const allocator_type& _Al
            );

        /// Constructs a ts_vector containing \a _Count default values.
        explicit
        ts_vector(
            size_type _Count
            );

        /// Constructs a ts_vector containing \a _Count copies of \a _Val.
        ts_vector(
            size_type _Count,
            const value_type& _Val
            );

        /**
         * \brief Constructs a ts_vector containing \a _Count copies of \a _Val
         *        using \a _Al.
         */
        ts_vector(
            size_type _Count,
            const value_type& _Val,
            const allocator_type& _Al
            );

        /// Copy Constructs a ts_vector.
        ts_vector(
            const my_type& _Right
            );

        /// Constructs a ts_vector containing the elements in [first, last).
        template<typename _Iter>
        ts_vector(
            _Iter _First,
            _Iter _Last
            ) :
                _container(_First, _Last),
                _mutex(PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /**
         * \brief Constructs a ts_vector containing the elements in
         *        [first, last) using \a _Al.
         */
        template<typename _Iter>
        ts_vector(
            _Iter _First,
            _Iter _Last,
            const allocator_type& _Al
            ) :
                _container(_First, _Last, _Al),
                _mutex(PTHREAD_MUTEX_RECURSIVE)
        {
        }




}; // class ts_vector



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
#include "./inline/ts_vector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_TS_VECTOR_HPP
