/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ContinuumDesignVariableNature.

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

        Tue Jun 03 08:55:45 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ContinuumDesignVariableNature class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_CONTINUUMDESIGNVARIABLENATURE_HPP
#define JEGA_UTILITIES_CONTINUUMDESIGNVARIABLENATURE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/DesignVariableNatureBase.hpp>







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
class ContinuumDesignVariableNature;







/*
================================================================================
Class Definition
================================================================================
*/

/// A nature for continuum design variables.
/**
 * A continuum is defined by merriam webster as:
 *
 * \verbatim
    "a coherent whole characterized as a collection,
    sequence, or progression of values or elements
    varying by minute degrees <"good" and "bad"...
    stand at opposite ends of a continuum instead
    of describing the two halves of a line -- Wayne Shumaker>
   \endverbatim
 * Thus, this class represents values of any type that exist
 * as part of a well defined progression.  For example, the set
 * of all integers from -100 to 100 constitutes a continuum as
 * does the set of all real numbers between -100 and 100.
 */
class JEGA_SL_IEDECL ContinuumDesignVariableNature :
    public DesignVariableNatureBase
{

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The upper bound of the continuum.
        double _maxVal;

        /// The lower bound of the continuum.
        double _minVal;

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
         * \return The string "Continuum".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns an exact duplicate of this nature object.
        /**
         * \param forType The type base with which this nature is being used.
         * \return An exact duplicate of this nature created for use with the
         *         supplied type.
         */
        virtual
        DesignVariableNatureBase*
        Clone(
            DesignVariableTypeBase& forType
            ) const;

        /**
         * \brief Returns the representation of the max value for this nature
         *        as a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the maximum value for this variable or
         *         -DBL_MAX if error.
         */
        virtual
        double
        GetMaxDoubleRep(
            ) const;

        /**
         * \brief Returns the representation of the min value for this nature
         *        as a double.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \return The representation of the minimum value for this variable or
         *         -DBL_MAX if error.
         */
        virtual
        double
        GetMinDoubleRep(
            ) const;

        /**
         * \brief Returns a random representation existing within
         *        the supplied range.
         *
         * A return of -DBL_MAX indicates failure.
         *
         * \param lb The lower bound on the desired random value.
         * \param ub The upper bound on the desired value.
         * \return The representation of a random value inside \a within.
         */
        virtual
        double
        GetRandomDoubleRep(
            double lb,
            double ub
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

        /// Returns the maximum value this nature may have as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The largest legitimate value for this variable.
         */
        virtual
        double
        GetMaxValue(
            ) const;

        /// Returns the minimum value this nature may have as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \return The smallest legitimate value for this variable.
         */
        virtual
        double
        GetMinValue(
            ) const;

        /// Returns the value represented by \a rep as a double.
        /**
         * A return of -DBL_MAX indicates failure.
         *
         * \param rep The representation to convert to a value.
         * \return The value associated with or represented by \a rep.
         */
        virtual
        double
        GetValueOf(
            double rep
            ) const;

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
         * This version of the method always fails and causes program abort.
         *
         * \param value The value that would be added to the list of discrete
         *              values.
         * \return false if not for the program abort.
         */
        virtual
        bool
        AddDiscreteValue(
            double value
            );

        /// This method empties the list of discrete values.
        /**
         * This method causes a program exit b/c continuum natured variables
         * cannot have discrete values.
         */
        virtual
        void
        ClearDiscreteValues(
            );

        /**
         * \brief This method allows the removal of the specified discrete
         *        value \a value.
         *
         * This method always fails with a program exit b/c discrete values
         * are not allowed in continuum variables.
         *
         * \param value The value that would be removed from the list of
         *              discrete values.
         * \return false if not for the program exit.
         */
        virtual
        bool
        RemoveDiscreteValue(
            double value
            );

        /**
         * \brief Sets the upper bound or largest value for this variable to
         *        \a value.
         *
         * \param value The new maximum value for this variable.
         */
        virtual
        void
        SetMaxValue(
            double value
            );

        /**
         * \brief Sets the lower bound or smallest value for this variable to
         *        \a value.
         *
         * \param value The new minimum value for this variable.
         */
        virtual
        void
        SetMinValue(
            double value
            );

        /**
         * \brief Always returns true b/c discrete values cannot be handled
         *        by this nature.
         *
         * \return true, always.
         */
        virtual
        bool
        IsDiscreteValueLocked(
            ) const;

        /**
         * \brief Returns true if \a value lies within the upper and lower
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
         * \brief Returns true if \a rep lies within the upper and lower bounds
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
         * \brief Returns true because to the best of its knowledge, this
         *        nature thinks it's ok.
         *
         * This is useful to be sure that an algorithm does not generate values
         * that cannot be used.  For example, if using a discrete variable, it
         * makes no sense to attempt to use the 10th variable if only 9 exist.
         *
         * \return true. Always.
         */
        virtual
        bool
        IsOutOfBoundsDefined(
            ) const;

        /// Returns false b/c this nature knows no better.
        /**
         * \return false, always.
         */
        virtual
        bool
        IsPrecisionLocked(
            ) const;

        /// Returns true if \a value is a valid value for this variable.
        /**
         * Valid values are those that may be returned by GetRandomValue.
         * This method considers a value to be valid if it is in bounds.
         *
         * \param value The value to check for validity with this variable
         *              nature.
         * \return true if \a value is a valid value for this variable and
         *        false  otherwise.
         */
        virtual
        bool
        IsValidValue(
            double value
            ) const;

        /**
         * \brief Returns true if \a rep is the representation of a valid value
         *        for this variable.
         *
         * Valid representations are those that may be returned by
         * GetRandomDoubleRep.  This method considers a representation to be
         * valid if it is in bounds.
         *
         * \param rep The representation to check for validity with this
         *            variable nature.
         * \return true if \a rep is a valid representation for this variable
         *         and false otherwise.
         */
        virtual
        bool
        IsValidDoubleRep(
            double rep
            ) const;

        /// Returns false b/c this is the continuum nature.
        /**
         * \return false, always.
         */
        virtual
        bool
        IsDiscrete(
            ) const;

        /// Returns true b/c this is the continuum nature.
        /**
         * \return true, always.
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

        /// Constructs a ContinuumDesignVariableNature known by \a type.
        /**
         * \param type The type along with which this nature will be used to
         *             describe/define the behavior of a design variable.
         */
        ContinuumDesignVariableNature(
            DesignVariableTypeBase& type
            );

        /// Copy constructs a ContinuumDesignVariableNature known by \a type.
        /**
         * \param copy The existing DesignVariableNatureBase from which to copy
         *             properties into this.
         * \param type The type along with which this nature will be used to
         *             describe/define the behavior of a design variable.
         */
        ContinuumDesignVariableNature(
            const ContinuumDesignVariableNature& copy,
            DesignVariableTypeBase& type
            );

        /// Destructs a ContinuumDesignVariableNature object.
        virtual
        ~ContinuumDesignVariableNature(
            );



}; // class ContinuumDesignVariableNature


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
#include "./inline/ContinuumDesignVariableNature.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_CONTINUUMDESIGNVARIABLENATURE_HPP
