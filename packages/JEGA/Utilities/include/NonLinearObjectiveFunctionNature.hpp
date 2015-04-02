/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NonLinearObjectiveFunctionNature.

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

        Sun Sep 14 16:39:53 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NonLinearObjectiveFunctionNature
 *        class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_NONLINEAROBJECTIVEFUNCTIONNATURE_HPP
#define JEGA_UTILITIES_NONLINEAROBJECTIVEFUNCTIONNATURE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/ObjectiveFunctionNatureBase.hpp>





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
class NonLinearObjectiveFunctionNature;







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
/// A nature for non-linear objective functions.
/**
 * A non-linear objective function has no predefined form except to say that it
 * does not fit the form of a linear objective function.
 */
class JEGA_SL_IEDECL NonLinearObjectiveFunctionNature :
    public ObjectiveFunctionNatureBase
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
        ObjectiveFunctionNatureBase*
        Clone(
            ObjectiveFunctionTypeBase& forType
            ) const;

        /// Returns the string name of this nature.
        /**
         * \return The string "Non-Linear".
         */
        virtual
        std::string
        ToString(
            ) const;

        /// This method does not work for this nature.
        /**
         * These types of constraints must be evaluated in a different way.
         *
         * \param des The Design for which to evaluate this objective.
         * \return false, always.
         */
        virtual
        bool
        EvaluateObjective(
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


        /// Constructs a NonLinearObjectiveFunctionNature known by "type".
        /**
         * \param type The type object with which this nature is being used.
         */
        NonLinearObjectiveFunctionNature(
            ObjectiveFunctionTypeBase& type
            );

        /// Copy constructs a ObjectiveFunctionTypeBase known by "info".
        /**
         * \param copy The nature object from which properties are to be copied
         *             into this.
         * \param type The type object with which this nature is being used.
         */
        NonLinearObjectiveFunctionNature(
            const NonLinearObjectiveFunctionNature& copy,
            ObjectiveFunctionTypeBase& type
            );

        /// Destructs a NonLinearObjectiveFunctionNature object.
        virtual
        ~NonLinearObjectiveFunctionNature(
            );




}; // class NonLinearObjectiveFunctionNature



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
#include "./inline/NonLinearObjectiveFunctionNature.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_NONLINEAROBJECTIVEFUNCTIONNATURE_HPP
