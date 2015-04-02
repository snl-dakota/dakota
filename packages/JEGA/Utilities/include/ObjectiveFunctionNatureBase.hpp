/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ObjectiveFunctionNatureBase.

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

        Sun Sep 14 13:59:56 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ObjectiveFunctionNatureBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_OBJECTIVEFUNCTIONNATUREBASE_HPP
#define JEGA_UTILITIES_OBJECTIVEFUNCTIONNATUREBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>







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
class DesignTarget;
class ObjectiveFunctionInfo;
class ObjectiveFunctionTypeBase;
class ObjectiveFunctionNatureBase;





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
/// The base class for all objective function natures.
/**
 * See ObjectiveFunctionInfo.hpp for a complete description of how the nature
 * fits into the idea of objective function information.
 *
 * The nature represents the behavior or form of the objective function.
 */
class JEGA_SL_IEDECL ObjectiveFunctionNatureBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:


        /// The type of the objective for which this is the nature.
        ObjectiveFunctionTypeBase& _type;




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


        /**
         * \brief Returns the ObjectiveFunctionTypeBase known by this object
         *        (non-const)
         *
         * \return The type object in association with this nature.
         */
        inline
        ObjectiveFunctionTypeBase&
        GetType(
            );

        /// Returns the ObjectiveFunctionTypeBase known by this object (const)
        /**
         * \return The type object in association with this nature.
         */
        inline
        const ObjectiveFunctionTypeBase&
        GetType(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget known to the objective function info object
         *         for which this is the nature.
         */
        const DesignTarget&
        GetDesignTarget(
            ) const;

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget known to the objective function info object
         *         for which this is the nature.
         */
        DesignTarget&
        GetDesignTarget(
            );

        /// Returns the ObjectiveFunctionInfo ultimately using this nature.
        /**
         * \return The ObjectiveFunctionInfo for which this is the nature.
         */
        ObjectiveFunctionInfo&
        GetObjectiveFunctionInfo(
            );

        /// Returns the ObjectiveFunctionInfo ultimately using this nature.
        /**
         * \return The ObjectiveFunctionInfo for which this is the nature.
         */
        const ObjectiveFunctionInfo&
        GetObjectiveFunctionInfo(
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

        /// Override to returns the string name of this nature.
        /**
         * Examples would be "Linear" and "Non-Linear".
         *
         * \return The string name of this nature.
         */
        virtual
        std::string
        ToString(
            ) const = 0;

        /// Returns an exact duplicate of this nature object.
        /**
         * \param forType The type object that this nature will be used with
         *                to describe an objective function.
         * \return A clone of this nature but for use with the supplied type
         *         object.
         */
        virtual
        ObjectiveFunctionNatureBase*
        Clone(
            ObjectiveFunctionTypeBase& forType
            ) const = 0;

        /**
         * \brief This method attempts to evalutate this objective function for
         *        \a des.
         *
         * This may not be possible.  Generally, only linear objective
         * functions can be evaluated in this way.  The method returns true if
         * the objective function value was properly computed and recorded in
         * \a des.  false otherwise.
         *
         * \param des The Design for which to evaluate the objective associated
         *            with this nature.
         * \return True if the objective could be evaluated and false
         *         otherwise.
         */
        virtual
        bool
        EvaluateObjective(
            Design& des
            ) const = 0;

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


        /// Constructs a ObjectiveFunctionNatureBase known by \a type.
        /**
         * \param type The type object to be associated with this new nature.
         */
        ObjectiveFunctionNatureBase(
            ObjectiveFunctionTypeBase& type
            );

        /// Copy constructs a ObjectiveFunctionTypeBase known by \a info.
        /**
         * \param copy The existing nature object from which to copy properties
         *             into this.
         * \param type The type object to be associated with this new nature.
         */
        ObjectiveFunctionNatureBase(
            const ObjectiveFunctionNatureBase& copy,
            ObjectiveFunctionTypeBase& type
            );

        /// Destructs a ObjectiveFunctionNatureBase object.
        virtual
        ~ObjectiveFunctionNatureBase(
            );


}; // class ObjectiveFunctionNatureBase



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
#include "./inline/ObjectiveFunctionNatureBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_OBJECTIVEFUNCTIONNATUREBASE_HPP
