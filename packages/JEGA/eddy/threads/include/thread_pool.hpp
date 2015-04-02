/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class thread_pool.

    NOTES:

        This project is modeled heavily after the boost thread library.  Some
        Serious shortcommings of the boost thread library make this one
        necessary such as the lack of ability to cancel threads.

        Hopefully, the shortcommings of the boost library will be taken care of
        at which time this library will likely be changed to use it.

        Also, see notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Mar 01 12:06:47 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the thread_pool class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_THREAD_POOL_HPP
#define EDDY_THREADS_THREAD_POOL_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include "mutex.hpp"
#include "thread.hpp"
#include "condition.hpp"

#include "ts_map.hpp"
#include "ts_list.hpp"


// Not all implementations of pthreads define this constant.  So use a default
// large value if not.
#ifndef PTHREAD_THREADS_MAX
/// The maximum allowable number of concurrent threads.
/**
 * This is only defined here if not defined in pthread.h.  The value of 1024
 * is simply a chosen value and is basically meaningless.
 */
#define PTHREAD_THREADS_MAX 1024
#endif

/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/








/*
================================================================================
Namespace Aliases
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
In-Namespace Forward Declares
================================================================================
*/
class thread;
class thread_pool;







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/







/*
================================================================================
Class Definition
================================================================================
*/
/// A convenience class for maintaining a pool of threads.
/**
 * This class is able to store a collection of created threads and
 * provide them to requesters whenever needed.
 *
 * This should be considered a buffer in which Threads may be stored
 * temporarily until needed.
 */
class EDDY_SL_IEDECL thread_pool
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

    private:

        /// The container used to hold the threads
        typedef
        ts_list<thread*>
        thread_list;

        typedef
        ts_map<thread*, condition*>
        cond_map;

        typedef
        ts_map<thread*, mutex*>
        mutex_map;

    protected:

    /*
    ============================================================================
    Nested Utility Class Declarations
    ============================================================================
    */
    private:




    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The pooled Threads.
        thread_list _idle_threads;

        thread_list _used_threads;

        cond_map _conditions;

        mutex_map _mutexes;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a thread_pool managing no more than \a maxThreads
         *        threads.
         */
        thread_pool(
            unsigned int maxThreads = PTHREAD_THREADS_MAX
            );


        /// Destructs a thread_pool.
        ~thread_pool(
            ) throw();


}; // class thread_pool



/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "./inline/thread_pool.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_THREAD_POOL_HPP
