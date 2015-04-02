/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignGroupVector.

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

        Fri Oct 10 10:31:27 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignGroupVector class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNGROUPVECTOR_HPP
#define JEGA_UTILITIES_DESIGNGROUPVECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <vector>








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
namespace JEGA {
    namespace Utilities {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class DesignGroup;
class DesignGroupVector;




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
/// This is a container for pointers to DesignGroup's.
/**
 * It would be a typedef but I would like to forward declare
 * it and I don't want to have to typedef it in many places.
 * This class only provides constructors.
 */
class JEGA_SL_IEDECL DesignGroupVector :
    public std::vector<DesignGroup*>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the base class of this class
        typedef
        std::vector<DesignGroup*>
        base_type;

    protected:


    private:

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:





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

        std::size_t
        GetTotalDesignCount(
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


        /// Default constructs an empty DesignGroupVector.
        inline
        DesignGroupVector(
            );

        /// Constructs a DesignGroupVector which uses "_Al" for allocation.
        /**
         * \param _Al An allocator to duplicate for use in this vector.
         */
        explicit inline
        DesignGroupVector(
            const allocator_type& _Al
            );

        /**
         * \brief Constructs a DesignGroupVector with "_Count" default initial
         *        elements.
         *
         * \param _Count The number of initial elements for the vector each
         *               created with a default value.
         */
        explicit inline
        DesignGroupVector(
            size_type _Count
            );

        /**
         * \brief Constructs a DesignGroupVector with "_Count" copies of "_Val"
         *        as elements.
         *
         * \param _Count The number of initial elements for the vector each
         *               created with value "_Val".
         * \param _Val The initial value to assign to each element created.
         */
        inline
        DesignGroupVector(
            size_type _Count,
            const value_type& _Val
            );

        /**
         * \brief Constructs a DesignGroupVector with "_Count" copies of
         *        "_Val" as elements and using "_Al" for allocation.
         *
         * \param _Count The number of initial elements for the vector each
         *               created with value "_Val".
         * \param _Val The initial value to assign to each element created.
         * \param _Al An allocator to duplicate for use in this vector.
         */
        inline
        DesignGroupVector(
            size_type _Count,
            const value_type& _Val,
            const allocator_type& _Al
            );

        /// Copy constructs a DesignGroupVector
        /**
         * \param _Right The existing DesignGroupVector to copy properties from
         *               into this.
         */
        inline
        DesignGroupVector(
            const DesignGroupVector& _Right
            );

        /**
         * \brief Constructs a DesignGroupVector consisting of copies of
         *        the elements in the range [first, last).
         *
         * \param _First The first element in the range to be copied into this.
         *               _First will be copied.
         * \param _Last One past the last element in the range to be copied
         *              into this.  _Last will not be copied.
         */
        template<class InputIterator>
        inline
        DesignGroupVector(
            InputIterator _First,
            InputIterator _Last
            );

        /**
         * \brief Constructs a DesignGroupVector consisting of copies of the
         *        elements in the range [first, last) using "_Al" for
         *        allocation.
         *
         * \param _First The first element in the range to be copied into this.
         *               _First will be copied.
         * \param _Last One past the last element in the range to be copied
         *              into this.  _Last will not be copied.
         * \param _Al An allocator to duplicate for use in this vector.
         */
        template<class InputIterator>
        inline
        DesignGroupVector(
            InputIterator _First,
            InputIterator _Last,
            const allocator_type& _Al
            );



}; // class DesignGroupVector



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "./inline/DesignGroupVector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNGROUPVECTOR_HPP
