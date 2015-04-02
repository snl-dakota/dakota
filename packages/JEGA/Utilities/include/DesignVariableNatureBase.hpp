/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignVariableNatureBase.

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

        Tue Jun 03 08:55:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignVariableNatureBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNVARIABLENATUREBASE_HPP
#define JEGA_UTILITIES_DESIGNVARIABLENATUREBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>

#include <utilities/include/int_types.hpp>







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
class RegionOfSpace;
class DesignVariableInfo;
class DesignVariableTypeBase;
class DesignVariableNatureBase;






/*
================================================================================
Class Definition
================================================================================
*/

/// The base class for all design variable natures.
/**
 * See DesignVariableInfo.hpp for a complete description of how the nature fits
 * into the idea of design variable information.
 *
 * The nature represents the possible values that a variable can take on.
 */
class JEGA_SL_IEDECL DesignVariableNatureBase
{

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The type of the variable for which this is the nature.
        DesignVariableTypeBase& _type;

        /// This is the desired or required decimal precision of this type.
        eddy::utilities::int16_t _precision;


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

        /// Returns the DesignVariableTypeBase known by this object (non-const)
        /**
         * \return The type that is used in conjunction with this nature to
         *         define the behavior of a variable.
         */
        inline
        DesignVariableTypeBase&
        GetType(
            );

        /// Returns the DesignVariableTypeBase known by this object (const)
        /**
         * \return The type that is used in conjunction with this nature to
         *         define the behavior of a variable.
         */
        inline
        const DesignVariableTypeBase&
        GetType(
            ) const;

        /**
         * \brief Returns the desired or required decimal precision of this
         *        design variable.
         *
         * This is in specific reference to the representation, not the value.
         *
         * \return The number of decimal places of interest for this variable.
         */
        inline
        eddy::utilities::int16_t
        GetPrecision(
            ) const;



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Rounds "val" according to the decimal precision.
        /**
         * \param val The value to be rounded according to this variables
         *            precision.
         * \return "val" rounded to this variables precision.
         */
        double
        AssertPrecision(
            double val
            ) const;

        /**
         * \brief Allows mutable access to the DesignVariableInfo for which
         *        this is the nature.
         *
         * \return A reference to the DesignVariableInfo using this nature.
         */
        DesignVariableInfo&
        GetDesignVariableInfo(
            );

        /**
         * \brief Allows immutable access to the DesignVariableInfo for which
         *        this is the nature.
         *
         * \return A reference to the DesignVariableInfo using this nature.
         */
        const DesignVariableInfo&
        GetDesignVariableInfo(
            ) const;

        /**
         * \brief Override to return the representation of a random "value" as
         *        a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of a random value.
         */
        double
        GetRandomDoubleRep(
            ) const;

        /**
         * \brief Override to return a random representation existing within
         *        the supplied region of space.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \param within The region of space of the desired random value.
         * \return The representation of a random value inside \a within.
         */
        double
        GetRandomDoubleRep(
            const RegionOfSpace& within
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

        /// Override to return the string name of this nature.
        /**
         * \return The name of this nature.  Examples are "Continuum" and
         *         "Discrete".
         */
        virtual
        std::string
        ToString(
            ) const = 0;

        /// Sets the desired decimal precision for this type.
        /**
         * This method will fail if IsPrecisionLocked returns true.
         * Negative numbers are allowed and have the effect of rounding to
         * the left of the radix.  For example, a precision of -1 will allow
         * control of a variable to the nearest 10.
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

        /// Override to return an exact duplicate of this nature object.
        /**
         * \param forType The type base with which this nature is being used.
         * \return An exact duplicate of this nature created for use with the
         *         supplied type.
         */
        virtual
        DesignVariableNatureBase*
        Clone(
            DesignVariableTypeBase& forType
            ) const = 0;

        /**
         * \brief Override to return the representation of the max value for
         *        this nature as a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the maximum value for this variable or
         *         -DBL_MAX if none.
         */
        virtual
        double
        GetMaxDoubleRep(
            ) const = 0;

        /**
         * \brief Override to return the representation of the min value for
         *        this nature as a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the minimum value for this variable or
         *         -DBL_MAX if none.
         */
        virtual
        double
        GetMinDoubleRep(
            ) const = 0;

        /**
         * \brief Override to return a random representation existing within
         *        the supplied range.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \param lb The lower bound on the desired random value.
         * \param ub The upper bound on the desired value.
         * \return The representation of a random value within \a lb and \a ub.
         */
        virtual
        double
        GetRandomDoubleRep(
            double lb,
            double ub
            ) const = 0;

        /**
         * \brief Override to return the proper representation of "value" as a
         *        double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \param value The value to retrieve the representation of.
         * \return The representation of the value "value".
         */
        virtual
        double
        GetDoubleRepOf(
            double value
            ) const = 0;

        /// Override to return a random valid value for this type as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return A random value for this variable for which IsValidValue will
         *         return true;
         */
        virtual
        double
        GetRandomValue(
            ) const = 0;

        /**
         * \brief Override to return the maximum value this nature may have as
         *        a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The largest legitimate value for this variable.
         */
        virtual
        double
        GetMaxValue(
            ) const = 0;

        /**
         * \brief Override to return the minimum value this nature may have as
         *        a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The smallest legitimate value for this variable.
         */
        virtual
        double
        GetMinValue(
            ) const = 0;

        /// Override to return the value represented by "rep" as a double.
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
            ) const = 0;

        /// Override to return the nearest valid value to "value".
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
            ) const = 0;

        /// Override to return the nearest valid double rep to "rep".
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
            ) const = 0;

        /**
         * \brief Override to return the distance between valid representations
         *        as a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The increment that exists between consecutive
         *         representations according to the decimal precision.
         */
        virtual
        double
        GetDistanceBetweenDoubleReps(
            ) const = 0;

        /**
         * \brief The mechanism by which discrete values can be added to this
         *        object.
         *
         * "value" is checked to see that it is unique in the list of discrete
         * values.  Assertions are made that this variable is of discrete
         * nature.
         *
         * \param value The value to be added to the list of discrete values.
         * \return true if "value" was successfully added and false otherwise.
         */
        virtual
        bool
        AddDiscreteValue(
            double value
            ) = 0;

        /// This method empties the list of discrete values.
        /**
         * Assertions are made that this variable is of discrete nature.
         */
        virtual
        void
        ClearDiscreteValues(
            ) = 0;

        /**
         * \brief This method allows the removal of the specified discrete
         *        value "value".
         *
         * Assertions are made that this variable is of discrete nature.
         *
         * \param value The value to remove from the list of discrete values.
         * \return true if "value" was removed and false otherwise (which
         *         usually means that "value" was not found).
         */
        virtual
        bool
        RemoveDiscreteValue(
            double value
            ) = 0;

        /**
         * \brief Override to set the upper bound or largest value for this
         *        variable to "value".
         *
         * \param value The new maximum value for this variable.
         */
        virtual
        void
        SetMaxValue(
            double value
            ) = 0;

        /**
         * \brief Override to set the lower bound or smallest value for this
         *        variable to "value".
         *
         * \param value The new minimum value for this variable.
         */
        virtual
        void
        SetMinValue(
            double value
            ) = 0;

        /**
         * \brief Override to returns true if this nature object can accept or
         *        remove discrete values.
         *
         * This is generally only false if the variable has a discrete
         * nature.
         *
         * \return true if this variable is unable to manage discrete values
         *         and false otherwise.
         */
        virtual
        bool
        IsDiscreteValueLocked(
            ) const = 0;

        /**
         * \brief Override to return true if "value" lies within the upper and
         *        lower bounds of this variable in terms of values
         *        (not representations).
         *
         * \param value The value to check to see if it is in bounds or not.
         * \return True if value is in bounds and false otherwise.
         */
        virtual
        bool
        IsValueInBounds(
            double value
            ) const = 0;

        /**
         * \brief Override to return true if "rep" lies within the upper and
         *        lower bounds of this variable in terms of representations
         *        ( not values ).
         *
         * \param rep The representation to check to see if it is in bounds or
         *            not.
         * \return True if rep is in bounds and false otherwise.
         */
        virtual
        bool
        IsRepInBounds(
            double rep
            ) const = 0;

        /**
         * \brief Override to return true if this type can take on values
         *        outside of the bounds and still be evaluated (even though it
         *        will no doubt be infeasible).
         *
         * This is useful to be sure that an algorithm does not generate values
         * that cannot be used.  For example, if using a discrete variable, it
         * makes no sense to attempt to use the 10th variable if only 9 exist.
         *
         * \return True if values outside of the known bounds are legitimate
         *         for purposes of evaluation and false otherwise.
         */
        virtual
        bool
        IsOutOfBoundsDefined(
            ) const = 0;

        /**
         * \brief Override to return true if the precision for this nature
         *        cannot be changed.
         *
         * This would be true for example of discrete natured variables whose
         * precision is always 0 because the representations are always
         * integer indices.
         *
         * \return True if the precision for this variable is immutable and
         *         false otherwise.
         */
        virtual
        bool
        IsPrecisionLocked(
            ) const = 0;

        /// Returns true if "value" is a valid value for this variable.
        /**
         * Valid values are those that may be returned by GetRandomValue.
         *
         * \param value The value to check for validity with this variable
         *              nature.
         * \return true if "value" is a valid value for this variable and false
         *         otherwise.
         */
        virtual
        bool
        IsValidValue(
            double value
            ) const;

        /**
         * \brief Returns true if "rep" is the representation of a valid value
         *        for this variable.
         *
         * Valid representations are those that may be returned by
         * GetRandomDoubleRep.
         *
         * \param rep The representation to check for validity with this
         *            variable nature.
         * \return true if "rep" is a valid representation for this variable
         *         and false otherwise.
         */
        virtual
        bool
        IsValidDoubleRep(
            double rep
            ) const;

        /// Override to return true if the nature of this variable is Discrete.
        /**
         * This method exists because Discrete and Continuum natures are
         * the two most common and so it is convienient to directly
         * poll for them.
         *
         * \return True if this design variable has a discrete nature and false
         *         otherwise.
         */
        virtual
        bool
        IsDiscrete(
            ) const = 0;

        /**
         * \brief Override to return true if the nature of this variable is
         *        Continuum.
         *
         * This method exists because Discrete and Continuum natures are
         * the two most common and so it is convienient to directly
         * poll for them.
         *
         * \return True if this design variable has a continuum nature and
         *         false otherwise.
         */
        virtual
        bool
        IsContinuum(
            ) const = 0;

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

        /// Constructs a DesignVariableNatureBase known by "type".
        /**
         * \param type The type along with which this nature will be used to
         *             describe/define the behavior of a design variable.
         */
        DesignVariableNatureBase(
            DesignVariableTypeBase& type
            );

        /// Copy constructs a DesignVariableTypeBase known by "info".
        /**
         * \param copy The existing DesignVariableNatureBase from which to copy
         *             properties into this.
         * \param type The type along with which this nature will be used to
         *             describe/define the behavior of a design variable.
         */
        DesignVariableNatureBase(
            const DesignVariableNatureBase& copy,
            DesignVariableTypeBase& type
            );

        /// Destructs a DesignVariableNatureBase object.
        virtual
        ~DesignVariableNatureBase(
            );



}; // class DesignVariableNatureBase


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
#include "./inline/DesignVariableNatureBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNVARIABLENATUREBASE_HPP
