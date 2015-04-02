/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ObjectiveFunctionTypeBase.

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

        Mon Sep 08 12:24:09 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ObjectiveFunctionTypeBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_OBJECTIVEFUNCTIONTYPEBASE_HPP
#define JEGA_UTILITIES_OBJECTIVEFUNCTIONTYPEBASE_HPP







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
/// The base class for objective function type objects.
/**
 * This class spells out the requirements for all objective function types
 * as well as provides some basic functionality.
 *
 * Specialize this class for specific types of objective functions such
 * as the minimize type etc.
 */
class JEGA_SL_IEDECL ObjectiveFunctionTypeBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:


        /// The ObjectiveFunctionInfo for which this is the type.
        ObjectiveFunctionInfo& _info;


        /// The nature of the objective which is actually stored here.
        /**
         * Some examples of natures are "Linear" and "Non-Linear".
         */
        ObjectiveFunctionNatureBase* _nature;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /**
         * \brief Tells this object of the nature of the objective it helps
         *        represent.
         *
         * This fails if the supplied nature is the current nature or if it
         * is null.
         *
         * \param nature The new nature to use in conjunction with this type.
         * \return true if the nature is successfully set and false otherwise.
         */
        bool
        SetNature(
            ObjectiveFunctionNatureBase* nature
            );


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /// Returns the ObjectiveFunctionInfo object known by this (non-const)
        /**
         * \return The ObjectiveFunctionInfo for which this is the type.
         */
        inline
        ObjectiveFunctionInfo&
        GetObjectiveFunctionInfo(
            );

        /// Returns the ObjectiveFunctionInfo object known by this (const)
        /**
         * \return The ObjectiveFunctionInfo for which this is the type.
         */
        inline
        const ObjectiveFunctionInfo&
        GetObjectiveFunctionInfo(
            ) const;

        /// Returns the nature of this type object.
        /**
         * \return The nature being used along with this type.
         */
        inline
        ObjectiveFunctionNatureBase&
        GetNature(
            ) const;


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns a string representation of the nature of this type.
        /**
         * Some examples are "Linear" and "Non-Linear".
         *
         * \return The name of the nature associated with this type.
         */
        std::string
        GetNatureString(
            ) const;

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget for which this describes the type of an
         *         objective.
         */
        const DesignTarget&
        GetDesignTarget(
            ) const;

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget for which this describes the type of a
         *         objective.
         */
        DesignTarget&
        GetDesignTarget(
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

        /**
         * \brief This method must be overridden to return a minimization
         *        version of \a val.
         *
         * \a val must be considered to be the actual result of an objective
         * function evaluation and should be converted such that a proper value
         * for minimization is returned.
         *
         * \param val The raw objective function value to be converted for
         *            minimization.
         * \return \a val after transformation for use in minimization
         *         techniques.
         */
        virtual
        double
        GetValueForMinimization(
            double val
            ) const = 0;

        /// Returns the string name of this type.
        /**
         * \return The name of this type such as "Minimize", "Maximize", etc.
         */
        virtual
        std::string
        ToString(
            ) const = 0;

        /// Returns the amount by which \a val1 is preferred to \a val2.
        /**
         * If \a val1 is not preferred to \a val2, 0.0 is returned.  Override
         * for custom behavior.  Default behavior is to return the difference
         * between the values for minimization.
         *
         * \param val1 The first value to compare.
         * \param val2 The second value to compare.
         * \return The amount by which \a val1 is preferred to \a val2.  Will
         *         be negative if \a val2 is actually preferred to \a val1.
         */
        virtual
        double
        GetPreferredAmount(
            double val1,
            double val2
            ) const;

        /// Returns an exact duplicate of this type object.
        /**
         * \param forOFI The ObjectiveFunctionInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by \a forOFI.
         */
        virtual
        ObjectiveFunctionTypeBase*
        Clone(
            ObjectiveFunctionInfo& forOFI
            ) const = 0;

        /**
         * \brief This method attempts to evalutate this objective function for
         *        \a des.
         *
         * This may not be possible.  Generally, only linear objective
         * functions can be evaluated in this way.  The method returns true if
         * the objective function value was properly computed and recorded in
         * \a des. False otherwise.
         *
         * \param des The Design for which this objective is to be evaluated.
         * \return true if Design is properly evaluated and false otherwise.
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


        /// Constructs an ObjectiveFunctionTypeBase for use with \a info.
        /**
         * Default nature is Non-Linear until otherwise specified.
         *
         * \param info The ObjectiveFunctionInfo for which this is to be the
         *             type.
         */
        ObjectiveFunctionTypeBase(
            ObjectiveFunctionInfo& info
            );

        /// Copy constructs an ObjectiveFunctionTypeBase for use with \a info.
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ObjectiveFunctionInfo for which this is to be
         *             the type.
         */
        ObjectiveFunctionTypeBase(
            const ObjectiveFunctionTypeBase& copy,
            ObjectiveFunctionInfo& info
            );

        /// Destructs an ObjectiveFunctionTypeBase.
        virtual
        ~ObjectiveFunctionTypeBase(
            );

}; // class ObjectiveFunctionTypeBase



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
#include "./inline/ObjectiveFunctionTypeBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_OBJECTIVEFUNCTIONTYPEBASE_HPP
