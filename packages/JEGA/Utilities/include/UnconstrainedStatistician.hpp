/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class UnconstrainedStatistician.

    NOTES:

        See notes under section Class Definition of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 08:05:53 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the UnconstrainedStatistician class.
 */


/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_UNCONSTRAINEDSTATISTICIAN_HPP
#define JEGA_UTILITIES_UNCONSTRAINEDSTATISTICIAN_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <functional>







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
class DesignGroup;
class DesignOFSortSet;
class DesignGroupVector;
class UnconstrainedStatistician;

template <typename VT>
class DesignValueMap;






/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/

/// A map to key double values by the Design for which they have meaning.
typedef
DesignValueMap<double>
DesignDoubleMap;






/*
================================================================================
Class Definition
================================================================================
*/

/// A classe that computes unconstrained statistics about DesignGroups.
/**
 * This class has the capabilities to compute various unconstrained
 * statistics about collections of Designs such as DesignGroups.  Examples are:
 *
 *   Computing penalty values using a specified penalty function.
 *
 *   Computing maximum, minimum, average, and total penalty values for
 *   Designs in a group.
 */
class UnconstrainedStatistician
{
    /*
    ============================================================================
    Nested Utility Class Declarations
    ============================================================================
    */
    public:

        /// A base class for Penalizer functors.
        /**
         * This class inherits the unary_function and provides a single method
         * useful for determining whether or not a Design is ok for penalty
         * computations.
         */
        class Penalizer :
            public std::unary_function<Design*, double>
        {
            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /**
                 * \brief Verifies that \a des is a suitable candidate for
                 *        penalty computation.
                 *
                 * To be suitable, \a des must be evaluated and
                 * non-illconditioned.  This method logs warnings if either of
                 * these criteria are not met.
                 *
                 * \param des The Design to check.
                 * \return true if \a des is evaluated and well-conditioned.
                 *         false otherwise.
                 */
                static
                bool
                CheckDesign(
                    const Design* des
                    );

        }; // class Penalizer

        /// Computes a penalty value based on design variable bound violations.
        class BoundsPenalizer :
            public Penalizer
        {
            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// Returns the violation computed for \a des.
                /**
                 * The violation amount is computed by determining the amount
                 * by which \a des is outside of the range of allowed values
                 * for each design variable.  The resulting penalty is the sum
                 * of the squares of those values.  If all variable values
                 * are within bounds, the return is 0.0.
                 *
                 * \param des The Design for which to compute the out of bounds
                 *            penalty.
                 * \return The sum of squares of the variable bound violations
                 *         or 0 if no bounds are violated by \a des.
                 */
                double
                operator()(
                    const Design* des
                    ) const;

        }; // class BoundsPenalizer

        /**
         * \brief A Penalizer to apply the exterior type penalty function to a
         *        Design.
         */
        class ExteriorPenalizer :
            public Penalizer
        {
            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            private:

                /**
                 * \brief A static multiplier value to apply to the final
                 *        penalty value.
                 *
                 * This is a (poor but simple) way to help deal with scaling
                 * issues between the objective functions and constraints.
                 */
                double _multiplier;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// Computes the exterior penalty function value for \a des.
                /**
                 * The exterior penalty function is computed as the sum of
                 * squares of the constraint violations.  The resulting penalty
                 * value is multiplied by \a _multiplier before it is returned.
                 *
                 * \param des The Design for which to compute the exterior
                 *            penalty function value.
                 * \return The exterior penalty function value of \a des.
                 */
                double
                operator()(
                    const Design* des
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /**
                 * \brief Constructs an ExteriorPenalizer with the supplied
                 *        \a multiplier.
                 *
                 * \param multiplier The value to multiply each penalty prior
                 *                   to returning it.
                 */
                inline
                ExteriorPenalizer(
                    double multiplier = 1.0
                    ) :
                        _multiplier(multiplier)
                {
                }

        }; // class ExteriorPenalizer


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




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Computes the penalty value for each member of \a designs
         *        using the supplied penalizer.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  It must also support the begin and end methods.
         *
         * PFunc must behave like a unary_function taking a Design* and
         * returning a double.
         *
         * \param designs The container of Design objects to apply penalties
         *                to.
         * \param penalizer The functor object to use on each Design to compute
         *                  the penalty value.
         * \return A mapping of each Design in \a designs to its penalty value.
         */
        template <typename DesignContainer, typename PFunc>
        static
        DesignDoubleMap
        ApplyPenalty(
            const DesignContainer& designs,
            PFunc penalizer
            );

        template <typename PFunc>
        static
        DesignDoubleMap
        ApplyPenalty(
            const DesignGroup& designs,
            PFunc penalizer
            );

        template <typename PFunc>
        static
        DesignDoubleMap
        ApplyPenalty(
            const DesignGroupVector& designs,
            PFunc penalizer
            );

        /**
         * \brief Alters the penalty value for each member of \a designs
         *        using the supplied penalizer.
         *
         * The penalty value computed for each Design is added to the value
         * currently stored in \a to.  If any Designs are not found in
         * \a to, they are added with the computed value.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  It must also support the begin and end methods.
         *
         * PFunc must behave like a unary_function taking a Design* and
         * returning a double.
         *
         * \param designs The container of Design objects to apply penalties
         *                to.
         * \param penalizer The functor object to use on each Design to compute
         *                  the penalty value.
         * \param to The existing map of values to which to add penalty values
         *           computed by \a penalizer.
         * \return \a to after it has been altered.
         */
        template <typename DesignContainer, typename PFunc>
        static
        const DesignDoubleMap&
        AddPenalty(
            const DesignContainer& designs,
            PFunc penalizer,
            DesignDoubleMap& to
            );

        template <typename PFunc>
        static
        const DesignDoubleMap&
        AddPenalty(
            const DesignGroup& designs,
            PFunc penalizer,
            DesignDoubleMap& to
            );

        template <typename PFunc>
        static
        const DesignDoubleMap&
        AddPenalty(
            const DesignGroupVector& designs,
            PFunc penalizer,
            DesignDoubleMap& to
            );

        /**
         * \brief Computes the penalty value for each member of "group" using
         *        the exterior penalty function formulation.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  It must also support the begin and end methods.
         *
         * PFunc must behave like a unary_function taking a Design* and
         * returning a double.
         *
         * \param designs The container of Design objects to apply penalties
         *                to.
         * \param multiplier The amount by which to multiply each penalty value
         *                   prior to insertion into the map.
         * \return A mapping of each Design in \a designs to its penalty value.
         */
        template <typename DesignContainer>
        static
        DesignDoubleMap
        ApplyExteriorPenalty(
            const DesignContainer& designs,
            double multiplier = 1.0
            );

        static
        DesignDoubleMap
        ApplyExteriorPenalty(
            const DesignGroup& designs,
            double multiplier = 1.0
            );

        static
        DesignDoubleMap
        ApplyExteriorPenalty(
            const DesignGroupVector& designs,
            double multiplier = 1.0
            );

        /**
         * \brief Computes the penalty value for each member of "group" using
         *        a quadratic penalty function formulation.
         *
         * This is currently identical to the exterior penalty function method.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  It must also support the begin and end methods.
         *
         * PFunc must behave like a unary_function taking a Design* and
         * returning a double.
         *
         * \param designs The container of Design objects to apply penalties
         *                to.
         * \param multiplier The amount by which to multiply each penalty value
         *                   prior to insertion into the map.
         * \return A mapping of each Design in \a designs to its penalty value.
         */
        template <typename DesignContainer>
        static
        DesignDoubleMap
        ApplyQuadraticPenalty(
            const DesignContainer& designs,
            double multiplier = 1.0
            );

        static
        DesignDoubleMap
        ApplyQuadraticPenalty(
            const DesignGroup& designs,
            double multiplier = 1.0
            );

        static
        DesignDoubleMap
        ApplyQuadraticPenalty(
            const DesignGroupVector& designs,
            double multiplier = 1.0
            );

        /**
         * \brief This method adds penalty amounts to the Designs of \a designs
         *        that have variable values outside of the bounds.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  It must also support the begin and end methods.
         *
         * PFunc must behave like a unary_function taking a Design* and
         * returning a double.
         *
         * \param designs The container of Design objects to apply penalties
         *                to.
         * \param to The existing map of values to which to add out of bounds
         *           penalty values.
         * \return \a to after it has been altered.
         */
        template <typename DesignContainer>
        static
        const DesignDoubleMap&
        AddOutOfBoundsPenalties(
            const DesignContainer& designs,
            DesignDoubleMap& to
            );

        static
        const DesignDoubleMap&
        AddOutOfBoundsPenalties(
            const DesignGroup& designs,
            DesignDoubleMap& to
            );

        static
        const DesignDoubleMap&
        AddOutOfBoundsPenalties(
            const DesignGroupVector& designs,
            DesignDoubleMap& to
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
    private:

        /// Default constructs an UnconstrainedStatistician.
        /**
         * This constructor is private and has no implementation.  This is
         * because all methods of this class are static and thus it
         * should not be instantiated.
         */
        UnconstrainedStatistician(
            );

}; // class UnconstrainedStatistician



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
#include "./inline/UnconstrainedStatistician.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_UNCONSTRAINEDSTATISTICIAN_HPP
