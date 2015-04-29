/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class IntegerDesignVariableType.

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

        Tue Jun 03 08:55:28 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the IntegerDesignVariableType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_INTEGERDESIGNVARIABLETYPE_HPP
#define JEGA_UTILITIES_INTEGERDESIGNVARIABLETYPE_HPP







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
class IntegerDesignVariableType;







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

/// A design variable type for integral numbers.
/**
 * The design variable values represented by this type are integers.
 * Although they are represented as real numbers, they are formatted
 * such that there are no digits to the right of the radix.
 */
class JEGA_SL_IEDECL IntegerDesignVariableType :
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
         * \return The string "Integer".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Sets the desired decimal precision for this type.
        /**
         * This method will fail if IsPrecisionLocked returns true.
         *
         * In the case of an Integer type variable, only 0 or negative numbers
         * are allowed for \a prec which has the effect of only allowing
         * integers but may allow something like "only every 10" or "only every
         * 100".
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
         * A return of -DBL_MAX indicates failure.  The returned
         * value is not necessarily valid.  It may be out of bounds
         * etc.
         *
         * \param rep The representation to convert to a value.
         * \return The value associated with or represented by \a rep.
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
         * \param within The region of space in which to restrict the value.
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

    protected:


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        double
        ubround(
            const double& value,
            const double& min,
            const double& max
            ) const;

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a IntegerDesignVariableType for use by \a info.
        /**
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        IntegerDesignVariableType(
            DesignVariableInfo& info
            );

        /**
         * \brief Copy constructs an IntegerDesignVariableType to be known by
         *        \a info.
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        IntegerDesignVariableType(
            const IntegerDesignVariableType& copy,
            DesignVariableInfo& info
            );



}; // class IntegerDesignVariableType


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
#include "./inline/IntegerDesignVariableType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_INTEGERDESIGNVARIABLETYPE_HPPS
