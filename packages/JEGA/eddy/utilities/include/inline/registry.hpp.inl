/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Inline methods of class registry.

    NOTES:

        See notes of registry.hpp

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 14 14:47:06 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the registry class.
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
Inline Mutators
================================================================================
*/








/*
================================================================================
Inline Accessors
================================================================================
*/
template<typename ValueType>
typename registry<ValueType>::const_iterator
registry<ValueType>::begin() const
{
    return this->_registry.begin();
}

template<typename ValueType>
typename registry<ValueType>::const_iterator
registry<ValueType>::end() const
{
    return this->_registry.end();
}

template<typename ValueType>
typename registry<ValueType>::size_type
registry<ValueType>::size() const
{
    return this->_registry.size();
}

template<typename ValueType>
typename registry<ValueType>::container&
registry<ValueType>::get_container()
{
    return this->_registry;
}




/*
================================================================================
Inline Public Methods
================================================================================
*/
template<typename ValueType>
bool
registry<ValueType>::register_(
    const ValueType& value
    )
{
    return this->_registry.insert(value).second;
}

template<typename ValueType>
bool
registry<ValueType>::is_registered(
    const ValueType& value
    ) const
{
    return (this->find(value) != this->_registry.end());
}

template<typename ValueType>
bool
registry<ValueType>::unregister(
    const ValueType& value
    )
{
    container::iterator it(this->find(value));
    if(it == this->_registry.end()) return false;
    this->_registry.erase(it);
    return true;
}

template<typename ValueType>
typename registry<ValueType>::const_iterator
registry<ValueType>::find(
    const ValueType& value
    ) const
{
    return this->_registry.find(value);
}

template<typename ValueType>
typename registry<ValueType>::iterator
registry<ValueType>::find(
    const ValueType& value
    )
{
    return this->_registry.find(value);
}

template<typename ValueType>
const typename registry<ValueType>::my_type&
registry<ValueType>::operator = (
    const my_type& rhs
    )
{
    if(this == &rhs) return *this;
    this->_registry = rhs._registry;
    return *this;
}

template<typename ValueType>
typename registry<ValueType>::size_type
registry<ValueType>::merge(
    const my_type& other
    )
{
    const_iterator it(other.begin()), e(other.end());
    size_type ret = 0;
    for(; it!=e; ++it)
        ret += this->register_(*it) ? 1 : 0;
    return ret;
}

template<typename ValueType>
void
registry<ValueType>::clear()
{
    this->_registry.clear();
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








/*
================================================================================
Inline Structors
================================================================================
*/
template <typename ValueType>
registry<ValueType>::registry(
    )
{
}


template <typename ValueType>
registry<ValueType>::registry(
    const my_type& copy
    ) :
        _registry(copy._registry)
{
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy
