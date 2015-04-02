/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class mutexed_variable.

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

        Sun Sep 14 14:16:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the mutexed_variable class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_MUTEXED_VARIABLE_HPP
#define EDDY_THREADS_MUTEXED_VARIABLE_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include "mutex_lock.hpp"






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
template <typename T>
class mutexed_variable;



/*
================================================================================
File Scope Typedefs
================================================================================
*/
/// A mutex protected boolean type integer variable.
typedef
mutexed_variable<bool>
MutexedBool;

/// A mutex protected int type variable.
typedef
mutexed_variable<int>
MutexedInt;

/// A mutex protected unsigned int type variable.
typedef
mutexed_variable<unsigned int>
MutexeduInt;

/// A mutex protected long type variable.
typedef
mutexed_variable<long>
MutexedLong;

/// A mutex protected unsigned long type variable.
typedef
mutexed_variable<unsigned long>
MutexeduLong;

/// A mutex protected float type variable.
typedef
mutexed_variable<float>
MutexedFloat;

/// A mutex protected double type variable.
typedef
mutexed_variable<double>
MutexedDouble;


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

/// A template class to act as a mutex protected variable of arbitrary type.
/**
 * The supplied type must support:
 * - assignment
 * - copy construction
 */
template <typename T>
class EDDY_SL_IEDECL mutexed_variable
{


    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /// A shorthand for this type.
        typedef
        mutexed_variable<T>
        my_type;

        /// A shorthand for the underlying data type.
        typedef
        T
        held_type;

        /// A shorthand for a reference to the data type.
        typedef
        T&
        reference;

        /// A shorthand for a constant reference to the data type.
        typedef
        const T&
        const_reference;

    protected:

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

        /// The mutex used to protect the value stored.
        mutable mutex _mutex;

        /// The actual underlying data.
        held_type _value;




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


        /// Sets the current value to \a to.
        /**
         * \param to The new value for the protected variable.
         */
        inline
        held_type
        set(
            const_reference to
            );

        /// Returns a duplicate of the current value.
        /**
         * \return The currently held value.
         */
        inline
        held_type
        value(
            ) const;

        /// Returns a duplicate of the current value.
        /**
         * \return The currently held value.
         */
        inline
        held_type
        operator ()(
            ) const;

        /// Allows for explicit casting from mutexed type to held type.
        inline
        operator held_type(
            ) const;

        /// Assigns the held value to \a rhs.
        /**
         * \param rhs An existing instance of type T to copy properties from
         *            into the instance held here.
         * \return This mutex protected variable after assignment is complete.
         */
        inline
        const my_type&
        operator = (
            const_reference rhs
            );

        /// Assigns this to \a rhs.
        /**
         * \param rhs An existing instance of this type to copy properties from
         *            into this.
         * \return This mutex protected variable after assignment is complete.
         */
        inline
        const my_type&
        operator = (
            const my_type& rhs
            );

        /// Locks the mutex of this variable.
        /**
         * Use this method to lock the variable over a series of operations.
         * Always pair this method with a subsequent call to unlock.
         *
         * This method may throw a lock_error if the mutex cannot be locked.
         */
        inline
        held_type
        lock(
            ) const;

        /// Unlocks the mutex of this variable.
        /**
         * Always pair this method with a previous call to lock.
         *
         * This method may throw a lock_error if the mutex cannot be unlocked.
         */
        inline
        held_type
        unlock(
            ) const;

        /// Creates and returns a scoped locking object for this variable.
        /**
         * The returned object will have locked this variable when it was
         * created and will unlock it when it goes out of scope or is otherwise
         * destroyed.
         *
         * Be sure to store the return in a local variable or the unlock will
         * occur immediately.
         *
         * \return An object that locked the mutex upon creation and that will
         *         unlock it when destroyed.
         */
        inline
        mutex_lock
        scoped_lock(
            ) const;


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


        /// Default constructs a mutex protected variable.
        inline
        mutexed_variable(
            );

        /**
         * \brief Constructs a mutex protected variable with a resulting value
         *        equal to \a value.
         *
         * \param value The value to be used in constructing this protected
         *              variable.
         */
        inline
        mutexed_variable(
            const_reference value
            );

        /// Copy constructs a mutex protected variable.
        /**
         * \param copy The existing instance of this type to duplicate into
         *             this.
         */
        inline
        mutexed_variable(
            const my_type& copy
            );

        /// Destructs a mutex protected variable.
        inline
        ~mutexed_variable(
            );


}; // class mutexed_variable



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
#include "./inline/mutexed_variable.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_MUTEXED_VARIABLE_HPP
