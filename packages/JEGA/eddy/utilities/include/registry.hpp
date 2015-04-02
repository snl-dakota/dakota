/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class registry.

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 14 14:47:06 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the registry class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_REGISTRY_HPP
#define EDDY_UTILITIES_REGISTRY_HPP







/*
================================================================================
Includes
================================================================================
*/
#include <set>
#include "config.hpp"








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
    namespace utilities {







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/

/// This class can be used to maintain a registry of any type.
/**
 * The methods available through this class provide a means of storing
 * a registry of unique values.  Elements can be added (registered), removed
 * (unregistered), or retrieved (found).
 */
template <typename ValueType>
class registry
{

    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /// The type of this class (short hand)
        typedef
        registry<ValueType>
        my_type;

        /// Type of the underlying container used.
        typedef
        std::set<ValueType>
        container;

        /// Type of the values being held in this container
        typedef
        typename container::value_type
        value_type;

        /// Type that can be used as a contant iterator over this container
        typedef
        typename container::const_iterator
        const_iterator;

        /// Type representing the size of this container
        typedef
        typename container::size_type
        size_type;

    protected:

        /// Type that can be used as an iterator over this container
        /**
         * This type is protected because it is not permitted to modify
         * this container except throught the provided methods.
         */
        typedef
        typename container::iterator
        iterator;


    private:




    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The underlying container in which elements are stored.
        container _registry;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Get an iterator to the first element in the container (const)
        /**
         * \return An iterator to the first element or end if none.
         */
        const_iterator
        begin(
            ) const;

        /// Get an iterator to the last element in the container (const)
        /**
         * \return An iterator to one past the last element.
         */
        const_iterator
        end(
            ) const;

        /// Get the current number of elements in the container
        /**
         * \return The number of elements in the registry.
         */
        size_type
        size(
            ) const;

    protected:

        /**
         * \brief Get the actual underlying container used to store registerees
         *        (non-const).
         *
         * This method allows mutable subclass access to the underlying
         * container.
         *
         * \return A reference to the container used to store registrees.
         */
        container&
        get_container(
            );

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Checks for a logically equivolent entry to value
        /**
         * \param value The value to check for registration in this.
         * \return True if \a value is registered here and false otherwise.
         */
        bool
        is_registered(
            const ValueType& value
            ) const;

        /// Adds value to the registry if it is not already present.
        /**
         * \param value The value to register in this registry.
         * \return True if \a value is successfully registeredand false
         *         otherwise.  Failure occurs if \a value is already registered.
         */
        bool
        register_(
            const ValueType& value
            );

        /// Removes value from the registry.
        /**
         * \param value The value to unregister in this registry.
         * \return True if \a value is successfully unregistered and false
         *         otherwise.  Failure occurs if \a value was not registered
         *         to begin with.
         */
        bool
        unregister(
            const ValueType& value
            );

        /**
         * \brief Returns an iterator pointing to \a value in the registry or
         *        end if not found (const).
         *
         * \param value The value to find in this registry.
         * \return Iterator to \a value or end if not found.
         */
        const_iterator
        find(
            const ValueType& value
            ) const;

        /// Assigns the entries of \a rhs to this registry.
        /**
         * This method allows direct assignment of one registry to another.
         *
         * \param rhs The other registry to copy properties from into this.
         * \return This registry after assignment.
         */
        const my_type&
        operator = (
            const my_type& rhs
            );

        /// Returns true if the container has size 0 and false otherwise.
        /**
         * \return True if this registry is empty and false otherwise.
         */
        bool
        empty(
            ) const;

        /// Assimilates the registrees of \a other into this.
        /**
         * Recall that duplicates are not permitted and so the return
         * is the number of successful assimilations.
         *
         * \param other The other registry whose members are to be registered
         *              in here.
         * \return The number of new registrations that occurred.
         */
        size_type
        merge(
            const my_type& other
            );

        /// This method empties all registered items from this registry.
        void
        clear(
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Returns an iterator pointing to \a value in the registry or
         *        end if not found (non-const).
         *
         * This method allows subclass access to mutable iterators describing
         * the locations of registrees.
         *
         * \param value The registree to find in this registry.
         * \return An iterator to \a value's location in this registry or end
         *         if it is not found.
         */
        iterator
        find(
            const ValueType& value
            );




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

        /// Default constructs an empty registry.
        registry(
            );

        /// Copy constructs a registry.
        /**
         * \param copy The existing registry to copy into this.
         */
        registry(
            const my_type& copy
            );



}; // class registry


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/registry.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_REGISTRY_HPP
