/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ConstraintInfo.

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

        Mon Jun 09 16:16:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ConstraintInfo class.
 */


/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_CONSTRAINTINFO_HPP
#define JEGA_UTILITIES_CONSTRAINTINFO_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/int_types.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/InfoBase.hpp>
#include <../Utilities/include/ConstraintTypeBase.hpp>

#ifdef JEGA_THREADSAFE
#include <threads/include/mutex.hpp>
#endif






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
File Scope Typedefs
================================================================================
*/






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

/// Stores and provides information about a constraint.
/**
 * Constraint information is divided up into two concepts similar to the way
 * design variable information is broken up.  They are the type and the nature.
 *
 * The type indicates what kind of constraint this is.  For example, some
 * types are EqualityConstraintType, InequalityConstraintType, and
 * TwoSidedInequalityConstraintType.
 *
 * The nature indicates the behavior of the constraint function.  Some examples
 * are LinearConstraintNature and NonLinearConstraintNature.
 *
 * Most of the information and capabilities provided by this class are provided
 * and carried out by forwarding work on to the type and nature.
 */
class JEGA_SL_IEDECL ConstraintInfo :
    public InfoBase
{

    /*
    ============================================================================
    Typedefs
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

        /// Represents the type of this constraint.
        /**
         * It is something like equality, inequality, etc.
         */
        ConstraintTypeBase* _type;

        /// This is the total violation recorded for this constraint.
        double _totalViolation;

        /**
         * \brief The total number of violations that have contributed to
         *        the _totalViolation.
         *
         * This is kept for the purpose of computing averages.
         */
        eddy::utilities::uint64_t _numViolations;

        JEGA_IF_THREADSAFE(mutable eddy::threads::mutex _violMutex);

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Tells this object what type of constraint it represents.
        /**
         * This object assumes ownership of \a type and will destroy
         * it as it sees fit.  Do not store \a type elsewhere.
         *
         * \param type The new type for this constraint.
         */
        void
        SetType(
            ConstraintTypeBase* type
            );




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /// Returns the type represented by this object.
        /**
         * \return The type object stored in this info object.
         */
        inline
        ConstraintTypeBase&
        GetType(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /**
         * \brief Returns the raw amount by which the value stored
         * in \a of violates this constraint.
         *
         * If \a of does not violate this constraint, the return is 0.0.
         *
         * This method does not attempt to calculate the constraint value based
         * on the variable values in \a of.  It simply looks at the value
         * stored in the appropriate constraint slot in of and uses that.
         *
         * \param of The Design to check for violation.
         * \return The raw amount by which \a of violates this constraint.
         */
        inline
        double
        GetViolationAmount(
            const Design& of
            ) const;

        /// Returns the raw amount by which \a of violates this constraint.
        /**
         * If \a of does not violate this constraint, the return is 0.0.
         *
         * \param of The value to check for violation.
         * \return The raw amount by which \a of violates this constraint.
         */
        inline
        double
        GetViolationAmount(
            double of
            ) const;

        /**
         * \brief Returns the percentage by which the value stored
         * in \a of violates this constraint.
         *
         * If \a of does not violate this constraint, the return is 0.0.
         *
         * This value is calculated using the current average constraint
         * violation which must be kept up-to-date using the RecordViolation
         * method as Design's are evaluated.  If RecordViolation has not
         * yet been called with a value that violates the constraint, the
         * return of this method will be either 100% if of violates or 0% if it
         * does not.
         *
         * This method does not attempt to calculate the constraint value based
         * on the variable values in \a of.  It simply looks at the value
         * stored in the appropriate constraint slot in of and uses that.
         *
         * \param of The Design to check for violation.
         * \return The percentage by which \a of violates this constraint in the
         *         range [0, 100].
         */
        inline
        double
        GetViolationPercentage(
            const Design& of
            ) const;

        /// Returns the percentage by which \a of violates this constraint.
        /**
         * This value is calculated using the current average constraint
         * violation which must be kept up-to-date using the RecordViolation
         * method as Design's are evaluated.
         *
         * See the overload for more information.
         *
         * \param of The value for which to compute the violation percentage.
         * \return The percentage vioation of the supplied value for this
         *         constraint.
         */
        double
        GetViolationPercentage(
            double of
            ) const;

        /**
         * \brief Returns a pointer to the preferred Design or Null if neither
         * is preferred.
         *
         * This is computed by comparing the raw violation amounts of the two
         * Designs.
         *
         * \param des1 The first Design to be used in the comparison.
         * \param des2 The second Design to be used in the comparison.
         * \return A pointer to the preferred Design.
         */
        const Design*
        GetPreferredDesign(
            const Design& des1,
            const Design& des2
            ) const;

        /// Returns the amount by which \a des1 is preferred to \a des2.
        /**
         * If \a des1 is not preferred to \a des2, 0.0 is returned.  The
         * comparison is made using the raw violation amounts.
         *
         * \param des1 The Design being checked as the preferred Design.
         * \param des2 The Design being checked as the non-preferred Design.
         * \return The amount by which \a des1 is preferred to \a des2 if any.
         */
        inline
        double
        GetPreferredAmount(
            const Design& des1,
            const Design& des2
            ) const;

        /// Returns a string representation of the type of this constraint.
        /**
         * Some examples are "Equality", "Inequality", etc.
         *
         * \return The string representation of the type of this constraint.
         */
        inline
        std::string
        GetTypeString(
            ) const;

        /// Returns a string representation of the nature of this constraint.
        /**
         * Some examples are "Linear" and "Non-Linear".
         *
         * \return The string representation of the nature of this constraint.
         */
        inline
        std::string
        GetNatureString(
            ) const;

        /// Tells this object of the nature of the variable it represents.
        /**
         * This method fails if IsNatureLocked returns true.  This object
         * assumes ownership of \a nature and will destroy it as it sees fit.
         * Do not store \a nature elsewhere.
         *
         * \param nature The new nature for this constraint info.
         * \return true on success and false on failure.
         */
        inline
        bool
        SetNature(
            ConstraintNatureBase* nature
            );

        /// Returns the nature of the variable represented by this object.
        /**
         * \return A reference to the nature object of this constraint info.
         */
        inline
        ConstraintNatureBase&
        GetNature(
            ) const;

        /// Returns the constraint value associated with this info object.
        /**
         * For example, if this is the 3rd ConstraintInfo, then
         * this method will return the number stored in the third
         * constraint location in \a des.
         *
         * \param des The Design from which to retrieve the proper value.
         * \return The corresponding constraint value of the supplied Design.
         */
        inline
        double
        Which(
            const Design& des
            ) const;

        /**
         * \brief Uses the violation amount of \a of to learn the average
         *        magnitude of violations of this constraint.
         *
         * This class keeps a running average of the violations of this
         * constraint to serve as a means of normalizing constraint values for
         * cross comparison.
         *
         * If \a of does not violate this constraint, then it's value is not
         * considered.  Only violations are of interest.
         *
         * \param of The Design to check for violation.
         */
        inline
        void
        RecordViolation(
            const Design& of
            );

        /**
         * \brief Uses the raw constraint value \a rawValue to learn the
         *        average magnitude of violations of this constraint.
         *
         * This class keeps a running average of the violations of this
         * constraint to serve as a means of normalizing constraint values for
         * cross comparison.
         *
         * \a rawValue is expected to be the actual result of a calculation of
         * this constraint.  Not the return of GetViolationAmount or anything
         * else. The violation amount will be computed based on \a rawValue.
         *
         * The absolute value of rawValue will be used.  Only the magnitude of
         * violations is of interest.  Signed values would potentially obscure
         * the magnitudes.
         *
         * If \a rawValue does not violate this constraint, then it's value is
         * not considered.  Only violations are of interest.
         *
         * \param rawValue The raw constraint value to check for violation.
         */
        void
        RecordViolation(
            double rawValue
            );

        /// Computes and returns the average violation observed thus far.
        /**
         * This only makes sense once RecordViolation has been called at least
         * one time with a Design or value that violates the constriant.
         * If this is called prior to that, the return is 0.0.
         *
         * \return The average violation observed thus far by calls to
         *         RecordViolation.
         */
        double
        GetAverageViolation(
            ) const;

        /// This method attempts to evalutate this constraint for \a des.
        /**
         * This may not be possible.  Generally, only linear constraints
         * can be evaluated in this way.  The method returns true if the
         * constraint value was properly computed and recorded in \a des.
         * False otherwise.
         *
         * \param des The Design for which to evaluate the constraint
         *            associated with this nature.
         * \return True if the constraint could be evaluated and false
         *         otherwise.
         */
        inline
        bool
        EvaluateConstraint(
            Design& des
            ) const;

        inline
        std::string
        GetEquation(
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

        /// Constructs a ConstraintInfo object knowing \a target.
        /**
         * Default properties are type = inequality and nature = non-linear.
         *
         * \param target The DesignTarget for which this info is being
         *               constructed.
         */
        ConstraintInfo(
            DesignTarget& target
            );

        /// Copy constructs a ConstraintInfo for use by \a target.
        /**
         * \param copy The info to copy properties from.
         * \param target The DesignTarget for which this info is being
         *               constructed.
         */
        ConstraintInfo(
            const ConstraintInfo& copy,
            DesignTarget& target
            );

        /// Copy constructs a ConstraintInfo object.
        /**
         * \param copy The info to copy properties from including the target.
         */
        ConstraintInfo(
            const ConstraintInfo& copy
            );

        /// Destructs a ConstraintInfo object.
        /**
         * This destructor destoys the type object.
         */
        virtual
        ~ConstraintInfo(
            );

}; // class ConstraintInfo





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
#include "./inline/ConstraintInfo.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_CONSTRAINTINFO_HPP
