/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Inline methods of class registry.

    NOTES:

        See notes of keyed_registry.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 15 07:29:38 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the keyed_registry class.
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
template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::const_iterator
keyed_registry<KeyType, ValueType>::begin() const
{
    return this->_registry.begin();
}

template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::const_iterator
keyed_registry<KeyType, ValueType>::end() const
{
    return this->_registry.end();
}

template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::size_type
keyed_registry<KeyType, ValueType>::size() const
{
    return this->_registry.size();
}

template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::container&
keyed_registry<KeyType, ValueType>::get_container()
{
    return this->_registry;
}








/*
================================================================================
Inline Public Methods
================================================================================
*/
template<typename KeyType, typename ValueType>
bool
keyed_registry<KeyType, ValueType>::register_(
  const KeyType& key,
  const ValueType& value
  )
{
    return this->_registry.insert(value_type(key, value)).second;
}

template<typename KeyType, typename ValueType>
bool
keyed_registry<KeyType, ValueType>::is_registered(
    const KeyType& key
    ) const
{
    return (this->find(key) != this->_registry.end());
}

template<typename KeyType, typename ValueType>
bool
keyed_registry<KeyType, ValueType>::unregister(
    const KeyType& value
    )
{
    typename container::iterator it(this->find(this->key));
    if(it == this->_registry.end()) return false;
    this->_registry.erase(it);
    return true;
}

template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::const_iterator
keyed_registry<KeyType, ValueType>::find(
    const KeyType& key
    ) const
{
    return this->_registry.find(key);
}

template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::iterator
keyed_registry<KeyType, ValueType>::find(
    const KeyType& key
    )
{
    return this->_registry.find(key);
}

template<typename KeyType, typename ValueType>
const keyed_registry<KeyType, ValueType>&
keyed_registry<KeyType, ValueType>::operator = (
    const my_type& rhs
    )
{
    if(this == &rhs) return *this;
    this->_registry.operator =(rhs);
    return *this;
}


template<typename KeyType, typename ValueType>
typename keyed_registry<KeyType, ValueType>::size_type
keyed_registry<KeyType, ValueType>::merge(
    const my_type& other
    )
{
    const_iterator it(other.begin()), e(other.end());
    size_type ret = 0;
    for(; it!=e; ++it)
        ret += this->register_((*it).first, (*it).second) ? 1 : 0;

    return ret;
}

template<typename KeyType, typename ValueType>
void
keyed_registry<KeyType, ValueType>::clear()
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
template <typename KeyType, typename ValueType>
keyed_registry<KeyType, ValueType>::keyed_registry(
    )
{
}


template <typename KeyType, typename ValueType>
keyed_registry<KeyType, ValueType>::keyed_registry(
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
