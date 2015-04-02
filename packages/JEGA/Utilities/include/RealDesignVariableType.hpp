/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RealDesignVariableType.

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

        Tue Jun 03 08:55:22 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RealDesignVariableType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_REALDESIGNVARIABLETYPE_HPP
#define JEGA_UTILITIES_REALDESIGNVARIABLETYPE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/DesignVariableTypeBase.hpp>






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
class RealDesignVariableType;







/*
================================================================================
Class Definition
================================================================================
*/

/// A design variable type for real numbers.
/**
 * The design variable values represented by this type truly are real numbers.
 * Although it is expected that all variable types can be represented as a
 * real number, these actually are.
 */
class JEGA_SL_IEDECL RealDesignVariableType :
    public DesignVariableTypeBase
{
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





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





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

        /// Returns the string name of this nature.
        /**
         * \return The string "Real".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns true if the nature of this type cannot be changed.
        /**
         * For example, some variable types may be discrete only.
         *
         * \return false if the nature can be changed and true otherwise.
         */
        virtual
        bool
        IsNatureLocked(
            ) const;

        /// Returns an exact duplicate of this type object.
        /**
         * \param forDVI The DesignVariableInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by "forDVI".
         */
        virtual
        DesignVariableTypeBase*
        Clone(
            DesignVariableInfo& forDVI
            ) const;

        /// Returns the value represented by "rep" as a double.
        /**
         * A return of -DBL_MAX indicates failure.  The returned
         * value is not necessarily valid.  It may be out of bounds
         * etc.
         *
         * \param rep The representation to convert to a value.
         * \return The value associated with or represented by "rep".
         */
        virtual
        double
        GetValueOf(
            double rep
            ) const;

        /// Returns the nearest valid value to "value".
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param value The value to correct to a valid value.
         * \return The nearest value to "value" for which IsValidValue will
         *         return true;
         */
        virtual
        double
        GetNearestValidValue(
            double value
            ) const;

        /// Returns the nearest valid double rep to "rep".
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param rep The representation to correct to a valid representation.
         * \return The nearest representation to "rep" for which
         *         IsValidDoubleRep will return true;
         */
        virtual
        double
        GetNearestValidDoubleRep(
            double rep
            ) const;

        /// Returns a random valid value for this type as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return A random value for this variable for which IsValidValue will
         *         return true;
         */
        virtual
        double
        GetRandomValue(
            ) const;

        /// Returns the proper representation of "value" as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param value The value to retrieve the representation of.
         * \return The representation of the value "value".
         */
        virtual
        double
        GetDoubleRepOf(
            double value
            ) const;

        /// Returns the representation of a random "value" as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of a random value.
         */
        virtual
        double
        GetRandomDoubleRep(
            ) const;

        /**
         * \brief Returns a random representation existing within the supplied
         *        region of space.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \param within The region of space in which to restrict the value.
         * \return The representation of a random value inside \a within.
         */
        virtual
        double
        GetRandomDoubleRep(
            const RegionOfSpace& within
            ) const;


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

        /// Constructs a RealDesignVariableType for use by "info".
        /**
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        RealDesignVariableType(
            DesignVariableInfo& info
            );

        /// Copy constructs an RealDesignVariableType to be known by "info".
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        RealDesignVariableType(
            const RealDesignVariableType& copy,
            DesignVariableInfo& info
            );



}; // class RealDesignVariableType


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/RealDesignVariableType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_REALDESIGNVARIABLETYPE_HPP
