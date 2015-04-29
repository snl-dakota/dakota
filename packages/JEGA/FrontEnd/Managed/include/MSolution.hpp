/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MSolution.

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

        Mon Feb 13 13:14:41 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MSolution class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MSOLUTION_HPP
#define JEGA_FRONTEND_MANAGED_MSOLUTION_HPP

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
================================================================================
*/
#pragma unmanaged
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
MANAGED_CLASS_FORWARD_DECLARE(public, MSolution);
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
 * \brief A class to store a solution from JEGA for the managed project.
 *
 * This does not acutally wrap a JEGA class.   It is however built using
 * a JEGA core Design class object and the interface to this class is similar
 * to that of a JEGA Design.
 */
MANAGED_CLASS(public, MSolution)
{
    /*
    ============================================================================
    Enumerations
    ============================================================================
    */
    public:


    protected:


    private:


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

        /// The list of design variable values of this solution.
        DoubleVector MOH _X;

        /// The list of objective function values of this solution.
        DoubleVector MOH _F;

        /// The list of constraint values of this solution.
        DoubleVector MOH _G;

        /// A bitmask of attributes used to eliminate the need for many bools.
        eddy::utilities::bitmask8_t _attributes;

        /// The ID of the design that was used to construct this solution.
        std::size_t _id;

        System::Object^ _tag;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


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

        /// Returns true if this Solution has been evaluated.
        /**
         * \return true if this Solution has the Evaluated attribute bit set
         *         and false otherwise.
         */
        inline
        bool
        IsEvaluated(
            );

        /// Returns true if no constraints are violated for this Solution.
        /**
         * This takes into account both bound and non-bound constraints.
         *
         * \return true if this Solution has the FeasibleBounds and
         *         FeasibleConstraints attribute bits set and false otherwise.
         */
        inline
        bool
        IsFeasible(
            );

        /**
         * \brief Returns true if no bound constraints are violated for this
         *        Solution.
         *
         * This takes into account only bound constraints.  This method may
         * return true even if IsFeasible returns false.
         *
         * \return true if this Solution has the FeasibleBounds attribute bit
         *         set and false otherwise.
         */
        inline
        bool
        SatisfiesBounds(
            );

        /**
         * \brief Returns true if no non-bound constraints are violated for
         *        this Solution.
         *
         * This takes into account only non-bound constraints.  This method may
         * return true even if IsFeasible returns false.
         *
         * \return true if this Solution has the FeasibleConstraints attribute
         *         bit set and false otherwise.
         */
        inline
        bool
        SatisfiesConstraints(
            );

        /// Returns true if evaluation failed for this Solution.
        /**
         * \return true if this Solution has the Illconditioned attribute
         *         bit set and false otherwise.
         */
        inline
        bool
        IsIllconditioned(
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:


        /// Check to see if the indicated attributes are set for this Solution.
        /**
         * This method is public because it is useful in checking the "extra"
         * attribute bits (4, 5, 6, and 7) which can be used freely in the
         * Design class from which the attributes of this solution were taken.
         *
         * \param attrIndex The index of the attribute to test for.
         * \return True if this Solution has all the attributes indicated by
         *         \a attrIndex and false otherwise.
         */
        bool
        HasAttribute(
            eddy::utilities::uint64_t attrIndex
            );




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

        /**
         * \brief Constructs a Managed solution wrapper using the supplied JEGA
         *        Design.
         *
         * Properties from the supplied design are written into this class such
         * as the variable and response data and the attributes of the Design.
         *
         * \param des The solution used to construct this managed wrapper
         *            class.
         */
        MSolution(
            const JEGA::Utilities::Design& des
            );

        MSolution(
            MDesign MOH des
            );

}; // class MSolution



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
#endif // JEGA_FRONTEND_MANAGED_MSOLUTION_HPP
