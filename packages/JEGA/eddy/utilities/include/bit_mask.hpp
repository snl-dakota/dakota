/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class bit_mask.

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

        Tue Apr 18 09:41:34 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the bit_mask class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_BIT_MASK_HPP
#define EDDY_UTILITIES_BIT_MASK_HPP







/*
================================================================================
Includes
================================================================================
*/
#include <string>
#include <iosfwd>
#include <sstream>
#include <cassert>
#include <numeric>
#include <cstddef>
#include <memory.h>
#include <stdexcept>
#include <algorithm>
#include "config.hpp"
#include "iterator.hpp"
#include "int_types.hpp"
#include "EDDY_DebugScope.hpp"





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
template <typename MDT, typename BS_T>
class bit_mask_facade;

template <std::size_t NBITS = 8*sizeof(uint8_t), typename BS_T = uint8_t>
class bit_mask;

template <typename BS_T = uint8_t>
class dynamic_bit_mask;





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
/// A base class for bit_masks that provides some common functionality.
/**
 * This class is meant to be inherited by a class that supplies its own
 * type as the first template parameter to this class as in:
 * \code
    class my_mask_type :
        public bit_mask_facade<my_mask_type, unsigned int>
    { ... };
   \endcode
 *
 * This class actually implements a great deal of functionality.  It requires
 * certain interface elements of derived classes which include the following
 * methods:
 * - set_bit(bit_index, bool)
 * - print_bits(std::basic_ostream<CharType, Traits, Alloc>) const
 * - size_type num_usable_bits() const
 * - clear(bool)
 * - toggle_all()
 * - toggle_bit(bit_index)
 * - bool get_bit(bit_index) const
 * - copy constructor
 * - member_at(bit_loc)
 *
 * \param MDT The actual most derived bit_mask type.
 * \param BS_T The integral type used to store the actual bits.
 */
template <typename MDT, typename BS_T>
class bit_mask_facade
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type used as a bit index.
        typedef
        std::size_t
        bit_index;

        /// The type used for size variables.
        typedef
        std::size_t
        size_type;

        /// The type used to store the actual bits for this mask.
        /**
         * This should be an integral type.
         */
        typedef
        BS_T
        bit_store_type;

    private:

        /// A shorthand for the type of the inheriting bit mask.
        typedef
        MDT
        most_derived_type;

        /// A shorthand for the type of the inheriting bit mask.
        typedef
        bit_mask_facade<MDT, BS_T>
        my_type;

    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    public:

        /// A proxy class to represent a bit location immutably.
        /**
         * Given an immutable bit location, you can determine what the bit
         * value at that location is as a boolean (false = 0 and true = 1) but
         * you may not change it.
         */
        class const_bit_location
        {
            /*
            ====================================================================
            Friend Declarations
            ====================================================================
            */

            /**
             * \brief The bit_mask is a friend so that it can access the
             *        protected constructor.
             */
            friend class bit_mask_facade<MDT, BS_T>;

            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            protected:

                /// The array member of the mask in which this location exists.
                const bit_store_type& _maskElem;

                /// A mask of all 0's with a 1 at the location of this bit.
                bit_store_type _locMask;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// Compares this bit location to the supplied boolean value.
                /**
                 * The comparison is done by first converting this bit location
                 * to a boolean.  The result is then compared to \a lval.
                 *
                 * \param lval The logical value to compare to this bit
                 *             locations value.
                 * \return True if the logical interpretation of the value at
                 *         this bit location (1 = true, 0 = false) is the same
                 *         as \a lval and false otherwise.
                 */
                inline
                bool
                operator ==(
                    bool lval
                    ) const;

                /// Converts this bit location into a value of type bool.
                /**
                 * \return true if the bit at this location is a 1 and false
                 *         if it is 0.
                 */
                inline
                operator bool(
                    ) const;

                inline
                bool
                operator ~(
                    ) const;

                inline
                bool
                operator &(
                    bool rhs
                    ) const;

                inline
                bool
                operator |(
                    bool rhs
                    ) const;

                inline
                bool
                operator ^(
                    bool rhs
                    ) const;

                inline
                bool
                operator -(
                    bool rhs
                    ) const;

            /*
            ====================================================================
            Subclass Visible Methods
            ====================================================================
            */
            protected:

                /**
                 * \brief A convenience method for derived classes that may not
                 *        be const constrained.
                 *
                 * This method is never used in this class and should not be
                 * used in any derivative that is meant to have const behavior.
                 *
                 * \return A non-const version of the bit-mask array element
                 *         reference known to this location object.
                 */
                inline
                bit_store_type&
                my_mask_elem(
                    );

                inline
                const bit_store_type&
                my_loc_mask(
                    );

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Copy constructs a const_bit_location.
                /**
                 * \param copy The existing const_bit_location from which to
                 *             copy properties.
                 */
                inline
                const_bit_location(
                    const const_bit_location& copy
                    );

            protected:

                /**
                 * \brief Constructs a const_bit_location of \a mask to
                 *         represent the bit at \a bitloc.
                 *
                 * \param mask The bit mask whose location is to be
                 *             represented.
                 * \param bitloc The location in \a mask that this bit location
                 *               represents.
                 */
                inline
                const_bit_location(
                    const bit_store_type& maskmem,
                    bit_store_type locmask
                    );

        }; // class bit_mask::const_bit_location

        /// A proxy class to represent and possibly modify a bit location.
        /**
         * Given a mutable bit location, you can determine what the bit value
         * at that location is as a boolean (false = 0 and true = 1) and you
         * may also change it by assignment to a boolean.
         */
        class bit_location :
            public const_bit_location
        {
            /*
            ====================================================================
            Friend Declarations
            ====================================================================
            */

            /**
             * \brief The bit_mask is a friend so that it can access the
             *        protected constructor.
             */
            friend class bit_mask_facade<MDT, BS_T>;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// Toggles the value at this bit location.
                /**
                 * true becomes false (1 becomes 0) and visa versa.
                 *
                 * \return This bit location after modification.
                 */
                inline
                bit_location&
                flip(
                    );

                /**
                 * \brief Changes the value of the bit at this location to the
                 *        boolean value of \a rhs (1 for true, 0 for false).
                 *
                 * \param rhs The value for this bit to have (true causes a
                 *            value of 1, false causes 0).
                 * \return This bit_location after modification.
                 */
                inline
                bit_location&
                operator =(
                    bool rhs
                    );

                inline
                bit_location&
                operator &=(
                    bool rhs
                    );

                inline
                bit_location&
                operator |=(
                    bool rhs
                    );

                inline
                bit_location&
                operator ^=(
                    bool rhs
                    );

                inline
                bit_location&
                operator -=(
                    bool rhs
                    );

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Copy constructs a bit_location.
                /**
                 * \param copy The existing bit_location from which to
                 *             copy properties.
                 */
                inline
                bit_location(
                    const bit_location& copy
                    );

            protected:

                /**
                 * \brief Constructs a bit_location of \a mask to represent the
                 *         bit at \a bitloc.
                 *
                 * \param mask The bit mask whose location is to be
                 *             represented.
                 * \param bitloc The location in \a mask that this bit location
                 *               represents.
                 */
                inline
                bit_location(
                    const bit_store_type& maskmem,
                    bit_store_type locmask
                    );

        }; // class bit_mask::bit_location


        /// A const iterator that iterates the bits of this bit_mask.
        /**
         * This iterator dereferences to a const_bit_location and does not
         * allow modification of the value at the location.  The direction of
         * iteration is from lowest index (0) to highest (NBITS-1).
         */
        class const_iterator :
            public eddy::utilities::random_access_iterator_facade<
                const_iterator, std::random_access_iterator_tag, bit_location
                >
        {
            /*
            ====================================================================
            Typedefs
            ====================================================================
            */
            public:

                /// The type of the bit_mask for which this is an iterator.
                typedef
                MDT
                bit_mask_type;

                /// The base iterator type of this iterator.
                typedef
                eddy::utilities::random_access_iterator_facade<
                    const_iterator,
                    std::random_access_iterator_tag,
                    bit_location
                    >
                base_type;

                /// The type used as a distance between iterators.
                typedef
                typename base_type::difference_type
                difference_type;

            /*
            ====================================================================
            Friend Declarations
            ====================================================================
            */

            /**
             * \brief The bit_mask is a friend so that it can access the
             *        protected constructor.
             */
            friend class bit_mask_facade<MDT, BS_T>;

            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            protected:

                /// The bit_mask being iterated.
                bit_mask_type* _theMask;

                /**
                 * \brief The location in the bit mask to which this iterator
                 *        currently points.
                 */
                bit_index _bitloc;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /**
                 * \brief Equates this iterator to rhs.
                 *
                 * \a rhs may iterate in a different bit mask in which case
                 * this iterator will henceforth iterate in that bitmask.
                 *
                 * \param rhs The iterator from which to assign the properties
                 *            of this.
                 * \return This iterator after reassignment of its properties.
                 */
                inline
                const const_iterator&
                operator =(
                    const const_iterator& rhs
                    );

                /// Returns true if rhs is equal to this and false otherwise.
                /**
                 * \param rhs The iterator to compare to this for equivolence.
                 * \return True if \a rhs points to the same element as this.
                 *         False otherwise.
                 */
                inline
                bool
                operator ==(
                    const const_iterator& rhs
                    ) const;

                /**
                 * \brief Returns true if rhs iterates to a later bit than this
                 *        and false otherwise.
                 *
                 * \param rhs The iterator to compare to this to see if this is
                 *            less.
                 * \return True if \a rhs is larger than this.  False
                 *         otherwise.  To be larger is to be at a higher index
                 *         position.
                 */
                inline
                bool
                operator <(
                    const const_iterator& rhs
                    ) const;

                /**
                 * \brief Returns true if rhs iterates to a previous bit
                 *        than this and false otherwise.
                 *
                 * \param rhs The iterator to compare to this to see if this is
                 *            greater.
                 * \return True if \a rhs is less than this.  False otherwise.
                 *         To be less is to be at a lower index position.
                 */
                inline
                bool
                operator >(
                    const const_iterator& rhs
                    ) const;

                /// Does pre-increment by 1 of a const_iterator
                /**
                 * \return This iterator after incrementation.
                 */
                inline
                const_iterator&
                operator ++(
                    );

                /// Does pre-decrement by 1 of a const_iterator
                /**
                 * \return This iterator after decrementation.
                 */
                inline
                const_iterator&
                operator --(
                    );

                /// Returns the distance between this iterator and rhs.
                /**
                 * rhs must be an iterator to the same bit mask as this or this
                 * method will throw a logic_error.
                 *
                 * \param rhs The iterator from which to find the distance from
                 *            this.
                 * \return The distance between this and rhs.
                 */
                inline
                difference_type
                operator -(
                    const const_iterator& rhs
                    ) const;

                /// Advances this iterator by \a diff and returns itself.
                /**
                 * \param diff The distance by which to advance this iterator.
                 * \return This iterator after being advanced by \a diff.
                 */
                inline
                const_iterator&
                operator +=(
                    difference_type diff
                    );

                /// Retreats this iterator by \a diff and returns itself.
                /**
                 * \param diff The distance by which to retreat this iterator.
                 * \return This iterator after being retreated by \a diff.
                 */
                inline
                const_iterator&
                operator -=(
                    difference_type diff
                    );

                /// Dereferences this iterator.
                /**
                 * \return The const_bit_location representing the bit to which
                 *         this iterator currently points.
                 */
                inline
                typename bit_mask_facade<MDT, BS_T>::const_bit_location
                operator *(
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Default constructs a const_iterator.
                /**
                 * This iterator is invalid until initialized.
                 */
                inline
                const_iterator(
                    );

                /// Copy constructs a const_iterator.
                /**
                 * Because of the inheritance relationship, a non-const
                 * iterator can be passed in as \a copy.
                 *
                 * \param copy The iterator from which to copy the properties
                 *             into this new iterator.
                 */
                inline
                const_iterator(
                    const const_iterator& copy
                    );

            protected:

                /**
                 * \brief Creates a new const_iterator for the supplied
                 *        location of the supplied mask.
                 *
                 * The resulting iterator will point to the beginning of the
                 * sequence.
                 *
                 * \param mask The bit mask in which this iterator will
                 *             iterate.
                 * \param bitloc The initial bit location to which this
                 *               iterator points.
                 */
                inline
                const_iterator(
                    const bit_mask_type& mask,
                    const bit_index& bitloc
                    );

        }; // class bit_mask::const_iterator

        /// An iterator that iterates the bits of this bit_mask.
        /**
         * This iterator dereferences to a bit_location and allows
         * modification of the value at the location.  The direction of
         * iteration is from lowest index (0) to highest (NBITS-1).
         */
        class iterator :
            public const_iterator
        {
            /*
            ====================================================================
            Typedefs
            ====================================================================
            */
            public:

                /// The type of the bit mask for which this is an iterator.
                typedef
                typename const_iterator::bit_mask_type
                bit_mask_type;

            /*
            ====================================================================
            Friend Declarations
            ====================================================================
            */

            /**
             * \brief The bit_mask is a friend so that it can access the
             *        protected constructor.
             */
            friend class bit_mask_facade<MDT, BS_T>;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /**
                 * \brief Equates this iterator to rhs.
                 *
                 * \a rhs may iterate in a different bit mask in which case
                 * this iterator will henceforth iterate in that bitmask.
                 *
                 * \param rhs The iterator from which to assign the properties
                 *            of this.
                 * \return This iterator after reassignment of its properties.
                 */
                inline
                const iterator&
                operator =(
                    const iterator& rhs
                    );

                /// Does pre-increment by 1 of a const_iterator
                /**
                 * \return This iterator after incrementation.
                 */
                inline
                iterator&
                operator ++(
                    );

                /// Does pre-decrement by 1 of a const_iterator
                /**
                 * \return This iterator after decrementation.
                 */
                inline
                iterator&
                operator --(
                    );

                /// Increments this iterator by \a diff.
                /**
                 * \param diff The distance by which to advance from this
                 *             iterator.
                 * \return An iterator that is advanced from this by \a diff.
                 */
                inline
                iterator
                operator +(
                    typename const_iterator::difference_type diff
                    ) const;

                /// Decrements this iterator by \a diff.
                /**
                 * \param diff The distance by which to retreat from this
                 *             iterator.
                 * \return An iterator that is retreated from this by \a diff.
                 */
                inline
                iterator
                operator -(
                    typename const_iterator::difference_type diff
                    ) const;

                /// Returns the distance between this iterator and rhs.
                /**
                 * rhs must be an iterator to the same bit mask as this or this
                 * method will throw a logic_error.
                 *
                 * This is only necessary b/c there are overloads of operator -
                 * in this scope.  It simply uses the base class implementation
                 * to get the work done.
                 *
                 * \param rhs The iterator from which to find the distance from
                 *            this.
                 * \return The distance between this and rhs.
                 */
                inline
                typename const_iterator::difference_type
                operator -(
                    const const_iterator& rhs
                    ) const
                {
                    EDDY_FUNC_DEBUGSCOPE
                    return this->const_iterator::operator -(rhs);
                }

                /// Advances this iterator by \a diff and returns itself.
                /**
                 * \param diff The distance by which to advance this iterator.
                 * \return This iterator after being advanced by \a diff.
                 */
                inline
                iterator&
                operator +=(
                    typename const_iterator::difference_type diff
                    );

                /// Retreats this iterator by \a diff and returns itself.
                /**
                 * \param diff The distance by which to retreat this iterator.
                 * \return This iterator after being retreated by \a diff.
                 */
                inline
                iterator&
                operator -=(
                    typename const_iterator::difference_type diff
                    );

                /// Dereferences this iterator.
                /**
                 * \return The bit_location representing the bit to which
                 *         this iterator currently points.
                 */
                inline
                typename bit_mask_facade<MDT, BS_T>::bit_location
                operator *(
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Default constructs a iterator.
                /**
                 * This iterator is invalid until initialized.
                 */
                inline
                iterator(
                    );

                /// Copy constructs an iterator.
                /**
                 * \param copy The iterator from which to copy the properties
                 *             into this new iterator.
                 */
                inline
                iterator(
                    const iterator& copy
                    );

            protected:

                /**
                 * \brief Creates a new iterator for the supplied
                 *        location of the supplied mask.
                 *
                 * The resulting iterator will point to the beginning of the
                 * sequence.
                 *
                 * \param mask The bit mask in which this iterator will
                 *             iterate.
                 * \param bitloc The initial bit location to which this
                 *               iterator points.
                 */
                inline
                iterator(
                    const bit_mask_type& mask,
                    const bit_index& bitloc
                    );

        }; // class bit_mask::iterator

#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC

        /// A reverse iterator that iterates the bits of this bit_mask.
        /**
         * This iterator dereferences to a bit_location and allows
         * modification of the value at the location.  The direction of
         * iteration is from highest index (NBITS-1) to lowest (0).
         */
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        /// A const reverse iterator that iterates the bits of this bit_mask.
        /**
         * This iterator dereferences to a const_bit_location and does not
         * allow modification of the value at the location.  The direction of
         * iteration is from highest index (NBITS-1) to lowest (0).
         */
        typedef std::reverse_iterator<iterator> reverse_iterator;

#else

        typedef std::reverse_iterator<
            const_iterator, std::random_access_iterator_tag, bit_location,
            const bit_location&, const bit_location*, ptrdiff_t
            > const_reverse_iterator;

        typedef std::reverse_iterator<
            iterator, std::random_access_iterator_tag, bit_location,
            bit_location&, bit_location*, ptrdiff_t
            > reverse_iterator;

#endif

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    protected:

        /**
         * \brief A class constant that stores the number of bits in a
         *        variable of type \a bit_store_type.
         */
        static const size_type MEMBER_BIT_COUNT = sizeof(bit_store_type)*8;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Sets the bit value at the supplied location to 1 or 0.
        /**
         * The value set depends on the value of \a to.  If \a to is true, the
         * value set will be 1.  If \a to is false, the value set will be 0.
         *
         * \param bitloc The location of the bit whose value to set.
         * \param to The value to set the bit.  true for 1 and false for 0.
         * \return This mask after modification of the bit.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        most_derived_type&
        set(
            const bit_index& bitloc,
            bool to = true
            );

        /// Sets all bits in this mask to a value of 1.
        /**
         * \return This mask after alteration.
         */
        inline
        most_derived_type&
        set(
            );

        /// Sets the bit value at the supplied location to 0.
        /**
         * \param bitloc The location of the bit whose value to set.
         * \return This mask after modification of the bit.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        most_derived_type&
        reset(
            const bit_index& bitloc
            );

        /// Sets all bits in this mask to a value of 0.
        /**
         * \return This mask after alteration.
         */
        inline
        most_derived_type&
        reset(
            );

        /// Returns an iterator to the first bit location in this mask.
        /**
         * The first bit location has index 0.
         *
         * \return An iterator to the bit at location 0.
         */
        inline
        iterator
        begin(
            );

        /**
         * \brief Returns a const_iterator iterator to the first bit location
         *        in this mask.
         *
         * The first bit location has index 0.
         *
         * \return A const_iterator iterator to the bit at location 0.
         */
        inline
        const_iterator
        begin(
            ) const;

        /// Returns an iterator to one past the last bit location in this mask.
        /**
         * The last bit location has a maximal bit index.  One past it has
         * an invalid bit location.  Therefore you cannot dereference the
         * iterator returned by this function.  Doing so will cause an
         * exception to be thrown by the bit_mask.
         *
         * \return An iterator to one past the bit at the hightest bit
         *         location.
         */
        inline
        iterator
        end(
            );

        /**
         * \brief Returns a const_iterator to one past the last bit location in
         *        this mask.
         *
         * The last bit location has a maximal bit index.  One past it has
         * an invalid bit location.  Therefore you cannot dereference the
         * iterator returned by this function.  Doing so will cause an
         * exception to be thrown by the bit_mask.
         *
         * \return A const_iterator to one past the bit at the hightest bit
         *         location.
         */
        inline
        const_iterator
        end(
            ) const;

        /**
         * \brief Returns a reverse_iterator to the first bit location in the
         *        reverse sequence of this mask.
         *
         * The first bit location in the reverse sequence has a maximal index.
         *
         * \return A reverse_iterator to the bit at the hightest bit location.
         */
        inline
        reverse_iterator
        rbegin(
            );

        /**
         * \brief Returns a const_reverse_iterator to the first bit location in
         *        the reverse sequence of this mask.
         *
         * The first bit location in the reverse sequence has a maximal index.
         *
         * \return A const_reverse_iterator to the bit at the hightest bit
         *         location which is the beginning of the reverse sequence.
         */
        inline
        const_reverse_iterator
        rbegin(
            ) const;

        /**
         * \brief Returns a reverse_iterator to one past the last bit location
         *        in the reverse sequence of this mask.
         *
         * The last bit location in the reverse sequence has index 0.  One past
         * it has an invalid bit location (-1).  Therefore you cannot
         * dereference the iterator returned by this function.  Doing so will
         * cause an exception to be thrown by the bit_mask.
         *
         * \return A reverse_iterator to one past the bit at the hightest bit
         *         location in the reverse sequence (which has minimal bit
         *         index).
         */
        inline
        reverse_iterator
        rend(
            );

        /**
         * \brief Returns a const_reverse_iterator to one past the last bit
         *        location in the reverse sequence of this mask.
         *
         * The last bit location in the reverse sequence has index 0.  One past
         * it has an invalid bit location (-1).  Therefore you cannot
         * dereference the iterator returned by this function.  Doing so will
         * cause an exception to be thrown by the bit_mask.
         *
         * \return A const_reverse_iterator to one past the bit at the hightest
         *         bit location in the reverse sequence (which has minimal bit
         *         index).
         */
        inline
        const_reverse_iterator
        rend(
            ) const;

        /// Toggles all the bits in this bit mask.
        /**
         * Any bits set to 1 become 0 and visa versa.  This is
         * implemented by a call to the most derived types toggle_all
         * function.
         *
         * \return This mask after alteration.
         */
        inline
        most_derived_type&
        flip(
            );

        /// Toggles the bit at the supplied location.
        /**
         * If the bit was a 1, it becomes a 0 and visa versa.  This is
         * implemented by a call to the most derived types toggle_bit
         * function.
         *
         * \param bitloc The location of the bit to flip.
         * \return This mask after modification.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        most_derived_type&
        flip(
            const bit_index& bitloc
            );

        /**
         * \brief Returns the const_bit_location object representing the bit
         *        at \a bitloc.
         *
         * \param bitloc The location of the bit for which a const_bit_location
         *               object is desired.
         * \return The const_bit_location representing the bit at \a bitloc.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        const_bit_location
        get_bit_location(
            const bit_index& bitloc
            ) const;

        /**
         * \brief Returns the bit_location object representing the bit
         *        at \a bitloc.
         *
         * \param bitloc The location of the bit for which a bit_location
         *               object is desired.
         * \return The bit_location representing the bit at \a bitloc.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bit_location
        get_bit_location(
            const bit_index& bitloc
            );

        /// Provides the number of usable bits in this bit mask.
        /**
         * \return The number of bit locations accessible in this mask.
         */
        inline
        bit_index
        size(
            ) const;

        /**
         * \brief Determines the value of the bit at the supplied location in
         *        this mask.
         *
         * This is implemented using the most derived types get_bit function.
         *
         * \param bitloc The location of the bit whose value is of interest.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bool
        test(
            const bit_index& bitloc
            ) const;

        /// Determines if one bit_mask is logically equivalent to another.
        /**
         * This can be used to compare any two bit masks including mixtures
         * of dynamic and static ones.  The two compared masks must have the
         * same size or the return will automatically be false.  Otherwise,
         * all bit locations must have the same logical value.
         *
         * \param other The other bitmask to compare for logical equivalence
         *              to this.
         * \return True if every bit location in the two masks have the same
         *         logical value and false otherwise.
         */
        template <typename BM_T>
        bool
        operator ==(
            const BM_T& other
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we can return without action.
            if((void*)this == (void*)&other) return true;

            // The size is potentially useful in multiple places so store it.
            size_type ms = this->size();

            // if the sizes are different, we can also get out (can't be ==).
            if(ms != other.size()) return false;

            // Do raw comparison of bits. We know they are the same size masks.
            for(bit_index i=0; i<ms; ++i)
                if(this->test(i) != other.test(i)) return false;

            // If we make it here, we are all set.
            return true;
        }

        /// Determines if one bit_mask is logically inequivalent to another.
        /**
         * This is implemented using the operator ==().  See it's documentation
         * for more information.
         *
         * \param other The other bitmask to compare for logical inequivalence
         *              to this.
         * \return True if at least one bit location in the two masks have
         *         different logical values and false otherwise.
         */
        template <typename BM_T>
        bool
        operator !=(
            const BM_T& other
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return !(operator == (other));
        }

        /**
         * \brief Returns the const_bit_location object representing the bit
         *        at \a bitloc.
         *
         * \param bitloc The location of the bit for which a const_bit_location
         *               object is desired.
         * \return The const_bit_location representing the bit at \a bitloc.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        const_bit_location
        operator [](
            const bit_index& bitloc
            ) const;

        /**
         * \brief Returns the bit_location object representing the bit
         *        at \a bitloc.
         *
         * \param bitloc The location of the bit for which a bit_location
         *               object is desired.
         * \return The bit_location representing the bit at \a bitloc.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bit_location
        operator [](
            const bit_index& bitloc
            );

        /**
         * \brief Converts this bit mask into a string of characters of the
         *        parameterized type CharType.
         *
         * The returned bits are ordered such that the first character in the
         * string is the bit at the highest location in the mask and the
         * last character in the string is the bit at location 0.
         *
         * \return A basic_string containing CharType representations of the
         *         bits in this mask.
         */
        template<typename CharType, typename Traits, typename Alloc>
        std::basic_string<CharType, Traits, Alloc>
        to_string(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            typedef std::basic_string<CharType, Traits, Alloc> string_type;
            typedef
            std::basic_ostringstream<CharType, Traits, Alloc>
            ostringstream_type;

            ostringstream_type toload;
            this->casted().print_bits(toload);
            return toload.str();
        }

        /**
         * \brief Reads in new contents for the supplied range of bits for
         *        this mask from the supplied stream.
         *
         * \param lo The lowest index location of the range to read bits for.
         *           This is the first location that will be overwritten.
         * \param hi The highest index location of the range to read bits for.
         *           This is the last location that will be overwritten.
         * \param from The input stream from which bits will be read.  The
         *             contents of the stream must be interpretable as 0's or
         *             1's.
         * \return The stream from which data was read.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         * \throws runtime_error if lo > hi.
         */
        template <typename Elem, typename Traits>
        std::basic_istream<Elem, Traits>&
        read_bits(
            bit_index lo,
            bit_index hi,
            std::basic_istream<Elem, Traits>& from
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(lo < this->casted().num_usable_bits());
            EDDY_ASSERT(hi < this->casted().num_usable_bits());
            EDDY_ASSERT(lo <= hi);

            size_type usable = this->casted().num_usable_bits();
            if(lo >= usable || hi >= usable)
                 throw std::out_of_range("invalid bit_mask position");

            if(lo > hi)
                throw std::runtime_error("invalid bit_mask range");

            // have to prepare to set the failbit if we read no data.
            bool readsome = false;

            // Have to read from lo to hi and stop if we hit eof.
            for(bit_index i=lo; !from.eof() && i<=hi; ++i)
            {
                Elem e;
                from >> e;
                if(!from.fail())
                { this->casted().set_bit(i, e != '0'); readsome = true; }
                else return from;
            }

            if(!readsome) from.setstate(std::ios_base::failbit);
            return from;
        }

        /// Writes all of the bits of this mask into the supplied stream.
        /**
         * The bits are written from most significant to least meaning that
         * the bit with index 0 will appear last in the stream.
         *
         * \param into The stream into which to write the bits of this mask.
         * \return The stream after the bits have been written in.
         */
        template <typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits>&
        print_bits(
            std::basic_ostream<Elem, Traits>& into
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->casted().print_bits(into);
        }

        /// Writes the supplied bits of this mask into the supplied stream.
        /**
         * The bits are written from most significant to least meaning that
         * the bit with index \a lo will appear last in the stream.
         *
         * \param lo The lowest or least significant bit to write to the
         *           stream.
         * \param hi The highest or least significant bit to write to the
         *           stream.
         * \param into The stream into which to write the bits of this mask.
         * \return The stream after the bits have been written in.
         * \throws out_of_range error if \a lo or \a hi is beyond the last
         *                      usable bit.
         * \throws runtime_error \a lo > \a hi.
         */
        template <typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits>&
        print_bits(
            bit_index lo,
            bit_index hi,
            std::basic_ostream<Elem, Traits>& into
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->casted().print_bits(lo, hi, into);
        }

        /// Determines if every set bit in \a other is also set in this.
        /**
         * This may have bits set that are not set in \a other but that does
         * not matter for this method.
         *
         * \param other The other bit mask to test for containment in this.
         * \return true of all bits set in \a other are also set in this and
         *              false otherwise.
         */
        inline
        bool
        contains(
            const most_derived_type& other
            ) const;

        /// Returns the number of bits in this mask with the supplied value.
        /**
         * true corresponds to a value of 1 and false to a value of 0.
         *
         * \param val The value sought to be counted.
         * \return The number of bits found with the supplied value.
         */
        inline
        size_type
        count(
            bool val = true
            ) const;

        /// Tests to see if any of the bits in this mask are set to 1.
        /**
         * \return True if any bits with a value of 1 are found and false
         *         otherwise.
         */
        inline
        bool
        any(
            ) const;

        /// Tests to see if all of the bits in this mask are set to 0.
        /**
         * \return True if all bits in this mask are set to 0 and false
         *         otherwise.
         */
        inline
        bool
        none(
            ) const;

        /// Returns an unsigned long with an equivolent bit pattern as this.
        /**
         * \return An unsigned long with the same pattern of bits as this.
         * \throws overflow_error if unsigned long is not large enough to store
         *                        all the bits of this mask.
         */
        unsigned long
        to_ulong(
            ) const;

        /**
         * \brief An operator to perform bitwise logical negation
         *        (ones-compliment) on this bitmask and return the result.
         *
         * This operation does not modify this bitmask at all.
         *
         * \return A new bitmask as the same type as this that is the ones
         *         compliment of this.
         */
        inline
        most_derived_type
        operator ~(
            ) const;

        /**
         * \brief Performs a right shift of the bits in this mask by the
         *        supplied number of locations.
         *
         * This actually alters this mask.
         *
         * \param off The number of locations to the right to shift the bits of
         *            this mask.
         * \return This mask after bit shifting.
         */
        most_derived_type&
        operator >>=(
            const bit_index& off
            );

        /**
         * \brief Creates a new mask comprised of the bits of this mask right
         *        shifted by the supplied number of locations.
         *
         * This operation does not modify this bitmask at all.
         *
         * \param off The number of locations to the rigth to shift the bits of
         *            this mask when creating the new one.
         * \return A new bitmask as the same type as this that has the same
         *         bits as this but right shifted by \a off locations.
         */
        most_derived_type
        operator >>(
            const bit_index& off
            );

        /**
         * \brief Performs a left shift of the bits in this mask by the
         *        supplied number of locations.
         *
         * This actually alters this mask.
         *
         * \param off The number of locations to the left to shift the bits of
         *            this mask.
         * \return This mask after bit shifting.
         */
        most_derived_type&
        operator <<=(
            const bit_index& off
            );

        /**
         * \brief Creates a new mask comprised of the bits of this mask left
         *        shifted by the supplied number of locations.
         *
         * This operation does not modify this bitmask at all.
         *
         * \param off The number of locations to the left to shift the bits of
         *            this mask when creating the new one.
         * \return A new bitmask as the same type as this that has the same
         *         bits as this but left shifted by \a off locations.
         */
        most_derived_type
        operator <<(
            const bit_index& off
            );

        /// Statically downcasts this to a most_derived_type&.
        /**
         * \return \a casted casted to a most_derived_type&.
         */
        inline
        most_derived_type&
        casted(
            );

        /// Statically downcasts this to a const most_derived_type&.
        /**
         * \return \a casted casted to a const most_derived_type&.
         */
        inline
        const most_derived_type&
        casted(
            ) const;

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Determines the index of the member of the array in which the
         *        supplied bit lies.
         *
         * This does not completely define the location of a bit.  It only
         * provides the index of the actual array element in which it lies.
         * many other bit locations may return the same array index.
         *
         * For example, if the array members are of type bit_store_type and
         * there are 8 bits in a bit_store_type, and you supply a value of 12
         * to this method, the return will be 1 (the second array member
         * index).
         *
         * When used in conjunction with the member_bit, you can exactly
         * determine where the bit of interest lies in the array.
         *
         * \param bitloc The location for which the index within the
         *               appropriate array member is sought.
         * \return The index of the array element that contains the bit of
         *         interest.
         */
        inline
        bit_index
        member_index(
            const bit_index& bitloc
            ) const;

        /**
         * \brief Determines the index within an individual array member of the
         *        supplied bit location.
         *
         * This method assumes that in some way or another, the actual bits
         * of this bitmask are stored in an array of bit_store_type.  This
         * determines what the index into whichever of the members of that
         * array is that this \a bitloc refers to.  As an example, if you are
         * using 8 bit integers to store bits and you have 4 of them in the
         * array in order to store enough bits, and you request the member
         * index of the 20 bit location, the result will be 3 which is the
         * 4th location in the 3rd member of the array.
         *
         * When used in conjunction with the member_index, you can exactly
         * determine where the bit of interest lies in the array.
         *
         * \param bitloc The location for which the index within the
         *               appropriate array member is sought.
         * \return The index within the appropriate array element of the bit of
         *         interest.
         */
        inline
        bit_index
        member_bit(
            const bit_index& bitloc
            ) const;

        /**
         * \brief Creates an item of type bit_store_type that contains all
         *        zeros with the exception of a 1 at the memloc location.
         *
         * For example, if a bit_store_type has 8 bits and you supply a value
         * of 3 to this method, the return will be 00001000 (note that location
         * indexing starts at 0!).
         *
         * Needless to say, memloc must be less than the size of a
         * bit_store_type
         *
         * \param The index of the location to which to assign a 1 in the
         *        returned item.
         * \return A bit_store_type with a 1 at location \a memloc and zeros
         *         everywhere else.
         */
        inline
        static
        bit_store_type
        to_bits(
            bit_index memloc
            );

        /**
         * \brief Pushes the bits of the supplied bit_store_type into the
         *        supplied stream.
         *
         * The bits are pushed in from highest (\a hi) index to lowest (\a lo).
         * Use of \a hi and \a lo allows one to write only a contiguous subset
         * of the bits in \a of into the stream.
         *
         * \param of The bit_store_type from which to obtain the bits to push
         *           into the supplied stream.
         * \param hi The highest bit index to push into the stream.
         * \param lo The lowest bit index to push into the stream.
         * \return The supplied stream after insertion of the desired bits.
         */
        template <typename Elem, typename Traits>
        static
        std::basic_ostream<Elem, Traits>&
        print_bits(
            const bit_store_type& of,
            bit_index lo,
            bit_index hi,
            std::basic_ostream<Elem, Traits>& into
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(lo < MEMBER_BIT_COUNT);
            EDDY_ASSERT(hi < MEMBER_BIT_COUNT);
            EDDY_ASSERT(lo <= hi);

            for(bit_index bitloc = hi; bitloc > lo; --bitloc)
                into << ((of >> bitloc) & 1);

            return into << ((of >> lo) & 1);
        }

}; // class bit_mask_facade

/**
 * \brief A class to encapsulate the functionality of a statically sized mask
 *        of bits.
 *
 * This stores a collection of integral values of type BS_T used to represent
 * multiple boolean values.  It is parameterized on the total number of bits
 * available (# of boolean flags) and the integer types used to
 * acutally store the bits.  NBITS is not limited by the size of the integer
 * type used to store the bits.  This class houses the bits in as many
 * of the BS_T's as necessary.  Using smaller integer types (ex. unsigned char)
 * has the advantage that there is less likely to be wasted bits.  for example,
 * using an 8 bit integer, there will be no more than 7 wasted bits whereas if
 * using a 64 bit type, there may be as many as 63 wasted bits.
 *
 * The STL version of a bitset was not used b/c it does not support iterators
 * which this class does (both forward and reverse with random access
 * symantics) and because this class parameterizes the integer type used to
 * store the bits whereas common stl practice is to use a basic_string.  This
 * class does implement the entire interface of the STL bitset mostly through
 * the facade base class.  Additional methods are supplied with more logical
 * names and additional capabilities.
 *
 * Initial time trials suggest that it doesn't matter much what you use as a
 * type for BS_T so memory should be the only consideration.  If you know a
 * particular integral type to be very fast on your system, by all means, use
 * it.
 *
 * \param NBITS The number of bits to be stored in this mask.
 * \param BS_T The integral type used to store the actual bits.
 */
template <std::size_t NBITS, typename BS_T>
class bit_mask :
    public bit_mask_facade<bit_mask<NBITS, BS_T>, BS_T>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    private:

        /// A shorthand for the type of this bit mask.
        typedef
        bit_mask<NBITS, BS_T>
        my_type;

        /// A shorthand for the facade base type of this bit mask.
        typedef
        bit_mask_facade<my_type, BS_T>
        base_type;

    public:

        /// The type used to store the actual bits for this mask.
        /**
         * This should be an integral type.
         */
        typedef
        typename base_type::bit_store_type
        bit_store_type;

        /// The type used as a bit index.
        typedef
        typename base_type::bit_index
        bit_index;

        /// The type used for size variables.
        typedef
        typename base_type::size_type
        size_type;

        /// The type of an iterator of the bits of a bit mask.
        typedef
        typename base_type::iterator
        iterator;

        /// The type of a constant iterator of the bits of a bit mask.
        typedef
        typename base_type::const_iterator
        const_iterator;

        /// The type of a reverse iterator of the bits of a bit mask.
        typedef
        typename base_type::reverse_iterator
        reverse_iterator;

        /// The type of a constant reverse iterator of the bits of a bit mask.
        typedef
        typename base_type::const_reverse_iterator
        const_reverse_iterator;

        /**
         * \brief The type of a representative object for a particular bit of a
         *        bit mask.
         */
        typedef
        typename base_type::bit_location
        bit_location;

        /**
         * \brief The type of an representative object for a particular bit of
         *        a bit mask with immutable semantics.
         */
        typedef
        typename base_type::const_bit_location
        const_bit_location;


    /*
    ============================================================================
    Friend Declarations
    ============================================================================
    */
    // The base class is a friend so that it can access certain protected
    // members.
    friend class bit_mask_facade<my_type, BS_T>;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// A constant to store the total number of usable bits in this mask.
        static const bit_index USABLE_BITS = NBITS;

        /**
         * \brief A constant to store the total number of elements in the array
         *        of bit_store_type's that actually store the bits.
         */
        static const bit_index ARRAY_SIZE =
            NBITS/base_type::MEMBER_BIT_COUNT +
            ((NBITS%base_type::MEMBER_BIT_COUNT) != 0 ? 1 : 0);

    protected:

        /// A constant to store the total number of bits occupied by this mask.
        /**
         * This includes both the usable bits and the wasted bits.
         */
        static const bit_index TOTAL_BITS =
            base_type::MEMBER_BIT_COUNT * ARRAY_SIZE;

        /**
         * \brief A constant to store the number of array elements that are
         *        fully utilized.
         */
        static const bit_index FULL_ARRAY_ELEMENTS =
            NBITS / base_type::MEMBER_BIT_COUNT;

    private:

        /**
         * \brief The actual array of bit_store_type instances in which the
         *        bits are actually kept.
         */
        bit_store_type _bits[ARRAY_SIZE];


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

        /**
         * \brief Sets all the bits in this mask to the supplied value the
         *        default for which is false (0).
         *
         * \param to The value to which to set every bit in this mask.  True
         *           to make them all 1 and false to make them all 0.
         */
        inline
        void
        clear(
            bool to = false
            );

        /**
         * \brief Changes the value of the bit at \a bitloc to 1 or 0 depending
         *        on the value of \a to.
         *
         * \param bitloc The index of the bit whose value is to be set.
         * \param to The value to which to set the bit at \a bitloc.  true to
         *           make it a 1 and false to make it a zero.
         * \return true if the bit mask changes as a result of this action and
         *         false otherwise.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bool
        set_bit(
            const bit_index& bitloc,
            bool to = true
            );

        /**
         * \brief Changes the bit at \a bitloc to be the opposite of what it
         *        currently is.
         *
         * \param bitloc The index of the bit to toggle.
         * \return The final value of the bit at the supplied location.  True
         *         means 1 and false means 0.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bool
        toggle_bit(
            const bit_index& bitloc
            );

        /**
         * \brief Toggles all bits in this bitmask.
         *
         * All 0's become 1's and visa versa.  This is the one's compliment.
         * This is the same as an operator ~=.
         */
        inline
        void
        toggle_all(
            );

        /// Returns the current value of the bit at \a bitloc.
        /**
         * The return is true if the bit at \a bitloc is 1 and false if it is
         * 0.
         *
         * \param bitloc The location of the bit whose value is of interest.
         * \return true if the bit at \a bitloc has a value of 1 and false
         *         otherwise.
         */
        inline
        bool
        get_bit(
            const bit_index& bitloc
            ) const;


        /// Writes all of the bits of this mask into the supplied stream.
        /**
         * The bits are written from most significant to least meaning that
         * the bit with index 0 will appear last in the stream.
         *
         * \param into The stream into which to write the bits of this mask.
         * \return The stream after the bits have been written in.
         */
        template <typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits>&
        print_bits(
            std::basic_ostream<Elem, Traits>& into
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            size_type usable = this->num_usable_bits();
            return (usable == 0) ?
                into : this->print_bits(0, usable-1, into);
        }

        /// Writes the supplied bits of this mask into the supplied stream.
        /**
         * The bits are written from most significant to least meaning that
         * the bit with index \a lo will appear last in the stream.
         *
         * \param lo The lowest or least significant bit to write to the
         *           stream.
         * \param hi The highest or least significant bit to write to the
         *           stream.
         * \param into The stream into which to write the bits of this mask.
         * \return The stream after the bits have been written in.
         * \throws out_of_range error if \a lo or \a hi is beyond the last
         *                      usable bit.
         * \throws runtime_error \a lo > \a hi.
         */
        template <typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits>&
        print_bits(
            bit_index lo,
            bit_index hi,
            std::basic_ostream<Elem, Traits>& into
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(lo < this->num_usable_bits());
            EDDY_ASSERT(hi < this->num_usable_bits());
            EDDY_ASSERT(lo <= hi);

            if(lo >= this->num_usable_bits() || hi >= this->num_usable_bits())
                throw std::out_of_range(
                    "invalid bit_mask<NBITS, BS_T> position"
                    );

            if(lo > hi)
                throw std::runtime_error(
                    "invalid bit_mask<NBITS, BS_T> range"
                    );

            // store the indices of the members for lo and hi.
            bit_index loMem = this->member_index(lo);
            bit_index hiMem = this->member_index(hi);

            // now store the bit locations within those members of lo and hi
            bit_index loLoc = this->member_bit(lo);
            bit_index hiLoc = this->member_bit(hi);

            // now, if loMem == hiMem, we only need to print one set of values.
            if(loMem == hiMem)
                return this->base_type::print_bits(
                    this->_bits[loMem], loLoc, hiLoc, into
                    );

            // otherwise, print the hi member using only hi.
            this->base_type::print_bits(this->_bits[hiMem], 0, hiLoc, into);

            // now print all between hi and lo in full.
            for(bit_index member = hiMem-1; member>loMem; --member)
                this->base_type::print_bits(
                    this->_bits[member], 0, base_type::MEMBER_BIT_COUNT-1, into
                    );

            // finally, print the lo member using only lo.
            return this->base_type::print_bits(
                this->_bits[loMem], loLoc, base_type::MEMBER_BIT_COUNT-1, into
                );
        }

        /**
         * \brief Reads in new contents for all bits for this mask from the
         *        supplied stream.
         *
         * \param from The input stream from which bits will be read.  The
         *             contents of the stream must be interpretable as 0's or
         *             1's.
         * \return The stream from which data was read.
         */
        template <typename Elem, typename Traits>
        std::basic_istream<Elem, Traits>&
        read_bits(
            std::basic_istream<Elem, Traits>& from
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            size_type usable = this->num_usable_bits();
            return (usable == 0) ? from : this->read_bits(0, usable-1, from);
        }

        /**
         * \brief Reads in new contents for the supplied range of bits for
         *        this mask from the supplied stream.
         *
         * \param lo The lowest index location of the range to read bits for.
         *           This is the first location that will be overwritten.
         * \param hi The highest index location of the range to read bits for.
         *           This is the last location that will be overwritten.
         * \param from The input stream from which bits will be read.  The
         *             contents of the stream must be interpretable as 0's or
         *             1's.
         * \return The stream from which data was read.
         * \throws runtime_error if lo > hi.
         */
        template <typename Elem, typename Traits>
        std::basic_istream<Elem, Traits>&
        read_bits(
            bit_index lo,
            bit_index hi,
            std::basic_istream<Elem, Traits>& from
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->base_type::read_bits(lo, hi, from);
        }

        /**
         * \brief Converts this bit mask into a string of characters of the
         *        parameterized type CharType.
         *
         * The returned bits are ordered such that the first character in the
         * string is the bit at the highest location in the mask and the
         * last character in the string is the bit at location 0.
         *
         * \return A basic_string containing CharType representations of the
         *         bits in this mask.
         */
        template <typename CharType, typename Traits, typename Alloc>
        std::basic_string<CharType, Traits, Alloc>
        to_string(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE

            return this->base_type::EDDY_TEMPLATE_KEYWORD_QUALIFIER to_string<
                CharType, Traits, Alloc
                >();
        }

        /// Returns the number of bits in this mask that are usable.
        /**
         * This is not necessarily the same as the number of bits of memory
         * that this mask requires.  This mask will require an integral
         * multiple of the size of the bit_store_type size no matter how
         * many you choose to use.
         *
         * \return The number of usable bits in this mask.  In the case of the
         *         statically sized mask, this is the same as the template
         *         parameter NBITS value.
         */
        inline
        size_type
        num_usable_bits(
            ) const;

        template <typename UINT_T>
        void
        from_uint(
            const UINT_T& uint
            )
        {
            this->clear();

#ifdef sun

            // continue to rip off chuncks of bits and putting them in our bits
            // array until either we have used all of the uint or all of the
            // bits array.
            size_type max_index = ARRAY_SIZE - 1;

            // figure out how many bits to extract at a time.
            size_type nbte = base_type::MEMBER_BIT_COUNT / 8;

            // figure out how many total extractions should take place.  It is
            // no more than the number of elements in the array and the number
            // of times nbte divides into the size of the ulng.
            size_type ndivs = sizeof(UINT_T) / nbte;

            // the min of the two is the number of interest.
            size_type num_extracts = ndivs > nbte ? nbte : ndivs;

            bit_store_type not_zero = ~static_cast<bit_store_type>(0);

            for(size_type i=0; i<num_extracts; ++i)
            {
                UINT_T ul_rip = not_zero >> (i * base_type::MEMBER_BIT_COUNT);
                this->_bits[i] = static_cast<bit_store_type>(uint & ul_rip);
            }

#else

            // Figure out how many bytes of information to copy over.
            // it is the min of the number avaiable from initBits and the
            // number this can hold.

            // First, figure out how many are available.
            bit_index oBytes = sizeof(UINT_T);

            // Now figure out how many we can hold.
            bit_index mBytes = ARRAY_SIZE / sizeof(uint8_t);

            // Now execute the copy with the max of o and m bytes.
            ::memcpy(this->_bits, &uint, oBytes > mBytes ? mBytes : oBytes);

#endif
        }

        /**
         * \brief Performs in-place bitwise logical and of this mask with the
         *        supplied one.
         *
         * \param other The supplied bit mask to "and" this one with.
         * \return This mask after it has been modified by the and operation.
         */
        template <typename BM_T>
        my_type&
        operator &=(
            const BM_T& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we can return without action.
            if((void*)this == (void*)&other) return *this;

            // We cannot resize this bit mask so we iterate to the lesser of
            // the number of bits in this mask and the number in other.
            size_type e =
                this->size() < other.size() ? this->size() : other.size();

            // If this bit and the other bit of interest are the same, nothing
            // has to be done (1&&1=1, 0&&0=0).  Otherwise, if they are
            // different, then one of them must be a 0 and we set this bit to 0
            // if it is not already.
            for(size_type i=0; i<e; ++i)

                // if this' bit is 0, we don't need to do anything.  We don't
                // even care what the other's bit value is.  If it is 1, we
                // check other.
                if(this->get_bit(i) && !other.test(i)) this->set_bit(i, false);

            // now we must blank out the remaining bits of this mask in keeping
            // with the idea that the missing bits of other are assumed to be
            // zero.
            size_type me = this->size();
            for(size_type i=e; i<me; ++i) this->set_bit(i, false);

            return *this;
        }

        /// Determines if one bit_mask is logically inequivalent to another.
        /**
         * This is implemented using the operator ==().  See it's documentation
         * for more information.
         *
         * \param other The other bitmask to compare for logical inequivalence
         *              to this.
         * \return True if at least one bit location in the two masks have
         *         different logical values and false otherwise.
         */
        template <typename BM_T>
        my_type&
        operator |=(
            const BM_T& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we can return without action.
            if((void*)this == (void*)&other) return *this;

            // We cannot resize this bit mask so we iterate to the lesser of
            // the number of bits in this mask and the number in other.
            size_type e =
                this->size() < other.size() ? this->size() : other.size();

            // If this bit and the other bit of interest are the same, nothing
            // has to be done (1||1=1, 0||0=0).  Otherwise, if they are
            // different, then one of them must be a 1 and we set this bit to 1
            // if it is not already.
            for(size_type i=0; i<e; ++i)

                // if this' bit is 1, we don't need to do anything.  We don't
                // even care what the other's bit value is.  If it is 0, we
                // check other.
                if(!this->get_bit(i) && other.test(i)) this->set_bit(i, true);

            // We can leave the rest of the bits in this alone b/c anything
            // or'ed with zero (the assumed value for the missing bits in
            // other) is itself.
            return *this;
        }

        template <typename BM_T>
        my_type&
        operator ^=(
            const BM_T& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we clear this and return.
            // We clear b/c anything xor'ed with itself is 0.
            if((void*)this == (void*)&other) {
                this->clear(false); return *this;
            }

            // We cannot resize this bit mask so we iterate to the lesser of
            // the number of bits in this mask and the number in other.
            size_type e =
                this->size() < other.size() ? this->size() : other.size();

            // If this bit and other are different, then the result is true.
            // If they are the same, then the result is false.
            for(size_type i=0; i<e; ++i)
            {
                if(this->get_bit(i)) {
                    if(other.test(i)) this->set_bit(i, false);
                }
                else if(other.test(i)) this->set_bit(i, true);
            }

            // We can leave the rest of the bits in this alone b/c anything
            // xor'ed with zero (the assumed value for the missing bits in
            // other) is itself.
            return *this;
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


        inline
        bit_store_type&
        member_at(
            const bit_index& bitloc
            );

        inline
        const bit_store_type&
        member_at(
            const bit_index& bitloc
            ) const;



    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a bit_mask.
        /**
         * All bit locations are initially 0 (false).
         */
        bit_mask(
            );

        /// Copy constructs a bit_mask.
        /**
         * This particular version is for when the copied bit mask type is
         * know to be the same type as is being constructed.  This can be
         * implemented very efficiently in that case.
         *
         * \param copy The existing bit_mask from which to copy properties.
         */
        bit_mask(
            const my_type& copy
            );

        template <typename UINT_T>
        bit_mask(
            const UINT_T& initBits
            )
        {
            this->from_uint(initBits);
        }

        template <std::size_t O_NBITS, typename O_BS_T>
        bit_mask(
            const bit_mask<O_NBITS, O_BS_T>& other
            )
        {
            clear();

            // Figure out how many bytes of information to copy over.
            // it is the min of the number avaiable from other and the number
            // this can hold.

            // First, figure out how many are available.
            std::size_t oBytes =
                bit_mask<O_NBITS, O_BS_T>::ARRAY_SIZE / sizeof(uint8_t);

            // Now figure out how many we can hold.
            std::size_t mBytes = ARRAY_SIZE / sizeof(uint8_t);

            // Now execute the copy with the max of o and m bytes.
            ::memcpy(
                this->_bits, other._bits, oBytes > mBytes ? mBytes : oBytes
                );
        }


}; // class bit_mask


/// A bitmask whose size is set and can be changed at runtime.
/**
 * This class supports all the same operations of the regular or static
 * bit_mask and can have its size changed.  This class is used instead of the
 * vector<bool> because it is similar in interface to the static bit mask.
 *
 * Initial time trials suggest that it doesn't matter much what you use as a
 * type for BS_T so memory should be the only consideration.  If you know a
 * particular integral type to be very fast on your system, by all means, use
 * it.
 *
 * \param BS_T The integral type used to store the actual bits.
 */
template <typename BS_T>
class dynamic_bit_mask :
    public bit_mask_facade<dynamic_bit_mask<BS_T>, BS_T>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    private:

        /// A shorthand for the type of this bit mask.
        typedef
        dynamic_bit_mask<BS_T>
        my_type;

        /// A shorthand for the facade base type of this bit mask.
        typedef
        bit_mask_facade<my_type, BS_T>
        base_type;

    public:

        /// The type used to store the actual bits for this mask.
        /**
         * This should be an integral type.
         */
        typedef
        typename base_type::bit_store_type
        bit_store_type;

        /// The type used as a bit index.
        typedef
        typename base_type::bit_index
        bit_index;

        /// The type used for size variables.
        typedef
        typename base_type::size_type
        size_type;

        /// The type of an iterator of the bits of a bit mask.
        typedef
        typename base_type::iterator
        iterator;

        /// The type of a constant iterator of the bits of a bit mask.
        typedef
        typename base_type::const_iterator
        const_iterator;

        /// The type of a reverse iterator of the bits of a bit mask.
        typedef
        typename base_type::reverse_iterator
        reverse_iterator;

        /// The type of a constant reverse iterator of the bits of a bit mask.
        typedef
        typename base_type::const_reverse_iterator
        const_reverse_iterator;

        /**
         * \brief The type of a representative object for a particular bit of a
         *        bit mask.
         */
        typedef
        typename base_type::bit_location
        bit_location;

        /**
         * \brief The type of an representative object for a particular bit of
         *        a bit mask with immutable semantics.
         */
        typedef
        typename base_type::const_bit_location
        const_bit_location;


    /*
    ============================================================================
    Friend Declarations
    ============================================================================
    */
    // The base class is a friend so that it can access certain protected
    // members.
    friend class bit_mask_facade<my_type, BS_T>;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The current number of bits being represented by this mask.
        size_type _nbits;

        /// The current number of array members used to store the bits.
        size_type _array_size;

        /// The array in which the actual bits are stored.
        bit_store_type* _bits;

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

        void
        resize(
            size_type nbits,
            bool lval = false
            );

        inline
        void
        reserve(
            size_type nbits
            );

        /**
         * \brief Returns the current maximum number of bits that can be held
         *        without allocating more memory.
         *
         * \return The current bit capacity of this mask.
         */
        inline
        size_type
        capacity(
            ) const;

        /**
         * \brief Sets all the bits in this mask to the supplied value the
         *        default for which is false (0).
         *
         * \param to The value to which to set every bit in this mask.  True
         *           to make them all 1 and false to make them all 0.
         */
        inline
        void
        clear(
            bool to = false
            );

        /**
         * \brief Changes the value of the bit at \a bitloc to 1 or 0 depending
         *        on the value of \a to.
         *
         * \param bitloc The index of the bit whose value is to be set.
         * \param to The value to which to set the bit at \a bitloc.  true to
         *           make it a 1 and false to make it a zero.
         * \return true if the bit mask changes as a result of this action and
         *         false otherwise.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bool
        set_bit(
            const bit_index& bitloc,
            bool to = true
            );

        /**
         * \brief Changes the bit at \a bitloc to be the opposite of what it
         *        currently is.
         *
         * \param bitloc The index of the bit to toggle.
         * \return The final value of the bit at the supplied location.  True
         *         means 1 and false means 0.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bool
        toggle_bit(
            const bit_index& bitloc
            );

        /**
         * \brief Toggles all bits in this bitmask.
         *
         * All 0's become 1's and visa versa.  This is the one's compliment.
         * This is the same as an operator ~=.
         */
        inline
        void
        toggle_all(
            );

        /// Returns the logical value of the bit at \a bitloc.
        /**
         * \param bitloc The location of the bit whose value is of interest.
         * \return true if the bit at \a bitloc is set to 1 and false
         *         otherwise.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         */
        inline
        bool
        get_bit(
            const bit_index& bitloc
            ) const;

        /// Writes all of the bits of this mask into the supplied stream.
        /**
         * The bits are written from most significant to least meaning that
         * the bit with index 0 will appear last in the stream.
         *
         * \param into The stream into which to write the bits of this mask.
         * \return The stream after the bits have been written in.
         */
        template <typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits>&
        print_bits(
            std::basic_ostream<Elem, Traits>& into
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            size_type usable = this->num_usable_bits();
            return (usable == 0) ? into :
                this->print_bits(0, usable-1, into);
        }

        /// Writes the supplied bits of this mask into the supplied stream.
        /**
         * The bits are written from most significant to least meaning that
         * the bit with index \a lo will appear last in the stream.
         *
         * \param lo The lowest or least significant bit to write to the
         *           stream.
         * \param hi The highest or least significant bit to write to the
         *           stream.
         * \param into The stream into which to write the bits of this mask.
         * \return The stream after the bits have been written in.
         * \throws out_of_range error if \a lo or \a hi is beyond the last
         *                      usable bit.
         * \throws runtime_error \a lo > \a hi.
         */
        template <typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits>&
        print_bits(
            bit_index lo,
            bit_index hi,
            std::basic_ostream<Elem, Traits>& into
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(lo < this->num_usable_bits());
            EDDY_ASSERT(hi < this->num_usable_bits());
            EDDY_ASSERT(lo <= hi);

            if(lo >= this->num_usable_bits() || hi >= this->num_usable_bits())
                throw std::out_of_range(
                    "invalid dynamic_bit_mask<BS_T> position"
                    );

            if(lo > hi)
                throw std::runtime_error(
                    "invalid dynamic_bit_mask<BS_T> range"
                    );

            // store the indices of the members for lo and hi.
            bit_index loMem = this->member_index(lo);
            bit_index hiMem = this->member_index(hi);

            // now store the bit locations within those members of lo and hi
            bit_index loLoc = this->member_bit(lo);
            bit_index hiLoc = this->member_bit(hi);

            // now, if loMem == hiMem, we only need to print one set of values.
            if(loMem == hiMem)
                return this->base_type::print_bits(
                    this->_bits[loMem], loLoc, hiLoc, into
                    );

            // otherwise, print the hi member using only hi.
            this->base_type::print_bits(this->_bits[hiMem], 0, hiLoc, into);

            // now print all between hi and lo in full.
            for(bit_index member = hiMem-1; member>loMem; --member)
                this->base_type::print_bits(
                    this->_bits[member], 0, base_type::MEMBER_BIT_COUNT-1, into
                    );

            // finally, print the lo member using only lo.
            return this->base_type::print_bits(
                this->_bits[loMem], loLoc, base_type::MEMBER_BIT_COUNT-1, into
                );
        }

        template <typename Elem, typename Traits>
        std::basic_istream<Elem, Traits>&
        read_bits(
            std::basic_istream<Elem, Traits>& from
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            size_type usable = this->num_usable_bits();
            return (usable == 0) ? from : this->read_bits(0, usable-1, from);
        }

        /**
         * \brief Reads in new contents for the supplied range of bits for
         *        this mask from the supplied stream.
         *
         * \param lo The lowest index location of the range to read bits for.
         *           This is the first location that will be overwritten.
         * \param hi The highest index location of the range to read bits for.
         *           This is the last location that will be overwritten.
         * \param from The input stream from which bits will be read.  The
         *             contents of the stream must be interpretable as 0's or
         *             1's.
         * \return The stream from which data was read.
         * \throws out_of_range error if \a bitloc is beyond the last usable
         *                      bit.
         * \throws runtime_error if lo > hi.
         */
        template <typename Elem, typename Traits>
        std::basic_istream<Elem, Traits>&
        read_bits(
            bit_index lo,
            bit_index hi,
            std::basic_istream<Elem, Traits>& from
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->base_type::read_bits(lo, hi, from);
        }

        /**
         * \brief Converts this bit mask into a string of characters of the
         *        parameterized type CharType.
         *
         * The returned bits are ordered such that the first character in the
         * string is the bit at the highest location in the mask and the
         * last character in the string is the bit at location 0.
         *
         * \return A basic_string containing CharType representations of the
         *         bits in this mask.
         */
        template<typename CharType, typename Traits, typename Alloc>
        std::basic_string<CharType, Traits, Alloc>
        to_string(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE

            return this->base_type::EDDY_TEMPLATE_KEYWORD_QUALIFIER to_string<
                CharType, Traits, Alloc
                >();
        }

        inline
        bit_index
        num_usable_bits(
            ) const;

        template <typename UINT_T>
        void
        from_uint(
            const UINT_T& uint
            )
        {
            this->clear();

#ifdef sun

            // continue to rip off chuncks of bits and putting them in our bits
            // array until either we have used all of the uint or all of the
            // bits array.
            size_type max_index = this->array_size() - 1;

            // figure out how many bits to extract at a time.
            size_type nbte = base_type::MEMBER_BIT_COUNT / 8;

            // figure out how many total extractions should take place.  It is
            // no more than the number of elements in the array and the number
            // of times nbte divides into the size of the ulng.
            size_type ndivs = sizeof(UINT_T) / nbte;

            // the min of the two is the number of interest.
            size_type num_extracts = ndivs > nbte ? nbte : ndivs;

            bit_store_type not_zero = ~static_cast<bit_store_type>(0);

            for(size_type i=0; i<num_extracts; ++i)
            {
                UINT_T ul_rip = not_zero >> (i * base_type::MEMBER_BIT_COUNT);
                this->_bits[i] = static_cast<bit_store_type>(uint & ul_rip);
            }

#else

            // Figure out how many bytes of information to copy over.
            // it is the min of the number avaiable from initBits and the
            // number this can hold.

            // First, figure out how many are available.
            bit_index oBytes = sizeof(UINT_T);

            // Now figure out how many we can hold.
            bit_index mBytes = this->array_size() / sizeof(uint8_t);

            // Now execute the copy with the max of o and m bytes.
            ::memcpy(this->_bits, &uint, oBytes > mBytes ? mBytes : oBytes);

#endif
        }

        template <typename BM_T>
        my_type&
        operator &=(
            const BM_T& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we can return without action.
            if((void*)this == (void*)&other) return *this;

            // If the other bitset is larger than this, then this is resized to
            // the size of other.  If this is larger, then the missing bits of
            // other are assumed to be zero.  If they are the same size, then
            // there is no problem
            if(this->size() < other.size()) this->resize(other.size());

            // now we know that this is at least as large as the other.  So we
            // will iterate up to the size of other for starters.
            size_type e = other.size();

            // If this bit and the other bit of interest are the same, nothing
            // has to be done (1&&1=1, 0&&0=0).  Otherwise, if they are
            // different, then one of them must be a 0 and we set this bit to 0
            // if it is not already.
            for(size_type i=0; i<e; ++i)

                // if this' bit is 0, we don't need to do anything.  We don't
                // even care what the other's bit value is.  If it is 1, we
                // check other.
                if(this->get_bit(i) && !other.test(i)) this->set_bit(i, false);

            // now we must blank out the remaining bits of this mask in keeping
            // with the idea that the missing bits of other are assumed to be
            // zero.
            size_type me = this->size();
            for(size_type i=e; i<me; ++i) this->set_bit(i, false);

            return *this;
        }

        template <typename BM_T>
        my_type&
        operator |=(
            const BM_T& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we can return without action.
            if((void*)this == (void*)&other) return *this;

            // If the other bitset is larger than this, then this is resized to
            // the size of other.  If this is larger, then the missing bits of
            // other are assumed to be zero.  If they are the same size, then
            // there is no problem
            if(this->size() < other.size()) this->resize(other.size());

            // now we know that this is at least as large as the other.  So we
            // will iterate up to the size of other for starters.
            size_type e = other.size();

            // If this bit and the other bit of interest are the same, nothing
            // has to be done (1||1=1, 0||0=0).  Otherwise, if they are
            // different, then one of them must be a 1 and we set this bit to 1
            // if it is not already.
            for(size_type i=0; i<e; ++i)

                // if this' bit is 1, we don't need to do anything.  We don't
                // even care what the other's bit value is.  If it is 0, we
                // check other.
                if(!this->get_bit(i) && other.test(i)) this->set_bit(i, true);

            // We can leave the rest of the bits in this alone b/c anything
            // or'ed with zero (the assumed value for the missing bits in
            // other) is itself.
            return *this;
        }


        template <typename BM_T>
        my_type&
        operator ^=(
            const BM_T& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // If other is actually this, then we clear this and return.
            // We clear b/c anything xor'ed with itself is 0.
            if((void*)this == (void*)&other) {
                this->clear(false); return *this;
            }

            // If the other bitset is larger than this, then this is resized to
            // the size of other.  If this is larger, then the missing bits of
            // other are assumed to be zero.  If they are the same size, then
            // there is no problem
            if(this->size() < other.size()) this->resize(other.size());

            // now we know that this is at least as large as the other.  So we
            // will iterate up to the size of other for starters.
            size_type e = other.size();

            // If this bit and other are different, then the result is true.
            // If they are the same, then the result is false.
            for(size_type i=0; i<e; ++i)
            {
                if(this->get_bit(i)) {
                    if(other.test(i)) this->set_bit(i, false);
                }
                else if(other.test(i)) this->set_bit(i, true);
            }

            // We can leave the rest of the bits in this alone b/c anything
            // xor'ed with zero (the assumed value for the missing bits in
            // other) is itself.
            return *this;
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

        void
        grow(
            size_type nbits
            );

    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        inline
        size_type
        fully_utilized_array_elements(
            ) const;

        inline
        size_type
        array_size(
            ) const;

        static
        inline
        size_type
        required_array_size(
            size_type nbits
            );

        inline
        bit_store_type&
        member_at(
            const bit_index& bitloc
            );

        inline
        const bit_store_type&
        member_at(
            const bit_index& bitloc
            ) const;

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        dynamic_bit_mask(
            );

        dynamic_bit_mask(
            size_type nbits
            );

        dynamic_bit_mask(
            const my_type& copy
            );

        template <typename INT_T>
        dynamic_bit_mask(
            size_type nbits,
            const INT_T& initBits
            ) :
                _nbits(nbits),
                _array_size(required_array_size(nbits)),
                _bits(new bit_store_type[required_array_size(nbits)])
        {
            this->from_uint(initBits);
        }

        template <typename O_BS_T>
        dynamic_bit_mask(
            const dynamic_bit_mask<O_BS_T>& other
            ) :
                _nbits(other._nbits),
                _array_size(required_array_size(other._nbits)),
                _bits(new bit_store_type[required_array_size(other._nbits)])
        {
            clear();

            // Figure out how many bytes of information to copy over.
            // it is the min of the number avaiable from other and the number
            // this can hold.

            // First, figure out how many are available.
            std::size_t oBytes =
                (other.array_size() * sizeof(O_BS_T)) / sizeof(uint8_t);

            // Now figure out how many we can hold.
            std::size_t mBytes =
                this->array_size() / sizeof(uint8_t);

            // Now execute the copy with the max of o and m bytes.
            ::memcpy(
                this->_bits, other._bits, oBytes > mBytes ? mBytes : oBytes
                );
        }

        ~dynamic_bit_mask(
            );


}; // class dynamic_bit_mask

/*
================================================================================
Utility Functions & Operators
================================================================================
*/
template <typename BM_T, typename BS_T>
inline
BM_T
operator &(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const bit_mask_facade<BM_T, BS_T>& rhs
    );

/**
 * \brief Performs bitwise logical AND between an eddy bit_mask type and
 *        some other bit mask type.
 *
 * \param lhs The eddy bit_mask type to use in the AND operation.
 * \param rhs The other bit_mask type to use in the AND operation.
 * \return A new eddy bit_mask type that is the bitwise logical AND result of
 *         lhs AND rhs.
 */
template <typename BM_T, typename BS_T, typename OSET_T>
inline
BM_T
operator &(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const OSET_T& rhs
    );

template <typename BM_T, typename BS_T>
inline
BM_T
operator |(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const bit_mask_facade<BM_T, BS_T>& rhs
    );

template <typename BM_T, typename BS_T, typename OSET_T>
inline
BM_T
operator |(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const OSET_T& rhs
    );

template <typename BM_T, typename BS_T>
inline
BM_T
operator ^(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const bit_mask_facade<BM_T, BS_T>& rhs
    );

template <typename BM_T, typename BS_T, typename OSET_T>
inline
BM_T
operator ^(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const OSET_T& rhs
    );

template <typename Elem, typename Traits, typename BM_T, typename BS_T>
inline
std::basic_istream<Elem, Traits>&
operator >> (
    std::basic_istream<Elem, Traits>& stream,
    bit_mask_facade<BM_T, BS_T>& mask
    );

template <typename Elem, typename Traits, typename BM_T, typename BS_T>
inline
std::basic_ostream<Elem, Traits>&
operator << (
    std::basic_ostream<Elem, Traits>& stream,
    const bit_mask_facade<BM_T, BS_T>& mask
    );

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
#include "inline/bit_mask.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_BIT_MASK_HPP
