/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MDesign.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Feb 15 08:41:41 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MDesign class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MDESIGN_HPP
#define JEGA_FRONTEND_MANAGED_MDESIGN_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
==========================
#pragma unmanaged======================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
    }
}



/*
================================================================================
Namespace Aliases
================================================================================
*/
#pragma managed
#using <mscorlib.dll>





/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace Managed {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
MANAGED_CLASS_FORWARD_DECLARE(public, MDesign);







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
/**
 * \brief
 *
 *
 */
MANAGED_CLASS(public, MDesign)
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

        JEGA::Utilities::Design* _design;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the indicated design variable value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ndv).
         *
         * \param num The index of the variable to set.
         * \param val The new value for the variable.
         */
        void
        SetVariable(
            eddy::utilities::uint64_t num,
            double val
            );

        /// Sets the indicated objective function value.
        /**
         * It is up to you to make sure that "num" is in bounds
         * (less than nof).
         *
         * \param num The index of the objective to set.
         * \param val The new value for the objective.
         */
        void
        SetObjective(
            eddy::utilities::uint64_t num,
            double val
            );

        /// Sets the indicated constraint function value.
        /**
         * It is up to you to make sure that "num" is in bounds
         * (less than ncn).
         *
         * \param num The index of the constraint to set.
         * \param val The new value for the constraint.
         */
        void
        SetConstraint(
            eddy::utilities::uint64_t num,
            double val
            );

        void
        SetTag(
            System::Object^ tag
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Retrieves the requested design variable value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ndv).
         *
         * In terms of the way the Design class works, this method is akin
         * to the GetVariableValue method.
         *
         * \param num The index of the variable whose value is sought.
         * \return The design variable value stored in location \a num.
         */
        double
        GetVariable(
            eddy::utilities::uint64_t num
            );

        /// Retrieves the requested objective function value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than nof).
         *
         * \param num The index of the objective whose value is sought.
         * \return The objective value stored in location \a num.
         */
        double
        GetObjective(
            eddy::utilities::uint64_t num
            );

        /// Retrieves the requested constraint function value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ncn).
         *
         * \param num The index of the constraint whose value is sought.
         * \return The constraint value stored in location \a num.
         */
        double
        GetConstraint(
            eddy::utilities::uint64_t num
            );

        eddy::utilities::uint64_t
        GetID(
            );

        System::Object^
        GetTag(
            );


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the number of design variables.
        /**
         * This information is retrieved from the known DesignTarget.
         *
         * \return The number of design variables for the current problem.
         */
        eddy::utilities::uint64_t
        GetNDV(
            );

        /// Returns the number of objective functions.
        /**
         * This information is retrieved from the known DesignTarget.
         *
         * \return The number of objective functions for the current problem.
         */
        eddy::utilities::uint64_t
        GetNOF(
            );

        /// Returns the number of constraints.
        /**
         * This information is retrieved from the known DesignTarget.
         *
         * \return The number of constraints for the current problem.
         */
        eddy::utilities::uint64_t
        GetNCN(
            );

        /// Returns true if this Design has been evaluated.
        /**
         * \return true if this Design has the Evaluated attribute bit set
         *         and false otherwise.
         */
        bool
        IsEvaluated(
            );

        /// Returns true if no constraints are violated for this Design.
        /**
         * This takes into account both bound and non-bound constraints.
         *
         * \return true if this Design has the FeasibleBounds and
         *         FeasibleConstraints attribute bits set and false otherwise.
         */
        bool
        IsFeasible(
            );

        /**
         * \brief Returns true if no bound constraints are violated for this
         *        Design.
         *
         * This takes into account only bound constraints.  This method may
         * return true even if IsFeasible returns false.
         *
         * \return true if this Design has the FeasibleBounds attribute bit
         *         set and false otherwise.
         */
        bool
        SatisfiesBounds(
            );

        /**
         * \brief Returns true if no non-bound constraints are violated for
         *        this Design.
         *
         * This takes into account only non-bound constraints.  This method may
         * return true even if IsFeasible returns false.
         *
         * \return true if this Design has the FeasibleConstraints attribute
         *         bit set and false otherwise.
         */
        bool
        SatisfiesConstraints(
            );

        /// Marks this Design as being illconditioned.
        /**
         * \param lval True to set the Illconditioned bit and false to
         *             unset it.
         */
        void
        SetIllconditioned(
            bool lval
            );

        /// Returns true if evaluation failed for this Design.
        /**
         * \return true if this Design has the Illconditioned attribute
         *         bit set and false otherwise.
         */
        bool
        IsIllconditioned(
            );

        void
        ResetDesign(
            JEGA::Utilities::Design* toWrap
            );

        static
        MDesign MOH
        Create(
            MDesign MOH hint
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

        virtual
        JEGA::Utilities::Design&
        Manifest(
            );

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

        static
        MDesign(
            );

        /**
         * \brief Constructs a Managed Design wrapper using the supplied JEGA
         *        Design.
         *
         * \param toWrap The Design used to construct this managed wrapper
         *               class.
         */
        MDesign(
            JEGA::Utilities::Design* toWrap
            );

        /**
         * \brief Destructs a Managed Design wrapper.
         *
         * This method does nothing but null the reference to the known native
         * Design class object.  It does not destroy (delete) it.
         */
        ~MDesign(
            );

}; // class MDesign



/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_MANAGED_MDESIGN_HPP
