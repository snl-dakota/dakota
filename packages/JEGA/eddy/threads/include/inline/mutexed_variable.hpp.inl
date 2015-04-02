/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class mutexed_variable.

    NOTES:

        See notes of mutexed_variable.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Sep 14 14:16:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the mutexed_variable class.
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


template <typename T>
inline
typename mutexed_variable<T>::held_type
mutexed_variable<T>::set(
    const T& to
    )
{
    mutex_lock lock(this->_mutex);
    return (this->_value = to);

} // set

template <typename T>
inline
T
mutexed_variable<T>::value(
    ) const
{
    mutex_lock lock(this->_mutex);
    return this->_value;

} // value

template <typename T>
inline
mutexed_variable<T>::operator T(
    ) const
{
    return this->value();

}

template <typename T>
inline
T
mutexed_variable<T>::operator ()(
    ) const
{
    return this->value();
}

template <typename T>
inline
const mutexed_variable<T>&
mutexed_variable<T>::operator = (
    const T& rhs
    )
{
    mutex_lock lock(this->_mutex);
    this->_value = rhs;
    return *this;
}

template <typename T>
inline
const mutexed_variable<T>&
mutexed_variable<T>::operator = (
    const my_type& rhs
    )
{
    mutex_lock lock(this->_mutex);
    mutex_lock rlock (&rhs._mutex);

    if(this == &rhs) return *this;
    this->_value = rhs._value;

    return *this;
}

template <typename T>
inline
typename mutexed_variable<T>::held_type
mutexed_variable<T>::lock(
    ) const
{
    this->_mutex.lock();
    return this->_value;
}

template <typename T>
inline
typename mutexed_variable<T>::held_type
mutexed_variable<T>::unlock(
    ) const
{
    held_type ret(this->_value);
    this->_mutex.unlock();
    return ret;
}

template <typename T>
inline
mutex_lock
mutexed_variable<T>::scoped_lock(
    ) const
{
    return mutex_lock(this->_mutex);
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



template <typename T>
mutexed_variable<T>::mutexed_variable(
    ) :
        _value(),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{
}

template <typename T>
mutexed_variable<T>::mutexed_variable(
    const T& ival
    ) :
        _value(ival),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{
}

template <typename T>
mutexed_variable<T>::mutexed_variable(
    const my_type& copy
    ) :
        _value(copy.value()),
        _mutex(PTHREAD_MUTEX_RECURSIVE)
{
}

template <typename T>
mutexed_variable<T>::~mutexed_variable(
    )
{
}








/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

