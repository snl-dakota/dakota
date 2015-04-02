/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of of class ConstraintTypeBase.

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

        Tue Jun 10 08:40:44 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ConstraintTypeBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_CONSTRAINTTYPEBASE_HPP
#define JEGA_UTILITIES_CONSTRAINTTYPEBASE_HPP







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

/// The base class for all constraint types.
/**
 * See ConstraintInfo.hpp for a complete description of how the type fits
 * into the idea of constraint information.
 *
 * The type represents the kind of constraint.
 */
class JEGA_SL_IEDECL ConstraintTypeBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The ConstraintInfo for which this is the type.
        ConstraintInfo& _info;


        /// The nature of the constraint which is actually stored here.
        /**
         * Some examples of natures are "Linear" and "Non-Linear".
         */
        ConstraintNatureBase* _nature;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /**
         * \brief Tells this object of the nature of the constraint it helps
         *        represent.
         *
         * This method fails if IsNatureLocked returns true.
         * Returns true on success and false on failure.
         *
         * \param nature The new nature to use in conjunction with this type.
         * \return true if the nature is successfully set and false otherwise.
         */
        bool
        SetNature(
            ConstraintNatureBase* nature
            );


    protected:


    private:



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the ConstraintInfo object known by this (non-const)
        /**
         * \return The ConstraintInfo for which this is the type.
         */
        inline
        ConstraintInfo&
        GetConstraintInfo(
            );

        /// Returns the ConstraintInfo object known by this (const)
        /**
         * \return The ConstraintInfo for which this is the type.
         */
        inline
        const ConstraintInfo&
        GetConstraintInfo(
            ) const;

        /// Returns the nature of this type object.
        /**
         * \return The nature being used along with this type.
         */
        inline
        ConstraintNatureBase&
        GetNature(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget for which this describes the type of a
         *         constraint.
         */
        const DesignTarget&
        GetDesignTarget(
            ) const;

        /// Returns the DesignTarget responsible for the info object.
        /**
         * \return The DesignTarget for which this describes the type of a
         *         constraint.
         */
        DesignTarget&
        GetDesignTarget(
            );

        /// Returns a string representation of the nature of this type.
        /**
         * Some examples are "Linear" and "Non-Linear".
         *
         * \return The name of the nature associated with this type.
         */
        std::string
        GetNatureString(
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

        /// Returns an exact duplicate of this type object.
        /**
         * \param forCNI The ConstraintInfo for which the clone is being
         *               created.
         * \return An exact duplicate of this type for use by "forCNI".
         */
        virtual
        ConstraintTypeBase*
        Clone(
            ConstraintInfo& forCNI
            ) const = 0;

        /// Returns the string name of this type.
        /**
         * \return The name of this type such as "Equality", "Inequality", etc.
         */
        virtual
        std::string
        ToString(
            ) const = 0;

        /**
         * \brief Returns true if the nature of this type cannot be changed.
         *
         * \return false if the nature can be changed and true otherwise.
         */
        virtual
        bool
        IsNatureLocked(
            ) const = 0;

        /**
         * \brief Override to return the amount by which "of" violates this
         *        constraint.
         *
         * The implementation of this in derived classes may allow for the
         * return of both positive and negative numbers.  See each derived type
         * for details.
         *
         * \param of The value to test against the requirements of this
         *           constraint.
         * \return The amount by which "of" violates this constraint.  0 if
         *         "of" is not in violation.
         */
        virtual
        double
        GetViolationAmount(
            double of
            ) const = 0;

        /// Returns the amount by which "des1" is preferred to "des2".
        /**
         * If "des1" is not preferred to "des2", 0.0 is returned.
         * Override for custom behavior.  Default behavior is to return
         * the difference between the violation amounts.
         *
         * \param des1 The first Design whose constraint value is to be
         *             considered.
         * \param des2 The second Design whose constraint value is to be
         *             considered.
         * \return The amount by which des1 is preferred to des2.  If des1
         *         is preferred, the return is a positive number.  If des2
         *         is preferred, the return is a negative number.  Otherwise,
         *         the return is 0.
         */
        virtual
        double
        GetPreferredAmount(
            const Design& des1,
            const Design& des2
            ) const;

        /// This method attempts to evalutate this constraint for "des".
        /**
         * This may not be possible.  Generally, only linear constraints
         * can be evaluated in this way.  The method returns true if the
         * constraint value was properly computed and recorded in "des".
         * False otherwise.
         *
         * \param des The Design for which this constraint is to be evaluated.
         * \return true if Design is properly evaluated and false otherwise.
         */
        virtual
        bool
        EvaluateConstraint(
            Design& des
            ) const;

        virtual
        std::string
        GetEquation(
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

        /// Constructs a ConstraintTypeBase to be known by "info".
        /**
         * Default nature is Non-Linear unless otherwise required.
         *
         * \param info The ConstraintInfo for which this is to be the type.
         */
        ConstraintTypeBase(
            ConstraintInfo& info
            );

        /// Copy constructs a ConstraintTypeBase to be known by "info".
        /**
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ConstraintInfo for which this is to be the type.
         */
        ConstraintTypeBase(
            const ConstraintTypeBase& copy,
            ConstraintInfo& info
            );

        /// Destructs a ConstraintTypeBase object.
        /**
         * This destructor causes destruction of the associated nature object
         * as well.
         */
        virtual
        ~ConstraintTypeBase(
            );




}; // class ConstraintTypeBase



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
#include "./inline/ConstraintTypeBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_CONSTRAINTTYPEBASE_HPP
