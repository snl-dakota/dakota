/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Inline methods of class extremes.

    NOTES:

        See notes of extremes.hpp

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri May 16 12:46:53 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the extremes class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <cstdlib>
#include "../Math.hpp"







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace utilities {







/*
================================================================================
Inline Mutators
================================================================================
*/
template<typename T>
inline
void
extremes<T>::set_max(
    size_type elem,
    const T& value
    )
{
    this->_data[elem].second = value;
}

template<typename T>
inline
void
extremes<T>::set_min(
    size_type elem,
    const T& value
    )
{
    this->_data[elem].first = value;
}








/*
================================================================================
Inline Accessors
================================================================================
*/
template<typename T>
inline
const T&
extremes<T>::get_max(
    size_type elem
    ) const
{
    return this->_data[elem].second;
}

template<typename T>
inline
const T&
extremes<T>::get_min(
    size_type elem
    ) const
{
    return this->_data[elem].first;
}

template<typename T>
inline
typename extremes<T>::size_type
extremes<T>::size() const
{
    return this->_data.size();
}








/*
================================================================================
Inline Public Methods
================================================================================
*/
template<typename T>
inline
void
extremes<T>::allocate(
    size_type size
    )
{
    this->_data.assign(size, std::make_pair(T(), T()));
}

template<typename T>
inline
void
extremes<T>::allocate(
    size_type size,
    const T& minVal,
    const T& maxVal
    )
{
    this->_data.assign(size, std::make_pair(T(minVal), T(maxVal)));
}

template<typename T>
inline
bool
extremes<T>::take_if_max(
   size_type elem,
   const T& val
   )
{
    return this->take_max_of(elem, val, this->get_max(elem)) == -1;

} // take_if_max

template<typename T>
inline
bool
extremes<T>::take_if_min(
   size_type elem,
   const T& val
   )
{
    return this->take_min_of(elem, val, this->get_min(elem)) == -1;

} // take_if_min

template<typename T>
int
extremes<T>::take_if_either(
    size_type elem,
    const T& val
    )
{
    bool tookMax = this->take_if_max(elem, val);
    bool tookMin = this->take_if_min(elem, val);
    return (tookMax && tookMin) ? 2 :
           (tookMax) ? 1 :
           (tookMin) ? -1 :
           0;
}

template<typename T>
inline
bool
extremes<T>::equals_max(
    size_type elem,
    const T& val
    ) const
{
    return val == this->get_max(elem);
}

template<typename T>
inline
bool
extremes<T>::equals_min(
    size_type elem,
    const T& val
    ) const
{
    return val == this->get_min(elem);
}

template<typename T>
inline
bool
extremes<T>::equals_max_or_min(
    size_type elem,
    const T& val
    ) const
{
    return this->equals_max(elem, val) || this->equals_min(elem, val);
}


template<typename T>
inline
void
extremes<T>::clear()
{
    this->allocate(this->size());
}

template<typename T>
inline
void
extremes<T>::clear(
   const T& minVal,
   const T& maxVal
   )
{
    this->_data.assign(this->size(), std::make_pair(minVal, maxVal));

} // clear

template<typename T>
inline
void
extremes<T>::flush()
{
    this->_data.clear();
}

template<typename T>
inline
bool
extremes<T>::empty() const
{
    return this->_data.empty();
}

template<typename T>
inline
T
extremes<T>::get_range(
    size_type elem
    ) const
{
    return this->get_max(elem) - this->get_min(elem);
}

template<typename T>
int
extremes<T>::take_max_of(
   size_type elem,
   const T& val1,
   const T& val2
   )
{
    const T& max = val1 > val2 ? val1 : val2;
    this->set_max(elem, max);
    return (&max == &val1) ? -1 : 1;

} // take_max_of

template<typename T>
int
extremes<T>::take_min_of(
   size_type elem,
   const T& val1,
   const T& val2
   )
{
    const T& min = val1 < val2 ? val1 : val2;
    this->set_min(elem, min);
    return (&min == &val1) ? -1 : 1;

} // take_min_of

template<typename T>
const typename extremes<T>::my_type&
extremes<T>::operator = (
    const my_type& rhs
    )
{
    if(this == &rhs) return *this;
    this->_data = rhs._data;
    return *this;
}

template<typename T>
bool
extremes<T>::merge(
   const my_type& rhs
   )
{
    // prepare a return variable.
    bool ret = false;

    // only go for the min of the two sizes.
    size_type nelem = Math::Min(this->size(), rhs.size());

    for(std::size_t elem=0; elem<nelem; ++elem)
    {
        ret |= this->take_if_min(elem, rhs.get_min(elem));
        ret |= this->take_if_max(elem, rhs.get_max(elem));
    }

    return ret;

} // merge


template<typename T>
typename extremes<T>::value_vector
extremes<T>::get_maxs(
    ) const
{
    value_vector ret(this->size());
    for(std::size_t elem=0; elem<this->size(); ++elem)
        ret[elem] = this->get_max(elem);
    return ret;

} // get_maxs

template<typename T>
typename extremes<T>::value_vector
extremes<T>::get_mins(
    ) const
{
    value_vector ret(this->size());
    for(std::size_t elem=0; elem<this->size(); ++elem)
        ret[elem] = this->get_min(elem);
    return ret;

} // get_mins




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








/*
================================================================================
Inline Structors
================================================================================
*/
template<typename T>
inline
extremes<T>::extremes(
    ) :
        _data()
{
}

template<typename T>
inline
extremes<T>::extremes(
    size_type size
    ) :
        _data(size, std::make_pair(T(), T()))
{
}

template<typename T>
inline
extremes<T>::extremes(
    size_type size,
    const T& minVal,
    const T& maxVal
    ) :
        _data(size, std::make_pair(T(minVal), T(maxVal)))
{
}


template<typename T>
inline
extremes<T>::extremes(
    const my_type& copy
    ) :
        _data(copy._data)
{
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy
