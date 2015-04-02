/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignVariableInfo.

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

        Tue Jun 03 07:34:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignVariableInfo class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNVARIABLEINFO_HPP
#define JEGA_UTILITIES_DESIGNVARIABLEINFO_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/InfoBase.hpp>
#include <../Utilities/include/JEGATypes.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
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
class Design;
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

/// This class stores information about design variabes.
/**
 * The information stored includes the type (real, integer, etc) and the
 * nature (continuum, discrete) as well as any inherited information from
 * InfoBase.  The majority of work done on behalf of this class is either
 * done by the type or the nature which must be derivatives of the
 * DesignVariableTypeBase and DesignVariableNatureBase respectively.
 */
class JEGA_SL_IEDECL DesignVariableInfo :
    public InfoBase
{

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// Class representing the type of this variable.
        /**
         * This member data represents the type of design variable.
         * It is something like integer, real, complex, etc.
         */
        DesignVariableTypeBase* _type;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Tells this object what type variable it represents.
        /**
         * \param type The new type for this design variable.
         */
        void
        SetType(
            DesignVariableTypeBase* type
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the type represented by this object.
        /**
         * \return The type object representing the type of this design
         *         variable.
         */
        inline
        DesignVariableTypeBase&
        GetType(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Adds all of the supplied values to the end of the list of
         *        discrete values.
         *
         * Each value is checked to see that it is unique in the list of
         * discrete values.  Returns true if all values were successfully added
         * and false otherwise.  Assertions are made that this variable is of
         * discrete nature.
         *
         * \param values The vector of discrete values to add to this variable.
         * \return true if all are successfully added and false otherwise.
         */
        bool
        AddDiscreteValues(
            const JEGA::DoubleVector& values
            );

        /**
         * \brief Returns the amount by which this value violates either bound
         *        by representation (not value).
         *
         * The number supplied must be a representation and not a value.  That
         * means that if you are getting it from a Design or derived class
         * object, you should use the GetVariableRep method.
         *
         * \param rep The representation to test for bound violation.
         * \return The amount by which the supplied representation violates
         *         either bound.  This is always a positive number.
         */
        double
        GetRepBoundViolation(
            double rep
            ) const;

        /**
         * \brief Returns the amount by which this Design violates
         *        either bound by representation (not value).
         *
         * \param des The Design whose representation for this variable is to
         *            be tested for bound violation.
         * \return The amount by which the representation of this variable in
         *         "des" violates either bound.  This is always a positive
         *         number.
         */
        inline
        double
        GetRepBoundViolation(
            const Design& des
            ) const;

        /**
         * \brief Returns the amount by which this value violates either bound
         *        by value (not representation).
         *
         * The number supplied must be a value and not a representation.  That
         * means that if you are getting it from a Design or derived class
         * object, you should use the GetVariableValue method.
         *
         * \param val The value test for value bound violation.
         * \return The amount by which the supplied value violates
         *         either bound.  This is always a positive number.
         */
        double
        GetValueBoundViolation(
            double val
            ) const;

        /**
         * \brief Returns the amount by which this Design violates either
         *        bound by value (not representation).
         *
         * \param des The Design whose value for this variable is to
         *            be tested for bound violation.
         * \return The amount by which the value of this variable in
         *         "des" violates either bound.  This is always a positive
         *         number.
         */
        inline
        double
        GetValueBoundViolation(
            const Design& des
            ) const;

        /// Rounds "val" according to the decimal precision.
        /**
         * \param val The value to be rounded according to this variables
         *            precision.
         * \return "val" rounded to this variables precision.
         */
        inline
        double
        AssertPrecision(
            double val
            ) const;

        /// Returns the default value for this design variable as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The default value for this variable or -DBL_MAX if none.
         */
        inline
        double
        GetDefaultValue(
            ) const;

        /// Returns the maximum value this variable may have as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The largest legitimate value for this variable.
         */
        inline
        double
        GetMaxValue(
            ) const;

        /// Returns the minimum value this variable may have as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The smallest legitimate value for this variable.
         */
        inline
        double
        GetMinValue(
            ) const;

        /// Returns the difference between the max and min values.
        /**
         * \return The magnitude of the range of possible values for this
         *         variable.
         */
        inline
        double
        GetValueRange(
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

        /// Returns the value represented by "rep" as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param rep The representation to convert to a value.
         * \return The value associated with or represented by "rep".
         */
        double
        GetValueOf(
            double rep
            ) const;

        /// Returns a random valid value for this variable as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return A random value for this variable for which IsValidValue will
         *         return true;
         */
        double
        GetRandomValue(
            ) const;

        /// Returns the representation of the default value as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the default value for this variable or
         *         -DBL_MAX if none.
         */
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
        inline
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
        inline
        double
        GetMinDoubleRep(
            ) const;

        /// Returns the difference between the max and min double rep.
        /**
         * \return The magnitude of the range of possible representations for
         *         this variable.
         */
        inline
        double
        GetDoubleRepRange(
            ) const;

        /// Returns the proper representation of "value" as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param value The value to retrieve the representation of.
         * \return The representation of the value "value".
         */
        inline
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
        inline
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
        inline
        double
        GetRandomDoubleRep(
            const RegionOfSpace& within
            ) const;

        /// Returns the distance between valid representations as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The increment that exists between consecutive
         *         representations according to the decimal precision.
         */
        inline
        double
        GetDistanceBetweenDoubleReps(
            ) const;

        /**
         * \brief Returns true if this info object can accept or remove
         *        discrete values.
         *
         * This is generally only false if the variable has a discrete
         * nature.
         *
         * \return true if this variable is unable to manage discrete values
         *         and false otherwise.
         */
        inline
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
        inline
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
        inline
        bool
        IsRepInBounds(
            double rep
            ) const;

        /// Returns true if the nature of this variable cannot be changed.
        /**
         * \return false if the nature can be changed and true otherwise.
         */
        inline
        bool
        IsNatureLocked(
            ) const;

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
        inline
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
        inline
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
        inline
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
        inline
        bool
        IsValidDoubleRep(
            double rep
            ) const;

        /// Returns a string representation of the type of this variable.
        /**
         * Some examples are "Integer", "Real", etc.
         *
         * \return The name of the type of this info object.
         */
        inline
        std::string
        GetTypeString(
            ) const;

        /// Returns a string representation of the nature of this variable.
        /**
         * Some examples are "Discrete" and "Continuum".
         *
         * \return The name of the nature of this info object.
         */
        inline
        std::string
        GetNatureString(
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
        inline
        bool
        AddDiscreteValue(
            double value
            );

        /// This method empties the list of discrete values.
        /**
         * Assertions are made that this variable is of discrete nature.
         */
        inline
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
        inline
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
        inline
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
        inline
        void
        SetMaxValue(
            double value
            );

        /// Sets the desired decimal precision for this variable.
        /**
         * This method will fail if IsPrecisionLocked returns true.
         * Return is true on success or false on failure.
         * Negative numbers are allowed and have the effect of rounding to
         * the left of the radix.  For example, a precision of -1 will allow
         * control of a variable to the nearest 10.
         *
         * \param prec The new number of decimal places of interest for this
         *             variable.
         * \return True if the precision is adopted and false otherwise.
         */
        inline
        bool
        SetPrecision(
            eddy::utilities::int16_t prec
            );

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

        /// Tells this object of the nature of the variable it represents.
        /**
         * This method fails if IsNatureLocked returns true.
         *
         * \param nature The new nature for this design variable.
         * \return true on success and false on failure.
         */
        inline
        bool
        SetNature(
            DesignVariableNatureBase* nature
            );

        /// Returns the nature of the variable represented by this object.
        /**
         * \return The nature object representing the nature of this design
         *         variable.
         */
        inline
        DesignVariableNatureBase&
        GetNature(
            ) const;

        /// Assigns the properties of "right" to this.
        const DesignVariableInfo&
        operator = (
            const DesignVariableInfo &right
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
        inline
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
        inline
        bool
        IsContinuum(
            ) const;

        /// Returns the variable value associated with this info object.
        /**
         * For example, if this is the 3rd DesignVariableInfo, then this method
         * will return the result of des.GetVariableValue(GetNumber()).
         *
         * \param des The Design for which the stored number for this
         *            variable is sought.
         * \return The number stored for this variable in "des".
         */
        double
        WhichValue(
            const Design& des
            ) const;

        /**
         * \brief Returns the variable representationassociated with this info
         *        object.
         *
         * For example, if this is the 3rd DesignVariableInfo, then this method
         * will return the result of des.GetVariableRep(GetNumber()).
         *
         * \param des The Design for which the stored number for this
         *            variable is sought.
         * \return The number stored for this variable in "des".
         */
        double
        WhichDoubleRep(
            const Design& des
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

        /// Constructs a DesignVariableInfo object knowing "target".
        /**
         * Default properties are type = real and nature = continuum.
         *
         * \param target The DesignTarget for which this info object is being
         *               constructed.
         */
        DesignVariableInfo(
            DesignTarget& target
            );

        /// Copy constructs a DesignVariableInfo for use by \a target.
        /**
         * \param copy The info to copy properties from.
         * \param target The DesignTarget for which this info is being
         *               constructed.
         */
        DesignVariableInfo(
            const DesignVariableInfo& copy,
            DesignTarget& target
            );

        /// Copy constructs a DesignVariableInfo object.
        /**
         * \param copy The existing info object from which to copy properties
         *             into this.
         */
        explicit
        DesignVariableInfo(
            const DesignVariableInfo& copy
            );

        /// Destructs a DesignVariableInfo object.
        /**
         * This destructor caused destruction of the type object as well.
         */
        virtual
        ~DesignVariableInfo(
            );


}; // class DesignVariableInfo





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
#include "./inline/DesignVariableInfo.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNVARIABLEINFO_HPP
