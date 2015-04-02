/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class BooleanDesignVariableType.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Thu Oct 26 15:34:46 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the BooleanDesignVariableType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_BOOLEANDESIGNVARIABLETYPE_HPP
#define JEGA_UTILITIES_BOOLEANDESIGNVARIABLETYPE_HPP







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
class BooleanDesignVariableType;







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

/// A design variable type for Boolean variables (true or false).
/**
 * The design variable values represented by this type are Boolean.
 * Although they are represented as real numbers, they are formatted
 * such that they are always true or false (1 or 0 respecitively).
 */
class BooleanDesignVariableType :
    public DesignVariableTypeBase
{
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

        /// Returns the string name of this nature.
        /**
         * \return The string "Integer".
         */
        virtual
        std::string
        ToString(
            ) const;

        /**
         * \brief Overridden to prevent the addition of discrete values to
         *        variables of this type.
         *
         * 0 and 1 are the only values ever allowed.
         *
         * \param value The value to be ignored.
         * \return false, always.
         */
        virtual
        bool
        AddDiscreteValue(
            double value
            );

        /// Sets the desired decimal precision for this type.
        /**
         * This method will fail if IsPrecisionLocked returns true.
         *
         * In the case of a Boolean type variable, only 0 is allowed for
         * \a prec.
         *
         * \param prec The new number of decimal places of interest for this
         *             variable.
         * \return True if the precision is adopted and false otherwise.
         */
        virtual
        bool
        SetPrecision(
            eddy::utilities::int16_t prec
            );
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

        /**
         * \brief Returns true if \a rep is the representation of a valid value
         *        for this variable.
         *
         * Valid representations are those that may be returned by
         * GetRandomDoubleRep.
         *
         * \param rep The representation to check for validity with this
         *            variable type.
         * \return true if \a rep is a valid representation for this variable
         *         and false otherwise.
         */
        virtual
        bool
        IsValidDoubleRep(
            double rep
            ) const;

        /// Returns an exact duplicate of this type object.
        /**
         * \param forDVI The DesignVariableInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by \a forDVI.
         */
        virtual
        DesignVariableTypeBase*
        Clone(
            DesignVariableInfo& forDVI
            ) const;

        /// Returns the value represented by \a rep as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param rep The representation to convert to a value.
         * \return 0.0 if \a rep is 0.0 and 1.0 otherwise.
         */
        virtual
        double
        GetValueOf(
            double rep
            ) const;

        /// Returns the nearest valid value to \a value.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param value The value to correct to a valid value.
         * \return The nearest value to \a value for which IsValidValue will
         *         return true;
         */
        virtual
        double
        GetNearestValidValue(
            double value
            ) const;

        /// Returns the nearest valid double rep to \a rep.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param rep The representation to correct to a valid representation.
         * \return The nearest representation to \a rep for which
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

        /// Returns the proper representation of \a value as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param value The value to retrieve the representation of.
         * \return The representation of the value \a value.
         */
        virtual
        double
        GetDoubleRepOf(
            double value
            ) const;

        /// Returns the representation of a random value as a double.
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
         * \return The representation of a random value inside \a within.
         */
        virtual
        double
        GetRandomDoubleRep(
            const RegionOfSpace& within
            ) const;

        /**
         * \brief Sets the lower bound or smallest value for this variable to
         *        \a value.
         *
         * This is implemented by passing along the request to the nature.
         *
         * \param value The new minimum value for this variable.
         */
        virtual
        void
        SetMinValue(
            double value
            );

        /**
         * \brief Sets the upper bound or smallest value for this variable to
         *        \a value.
         *
         * This is implemented by passing along the request to the nature.
         *
         * \param value The new maximum value for this variable.
         */
        virtual
        void
        SetMaxValue(
            double value
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

        /// Constructs a BooleanDesignVariableType for use by \a info.
        /**
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        BooleanDesignVariableType(
            DesignVariableInfo& info
            );

        /**
         * \brief Copy constructs an BooleanDesignVariableType to be known by
         *        \a info.
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        BooleanDesignVariableType(
            const BooleanDesignVariableType& copy,
            DesignVariableInfo& info
            );





}; // class BooleanDesignVariableType



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
#include "inline/BooleanDesignVariableType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_BOOLEANDESIGNVARIABLETYPE_HPP
