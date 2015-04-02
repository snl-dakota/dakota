/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class LinearConstraintNature.

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

        Tue Jun 10 10:23:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the LinearConstraintNature class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_LINEARCONSTRAINTNATURE_HPP
#define JEGA_UTILITIES_LINEARCONSTRAINTNATURE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/JEGATypes.hpp>

#include <../Utilities/include/ConstraintNatureBase.hpp>







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
class LinearConstraintNature;





/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/






/*
================================================================================
Class Definition
================================================================================
*/

/// A nature for linear constraints.
/**
 * A linear constraint can be represented like this:
 * \verbatim
             A*x (logical operator) (right hand side)
   \endverbatim
 * Where A is a row vector of coefficients and x is a column vector of design
 * variable values.  For the purposes of the nature, the logical operator and
 * right hand side are unimportant.  It may very well be the case that there
 * is a far left hand side and another logical operator.
 *
 * If this object is supplied with A, it is able to compute the values
 * of constraints for passed in designs.
 */
class JEGA_SL_IEDECL LinearConstraintNature :
    public ConstraintNatureBase
{


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// This is an optional vector of coefficients for the constraints.
        JEGA::DoubleVector _coeffs;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the coefficients to those stored in "coeffs".
        /**
         * \param coeffs The new vector of coefficients that is to replace
         *               the current one.
         */
        void
        SetCoefficients(
            const JEGA::DoubleVector& coeffs
            );

        /// Sets coefficient "elem" to "val".
        /**
         * \param elem The index of the coefficient to change.
         * \param val The new value for the "elem"th coefficient.
         */
        void
        SetCoefficient(
            JEGA::DoubleVector::size_type elem,
            double val
            );


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns a reference to the vector of coefficients (const).
        /**
         * \return The set of coefficients currently in use.
         */
        inline
        const JEGA::DoubleVector&
        GetCoefficients(
            ) const;

        /// Returns the "elem"'th coefficient.
        /**
         * \param elem The index of the coefficient to return.
         * \return The coefficients currently in use for the "elem"th design
         *         variable.
         */
        inline
        double
        GetCoefficient(
            JEGA::DoubleVector::size_type elem
            ) const;

    protected:

        /// Returns a reference to the vector of coefficients (non-const).
        /**
         * \return A mutable reference to the coefficients currently in use by
         *         this linear nature.
         */
        inline
        JEGA::DoubleVector&
        GetCoefficients(
            );


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the identifier for this nature which is "Linear".
        /**
         * \return The string "Linear".
         */
        static
        const std::string&
        GetName(
            );



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
         * \return The string "Linear".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns an exact duplicate of this nature object.
        /**
         * \param forType the type object for which this nature is being
         *        created.
         * \return An exact duplicate of the fully derived type of this nature.
         */
        virtual
        ConstraintNatureBase*
        Clone(
            ConstraintTypeBase& forType
            ) const;

        /// Evaluates this constraint according to "_coeffs" for "des".
        /**
         * The result will be stored in the appropriate place in "des".
         * The return value is true if all is well and false otherwise.
         * Currently, all is well unless the coefficients vector is of
         * incorrect size.
         *
         * \param des The Design object for which the constraint for which this
         *            is the nature is to be evaluated.
         * \return True if the constraint is successfully evaluated and false
         *         otherwise.
         */
        virtual
        bool
        EvaluateConstraint(
            Design& des
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

        /// Constructs a LinearConstraintNature known by "type".
        /**
         * \param type The type object with which this nature is being used.
         */
        LinearConstraintNature(
            ConstraintTypeBase& type
            );

        /// Copy constructs a ConstraintTypeBase known by "info".
        /**
         * \param copy The nature object from which properties are to be copied
         *             into this.
         * \param type The type object with which this nature is being used.
         */
        LinearConstraintNature(
            const LinearConstraintNature& copy,
            ConstraintTypeBase& type
            );

        /// Destructs a LinearConstraintNature object.
        virtual
        ~LinearConstraintNature(
            );



}; // class LinearConstraintNature


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
#include "./inline/LinearConstraintNature.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_LINEARCONSTRAINTNATURE_HPP
