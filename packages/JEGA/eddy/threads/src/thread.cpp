/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Implementation of class thread.

    NOTES:

        See notes of thread.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Mar 01 15:39:24 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the thread class.
 */




/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <ctime>
#include <cerrno>
#include "../include/thread.hpp"
#include "../include/condition.hpp"
#include "../include/mutex_lock.hpp"

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;




/*
================================================================================
File Scope Operator Overloads
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


thread::old_cancel_state_map&
thread::old_cancel_types(
    )
{
    static old_cancel_state_map map;
    return map;

} // thread::old_cancel_types





/*
================================================================================
Public Methods
================================================================================
*/

void
thread::sleep(
    unsigned int msecs
    )
{
    struct timespec ts = { msecs/1000, (msecs%1000)*1000000 };

#if defined(EDDY_WINDOWS)
    pthread_delay_np(&ts);
#else
    nanosleep(&ts, 0);
#endif

} // thread::sleep

void
thread::yield(
    )
{
    sched_yield();

} // thread::yield

void
thread::cancel(
    )
{
    if(pthread_cancel(_thread) != 0)
        throw resource_error("thread::cancel - could not cancel thread.");
    old_cancel_types().erase(&_thread);

} // thread::cancel

void
thread::set_asynch_cancelation(
    )
{
    set_cancelation(PTHREAD_CANCEL_ASYNCHRONOUS);

} // thread::set_asynch_cancelation

void
thread::set_deferred_cancelation(
    )
{
    set_cancelation(PTHREAD_CANCEL_DEFERRED);

} // thread::set_deferred_cancelation

void
thread::disable_cancelation(
    )
{
    set_cancelation(PTHREAD_CANCEL_DISABLE);

} // thread::disable_cancelation

void
thread::enable_cancelation(
    )
{
    set_cancelation(PTHREAD_CANCEL_ENABLE);

} // thread::enable_cancelation

void
thread::restore_cancelation_state(
    )
{
    pthread_t self = pthread_self();
    pthread_setcanceltype(
        old_cancel_types()[&self],
        &old_cancel_types()[&self]
        );

} // thread::restore_cancelation_state

const thread&
thread::operator =(
    const thread& other
    )
{
    if(this == &other) return other;
    _thread = other._thread;
    return *this;
}

bool
thread::is_this_thread(
    ) const
{
    return pthread_equal(_thread, pthread_self()) != 0;

} // thread::is_this_thread

bool
thread::operator ==(
    const thread& other
    ) const
{
    return pthread_equal(_thread, other._thread ) != 0;
}

bool
thread::operator !=(
    const thread& other
    ) const
{
    return !(operator ==(other));
}

void
thread::join(
    thread& other
    )
{
    pthread_join(other._thread, 0x0);

} // thread::join





/*
================================================================================
Subclass Visible Methods
================================================================================
*/

void
thread::set_cancelation(
    int state
    )
{
    int old = 0;
    pthread_setcanceltype(state, &old);
    pthread_t self = pthread_self();
    old_cancel_types()[&self] = old;
}







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

thread::thread(
    ) :
        _thread()
{

    _thread = pthread_self();

    // This thread may or may not already be in the map but that's not a
    // problem.  It willnot be let in again b/c of the workings of the map.
    old_cancel_types().insert(
        old_cancel_state_map::value_type(&_thread, PTHREAD_CANCEL_DEFERRED)
        );

} // thread::thread

thread::thread(
    const thread& copy
    ) :
        _thread(copy._thread)
{
    // _thread had better already be in the cancel state map.

} // thread::thread

thread::thread(
    main_func func,
    void* data,
    int detachstate,
    int inheritsched
    ) :
        _thread()
{
    pthread_attr_t attributes;

    // if the init of attributes fails, it is a resource error.
    if(pthread_attr_init(&attributes) != 0)
        throw resource_error("thread::thread failed to init attributes.");

    // if the seting of attributes fails, it means sharing is invalid
    if(pthread_attr_setdetachstate(&attributes, detachstate) != 0)
        throw logical_error("thread::thread invalid detachstate argument");

    // if the seting of attributes fails, it means sharing is invalid
    if(pthread_attr_setinheritsched(&attributes, inheritsched) != 0)
        throw logical_error("thread::thread invalid inheritsched argument");

    // if the init of cond fails, it is a resource error.
    if(pthread_create(&_thread, &attributes, func, data) != 0)
        throw resource_error("thread::thread failed to create thread.");

    // The new thread was created so account for it in our cancel state map.
    old_cancel_types().insert(
        old_cancel_state_map::value_type(&_thread, PTHREAD_CANCEL_DEFERRED)
        );

    // destroy the attributes object b/c it is not needed anymore.
    pthread_attr_destroy(&attributes);

} // thread::thread

thread::~thread(
    ) throw()
{
} // thread::~thread





/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

