/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of iterator specific utility functions.

    NOTES:

        See notes under various sections of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Feb 21 13:26:22 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains iterator specific utility functions.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_ITERATOR_HPP
#define EDDY_UTILITIES_ITERATOR_HPP






/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"
#include <iterator>






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
    namespace utilities {





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
Functions
================================================================================
*/
/**
 * \brief A method like the std::advance function that returns the advanced
 *        iterator.
 *
 * \param it The iterator to advance by \a dist.
 * \param dist The distance to advance \a it.
 * \return \a it after having advanced it by \a dist.
 */
template <typename InputIterator>
InputIterator&
advance_ip(
    InputIterator& it,
    typename InputIterator::difference_type dist
    )
{
    std::advance(it, dist);
    return it;
}

/**
 * \brief A method like the std::advance function that returns the advanced
 *        iterator having left the original unchanged.
 *
 * \param it A duplicate of the iterator from which to advance.
 * \param dist The distance to advance \a it.
 * \return A new iterator advanced by \a dist from \a it.
 */
template <typename InputIterator>
InputIterator
advance(
    InputIterator it,
    typename InputIterator::difference_type dist
    )
{
    return eddy::utilities::advance_ip(it, dist);
}

/// A facade for iterators which implements some common behavior.
/**
 * This imlements the operator != as !(==), the ++(int) i.t.o. ++(), and the
 * --(int) i.t.o. --().
 *
 * To use this facade, inherit it with the first template argument being the
 * most derived type, i.e. the type that is inheriting this.  The remaining
 * template arguments have the usual iterator meanings and are passed directly
 * to the std::iterator base class.  Node that MDT must be default
 * constructable.
 */
template <
    typename MDT,
    typename CAT,
    typename ELEM,
    typename DIFF = ptrdiff_t,
    typename PTR = ELEM *,
    typename REF = ELEM&
    >
class iterator_facade :
    public std::iterator<CAT, ELEM, DIFF, PTR, REF>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the iterator base class of this class.
        typedef
        std::iterator<CAT, ELEM, DIFF, PTR, REF>
        base_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns true if rhs is not equal to this and false otherwise.
        /**
         * \param rhs The iterator to compare to this for inequivolence.
         * \return True if "rhs" does not point to the same element as
         *         this. false otherwise.
         */
        inline
        bool
        operator !=(
            const MDT& rhs
            ) const
        {
            return !(this->this_mdt().operator ==(rhs));
        }

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Statically downcasts this to a MDT&.
        /**
         * \return this_mdt casted to a MDT&.
         */
        inline
        MDT&
        this_mdt(
            )
        {
            return static_cast<MDT&>(*this);
        }

        /// Statically downcasts this to a const MDT&.
        /**
         * \return this_mdt casted to a const MDT&.
         */
        inline
        const MDT&
        this_mdt(
            ) const
        {
            return static_cast<const MDT&>(*this);
        }

}; // class iterator_facade

/**
 * \brief A facade for iterators which implements some common forward iterator
 *      behavior.
 *
 * This inherits all the iterator_facade methods and implements
 * the operator ++(int) i.t.o. ++().
 *
 * To use this facade, inherit it with the first template argument being the
 * most derived type, i.e. the type that is inheriting this.  The remaining
 * template arguments have the usual iterator meanings and are passed directly
 * to the std::iterator base class.  Node that MDT must be default
 * constructable.
 */
template <
    typename MDT,
    typename CAT,
    typename ELEM,
    typename DIFF = ptrdiff_t,
    typename PTR = ELEM *,
    typename REF = ELEM&
    >
class forward_iterator_facade :
    public iterator_facade<MDT, CAT, ELEM, DIFF, PTR, REF>
{

    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the iterator base class of this class.
        typedef
        iterator_facade<MDT, CAT, ELEM, DIFF, PTR, REF>
        base_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Does post-increment by 1 of an iterator
        /**
         * \return An iterator that is a copy of this prior to incrementation.
         */
        inline
        MDT
        operator ++(
            int
            )
        {
            return MDT(this->this_mdt()).operator ++();
        }

}; // class forward_iterator_facade


/**
 * \brief A facade for iterators which implements some common bi-directional
 *        iterator behavior.
 *
 * This inherits all the forward_iterator_facade methods and implements
 * the operator != as !(==), and the --(int) i.t.o. --().
 *
 * To use this facade, inherit it with the first template argument being the
 * most derived type, i.e. the type that is inheriting this.  The remaining
 * template arguments have the usual iterator meanings and are passed directly
 * to the std::iterator base class.  Node that MDT must be default
 * constructable.
 */
template <
    typename MDT,
    typename CAT,
    typename ELEM,
    typename DIFF = ptrdiff_t,
    typename PTR = ELEM *,
    typename REF = ELEM&
    >
class bidirectional_iterator_facade :
    public forward_iterator_facade<MDT, CAT, ELEM, DIFF, PTR, REF>
{

    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the iterator base class of this class.
        typedef
        forward_iterator_facade<MDT, CAT, ELEM, DIFF, PTR, REF>
        base_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Does post-decrement by 1 of an iterator
        /**
         * \return An iterator that is a copy of this prior to decrementation.
         */
        inline
        MDT
        operator --(
            int
            )
        {
            return MDT(this->this_mdt()).operator --();
        }

}; // class bidirectional_iterator_facade


/**
 * \brief A facade for iterators which implements some common random accsss
 *        type behavior along with basic bidirectional iterator behavior.
 *
 * This inherits all the bidirectional_iterator_facade methods and implements
 * operator >= as !(<), <= as !(>()), + i.t.o. ++ on a duplicate, and -
 * i.t.o. -- on a duplicate.  MDT must be default and copy constructable.
 *
 * To use this facade, inherit it with the first template argument being the
 * most derived type, i.e. the type that is inheriting this.  The remaining
 * template arguments have the usual iterator meanings and are passed directly
 * to the std::iterator base class.  Node that MDT must be default
 * constructable.
 */
template <
    typename MDT,
    typename CAT,
    typename ELEM,
    typename DIFF = ptrdiff_t,
    typename PTR = ELEM *,
    typename REF = ELEM&
    >
class random_access_iterator_facade :
    public bidirectional_iterator_facade<MDT, CAT, ELEM, DIFF, PTR, REF>
{

    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the iterator base class of this class.
        typedef
        bidirectional_iterator_facade<MDT, CAT, ELEM, DIFF, PTR, REF>
        base_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Returns true if this iterates to a location not less
         *        than the one to which rhs iterates.
         *
         * \param rhs The iterator to compare to this to see if this is
         *            greater or equal.
         * \return True if "rhs" is less than or equal to this.  False
         *         otherwise.
         */
        inline
        bool
        operator >=(
            const MDT& rhs
            ) const
        {
            return !(this->this_mdt().operator <(rhs));
        }

        /**
         * \brief Returns true if this iterates to a location not greater than
         *        the one to which rhs iterates.
         *
         * \param rhs The iterator to compare to this to see if this is less
         *            or equal.
         * \return True if "rhs" is equal to or larger than this.  False
         *         otherwise.
         */
        inline
        bool
        operator <=(
            const MDT& rhs
            ) const
        {
            return !(this->this_mdt().operator >(rhs));
        }

        /// Returns an iterator that is advanced from this by "diff".
        /**
         * \param diff The distance by which to advance from this iterator.
         * \return An iterator that is advanced from this by "diff".
         */
        inline
        MDT
        operator +(
            typename base_type::difference_type diff
            ) const
        {
            return MDT(this->this_mdt()) +=(diff);
        }

        /// Returns an iterator that is retreated from this by "diff".
        /**
         * \param diff The distance by which to retreat from this iterator.
         * \return An iterator that is retreated from this by "diff".
         */
        inline
        MDT
        operator -(
            typename base_type::difference_type diff
            ) const
        {
            return operator +(-diff);
        }

}; // class random_access_iterator_facade


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an inlined functions file



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_ITERATOR_HPP
