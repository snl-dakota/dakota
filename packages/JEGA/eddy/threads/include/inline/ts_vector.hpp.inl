/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class ts_vector.

    NOTES:

        See notes of ts_vector.hpp.

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
 * \brief Contains the inline methods of the ts_vector class.
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
template<typename ValueType, typename AllocType>
void
ts_vector<ValueType, AllocType>::lock(
    ) const
{
    _mutex.lock();

} // lock

template<typename ValueType, typename AllocType>
void
ts_vector<ValueType, AllocType>::unlock(
    ) const
{
    _mutex.unlock();

} // unlock

template<typename ValueType, typename AllocType>
const typename ts_vector<ValueType, AllocType>::my_type&
ts_vector<ValueType, AllocType>::operator =(
    const my_type& rhs
    )
{
    mutex_lock lock1(_mutex);
    mutex_lock lock2(rhs._mutex);
    if(this == &rhs) return *this;
    _container = rhs._container;
    return *this;
}


template<typename ValueType, typename AllocType>
const typename ts_vector<ValueType, AllocType>::my_type&
ts_vector<ValueType, AllocType>::operator =(
    const container_type& rhs
    )
{
    mutex_lock lock(_mutex);
    if(&_container == &rhs) return *this;
    _container = rhs;
    return *this;
}


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::reserve(
    size_type _Count
    )
{
    mutex_lock lock(_mutex);
    _container.reserve(_Count);

} // reserve


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::size_type
ts_vector<ValueType, AllocType>::capacity(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.capacity();

} // capacity


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::iterator
ts_vector<ValueType, AllocType>::begin(
    )
{
    mutex_lock lock(_mutex);
    return _container.begin();

} // begin


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_iterator
ts_vector<ValueType, AllocType>::begin(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.begin();

} // begin


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::iterator
ts_vector<ValueType, AllocType>::end(
    )
{
    mutex_lock lock(_mutex);
    return _container.end();

} // end


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_iterator
ts_vector<ValueType, AllocType>::end(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.end();

} // end


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::reverse_iterator
ts_vector<ValueType, AllocType>::rbegin(
    )
{
    mutex_lock lock(_mutex);
    return _container.rbegin();

} // rbegin


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_reverse_iterator
ts_vector<ValueType, AllocType>::rbegin(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.rbegin();

} // rbegin


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::reverse_iterator
ts_vector<ValueType, AllocType>::rend(
    )
{
    mutex_lock lock(_mutex);
    return _container.rend();

} // rend


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_reverse_iterator
ts_vector<ValueType, AllocType>::rend(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.rend();

} // rend


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::resize(
    size_type _Newsize
    )
{
    mutex_lock lock(_mutex);
    _container.resize(_Newsize);

} // resize


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::resize(
    size_type _Newsize,
    value_type _Val
    )
{
    mutex_lock lock(_mutex);
    _container.resize(_Newsize, _Val);

} // resize


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::size_type
ts_vector<ValueType, AllocType>::size(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.size();

} // size


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::size_type
ts_vector<ValueType, AllocType>::max_size(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.max_size();

} // max_size


template<typename ValueType, typename AllocType>
inline
bool
ts_vector<ValueType, AllocType>::empty(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.empty();

} // empty


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::allocator_type
ts_vector<ValueType, AllocType>::get_allocator(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.get_allocator();

} // get_allocator


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_reference
ts_vector<ValueType, AllocType>::at(
    size_type _Off
    ) const
{
    mutex_lock lock(_mutex);
    return _container.at(_Off);

} // at


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::reference
ts_vector<ValueType, AllocType>::at(
    size_type _Off
    )
{
    mutex_lock lock(_mutex);
    return _container.at(_Off);

} // at


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::reference
ts_vector<ValueType, AllocType>::operator[](
    size_type _Off
    )
{
    mutex_lock lock(_mutex);
    return _container[_Off];
}


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_reference
ts_vector<ValueType, AllocType>::operator[](
    size_type _Off
    ) const
{
    mutex_lock lock(_mutex);
    return _container[_Off];
}


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::reference
ts_vector<ValueType, AllocType>::front(
    )
{
    mutex_lock lock(_mutex);
    return _container.front();

} // front


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_reference
ts_vector<ValueType, AllocType>::front(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.front();

} // front


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::reference
ts_vector<ValueType, AllocType>::back(
    )
{
    mutex_lock lock(_mutex);
    return _container.back();

} // back


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::const_reference
ts_vector<ValueType, AllocType>::back(
    ) const
{
    mutex_lock lock(_mutex);
    return _container.back();

} // back


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::push_back(
    const value_type& _Val
    )
{
    mutex_lock lock(_mutex);
    _container.push_back(_Val);

} // push_back


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::pop_back(
    )
{
    mutex_lock lock(_mutex);
    _container.pop_back();

} // pop_back


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::assign(
    size_type _Count,
    const value_type& _Val
    )
{
    mutex_lock lock(_mutex);
    _container.assign(_Count, _Val);

} // assign


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::iterator
ts_vector<ValueType, AllocType>::insert(
    iterator _Where,
    const value_type& _Val
    )
{
    mutex_lock lock(_mutex);
    return _container.insert(_Where, _Val);

} // insert


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::insert(
    iterator _Where,
    size_type _Count,
    const value_type& _Val
    )
{
    mutex_lock lock(_mutex);
    _container.insert(_Where, _Count, _Val);

} // insert


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::iterator
ts_vector<ValueType, AllocType>::erase(
    iterator _Where
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(_Where);

} // erase


template<typename ValueType, typename AllocType>
inline
typename ts_vector<ValueType, AllocType>::iterator
ts_vector<ValueType, AllocType>::erase(
    iterator _First,
    iterator _Last
    )
{
    mutex_lock lock(_mutex);
    return _container.erase(_First, _Last);

} // erase


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::clear(
    )
{
    mutex_lock lock(_mutex);
    _container.clear();

} // clear


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::swap(
    container_type& rhs
    )
{
    mutex_lock lock(_mutex);
    _container.swap(rhs);

} // swap


template<typename ValueType, typename AllocType>
inline
void
ts_vector<ValueType, AllocType>::swap(
    my_type& rhs
    )
{
    mutex_lock lock1(_mutex);
    mutex_lock lock2(rhs._mutex);
    _container.swap(rhs._container);

} // swap








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



template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    ) :
        _container(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector

template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    const container_type& held
    ) :
        _container(held),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector

template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    const allocator_type& _Al
    ) :
        _container(_Al),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector

template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    size_type _Count
    ) :
        _container(_Count),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector

template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    size_type _Count,
    const value_type& _Val
    ) :
        _container(_Count, _Val),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector

template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    size_type _Count,
    const value_type& _Val,
    const allocator_type& _Al
    ) :
        _container(_Count, _Val, _Al),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector

template<typename ValueType, typename AllocType>
ts_vector<ValueType, AllocType>::ts_vector(
    const my_type& rhs
    ) :
        _container(rhs._container),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_vector




/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

