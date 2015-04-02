/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ConstraintNatureBase.

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

        Tue Jun 10 10:23:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ConstraintNatureBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_CONSTRAINTNATUREBASE_HPP
#define JEGA_UTILITIES_CONSTRAINTNATUREBASE_HPP







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
class ConstraintInfo;
class ConstraintTypeBase;
class ConstraintNatureBase;







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

/// The base class for all constraint natures.
/**
 * See ConstraintInfo.hpp for a complete description of how the nature fits
 * into the idea of constraint information.
 *
 * The nature represents the behavior or form of the constraint function.
 */
class JEGA_SL_IEDECL ConstraintNatureBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The type of the constraint for which this is the nature.
        ConstraintTypeBase& _type;



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

        /// Returns the ConstraintTypeBase known by this object (non-const)
        /**
         * \return The ConstraintTypeBase for of the ConstraintInfo for which
         *         this is the nature.
         */
        inline
        ConstraintTypeBase&
        GetType(
            );

        /// Returns the ConstraintTypeBase known by this object (const)
        /**
         * \return The ConstraintTypeBase for of the ConstraintInfo for which
         *         this is the nature.
         */
        inline
        const ConstraintTypeBase&
        GetType(
            ) const;



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget being used by the ConstraintInfo as well
         *         as this nature.
         */
        const DesignTarget&
        GetDesignTarget(
            ) const;

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget being used by the ConstraintInfo as well
         *         as this nature.
         */
        DesignTarget&
        GetDesignTarget(
            );

        /// Returns the ConstraintInfo ultimately using this nature.
        /**
         * \return The ConstraintInfo for which this is the nature.
         */
        ConstraintInfo&
        GetConstraintInfo(
            );

        /// Returns the ConstraintInfo ultimately using this nature.
        /**
         * \return The ConstraintInfo for which this is the nature.
         */
        const ConstraintInfo&
        GetConstraintInfo(
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
            ) const = 0;

        /// Returns the string name of this nature.
        /**
         * Subclasses must override this method to return an appropriate name
         * for this nature.  Examples include "linear" and "non-linear".
         *
         * \return The name of this constraint nature.
         */
        virtual
        std::string
        ToString(
            ) const = 0;

        /// This method attempts to evalutate this constraint for "des".
        /**
         * This may not be possible.  Generally, only linear constraints
         * can be evaluated in this way.  The method returns true if the
         * constraint value was properly computed and recorded in "des".
         * false otherwise.
         *
         * Subclasses must override this method.
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

        /// Constructs a ConstraintNatureBase known by "type".
        /**
         * \param type The type object with which this nature is being used.
         */
        ConstraintNatureBase(
            ConstraintTypeBase& type
            );

        /// Copy constructs a ConstraintTypeBase known by "info".
        /**
         * \param copy The nature object from which properties are to be copied
         *             into this.
         * \param type The type object with which this nature is being used.
         */
        ConstraintNatureBase(
            const ConstraintNatureBase& copy,
            ConstraintTypeBase& type
            );

        /// Destructs a ConstraintNatureBase object.
        virtual
        ~ConstraintNatureBase(
            );


}; // class ConstraintNatureBase



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
#include "./inline/ConstraintNatureBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_CONSTRAINTNATUREBASE_HPP
