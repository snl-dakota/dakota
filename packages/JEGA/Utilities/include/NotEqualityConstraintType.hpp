/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NotEqualityConstraintType.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Aug 29 11:01:17 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NotEqualityConstraintType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_NOTEQUALITYCONSTRAINTTYPE_HPP
#define JEGA_UTILITIES_NOTEQUALITYCONSTRAINTTYPE_HPP







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
class NotEqualityConstraintType;







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
/// A type for not-equality constraints.
/**
 * A not-equality constraint is one for which there is a particular value that
 * is forbidden and for which all other values are acceptale.
 */
class JEGA_SL_IEDECL NotEqualityConstraintType :
    public ConstraintTypeBase
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// This is the value to which the expression cannot equate.
        /**
         * Default value is 0.0.
         */
        double _value;

        /// This is the amount reported as the violation amount.
        /**
         * This value will be a postive, non-zero multiple of 10 on the order
         * of magnitude of the \a _value.
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
         * \param val The new singular infeasible value for this constraint.
         */
        void
        SetTabooValue(
            double val
            );


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the value of the \a _value member variable.
        /**
         * \return The singular infeasible value for this constraint.
         */
        inline
        double
        GetTabooValue(
            ) const;


    protected:


    private:


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
         * For the not-equality constraint type, this is a nominal value
         * computed to be on the order of the avoided value.  If the avoided
         * value is 0, the violation will be 10.  Otherwise it will be the
         * multiple of 10 that is closest to the avoided value.  It will always
         * be a positive value or 0.
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


        /// Constructs a NotEqualityConstraintType to be known by \a info.
        /**
         * \param info The ConstraintInfo for which this is to be the type.
         */
        NotEqualityConstraintType(
            ConstraintInfo& info
            );

        /// Copy constructs a NotEqualityConstraintType to be known by \a info.
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ConstraintInfo for which this is to be the type.
         */
        NotEqualityConstraintType(
            const NotEqualityConstraintType& copy,
            ConstraintInfo& info
            );




}; // class NotEqualityConstraintType



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/NotEqualityConstraintType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_NOTEQUALITYCONSTRAINTTYPE_HPP
