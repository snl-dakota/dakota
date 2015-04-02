/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class ts_multiset.

    NOTES:

        See notes of ts_multiset.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Apr 08 16:07:28 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the ts_multiset class.
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
template <typename Key, typename Compare, typename Allocator>
inline
void
ts_multiset<Key, Compare, Allocator>::lock(
    ) const
{
    _mutex.lock();

} // lock

template <typename Key, typename Compare, typename Allocator>
inline
void
ts_multiset<Key, Compare, Allocator>::unlock(
    ) const
{
    _mutex.unlock();

} // unlock

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_iterator
ts_multiset<Key, Compare, Allocator>::begin(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.begin();

} // begin

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::begin(
    )
{
    mutex_lock lock(_mutex);
    return _container.begin();

} // begin

template <typename Key, typename Compare, typename Allocator>
inline
void
ts_multiset<Key, Compare, Allocator>::clear(
    )
{
    mutex_lock lock(_mutex);
    _container.clear();

} // clear

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::size_type
ts_multiset<Key, Compare, Allocator>::count(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.count(key);

} // count

template <typename Key, typename Compare, typename Allocator>
bool
ts_multiset<Key, Compare, Allocator>::empty(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.empty();

} // empty

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_iterator
ts_multiset<Key, Compare, Allocator>::end(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.end();

} // end

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::end(
    )
{
    mutex_lock lock(_mutex);
    return _container.end();

} // end

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_iterator_pair
ts_multiset<Key, Compare, Allocator>::equal_range (
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.equal_range(key);
}

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator_pair
ts_multiset<Key, Compare, Allocator>::equal_range (
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.equal_range(key);
}

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::erase(
    iterator where
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(where);

} // erase

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::erase(
    iterator first,
    iterator last
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(first, last);

} // erase

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::size_type
ts_multiset<Key, Compare, Allocator>::erase(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(key);

} // erase

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::find(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.find(key);

} // find

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_iterator
ts_multiset<Key, Compare, Allocator>::find(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.find(key);

} // find

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::allocator_type
ts_multiset<Key, Compare, Allocator>::get_allocator(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.get_allocator();

} // get_allocator

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::insert(
    const value_type& value
    )
{
    mutex_lock lock(_mutex);
    return _container.insert(value);

} // insert

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::insert(
    iterator where,
    const value_type& value
    )
{
    mutex_lock lock(_mutex);
    return _container.insert(where, value);

} // insert

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::key_compare
ts_multiset<Key, Compare, Allocator>::key_comp(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.key_comp();

} // key_comp

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_iterator
ts_multiset<Key, Compare, Allocator>::lower_bound(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.lower_bound(key);

} // lower_bound

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::lower_bound(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.lower_bound(key);

} // lower_bound

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::size_type
ts_multiset<Key, Compare, Allocator>::max_size(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.max_size();

} // max_size

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_reverse_iterator
ts_multiset<Key, Compare, Allocator>::rbegin(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.rbegin();

} // rbegin

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::reverse_iterator
ts_multiset<Key, Compare, Allocator>::rbegin(
    )
{
    mutex_lock lock(_mutex);
    return _container.rbegin();

} // rbegin

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_reverse_iterator
ts_multiset<Key, Compare, Allocator>::rend(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.rend();

} // rend

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::reverse_iterator
ts_multiset<Key, Compare, Allocator>::rend(
    )
{
    mutex_lock lock(_mutex);
    return _container.rend();

} // rend

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::size_type
ts_multiset<Key, Compare, Allocator>::size(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.size();

} // size

template <typename Key, typename Compare, typename Allocator>
inline
void
ts_multiset<Key, Compare, Allocator>::swap(
    container_type& right
    )
{
    mutex_lock lock(_mutex);
    _container.swap(right);

} // swap

template <typename Key, typename Compare, typename Allocator>
inline
void
ts_multiset<Key, Compare, Allocator>::swap(
    my_type& right
    )
{
    mutex_lock lock(_mutex);
    right.lock();
    _container.swap(right._container);
    right.unlock();

} // swap

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::const_iterator
ts_multiset<Key, Compare, Allocator>::upper_bound(
    const key_type& key
    ) const
{
    mutex_lock lock(_mutex);
    return _container.upper_bound(key);

} // upper_bound

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::iterator
ts_multiset<Key, Compare, Allocator>::upper_bound(
    const key_type& key
    )
{
    mutex_lock lock(_mutex);
    return _container.upper_bound(key);

} // upper_bound

template <typename Key, typename Compare, typename Allocator>
inline
typename ts_multiset<Key, Compare, Allocator>::value_compare
ts_multiset<Key, Compare, Allocator>::value_comp(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.value_comp();

} // value_comp








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

template <typename Key, typename Compare, typename Allocator>
inline
ts_multiset<Key, Compare, Allocator>::ts_multiset(
    ) :
        _container(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multiset


template <typename Key, typename Compare, typename Allocator>
inline
ts_multiset<Key, Compare, Allocator>::ts_multiset(
    const key_compare& comp
    ) :
        _container(comp),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multiset

template <typename Key, typename Compare, typename Allocator>
inline
ts_multiset<Key, Compare, Allocator>::ts_multiset(
    const key_compare& comp,
    const allocator_type& al
    ) :
        _container(comp, al),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multiset

template <typename Key, typename Compare, typename Allocator>
inline
ts_multiset<Key, Compare, Allocator>::ts_multiset(
    const my_type& copy
    ) :
        _container(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{
    mutex_lock lock(copy._mutex);
    _container.insert(copy.begin(), copy.end());

} // ts_multiset

template <typename Key, typename Compare, typename Allocator>
inline
ts_multiset<Key, Compare, Allocator>::ts_multiset(
    const container_type& copy
    ) :
        _container(copy),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_multiset







/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

