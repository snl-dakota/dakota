/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class scoped_lock_concept.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sat Jun 05 08:44:24 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the scoped_lock_concept class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_SCOPED_LOCK_CONCEPT_HPP
#define EDDY_THREADS_SCOPED_LOCK_CONCEPT_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"








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
template <typename Cont>
class scoped_lock_concept;







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

/**
 * \brief A scoped lock for objects that support the lock() and unlock()
 * method interface.
 *
 * The type \a Cont must be an object that has the following methods
 * implemented:
 * \code
    <whatever return type>
    lock(
        ) const;

    <whatever return type>
    unlock(
        ) const;
   \endcode
 * An instance of the container type must be passed into the constructor at
 * which time the lock method is called.  Upon destruction, the unlock method
 * is called on that same instance.  Be careful not to unlock the container
 * explicitly while the scoped lock is active unless of course it is a
 * recursive mutex and you have also locked it during that time as well.
 */
template <typename Cont>
class EDDY_SL_IEDECL scoped_lock_concept
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A shorthand for the locking container type.
        typedef
        Cont
        container_type;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The container that will be locked and unlocked by this object.
        const container_type& _container;


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a scoped lock for a container and locks it.
        /**
         * \param container The container to lock for the life of this object.
         */
        inline
        scoped_lock_concept(
            const container_type& container
            ) :
                _container(container)
        {
            _container.lock();
        }

        /// Destructs a scoped lock for a container which unlocks it.
        inline
        ~scoped_lock_concept(
            )
        {
            _container.unlock();
        }

}; // class scoped_lock_concept



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
// no inline methods.  All implementations in-class.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_SCOPED_LOCK_CONCEPT_HPP
