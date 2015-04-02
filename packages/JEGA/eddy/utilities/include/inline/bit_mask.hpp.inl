/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Inline methods of class bit_mask.

    NOTES:

        See notes of bit_mask.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Apr 18 09:41:35 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the bit_mask class.
 */




/*
================================================================================
Includes
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
Static Member Data Definitions
================================================================================
*/




/*
================================================================================
Facade Implementation
================================================================================
*/

template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::set(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->casted().clear(true);
    return this->casted();
}

template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::set(
    const bit_index& bitloc,
    bool to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->casted().set_bit(bitloc, to);
    return this->casted();
}

template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::reset(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->casted().clear();
    return this->casted();
}

template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::reset(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->casted().set_bit(bitloc, false);
    return casted();
}


template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::iterator
bit_mask_facade<MDT, BS_T>::begin(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return iterator(this->casted(), 0);
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator
bit_mask_facade<MDT, BS_T>::begin(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return const_iterator(this->casted(), 0);
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::iterator
bit_mask_facade<MDT, BS_T>::end(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return iterator(this->casted(), this->casted().num_usable_bits());
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator
bit_mask_facade<MDT, BS_T>::end(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return const_iterator(this->casted(), this->casted().num_usable_bits());
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::reverse_iterator
bit_mask_facade<MDT, BS_T>::rbegin(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return reverse_iterator(
        casted(),
        static_cast<eddy::utilities::intmax_t>(
            this->casted().num_usable_bits()
            ) - 1
        );
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_reverse_iterator
bit_mask_facade<MDT, BS_T>::rbegin(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return const_reverse_iterator(
        casted(),
        static_cast<eddy::utilities::intmax_t>(
            this->casted().num_usable_bits()
            ) - 1
        );
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::reverse_iterator
bit_mask_facade<MDT, BS_T>::rend(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return reverse_iterator(this->casted(), -1);
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_reverse_iterator
bit_mask_facade<MDT, BS_T>::rend(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return const_reverse_iterator(this->casted(), -1);
}


template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::flip(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->casted().toggle_all();
    return this->casted();
}

template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::flip(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->casted().toggle_bit(bitloc);
    return this->casted();
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_bit_location
bit_mask_facade<MDT, BS_T>::get_bit_location(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->casted().num_usable_bits());

    if(bitloc >= this->casted().num_usable_bits())
        throw std::out_of_range("invalid bit_mask position");

    return const_bit_location(
        this->casted().member_at(bitloc),
        this->to_bits(this->member_bit(bitloc))
        );
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location
bit_mask_facade<MDT, BS_T>::get_bit_location(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->casted().num_usable_bits());

    if(bitloc >= this->casted().num_usable_bits())
        throw std::out_of_range("invalid bit_mask position");

    return bit_location(
        this->casted().member_at(bitloc),
        this->to_bits(this->member_bit(bitloc))
        );
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::contains(
    const most_derived_type& other
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this == &other) ? true : ((*this & other) == other);
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::size_type
bit_mask_facade<MDT, BS_T>::count(
    bool val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    size_type ct = std::accumulate(this->begin(), this->end(), 0U);
    return val ? ct : (this->casted().num_usable_bits() - ct);
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::any(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return std::find(this->begin(), this->end(), true) != this->end();
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::none(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return !this->any();
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_index
bit_mask_facade<MDT, BS_T>::size(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->casted().num_usable_bits();
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::test(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->casted().get_bit(bitloc);
}

template <typename MDT, typename BS_T>
unsigned long
bit_mask_facade<MDT, BS_T>::to_ulong(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->casted().num_usable_bits() <= (sizeof(unsigned long)*8));

    if(this->casted().num_usable_bits() > (sizeof(unsigned long)*8))
        throw std::overflow_error("bit_mask overflow");

    // prepare a return value.  Initialize it to the first array element.
    unsigned long ret = 0;

    // now iterate this bit mask and insert the bits to ret.
    size_type num_bits = this->casted().num_usable_bits();
    for(bit_index i=0; i<num_bits; ++i)
        ret |= (this->test(i) ? 1UL : 0UL) << i;

    return ret;
}

template <typename MDT, typename BS_T>
inline
MDT
bit_mask_facade<MDT, BS_T>::operator ~(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return most_derived_type(this->casted()).flip();
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_bit_location
bit_mask_facade<MDT, BS_T>::operator [](
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->get_bit_location(bitloc);
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location
bit_mask_facade<MDT, BS_T>::operator [](
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->get_bit_location(bitloc);
}

template <typename MDT, typename BS_T>
MDT&
bit_mask_facade<MDT, BS_T>::operator >>=(
    const bit_index& off
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if the ofset is greater than (or equal to) the number of useable
    // bits, we can just clear the mask.
    if(off >= this->casted().num_usable_bits())
    { this->reset(); return this->casted(); }

    const_iterator from(this->casted().begin() + off);
    iterator to(this->begin());
    const_iterator e(this->end());

    for(; from!=e; ++from, ++to) *to = *from;
    for(; to!=e; ++to) *to = false;
    return this->casted();
}

template <typename MDT, typename BS_T>
MDT
bit_mask_facade<MDT, BS_T>::operator >>(
    const bit_index& off
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return most_derived_type(this->casted()) >>= off;
}

template <typename MDT, typename BS_T>
MDT&
bit_mask_facade<MDT, BS_T>::operator <<=(
    const bit_index& off
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if the ofset is greater than (or equal to) the number of useable
    // bits, we can just clear the mask.
    if(off >= this->casted().num_usable_bits())
    { this->reset(); return this->casted(); }

    const_reverse_iterator from(this->rbegin() + off);
    reverse_iterator to(this->rbegin());
    const_reverse_iterator e(this->rend());

    for(; from!=e; ++from, ++to) *to = *from;
    for(; to!=e; ++to) *to = false;
    return this->casted();
}

template <typename MDT, typename BS_T>
MDT
bit_mask_facade<MDT, BS_T>::operator <<(
    const bit_index& off
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return most_derived_type(this->casted()) <<= off;
}


template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_index
bit_mask_facade<MDT, BS_T>::member_index(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->casted().num_usable_bits());
    return bitloc/MEMBER_BIT_COUNT;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_index
bit_mask_facade<MDT, BS_T>::member_bit(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->casted().num_usable_bits());
    return bitloc%MEMBER_BIT_COUNT;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_store_type
bit_mask_facade<MDT, BS_T>::to_bits(
    bit_index memloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(memloc < MEMBER_BIT_COUNT);
    return static_cast<bit_store_type>(1 << memloc);
}

template <typename MDT, typename BS_T>
inline
MDT&
bit_mask_facade<MDT, BS_T>::casted(
    )
{
    return static_cast<MDT&>(*this);
}

template <typename MDT, typename BS_T>
inline
const MDT&
bit_mask_facade<MDT, BS_T>::casted(
    ) const
{
    return static_cast<const MDT&>(*this);
}





/*
================================================================================
Nested Utility Class Implementations
================================================================================
*/
template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::const_bit_location::operator bool(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->_maskElem & this->_locMask) != 0;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_bit_location::operator ==(
    bool lval
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->operator bool() == lval;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_bit_location::operator ~(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return !this->operator bool();
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_bit_location::operator &(
    bool rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->operator bool() && rhs;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_bit_location::operator |(
    bool rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->operator bool() || rhs;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_bit_location::operator ^(
    bool rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    // can I us ^ here???  Depends on if a bool can ever have bits other
    // than 00000000 or 00000001.
    bool me = this->operator bool();
    return me || rhs && me != rhs;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_bit_location::operator -(
    bool rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return !this->operator bool();
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_store_type&
bit_mask_facade<MDT, BS_T>::const_bit_location::my_mask_elem(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return const_cast<bit_store_type&>(this->_maskElem);
}

template <typename MDT, typename BS_T>
inline
const typename bit_mask_facade<MDT, BS_T>::bit_store_type&
bit_mask_facade<MDT, BS_T>::const_bit_location::my_loc_mask(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_locMask;
}


template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::const_bit_location::const_bit_location(
    const const_bit_location& copy
    ) :
        _maskElem(copy._maskElem),
        _locMask(copy._locMask)
{
    EDDY_FUNC_DEBUGSCOPE
}

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::const_bit_location::const_bit_location(
    const bit_store_type& maskmem,
    bit_store_type locmask
    ) :
        _maskElem(maskmem),
        _locMask(locmask)
{
    EDDY_FUNC_DEBUGSCOPE
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location&
bit_mask_facade<MDT, BS_T>::bit_location::flip(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->my_mask_elem() ^= this->my_loc_mask();
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location&
bit_mask_facade<MDT, BS_T>::bit_location::operator =(
    bool rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    bit_store_type& me = this->my_mask_elem();
    rhs ? me |= this->my_loc_mask() : me &= ~this->my_loc_mask();
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location&
bit_mask_facade<MDT, BS_T>::bit_location::operator &=(
    bool rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(!rhs) this->operator =(false);
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location&
bit_mask_facade<MDT, BS_T>::bit_location::operator |=(
    bool rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(rhs) this->operator =(true);
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location&
bit_mask_facade<MDT, BS_T>::bit_location::operator ^=(
    bool rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(rhs) this->flip();
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location&
bit_mask_facade<MDT, BS_T>::bit_location::operator -=(
    bool rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->flip();
}

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::bit_location::bit_location(
    const bit_location& copy
    ) :
        const_bit_location(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::bit_location::bit_location(
    const bit_store_type& maskmem,
    bit_store_type locmask
    ) :
        const_bit_location(maskmem, locmask)
{
    EDDY_FUNC_DEBUGSCOPE
}







template <typename MDT, typename BS_T>
inline
const typename bit_mask_facade<MDT, BS_T>::const_iterator&
bit_mask_facade<MDT, BS_T>::const_iterator::operator =(
    const const_iterator& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(&rhs == this) return rhs;

    this->_theMask = rhs._theMask;
    this->_bitloc = rhs._bitloc;

    return *this;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_iterator::operator ==(
    const const_iterator& rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->_bitloc == rhs._bitloc) &&
           (this->_theMask == rhs._theMask);
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_iterator::operator <(
    const const_iterator& rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(rhs._theMask == this->_theMask);
    return this->_bitloc < rhs._bitloc;
}

template <typename MDT, typename BS_T>
inline
bool
bit_mask_facade<MDT, BS_T>::const_iterator::operator >(
    const const_iterator& rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(rhs._theMask == this->_theMask);
    return this->_bitloc > rhs._bitloc;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator&
bit_mask_facade<MDT, BS_T>::const_iterator::operator ++(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ++this->_bitloc;
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator&
bit_mask_facade<MDT, BS_T>::const_iterator::operator --(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    --this->_bitloc;
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator::difference_type
bit_mask_facade<MDT, BS_T>::const_iterator::operator -(
    const const_iterator& rhs
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(rhs._theMask == this->_theMask);
    if(rhs._theMask != this->_theMask)
        throw std::logic_error(
            "subtraction of iterators to different bit masks"
            );

    return this->_bitloc - rhs._bitloc;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator&
bit_mask_facade<MDT, BS_T>::const_iterator::operator +=(
    difference_type diff
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_bitloc += diff;
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_iterator&
bit_mask_facade<MDT, BS_T>::const_iterator::operator -=(
    difference_type diff
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_bitloc -= diff;
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::const_bit_location
bit_mask_facade<MDT, BS_T>::const_iterator::operator *(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theMask->get_bit_location(this->_bitloc);
}

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::const_iterator::const_iterator(
    ) :
        _theMask(0x0),
        _bitloc(0)
{
    EDDY_FUNC_DEBUGSCOPE
} // const_iterator::const_iterator

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::const_iterator::const_iterator(
    const const_iterator& copy
    ) :
        _theMask(copy._theMask),
        _bitloc(copy._bitloc)
{
    EDDY_FUNC_DEBUGSCOPE
} // const_iterator::const_iterator

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::const_iterator::const_iterator(
    const bit_mask_type& mask,
    const bit_index& bitloc
    ) :
        _theMask(const_cast<bit_mask_type*>(&mask)),
        _bitloc(bitloc)
{
    EDDY_FUNC_DEBUGSCOPE
} // const_iterator::const_iterator


template <typename MDT, typename BS_T>
inline
const typename bit_mask_facade<MDT, BS_T>::iterator&
bit_mask_facade<MDT, BS_T>::iterator::operator =(
    const iterator& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->const_iterator::operator =(rhs);
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::iterator&
bit_mask_facade<MDT, BS_T>::iterator::operator ++(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->const_iterator::operator ++();
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::iterator&
bit_mask_facade<MDT, BS_T>::iterator::operator --(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->const_iterator::operator --();
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::iterator&
bit_mask_facade<MDT, BS_T>::iterator::operator +=(
    typename const_iterator::difference_type diff
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->const_iterator::operator +=(diff);
    return *this;
}
//
//template <typename MDT, typename BS_T>
//inline
//typename bit_mask_facade<MDT, BS_T>::const_iterator::difference_type
//bit_mask_facade<MDT, BS_T>::iterator::operator -(
//    const const_iterator& rhs
//    ) const
//{
//    EDDY_FUNC_DEBUGSCOPE
//    return this->const_iterator::operator -(rhs);
//}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::iterator&
bit_mask_facade<MDT, BS_T>::iterator::operator -=(
    typename const_iterator::difference_type diff
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->const_iterator::operator -=(diff);
    return *this;
}

template <typename MDT, typename BS_T>
inline
typename bit_mask_facade<MDT, BS_T>::bit_location
bit_mask_facade<MDT, BS_T>::iterator::operator *(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theMask->get_bit_location(this->_bitloc);
}

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::iterator::iterator(
    ) :
        const_iterator()
{
    EDDY_FUNC_DEBUGSCOPE
} // iterator::iterator

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::iterator::iterator(
    const iterator& copy
    ) :
        const_iterator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
} // iterator::iterator

template <typename MDT, typename BS_T>
inline
bit_mask_facade<MDT, BS_T>::iterator::iterator(
    const bit_mask_type& mask,
    const bit_index& bitloc
    ) :
        const_iterator(mask, bitloc)
{
    EDDY_FUNC_DEBUGSCOPE
} // iterator::iterator



/*
================================================================================
Inline Mutators
================================================================================
*/








/*
================================================================================
Inline Accessors
================================================================================
*/








/*
================================================================================
Inline Public Methods
================================================================================
*/

template <std::size_t NBITS, typename BS_T>
inline
void
bit_mask<NBITS, BS_T>::clear(
    bool to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ::memset(this->_bits, to ? ~0 : 0, ARRAY_SIZE*sizeof(bit_store_type));
}

template <std::size_t NBITS, typename BS_T>
inline
bool
bit_mask<NBITS, BS_T>::toggle_bit(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->num_usable_bits());

    // First, determine the individual bit location within the relevant member
    // to be modified. The "xor" the relevant member with the to_bits of that.
    this->member_at(bitloc) ^= this->to_bits(this->member_bit(bitloc));

    // now return the resulting value of that bit.
    return this->get_bit(bitloc);
}

template <std::size_t NBITS, typename BS_T>
inline
void
bit_mask<NBITS, BS_T>::toggle_all(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    for(bit_index m=0; m<ARRAY_SIZE; ++m) this->_bits[m] = ~this->_bits[m];
}


template <std::size_t NBITS, typename BS_T>
bool
bit_mask<NBITS, BS_T>::set_bit(
    const bit_index& bitloc,
    bool to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->num_usable_bits());

    // begin by determining which member of the array will be modified.
    bit_store_type& toMod = this->member_at(bitloc);

    // duplicate that value so we can determine if this changes the mask at all
    bit_store_type oldVal = toMod;

    // store the modifier computed using to_bits.
    bit_store_type mod = this->to_bits(this->member_bit(bitloc));

    // now use the "or" operator to modify the bit at the bitloc location.
    (to) ? (toMod |= mod) : (toMod &= ~mod);

    // return true if _bits changed and false otherwise
    return (toMod != oldVal);
}

template <std::size_t NBITS, typename BS_T>
inline
bool
bit_mask<NBITS, BS_T>::get_bit(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->num_usable_bits());

    return (this->member_at(bitloc) &
            this->to_bits(this->member_bit(bitloc))) != 0;
}

template <std::size_t NBITS, typename BS_T>
inline
typename bit_mask<NBITS, BS_T>::size_type
bit_mask<NBITS, BS_T>::num_usable_bits(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return USABLE_BITS;
}

/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/

template <std::size_t NBITS, typename BS_T>
inline
typename bit_mask<NBITS, BS_T>::bit_store_type&
bit_mask<NBITS, BS_T>::member_at(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_bits[this->member_index(bitloc)];
}

template <std::size_t NBITS, typename BS_T>
inline
const typename bit_mask<NBITS, BS_T>::bit_store_type&
bit_mask<NBITS, BS_T>::member_at(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_bits[this->member_index(bitloc)];
}


/*
================================================================================
Inline Structors
================================================================================
*/

template <std::size_t NBITS, typename BS_T>
bit_mask<NBITS, BS_T>::bit_mask(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    clear();
}


template <std::size_t NBITS, typename BS_T>
bit_mask<NBITS, BS_T>::bit_mask(
    const my_type& copy
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ::memcpy(this->_bits, copy._bits, ARRAY_SIZE*sizeof(bit_store_type));
}
















template <typename BS_T>
void
dynamic_bit_mask<BS_T>::grow(
    size_type nbits
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(nbits <= this->capacity()) return;

    // compute the new capacity.
    size_type newarrsz = required_array_size(nbits);

    // create a new set of bits to take the place of the old one.
    bit_store_type* newbits = new bit_store_type[newarrsz];

    // now copy all the old bits into the newbits buffer.
    for(size_type i=0; i<this->_array_size; ++i) newbits[i] = this->_bits[i];

    // now update the array size.
    this->_array_size = newarrsz;

    // finally, destroy the old bits and adopt the new ones.
    delete [] this->_bits;
    this->_bits = newbits;
}




template <typename BS_T>
void
dynamic_bit_mask<BS_T>::resize(
    size_type nbits,
    bool lval
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // make sure our capacity is sufficient for this resize.
    this->grow(nbits);

    // prepare an iterator to the first location to which we wish
    // to do assigning.
    iterator f(eddy::utilities::advance(this->begin(), this->_nbits));

    // now update the number of bits _nbits.
    this->_nbits = nbits;

    // now do the assignment of the lval to all new bits.
    std::fill(f, this->end(), lval);
}

template <typename BS_T>
inline
void
dynamic_bit_mask<BS_T>::reserve(
    size_type nbits
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->grow(nbits);
}

template <typename BS_T>
inline
typename dynamic_bit_mask<BS_T>::size_type
dynamic_bit_mask<BS_T>::capacity(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->array_size() * base_type::MEMBER_BIT_COUNT;
}



template <typename BS_T>
inline
void
dynamic_bit_mask<BS_T>::clear(
    bool to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ::memset(
        this->_bits, to ? ~0 : 0,
        required_array_size(this->size())*sizeof(bit_store_type)
        );
}

template <typename BS_T>
inline
bool
dynamic_bit_mask<BS_T>::toggle_bit(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->num_usable_bits());

    // First, determine the individual bit location within the relevant member
    // to be modified. Then "xor" the relevant member with the to_bits value.
    this->member_at(bitloc) ^= this->to_bits(this->member_bit(bitloc));

    // now return the resulting value of that bit.
    return this->get_bit(bitloc);
}

template <typename BS_T>
inline
void
dynamic_bit_mask<BS_T>::toggle_all(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    size_type nelems = required_array_size(this->size());
    for(size_type m=0; m<nelems; ++m) this->_bits[m] = ~this->_bits[m];
}

template <typename BS_T>
bool
dynamic_bit_mask<BS_T>::set_bit(
    const bit_index& bitloc,
    bool to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->num_usable_bits());

    // begin by determining which member of the array will be modified.
    bit_store_type& toMod = this->member_at(bitloc);

    // duplicate that value so we can determine if this changes the mask at all
    bit_store_type oldVal = toMod;

    // store the modifier computed using to_bits.
    bit_store_type mod = this->to_bits(this->member_bit(bitloc));

    // now use the "or" operator to modify the bit at the bitloc location.
    (to) ? (toMod |= mod) : (toMod &= ~mod);

    // return true if _bits changed and false otherwise
    return (toMod != oldVal);
}

template <typename BS_T>
inline
bool
dynamic_bit_mask<BS_T>::get_bit(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(bitloc < this->num_usable_bits());

    return (this->member_at(bitloc) &
            this->to_bits(this->member_bit(bitloc))) != 0;
}

template <typename BS_T>
inline
typename dynamic_bit_mask<BS_T>::bit_index
dynamic_bit_mask<BS_T>::num_usable_bits(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_nbits;
}

template <typename BS_T>
inline
typename dynamic_bit_mask<BS_T>::size_type
dynamic_bit_mask<BS_T>::fully_utilized_array_elements(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->num_usable_bits() / base_type::MEMBER_BIT_COUNT;
}

template <typename BS_T>
inline
typename dynamic_bit_mask<BS_T>::size_type
dynamic_bit_mask<BS_T>::array_size(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_array_size;// required_array_size(this->size());
}

template <typename BS_T>
inline
typename dynamic_bit_mask<BS_T>::size_type
dynamic_bit_mask<BS_T>::required_array_size(
    size_type nbits
    )
{
    EDDY_FUNC_DEBUGSCOPE
    // This calculates the required size of an array to hold the supplied
    // number of bits.  When we divide nbits by the number of bits/array
    // member and truncate, we get an initial cut at our answer.  Now, if
    // there are still unaccounted for bits, which is the case if nbits does
    // not evenly divide by bits/member, then we need to add 1 more member.
    return  (nbits / base_type::MEMBER_BIT_COUNT) +
           ((nbits % base_type::MEMBER_BIT_COUNT) != 0 ? 1 : 0);
}

template <typename BS_T>
inline
typename dynamic_bit_mask<BS_T>::bit_store_type&
dynamic_bit_mask<BS_T>::member_at(
    const bit_index& bitloc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_bits[this->member_index(bitloc)];
}

template <typename BS_T>
inline
const typename dynamic_bit_mask<BS_T>::bit_store_type&
dynamic_bit_mask<BS_T>::member_at(
    const bit_index& bitloc
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_bits[this->member_index(bitloc)];
}

template <typename BS_T>
dynamic_bit_mask<BS_T>::dynamic_bit_mask(
    ) :
        _nbits(0),
        _array_size(0),
        _bits(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
}

template <typename BS_T>
dynamic_bit_mask<BS_T>::dynamic_bit_mask(
    size_type nbits
    ) :
        _nbits(nbits),
        _array_size(required_array_size(nbits)),
        _bits(new bit_store_type[required_array_size(nbits)])
{
    EDDY_FUNC_DEBUGSCOPE
    this->clear();
}


template <typename BS_T>
dynamic_bit_mask<BS_T>::dynamic_bit_mask(
    const my_type& copy
    ) :
        _nbits(copy._nbits),
        _array_size(required_array_size(copy.size())),
        _bits(new bit_store_type[required_array_size(copy.size())])
{
    EDDY_FUNC_DEBUGSCOPE
    ::memcpy(
        this->_bits, copy._bits, this->array_size()*sizeof(bit_store_type)
        );
}

template <typename BS_T>
dynamic_bit_mask<BS_T>::~dynamic_bit_mask(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete [] this->_bits;
}














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
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return BM_T(lhs.casted()) &= rhs;
}

template <typename BM_T, typename BS_T, typename OSET_T>
inline
BM_T
operator &(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const OSET_T& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return BM_T(lhs.casted()) &= rhs;
}

template <typename BM_T, typename BS_T>
inline
BM_T
operator |(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const bit_mask_facade<BM_T, BS_T>& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return BM_T(lhs.casted()) |= rhs;
}

template <typename BM_T, typename BS_T, typename OSET_T>
inline
BM_T
operator |(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const OSET_T& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return BM_T(lhs.casted()) |= rhs;
}

template <typename BM_T, typename BS_T>
inline
BM_T
operator ^(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const bit_mask_facade<BM_T, BS_T>& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return BM_T(lhs.casted()) ^= rhs;
}

template <typename BM_T, typename BS_T, typename OSET_T>
inline
BM_T
operator ^(
    const bit_mask_facade<BM_T, BS_T>& lhs,
    const OSET_T& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return BM_T(lhs.casted()) ^= rhs;
}

template <typename Elem, typename Traits, typename BM_T, typename BS_T>
inline
std::basic_istream<Elem, Traits>&
operator >> (
    std::basic_istream<Elem, Traits>& stream,
    bit_mask_facade<BM_T, BS_T>& mask
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return mask.read_bits(stream);
}

template <typename Elem, typename Traits, typename BM_T, typename BS_T>
inline
std::basic_ostream<Elem, Traits>&
operator << (
    std::basic_ostream<Elem, Traits>& stream,
    const bit_mask_facade<BM_T, BS_T>& mask
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return mask.print_bits(stream);
}


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy


