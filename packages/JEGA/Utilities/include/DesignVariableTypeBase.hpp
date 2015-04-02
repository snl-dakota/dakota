/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignVariableTypeBase.

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

        Tue Jun 03 08:55:03 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignVariableTypeBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNVARIABLETYPEBASE_HPP
#define JEGA_UTILITIES_DESIGNVARIABLETYPEBASE_HPP







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
class DesignTarget;
class RegionOfSpace;
class DesignVariableInfo;
class DesignVariableTypeBase;
class DesignVariableNatureBase;






/*
================================================================================
Class Definition
================================================================================
*/

/// The base class for all design variable types.
/**
 * See DesignVariableInfo.hpp for a complete description of how the type fits
 * into the idea of design variable information.
 *
 * The type represents the kind of design variable in the sense of data types.
 */
class JEGA_SL_IEDECL DesignVariableTypeBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The DesignVariableInfo for which this is the type.
        DesignVariableInfo& _info;

        /// This object is used to define the nature of the design variable.
        /**
         * Some examples of natures are "Descrete" and "Continuum".
         */
        DesignVariableNatureBase* _nature;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Tells this object of the nature of the variable it helps represent.
        /**
         * This method fails if IsNatureLocked returns true.
         * Returns true on success and false on failure.
         *
         * \param nature The new nature to use in conjunction with this type.
         * \return true if the nature is successfully set and false otherwise.
         */
        bool
        SetNature(
            DesignVariableNatureBase* nature
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the DesignVariableInfo object known by this (non-const)
        /**
         * \return The DesignVariableInfo for which this is the type.
         */
        inline
        DesignVariableInfo&
        GetDesignVariableInfo(
            );

        /// Returns the DesignVariableInfo object known by this (const)
        /**
         * \return The DesignVariableInfo for which this is the type.
         */
        inline
        const DesignVariableInfo&
        GetDesignVariableInfo(
            ) const;

        /// Returns the nature of this type object.
        /**
         * \return The nature being used along with this type.
         */
        inline
        DesignVariableNatureBase&
        GetNature(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget for which this describes the type of a
         *         variable.
         */
        const DesignTarget&
        GetDesignTarget(
            ) const;

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget for which this describes the type of a
         *         variable.
         */
        DesignTarget&
        GetDesignTarget(
            );

        /// Returns a string representation of the nature of this type.
        /**
         * Some examples are "Discrete" and "Continuum".
         *
         * \return The name of the nature associated with this type.
         */
        std::string
        GetNatureString(
            ) const;

        /// Rounds "val" according to the decimal precision.
        /**
         * \param val The value on which to assert the precision of this
         *            variable.
         * \return The value after it has been rounded according to the
         *         decimal precision of this variable.
         */
        double
        AssertPrecision(
            double val
            ) const;

        /**
         * \brief Returns the desired or required decimal precision of this
         *        design variable.
         *
         * This is in specific reference to the representation, not the value.
         *
         * \return The number of decimal places of interest for this variable.
         */
        eddy::utilities::int16_t
        GetPrecision(
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

        /// Override to return the string name of this type.
        /**
         * \return The name of this type such as "Real", "Integer", etc.
         */
        virtual
        std::string
        ToString(
            ) const = 0;

        /// Sets the desired decimal precision for this type.
        /**
         * This method will fail if IsPrecisionLocked returns true.
         * Negative numbers are allowed by some types and natures and have the
         * effect of rounding to the left of the radix.  For example, a
         * precision of -1 will allow control of a variable to the nearest 10.
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

        /**
         * \brief Returns true if this type object can accept or remove
         *        discrete values.
         *
         * This is generally handled by the nature and so the default
         * implementation passes the query along to the nature.  Override only
         * if that is not the desired behavior.
         *
         * \return true if this variable is unable to manage discrete values
         *         and false otherwise.
         */
        virtual
        bool
        IsDiscreteValueLocked(
            ) const;

        /**
         * \brief Returns true if "value" lies within the upper and lower
         *        bounds of this variable in terms of values
         *        (not representations).
         *
         * \param value The value to check to see if it is in bounds or not.
         * \return True if value is in bounds and false otherwise.
         */
        virtual
        bool
        IsValueInBounds(
            double value
            ) const;

        /**
         * \brief Returns true if "rep" lies within the upper and lower bounds
         *        of this variable in terms of representations ( not values ).
         *
         * \param rep The representation to check to see if it is in bounds or
         *            not.
         * \return True if rep is in bounds and false otherwise.
         */
        virtual
        bool
        IsRepInBounds(
            double rep
            ) const;

        /**
         * \brief Returns true if the nature of this type cannot be changed.
         *
         * \return false if the nature can be changed and true otherwise.
         */
        virtual
        bool
        IsNatureLocked(
            ) const = 0;

        /**
         * \brief Returns true if this type can take on values outside of the
         *        bounds and still be evaluated (even though it will no doubt
         *        be infeasible).
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
            ) const;

        /// Returns true if the precision for this type cannot be changed.
        /**
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
            ) const;

        /// Returns true if "value" is a valid value for this variable.
        /**
         * Valid values are those that may be returned by GetRandomValue.
         *
         * \param value The value to check for validity with this variable
         *              type.
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
         *            variable type.
         * \return true if "rep" is a valid representation for this variable
         *         and false otherwise.
         */
        virtual
        bool
        IsValidDoubleRep(
            double rep
            ) const;

        /// Override to return an exact duplicate of this type object.
        /**
         * \param forDVI The DesignVariableInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by "forDVI".
         */
        virtual
        DesignVariableTypeBase*
        Clone(
            DesignVariableInfo& forDVI
            ) const = 0;

        /// Returns the default value for this type as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The default value for this variable or -DBL_MAX if none.
         */
        virtual
        double
        GetDefaultValue(
            ) const;

        /// Returns the maximum value this type may have as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The largest legitimate value for this variable.
         */
        virtual
        double
        GetMaxValue(
            ) const;

        /// Returns the minimum value this type may have as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The smallest legitimate value for this variable.
         */
        virtual
        double
        GetMinValue(
            ) const;

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

        /// Returns the representation of the default value as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the default value for this variable or
         *         -DBL_MAX if none.
         */
        virtual
        double
        GetDefaultDoubleRep(
            ) const;

        /**
         * \brief Returns the representation of the max value for this type as
         *        a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the maximum value for this variable or
         *         -DBL_MAX of none.
         */
        virtual
        double
        GetMaxDoubleRep(
            ) const;

        /**
         * \brief Returns the representation of the min value for this type as
         *        a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the minimum value for this variable or
         *         -DBL_MAX of none.
         */
        virtual
        double
        GetMinDoubleRep(
            ) const;

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

        /**
         * \brief Override to return the representation of a random value as
         *        a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of a random value.
         */
        virtual
        double
        GetRandomDoubleRep(
            ) const = 0;

        /**
         * \brief Override to return a random representation existing within
         *        the supplied region of space.
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
            ) const = 0;

        /// Returns the distance between valid representations as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The increment that exists between consecutive
         *         representations according to the decimal precision.
         */
        virtual
        double
        GetDistanceBetweenDoubleReps(
            ) const;

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
            );

        /// This method empties the list of discrete values.
        /**
         * Assertions are made that this variable is of discrete nature.
         */
        virtual
        void
        ClearDiscreteValues(
            );

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
            );

        /**
         * \brief Sets the lower bound or smallest value for this variable to
         *        "value".
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
         *        "value".
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

        /// Returns true if the nature of this variable is Discrete.
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
            ) const;

        /// Returns true if the nature of this variable is Continuum.
        /**
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

        /// Constructs a DesignVariableTypeBase to be known by "info".
        /**
         * Default nature is Continuum until otherwise specified.
         *
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        DesignVariableTypeBase(
            DesignVariableInfo& info
            );

        /// Copy constructs a DesignVariableTypeBase to be known by "info".
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The DesignVariableInfo for which this is to be the type.
         */
        DesignVariableTypeBase(
            const DesignVariableTypeBase& copy,
            DesignVariableInfo& info
            );

        /// Destructs a DesignVariableTypeBase object.
        /**
         * This destructor causes destruction of the associated nature object
         * as well.
         */
        virtual
        ~DesignVariableTypeBase(
            );

}; // class DesignVariableTypeBase


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
#include "./inline/DesignVariableTypeBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNVARIABLETYPEBASE_HPP
