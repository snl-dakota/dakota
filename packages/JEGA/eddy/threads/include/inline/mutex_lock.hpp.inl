/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class mutex_lock.

    NOTES:

        See notes of mutex_lock.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sat Sep 13 14:02:10 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the mutex_lock class.
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

inline
void
mutex_lock::lock(
    )
{
    this->_mutex->lock();

} // mutex_lock::lock

inline
void
mutex_lock::unlock(
    )
{
    this->_mutex->unlock();

} // mutex_lock::unlock

inline
mutex&
mutex_lock::get_mutex(
    )
{
    return *this->_mutex;

} // mutex_lock::get_mutex





/*
================================================================================
Inline Structors
================================================================================
*/



inline
mutex_lock::mutex_lock(
    mutex& mutex
    ) :
        _mutex(&mutex)
{
    // lock the mutex and allow any errors to pass through.
    this->lock();

} // mutex_lock::mutex_lock


inline
mutex_lock::~mutex_lock(
    ) throw()
{
    // try to unlock and don't allow any errors through.
    try
    {
        this->unlock();
    }
    catch(...){}

} // mutex_lock::~mutex_lock





/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

