/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class SeekValueObjectiveFunctionType.

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

        Sun Oct 12 17:36:42 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the SeekValueObjectiveFunctionType class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_SEEKVALUEOBJECTIVEFUNCTIONTYPE_HPP
#define JEGA_UTILITIES_SEEKVALUEOBJECTIVEFUNCTIONTYPE_HPP







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
class SeekValueObjectiveFunctionType;







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
/// An objective function type whereby a particular value is sought.
/**
 * This type is specialized to seek a singular value for an objective.  The
 * formulation is like this:
 * \verbatim
        F(x) = t
   \endverbatim
 * where t is the target value.
 *
 * This type reports it's value for minimization as the absolute value between
 * the current value and the target value.
 */
class JEGA_SL_IEDECL SeekValueObjectiveFunctionType :
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


        /// The value to seek.
        double _value;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the value that this objective function seeks.
        /**
         * \param value The new target value sought by this objective.
         */
        inline
        void
        SetValue(
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

        /// Sets the value that this objective function seeks.
        /**
         * \return The target value currently being sought by this objective.
         */
        inline
        double
        GetValue(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /// Returns the identifier for this nature which is "Seek Value".
        /**
         * \return The string "Seek Value".
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

        /**
         * \brief This method returns the amount by which \a val deviates from
         *        the target.
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
            ) const;

        /// Returns the string name of this type.
        /**
         * \return The string "Seek Value".
         */
        virtual
        std::string
        ToString(
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



        /// Constructs an SeekValueObjectiveFunctionType for use with \a info.
        /**
         * \param info The ObjectiveFunctionInfo for which this is to be the
         *             type.
         */
        SeekValueObjectiveFunctionType(
            ObjectiveFunctionInfo& info
            );

        /**
         * \brief Copy constructs an SeekValueObjectiveFunctionType for use
         *        with \a info.
         *
         * \param copy The existing type from which to copy properties into
         *             this.
         * \param info The ObjectiveFunctionInfo for which this is to be
         *             the type.
         */
        SeekValueObjectiveFunctionType(
            const SeekValueObjectiveFunctionType& copy,
            ObjectiveFunctionInfo& info
            );

        /// Destructs an SeekValueObjectiveFunctionType.
        virtual
        ~SeekValueObjectiveFunctionType(
            );




}; // class SeekValueObjectiveFunctionType



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
#include "./inline/SeekValueObjectiveFunctionType.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_SEEKVALUEOBJECTIVEFUNCTIONTYPE_HPP
