/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class EqualityConstraintType.

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

        Tue Jun 10 08:43:33 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the EqualityConstraintType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_EQUALITYCONSTRAINTTYPE_HPP
#define JEGA_UTILITIES_EQUALITYCONSTRAINTTYPE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/ConstraintTypeBase.hpp>







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
class EqualityConstraintType;







/*
================================================================================
Class Definition
================================================================================
*/

/// A type for equality constraints.
/**
 * An equality constraint looks like this:
 * \verbatim
             h(x) = target
   \endverbatim
 */
class JEGA_SL_IEDECL EqualityConstraintType :
    public ConstraintTypeBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// This is the value to which the expression is equated (the target).
        /**
         * Default value is 0.0.
         */
        double _value;

        /**
         * \brief This variable can be used to provide the constraint with
         *        "thickness".
         *
         * It represents the amount by which a value may differ from the
         * target \a _value and still be considered feasible.
         */
        double _viol;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the value of the \a _value member variable.
        /**
         * \param val The new singular feasible value for this constraint.
         */
        inline
        void
        SetTargetValue(
            double val
            );

        /// Sets the value of the \a _viol member variable.
        /**
         * \param viol The "thickness" of the equality constraint.
         */
        inline
        void
        SetAllowableViolation(
            double viol
            );



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /// Gets the value of the \a _value member variable.
        /**
         * \return The singular feasible value for this constraint.
         */
        inline
        double
        GetTargetValue(
            ) const;

        /// Gets the value of the \a _viol member variable.
        /**
         * \return The thickness of this constraint.
         */
        inline
        double
        GetAllowableViolation(
            ) const;



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

        /// Returns the string name of this type.
        /**
         * \return The string "Equality".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns true if the nature of this type cannot be changed.
        /**
         * As far as the equality constraint type is concerned, the nature can
         * change.
         *
         * \return false, always.
         */
        virtual
        bool
        IsNatureLocked(
            ) const;

        /// Returns the amount by which \a of violates this constraint.
        /**
         * For the equality constraint type, this is the amount by which
         * \a of deviates from the target value.  If the deviation is within
         * the acceptable violation amount, 0 is returned.  This value will
         * be signed such that values below the acceptable return negative
         * numbers and those above will return positive.
         *
         * \param of The value to test against the requirements of this
         *           constraint.
         * \return The amount by which \a of violates this constraint.  0 if
         *         \a of is not in violation.
         */
        virtual
        double
        GetViolationAmount(
            double of
            ) const;

        /// Returns an exact duplicate of this type object.
        /**
         * \param forCNI The ConstraintInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by \a forCNI.
         */
        virtual
        ConstraintTypeBase*
        Clone(
            ConstraintInfo& forCNI
            ) const;

        virtual
        std::string
        GetEquation(
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

        /// Constructs a EqualityConstraintType to be known by \a info.
        /**
         * \param info The ConstraintInfo for which this is to be the type.
         */
        EqualityConstraintType(
            ConstraintInfo& info
            );

        /// Copy constructs a EqualityConstraintType to be known by \a info.
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ConstraintInfo for which this is to be the type.
         */
        EqualityConstraintType(
            const EqualityConstraintType& copy,
            ConstraintInfo& info
            );


}; // class EqualityConstraintType


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
#include "./inline/EqualityConstraintType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_EQUALITYCONSTRAINTTYPE_HPP
