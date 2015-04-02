/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Implementation of class condition.

    NOTES:

        See notes of condition.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Mar 01 18:50:46 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the condition class.
 */




/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <cerrno>
#include "../include/condition.hpp"
#include "../include/mutex_lock.hpp"

#if defined(_WIN32)
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;







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

void
condition::wait(
    mutex_lock& lock
    )
{
    int success = pthread_cond_wait(&_condition, lock.get_mutex());
    if(success != 0)
    {
        if(success == EINVAL)
            throw lock_error("condition::wait - Wait returned EINVAL.");
        else
            throw lock_error("condition::wait - Unspecified error.");
    }

} // condition::wait

bool
condition::timed_wait(
    mutex_lock& lock,
    unsigned int msecs
    )
{

    // prepare a timespec for use in the timed wait.
    timespec ts = {0};

#if defined(_WIN32)
    struct __timeb64 tp = {0};

#   if _MSC_VER >= 1400
        _ftime_s(&tp);
#   else
        _ftime64(&tp);
#   endif

    ts.tv_sec = static_cast<long>(tp.time + msecs/1000);
    ts.tv_nsec = ((msecs%1000)+tp.millitm)*1000000;
#else
    struct timeval tp = {0};
    gettimeofday(&tp, 0);
    ts.tv_sec = tp.tv_sec + msecs/1000;
    // tp.tv_usec is in micro seconds (x10^-6);
    // msecs is in milli seconds (x10^-3);
    // need nano seconds (x10^-9);
    ts.tv_nsec = ((msecs%1000)*1000+tp.tv_usec)*1000;
#endif

    // now execute the wait
    // The status is ok if the wait is successful or just timed out.
    // the return is different however.
    int success = pthread_cond_timedwait(&_condition, lock.get_mutex(), &ts);
    if((success != 0) && (success != ETIMEDOUT))
    {
        if(success == EINVAL)
            throw lock_error("condition::timed_wait - Wait returned EINVAL.");
        else
            throw lock_error("condition::timed_wait - Unspecified error.");
    }
    return (success == 0) ? true : false;

} // condition::timed_wait

void
condition::notify_one(
    )
{
    if(pthread_cond_signal(&_condition) != 0)
        throw lock_error("condition::notify_one - signal failed.");

} // condition::notify_one

void
condition::notify_all(
    )
{
    if(pthread_cond_broadcast(&_condition) != 0)
        throw lock_error("condition::notify_all - broadcast failed.");

} // condition::notify_all







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

condition::condition(
    int sharing
    ) :
        _condition()
{
    pthread_condattr_t attributes;

    // if the init of attributes fails, it is a resource error.
    if(pthread_condattr_init(&attributes) != 0)
        throw resource_error("condition::condition failed to init attributes.");

    // if the seting of attributes fails, it means sharing is invalid
    if(pthread_condattr_setpshared(&attributes, sharing) != 0)
        throw logical_error("condition::condition invalid sharing argument");

    // if the init of cond fails, it is a resource error.
    if(pthread_cond_init( &_condition, &attributes ) != 0)
        throw resource_error("condition::condition failed to init condition.");

    // destroy the attributes object b/c it is not needed anymore.
    pthread_condattr_destroy(&attributes);

} // condition::condition


condition::~condition(
    ) throw()
{
    pthread_cond_destroy(&_condition);

} // condition::~condition








/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

