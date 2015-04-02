/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class TwoSidedInequalityConstraintType.

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

        Tue Jun 10 15:04:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the TwoSidedInequalityConstraintType
 *        class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_TWOSIDEDINEQUALITYCONSTRAINTTYPE_HPP
#define JEGA_UTILITIES_TWOSIDEDINEQUALITYCONSTRAINTTYPE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/InequalityConstraintType.hpp>






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
class TwoSidedInequalityConstraintType;







/*
================================================================================
Class Definition
================================================================================
*/

/// A type for inequality constraints with upper and lower bounds.
/**
 * An inequality constraint with upper and lower bounds looks like this:
 * \verbatim
             lb <= g(x) <= ub
   \endverbatim
 * The default upper bound is 0 and the default lower bound is -infinity.
 */
class JEGA_SL_IEDECL TwoSidedInequalityConstraintType :
    public InequalityConstraintType
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// This is the lower bound for feasible designs.
        /**
         * Default value is -infinity.
         */
        double _lowerValue;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the value of the \a _lowerValue member variable.
        /**
         * The lower value is the value that the result of a constraint
         * calculation must be greater than or equal to in order to be
         * feasible.
         * \verbatim
            lower value <= g(x) <= upper value
           \endverbatim
         *
         * \param val The new lower value for this inequality constraint.
         */
        void
        SetLowerValue(
            double val
            );




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the value of the \a _lowerValue member variable.
        /**
         * \return The current lower value for this two sided inequality
         *         constraint.
         */
        inline
        double
        GetLowerValue(
            ) const;



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the string name of this type.
        /**
         * \return The string "Two Sided Inequality".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns the amount by which \a of violates this constraint.
        /**
         * For the two sided inequality constraint type, this is the amount by
         * which \a of is out of the boundaries set by the upper and lower
         * values.  The return is signed such that violation of the upper value
         * is positive and violation of the lower value is negative.
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

        /**
         * \brief Constructs a TwoSidedInequalityConstraintType to be known by
         *        \a info.
         *
         * \param info The ConstraintInfo for which this is to be the type.
         */
        TwoSidedInequalityConstraintType(
            ConstraintInfo& info
            );

        /**
         * \brief Copy constructs a TwoSidedInequalityConstraintType to be
         *        known by \a info.
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ConstraintInfo for which this is to be the type.
         */
        TwoSidedInequalityConstraintType(
            const TwoSidedInequalityConstraintType& copy,
            ConstraintInfo& info
            );



}; // class TwoSidedInequalityConstraintType


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
#include "./inline/TwoSidedInequalityConstraintType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_TWOSIDEDINEQUALITYCONSTRAINTTYPE_HPP
