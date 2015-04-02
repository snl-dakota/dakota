/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class SeekRangeObjectiveFunctionType.

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

        Sun Oct 12 17:36:14 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the SeekRangeObjectiveFunctionType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_SEEKRANGEOBJECTIVEFUNCTIONTYPE_HPP
#define JEGA_UTILITIES_SEEKRANGEOBJECTIVEFUNCTIONTYPE_HPP







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
class SeekRangeObjectiveFunctionType;






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
/// An objective function type whereby a particular range of values is sought.
/**
 * This type is specialized to seek any in a range of values for an objective.
 * The formulation is like this:
 * \verbatim
        lb <= F(x) <= ub
   \endverbatim
 * where lb is the lower bound on the range and ub is the upper bound.
 *
 * This type reports it's value for minimization as the amount by which the
 * current value lies outside the range.  The reported value is always
 * positive.  The minimization value is 0 if the current value satisfies
 * the above inequality.
 */
class JEGA_SL_IEDECL SeekRangeObjectiveFunctionType :
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


        /// The lower bound of the range to seek
        double _lowerBound;

        /// The upper bound of the range to seek
        double _upperBound;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the lower bound on the range this objective seeks.
        /**
         * \param value The new lower bound on the range sought by this
         *              objective.
         */
        inline
        void
        SetLowerBound(
            double value
            );

        /// Sets the upper bound on the range this objective seeks.
        /**
         * \param value The new upper bound on the range sought by this
         *              objective.
         */
        inline
        void
        SetUpperBound(
            double value
            );


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the lower bound on the range this objective seeks.
        /**
         * \return The lower bound on the range sought by this objective.
         */
        inline
        double
        GetLowerBound(
            ) const;

        /// Returns the upper bound on the range this objective seeks.
        /**
         * \return The upper bound on the range sought by this objective.
         */
        inline
        double
        GetUpperBound(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the identifier for this nature which is "Seek Range".
        /**
         * \return The string "Seek Range".
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


        /// This method returns the amount by which "val" is out of bounds.
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
         * \return The string "Seek Range".
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


        /// Constructs an SeekRangeObjectiveFunctionType for use with "info".
        /**
         * \param info The ObjectiveFunctionInfo for which this is to be the
         *             type.
         */
        SeekRangeObjectiveFunctionType(
            ObjectiveFunctionInfo& info
            );

        /**
         * \brief Copy constructs an SeekRangeObjectiveFunctionType for use
         *        with "info".
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ObjectiveFunctionInfo for which this is to be
         *             the type.
         */
        SeekRangeObjectiveFunctionType(
            const SeekRangeObjectiveFunctionType& copy,
            ObjectiveFunctionInfo& info
            );

        /// Destructs an SeekRangeObjectiveFunctionType.
        virtual
        ~SeekRangeObjectiveFunctionType(
            );




}; // class SeekRangeObjectiveFunctionType



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
#include "./inline/SeekRangeObjectiveFunctionType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_SEEKRANGEOBJECTIVEFUNCTIONTYPE_HPP
