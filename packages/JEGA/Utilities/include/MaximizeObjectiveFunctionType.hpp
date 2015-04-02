/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MaximizeObjectiveFunctionType.

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

        Mon Sep 08 12:25:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MaximizeObjectiveFunctionType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_MAXIMIZEOBJECTIVEFUNCTIONTYPE_HPP
#define JEGA_UTILITIES_MAXIMIZEOBJECTIVEFUNCTIONTYPE_HPP







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
class MaximizeObjectiveFunctionType;






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
/// An objective function type whereby the highest possible value is sought.
/**
 * This type is specialized to seek the maximum value for an objective.
 * The formulation is like this:
 * \verbatim
        max F(x)
   \endverbatim
 *
 * This type reports it's value for minimization as the negation of the current
 * value.
 */
class JEGA_SL_IEDECL MaximizeObjectiveFunctionType :
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

        /// Returns the identifier for this nature which is "Maximize".
        /**
         * \return The string "Maximize".
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


        /// This method returns the negative of "val".
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
         * \return The string "Maximize".
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


        /// Constructs an MaximizeObjectiveFunctionType for use with "info".
        /**
         * \param info The ObjectiveFunctionInfo for which this is to be the
         *             type.
         */
        MaximizeObjectiveFunctionType(
            ObjectiveFunctionInfo& info
            );

        /**
         * \brief Copy constructs an MaximizeObjectiveFunctionType for use
         *        with "info".
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ObjectiveFunctionInfo for which this is to be
         *             the type.
         */
        MaximizeObjectiveFunctionType(
            const MaximizeObjectiveFunctionType& copy,
            ObjectiveFunctionInfo& info
            );

        /// Destructs an MaximizeObjectiveFunctionType.
        virtual
        ~MaximizeObjectiveFunctionType(
            );



}; // class MaximizeObjectiveFunctionType



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
#include "./inline/MaximizeObjectiveFunctionType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_MAXIMIZEOBJECTIVEFUNCTIONTYPE_HPP
