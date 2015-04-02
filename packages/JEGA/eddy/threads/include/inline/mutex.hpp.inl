/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Inline methods of class mutex.

    NOTES:

        See notes of mutex.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Mar 01 18:11:58 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the mutex class.
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

inline
void
mutex::lock(
    ) const
{
    int err_code = pthread_mutex_lock( &_mutex );
    if(err_code != 0) throw lock_error("Mutex failed to lock.", err_code);

} // mutex::lock

inline
void
mutex::unlock(
    ) const
{
    int err_code = pthread_mutex_unlock( &_mutex );
    if(err_code != 0) throw lock_error("Mutex failed to unlock.", err_code);

} // mutex::unlock










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
mutex::operator pthread_mutex_t (
    ) const
{
    return this->_mutex;
}

inline
mutex::operator pthread_mutex_t* (
    ) const
{
    return &this->_mutex;
}






/*
================================================================================
Inline Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

