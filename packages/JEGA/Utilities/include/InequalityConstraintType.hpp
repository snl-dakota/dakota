/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class InequalityConstraintType.

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

        Tue Jun 10 08:43:25 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the InequalityConstraintType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_INEQUALITYCONSTRAINTTYPE_HPP
#define JEGA_UTILITIES_INEQUALITYCONSTRAINTTYPE_HPP







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
class InequalityConstraintType;







/*
================================================================================
Class Definition
================================================================================
*/

/// A type for inequality constraints.
/**
 * An inequality constraint looks like this:
 * \verbatim
             g(x) <= ub
   \endverbatim
 */
class JEGA_SL_IEDECL InequalityConstraintType :
    public ConstraintTypeBase
{

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// This is the value on the right hand side of the expression.
        /**
         * Default value is 0.0.
         */
        double _upperValue;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the value of the "_upperValue" member variable.
        /**
         * The upper value is the value that the result of a constraint
         * calculation must be less than or equal to in order to be feasible.
         * \verbatim
            g(x) <= upper value
           \endverbatim
         *
         * \param val The new upper value for this inequality constraint.
         */
        inline
        void
        SetUpperValue(
            double val
            );



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the value of the "_upperValue" member variable.
        /**
         * \return The current upper value for this inequality constraint.
         */
        inline
        double
        GetUpperValue(
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
         * \return The string "Inequality".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns true if the nature of this type cannot be changed.
        /**
         * As far as the inequality constraint type is concerned, the nature
         * can change.
         *
         * \return false, always.
         */
        virtual
        bool
        IsNatureLocked(
            ) const;

        /// Returns the amount by which "of" violates this constraint.
        /**
         * For the inequality constraint type, this is the amount by which "of"
         * exceeds the upper value and thus will always be positive or 0.
         *
         * \param of The value to test against the requirements of this
         *           constraint.
         * \return The amount by which "of" violates this constraint.  0 if
         *         "of" is not in violation.
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
         * \return An exact duplicate of this type for use by "forCNI".
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

        /// Constructs a InequalityConstraintType to be known by "info".
        /**
         * \param info The ConstraintInfo for which this is to be the type.
         */
        InequalityConstraintType(
            ConstraintInfo& info
            );

        /// Copy constructs a InequalityConstraintType to be known by "info".
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ConstraintInfo for which this is to be the type.
         */
        InequalityConstraintType(
            const InequalityConstraintType& copy,
            ConstraintInfo& info
            );


}; // class InequalityConstraintType


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
#include "./inline/InequalityConstraintType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_INEQUALITYCONSTRAINTTYPE_HPP
