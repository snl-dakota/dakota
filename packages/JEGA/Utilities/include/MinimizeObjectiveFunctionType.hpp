/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MinimizeObjectiveFunctionType.

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

        Mon Sep 08 12:24:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MinimizeObjectiveFunctionType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_MINIMIZEOBJECTIVEFUNCTIONTYPE_HPP
#define JEGA_UTILITIES_MINIMIZEOBJECTIVEFUNCTIONTYPE_HPP






/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>






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
class MinimizeObjectiveFunctionType;






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
/// An objective function type whereby the lowest possible value is sought.
/**
 * This type is specialized to seek the minimum value for an objective.
 * The formulation is like this:
 * \verbatim
        min F(x)
   \endverbatim
 *
 * This type reports it's value for minimization as the current value.  The
 * value is unchanged.
 */
class JEGA_SL_IEDECL MinimizeObjectiveFunctionType :
    public ObjectiveFunctionTypeBase
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


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the identifier for this nature which is "Minimize".
        /**
         * \return The string "Minimize".
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

        /// This method returns the "val" unchanged.
        /*
         * \param val The raw objective function value to be converted for
         *            minimization.
         * \return "val" after transformation for use in minimization
         *         techniques.
         */
        virtual
        double
        GetValueForMinimization(
            double val
            ) const;

        /// Returns the string name of this type.
        /**
         * \return The string "Minimize".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// Returns an exact duplicate of this type object.
        /**
         * \param forOFI The ObjectiveFunctionInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by "forOFI".
         */
        virtual
        ObjectiveFunctionTypeBase*
        Clone(
            ObjectiveFunctionInfo& forOFI
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

        /// Constructs an MinimizeObjectiveFunctionType for use with "info".
        /**
         * \param info The ObjectiveFunctionInfo for which this is to be the
         *             type.
         */
        MinimizeObjectiveFunctionType(
            ObjectiveFunctionInfo& info
            );

        /**
         * \brief Copy constructs an MinimizeObjectiveFunctionType for use
         *        with "info".
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ObjectiveFunctionInfo for which this is to be
         *             the type.
         */
        MinimizeObjectiveFunctionType(
            const MinimizeObjectiveFunctionType& copy,
            ObjectiveFunctionInfo& info
            );

        /// Destructs an MinimizeObjectiveFunctionType.
        virtual
        ~MinimizeObjectiveFunctionType(
            );


}; // class MinimizeObjectiveFunctionType



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
#include "./inline/MinimizeObjectiveFunctionType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_MINIMIZEOBJECTIVEFUNCTIONTYPE_HPP
