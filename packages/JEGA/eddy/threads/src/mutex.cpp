/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Implementation of class mutex.

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
 * \brief Contains the implementation of the mutex class.
 */




/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include "../include/mutex.hpp"




/*
================================================================================
Namespace Using Directives
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
Static Member Data Definitions
================================================================================
*/








/*
================================================================================
Mutators
================================================================================
*/








/*
================================================================================
Accessors
================================================================================
*/








/*
================================================================================
Public Methods
================================================================================
*/






/*
================================================================================
Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Subclass Overridable Methods
================================================================================
*/








/*
================================================================================
Private Methods
================================================================================
*/








/*
================================================================================
Structors
================================================================================
*/



mutex::mutex(
    int type
    ) :
        _mutex()
{
    pthread_mutexattr_t attributes;

    // if the init of attributes fails, it is a resource error.
    if(pthread_mutexattr_init(&attributes) != 0)
        throw resource_error("mutex::mutex failed to init attributes.");

    // if the seting of attributes fails, it means sharing is invalid
    if(pthread_mutexattr_settype(&attributes, type) != 0)
        throw logical_error("mutex::mutex invalid type argument");

    // if the init of cond fails, it is a resource error.
    if(pthread_mutex_init( &_mutex, &attributes ) != 0)
        throw resource_error("mutex::mutex failed to init mutex.");

    // destroy the attributes object b/c it is not needed anymore.
    pthread_mutexattr_destroy(&attributes);

} // mutex::mutex



mutex::~mutex(
    ) throw()
{
    pthread_mutex_destroy(&_mutex);

} // mutex::~mutex




/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

