/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class keyed_registry.

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

        Thu May 15 07:29:38 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the keyed_registry class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_KEYED_REGISTRY_HPP
#define EDDY_UTILITIES_KEYED_REGISTRY_HPP







/*
================================================================================
Includes
================================================================================
*/
#include <map>
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

/// This class can be used to maintain a keyed registry of any type.
/**
 * The methods available through this class provide a means of storing
 * a registry of key-value pairs with unique keys.  Elements can be added
 * (registered), removed (unregistered), or retrieved (found).
 */
template <typename KeyType, typename ValueType>
class keyed_registry
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /// The type of this class (short hand)
        typedef
        keyed_registry<KeyType, ValueType>
        my_type;

        /// Type of the underlying container used.
        typedef
        std::map<KeyType, ValueType>
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

        /// Checks for a logically equivolent entry to key
        /**
         * \param key The key to check for registration in this.
         * \return True if \a key is associated with a registree here and false
         *         otherwise.
         */
        bool
        is_registered(
            const KeyType& key
            ) const;

        /**
         * \brief Adds the key-value pair to the registry if the key is not
         *        already present.
         *
         * \param key The key by which to register \a value.
         * \param value The value to be registred and retrievable by \a key.
         * \return True if \a value is successfully registered and false
         *         otherwise.  Failure occurs if \a key is already used.
         */
        bool
        register_(
            const KeyType& key,
            const ValueType& value
            );

        /// Removes \a key and the associated value from the registry.
        /**
         * \param key The key whose value to unregister in this registry.
         * \return True if \a key and its associated value are successfully
         *         unregistered and false otherwise.  Failure occurs if
         *         \a key was not registered to begin with.
         */
        bool
        unregister(
            const KeyType& key
            );

        /**
         * \brief Returns an iterator pointing to key-value pair in the
         *        registry or end() if not found (const).
         *
         * \param key The key whose key,value pair is to be found in this
         *            registry.
         * \return Iterator to the location of the key,value pair of \a key or
         *         end if not found.
         */
        const_iterator
        find(
            const KeyType& key
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
         * \brief Returns an iterator pointing to key-value in the registry
         *        or end() if not found (non-const).
         *
         * This method allows subclass access to mutable iterators describing
         * the locations of registrees.
         *
         * \param key The key whose key,value pair is to be found in this
         *            registry.
         * \return Iterator to the location of the key,value pair of \a key or
         *         end if not found.
         */
        iterator
        find(
            const KeyType& key
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

        /// Default construct an empty registry.
        keyed_registry(
            );

        /// Copy construct a registry.
        /**
         * \param copy The existing keyed_registry to copy into this.
         */
        keyed_registry(
            const my_type& copy
            );




}; // class keyed_registry


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
#include "./inline/keyed_registry.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_KEYED_REGISTRY_HPP
