/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class ts_list.

    NOTES:

        See notes of ts_list.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Mar 02 20:42:09 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the ts_list class.
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
ts_list<ValueType, AllocType>::lock(
    ) const
{
    this->_mutex.lock();

} // lock

template<typename ValueType, typename AllocType>
void
ts_list<ValueType, AllocType>::unlock(
    ) const
{
    this->_mutex.unlock();

} // unlock

template<typename ValueType, typename AllocType>
const typename ts_list<ValueType, AllocType>::my_type&
ts_list<ValueType, AllocType>::operator =(
    const my_type& rhs
    )
{
    mutex_lock lock1(this->_mutex);
    mutex_lock lock2(rhs._mutex);
    if(this == &rhs) return *this;
    this->_container = rhs._container;
    return *this;
}

template<typename ValueType, typename AllocType>
const typename ts_list<ValueType, AllocType>::my_type&
ts_list<ValueType, AllocType>::operator =(
    const container_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    if(&this->_container == &rhs) return *this;
    this->_container = rhs;
    return *this;
}

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::iterator
ts_list<ValueType, AllocType>::begin(
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.begin();

} // begin

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::const_iterator
ts_list<ValueType, AllocType>::begin(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.begin();

} // begin

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::iterator
ts_list<ValueType, AllocType>::end(
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.end();

} // end

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::const_iterator
ts_list<ValueType, AllocType>::end(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.end();

} // end

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::reverse_iterator
ts_list<ValueType, AllocType>::rbegin(
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.rbegin();

} // rbegin

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::const_reverse_iterator
ts_list<ValueType, AllocType>::rbegin(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.rbegin();

} // rbegin

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::reverse_iterator
ts_list<ValueType, AllocType>::rend(
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.rend();

} // rend

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::const_reverse_iterator
ts_list<ValueType, AllocType>::rend(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.rend();

} // rend

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::resize(
    size_type _Newsize
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.resize(_Newsize);

} // resize

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::resize(
    size_type _Newsize,
    value_type _Val
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.resize(_Newsize, _Val);

} // resize

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::size_type
ts_list<ValueType, AllocType>::size(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.size();

} // size

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::size_type
ts_list<ValueType, AllocType>::max_size(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.max_size();

} // max_size

template<typename ValueType, typename AllocType>
inline
bool
ts_list<ValueType, AllocType>::empty(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.empty();

} // empty

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::allocator_type
ts_list<ValueType, AllocType>::get_allocator(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.get_allocator();

} // get_allocator

template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::reference
ts_list<ValueType, AllocType>::front(
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.front();

} // front


template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::const_reference
ts_list<ValueType, AllocType>::front(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.front();

} // front


template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::reference
ts_list<ValueType, AllocType>::back(
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.back();

} // back


template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::const_reference
ts_list<ValueType, AllocType>::back(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_container.back();

} // back


template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::push_back(
    const value_type& _Val
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.push_back(_Val);

} // push_back

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::push_front(
    const value_type& _Val
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.push_front(_Val);

} // push_front

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::pop_back(
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.pop_back();

} // pop_back

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::pop_front(
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.pop_front();

} // pop_front

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::assign(
    size_type _Count,
    const value_type& _Val
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.assign(_Count, _Val);

} // assign


template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::iterator
ts_list<ValueType, AllocType>::insert(
    iterator _Where,
    const value_type& _Val
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.insert(_Where, _Val);

} // insert


template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::insert(
    iterator _Where,
    size_type _Count,
    const value_type& _Val
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.insert(_Where, _Count, _Val);

} // insert


template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::iterator
ts_list<ValueType, AllocType>::erase(
    iterator _Where
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.erase(_Where);

} // erase


template<typename ValueType, typename AllocType>
inline
typename ts_list<ValueType, AllocType>::iterator
ts_list<ValueType, AllocType>::erase(
    iterator _First,
    iterator _Last
    )
{
    mutex_lock lock(this->_mutex);
    return this->_container.erase(_First, _Last);

} // erase


template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::clear(
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.clear();

} // clear

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::remove(
    const value_type& _Val
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.remove(_Val);

} // remove

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::merge(
    my_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.merge(rhs._container);

} // merge

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::merge(
    container_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.merge(rhs);

} // merge

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::swap(
    container_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.swap(rhs);

} // swap


template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::swap(
    my_type& rhs
    )
{
    mutex_lock lock1(this->_mutex);
    mutex_lock lock2(rhs._mutex);
    this->_container.swap(rhs._container);

} // swap


template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::sort(
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.sort();

} // sort


template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::splice(
    iterator _Where,
    my_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.splice(_Where, rhs._container);

} // splice

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::splice(
    iterator _Where,
    my_type& rhs,
    iterator _First
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.splice(_Where, rhs._container, _First);

} // splice

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::splice(
    iterator _Where,
    my_type& rhs,
    iterator _First,
    iterator _Last
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.splice(_Where, rhs._container, _First, _Last);

} // splice

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::splice(
    iterator _Where,
    container_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.splice(_Where, rhs);

} // splice

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::splice(
    iterator _Where,
    container_type& rhs,
    iterator _First
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.splice(_Where, rhs, _First);

} // splice

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::splice(
    iterator _Where,
    container_type& rhs,
    iterator _First,
    iterator _Last
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.splice(_Where, rhs, _First, _Last);

} // splice

template<typename ValueType, typename AllocType>
inline
void
ts_list<ValueType, AllocType>::unique(
    )
{
    mutex_lock lock(this->_mutex);
    this->_container.unique();

} // unique



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
ts_list<ValueType, AllocType>::ts_list(
    ) :
        _container(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list

template<typename ValueType, typename AllocType>
ts_list<ValueType, AllocType>::ts_list(
    const container_type& held
    ) :
        _container(held),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list

template<typename ValueType, typename AllocType>
ts_list<ValueType, AllocType>::ts_list(
    const allocator_type& _Al
    ) :
        _container(_Al),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list

template<typename ValueType, typename AllocType>
ts_list<ValueType, AllocType>::ts_list(
    size_type _Count
    ) :
        _container(_Count),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list

template<typename ValueType, typename AllocType>
ts_list<ValueType, AllocType>::ts_list(
    size_type _Count,
    const value_type& _Val
    ) :
        _container(_Count, _Val),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list

template<typename ValueType, typename AllocType>
ts_list<ValueType, AllocType>::ts_list(
    size_type _Count,
    const value_type& _Val,
    const allocator_type& _Al
    ) :
        _container(_Count, _Val, _Al),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list

template<typename ValueType, typename AllocType>
ts_list<ValueType, AllocType>::ts_list(
    const my_type& rhs
    ) :
        _container(rhs._container),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{

} // ts_list






/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

