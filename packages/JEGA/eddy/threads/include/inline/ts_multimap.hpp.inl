/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class ts_multimap.

    NOTES:

        See notes of ts_multimap.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Dec 03 10:30:00 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the ts_multimap class.
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
    namespace threads {





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
template <typename Key, typename Type, typename Compare, typename Allocator>
inline
void
ts_multimap<Key, Type, Compare, Allocator>::lock(
    ) const
{
    _mutex.lock();

} // lock

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
void
ts_multimap<Key, Type, Compare, Allocator>::unlock(
    ) const
{
    _mutex.unlock();

} // unlock

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_iterator
ts_multimap<Key, Type, Compare, Allocator>::begin(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.begin();

} // begin

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::begin(
    )
{
    mutex_lock lock(_mutex);
    return _container.begin();

} // begin

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
void
ts_multimap<Key, Type, Compare, Allocator>::clear(
    )
{
    mutex_lock lock(_mutex);
    _container.clear();

} // clear

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::size_type
ts_multimap<Key, Type, Compare, Allocator>::count(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.count(key);

} // count

template <typename Key, typename Type, typename Compare, typename Allocator>
bool
ts_multimap<Key, Type, Compare, Allocator>::empty(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.empty();

} // empty

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_iterator
ts_multimap<Key, Type, Compare, Allocator>::end(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.end();

} // end

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::end(
    )
{
    mutex_lock lock(_mutex);
    return _container.end();

} // end

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_iterator_pair
ts_multimap<Key, Type, Compare, Allocator>::equal_range (
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.equal_range(key);
}

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator_pair
ts_multimap<Key, Type, Compare, Allocator>::equal_range (
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.equal_range(key);
}

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::erase(
    iterator where
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(where);

} // erase

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::erase(
    iterator first,
    iterator last
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(first, last);

} // erase

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::size_type
ts_multimap<Key, Type, Compare, Allocator>::erase(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(key);

} // erase

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::find(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.find(key);

} // find

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_iterator
ts_multimap<Key, Type, Compare, Allocator>::find(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.find(key);

} // find

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::allocator_type
ts_multimap<Key, Type, Compare, Allocator>::get_allocator(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.get_allocator();

} // get_allocator

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::insert(
    const value_type& value
    )
{
    mutex_lock lock(_mutex);
    return _container.insert(value);

} // insert

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::insert(
    iterator where,
    const value_type& value
    )
{
    mutex_lock lock(_mutex);
    return _container.insert(where, value);

} // insert

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::key_compare
ts_multimap<Key, Type, Compare, Allocator>::key_comp(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.key_comp();

} // key_comp

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_iterator
ts_multimap<Key, Type, Compare, Allocator>::lower_bound(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.lower_bound(key);

} // lower_bound

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::lower_bound(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.lower_bound(key);

} // lower_bound

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::size_type
ts_multimap<Key, Type, Compare, Allocator>::max_size(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.max_size();

} // max_size

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_reverse_iterator
ts_multimap<Key, Type, Compare, Allocator>::rbegin(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.rbegin();

} // rbegin

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::reverse_iterator
ts_multimap<Key, Type, Compare, Allocator>::rbegin(
    )
{
    mutex_lock lock(_mutex);
    return _container.rbegin();

} // rbegin

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_reverse_iterator
ts_multimap<Key, Type, Compare, Allocator>::rend(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.rend();

} // rend

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::reverse_iterator
ts_multimap<Key, Type, Compare, Allocator>::rend(
    )
{
    mutex_lock lock(_mutex);
    return _container.rend();

} // rend

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::size_type
ts_multimap<Key, Type, Compare, Allocator>::size(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.size();

} // size

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
void
ts_multimap<Key, Type, Compare, Allocator>::swap(
    container_type& right
    )
{
    mutex_lock lock(_mutex);
    _container.swap(right);

} // swap

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
void
ts_multimap<Key, Type, Compare, Allocator>::swap(
    my_type& right
    )
{
    mutex_lock lock(_mutex);
    right.lock();
    _container.swap(right._container);
    right.unlock();

} // swap

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::const_iterator
ts_multimap<Key, Type, Compare, Allocator>::upper_bound(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.upper_bound(key);

} // upper_bound

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::iterator
ts_multimap<Key, Type, Compare, Allocator>::upper_bound(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.upper_bound(key);

} // upper_bound

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
typename ts_multimap<Key, Type, Compare, Allocator>::value_compare
ts_multimap<Key, Type, Compare, Allocator>::value_comp(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.value_comp();

} // value_comp

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
const ts_multimap<Key, Type, Compare, Allocator>&
ts_multimap<Key, Type, Compare, Allocator>::operator =(
    const my_type& rhs
    )
{
    mutex_lock lock1(_mutex);
    mutex_lock lock2(rhs._mutex);
    if(this == &rhs) return *this;
    _container = rhs._container;
    return *this;
}

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
const ts_multimap<Key, Type, Compare, Allocator>&
ts_multimap<Key, Type, Compare, Allocator>::operator =(
    const container_type& rhs
    )
{
    mutex_lock lock(_mutex);
    if(&_container == &rhs) return *this;
    _container = rhs;
    return *this;
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

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
ts_multimap<Key, Type, Compare, Allocator>::ts_multimap(
    ) :
        _container(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multimap


template <typename Key, typename Type, typename Compare, typename Allocator>
inline
ts_multimap<Key, Type, Compare, Allocator>::ts_multimap(
    const key_compare& comp
    ) :
        _container(comp),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multimap

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
ts_multimap<Key, Type, Compare, Allocator>::ts_multimap(
    const key_compare& comp,
    const allocator_type& al
    ) :
        _container(comp, al),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multimap

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
ts_multimap<Key, Type, Compare, Allocator>::ts_multimap(
    const my_type& copy
    ) :
        _container(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{
    mutex_lock lock(copy._mutex);
    _container.insert(copy.begin(), copy.end());

} // ts_multimap

template <typename Key, typename Type, typename Compare, typename Allocator>
inline
ts_multimap<Key, Type, Compare, Allocator>::ts_multimap(
    const container_type& copy
    ) :
        _container(copy),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multimap







/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

