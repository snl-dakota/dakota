/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NonLinearConstraintNature.

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

        Tue Jun 10 10:23:56 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NonLinearConstraintNature class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_NONLINEARCONSTRAINTNATURE_HPP
#define JEGA_UTILITIES_NONLINEARCONSTRAINTNATURE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

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
class NonLinearConstraintNature;







/*
================================================================================
Class Definition
================================================================================
*/

/// A nature for non-linear constraints.
/**
 * A non-linear constraint has no predefined form except to say that it
 * does not fit the form of a linear constraint.  This nature object
 * is unable to evaluate constraint values.
 */
class JEGA_SL_IEDECL NonLinearConstraintNature :
    public ConstraintNatureBase
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


        /// Returns the identifier for this nature which is "Non-Linear".
        /**
         * \return The string "Non-Linear".
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
         * \return The string "Non-Linear".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns an exact duplicate of this nature object.
        /**
         * Subclasses must override this method to return an exact duplicate
         * of themselves.
         *
         * \param forType the type object for which this nature is being
         *        created.
         * \return An exact duplicate of the fully derived type of this nature.
         */
        virtual
        ConstraintNatureBase*
        Clone(
            ConstraintTypeBase& forType
            ) const;

        /// This method does not work for this nature.
        /**
         * These types of constraints must be evaluated in a different way.
         *
         * \param des The Design for which to evaluate this constraint.
         * \return false, always.
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

        /// Constructs a NonLinearConstraintNature known by "type".
        /**
         * \param type The type object with which this nature is being used.
         */
        NonLinearConstraintNature(
            ConstraintTypeBase& type
            );

        /// Copy constructs a ConstraintTypeBase known by "info".
        /**
         * \param copy The nature object from which properties are to be copied
         *             into this.
         * \param type The type object with which this nature is being used.
         */
        NonLinearConstraintNature(
            const NonLinearConstraintNature& copy,
            ConstraintTypeBase& type
            );

        /// Destructs a ConstraintNatureBase object.
        virtual
        ~NonLinearConstraintNature(
            );



}; // class NonLinearConstraintNature


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
#include "./inline/NonLinearConstraintNature.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_NONLINEARCONSTRAINTNATURE_HPP
