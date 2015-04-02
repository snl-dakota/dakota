/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignStatistician.

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

        Thu May 22 07:45:06 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignStatistician class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNSTATISTICIAN_HPP
#define JEGA_UTILITIES_DESIGNSTATISTICIAN_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/DesignMultiSet.hpp>





/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace eddy
{
    namespace utilities
    {

        template <typename T>
        class extremes;

        typedef extremes<double> DoubleExtremes;
    }
}







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
class DesignGroup;
class DesignTarget;
class DesignGroupVector;
class DesignStatistician;

class DesignOFSortSet;
class DesignDVSortSet;






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

/**
 * \brief A class to house general operations that can be performed on a Design
 *        or collection of Designs.
 *
 * This class has a very general charter.  It may house methods for performing
 * any non-algorithm specific operations on Designs or groups of Designs.
 * Examples of such operations include things like searching for Designs with
 * particular properties, computing statistics on those properties, breaking
 * groups up according to properties, etc.
 */
class DesignStatistician
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
         * \brief Finds the Designs in the passed in container that have the
         *        least total constraint violations.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.
         *
         * \param cont The container of Designs to search.
         * \return A mapping of the minimal constraint violation value to the
         *         locations of all Designs that have it in \a cont.
         */
        template <typename DesignContainer>
        static
        std::pair<
            double,
            std::vector<typename DesignContainer::const_iterator>
            >
        FindLeastViolateDesigns(
            const DesignContainer& cont
            );

        /// Pareto-like constraint comparison of two Design's.
        /**
         * This method compares \a des1's and \a des2's constraints in a manner
         * similar but not identical to the way a multi-objective algorithm
         * might compare objective functions to determine Pareto dominance.
         *
         * It begins by comparing the feasibility of the two.  If one is
         * feasible and the other is not, the feasible is returned as the
         * preferred Design.  If they are both feasible, the return is 0.
         * Otherwise, the method continues on to compare the constraint
         * violations.
         *
         * Once it is known that both Designs are infeasible, the method first
         * considers bound contraints.  These are considered to be the most
         * important since violation of these can often result in
         * designs that cannot even be evaluated.  If one of the designs
         * satisfies the bounds and the other does not, the one that does is
         * preferred.  If neither do, the method continues by comparing the
         * bound violations.  If both do, the method continues with the
         * functional constraints.
         *
         * When comparing violations, both bound and functional, the designs
         * are considered one constraint at a time.  If either of the designs
         * is no worse with respect to all constraints and better with respect
         * to at least 1, then that Design wins.  Otherwise, neither wins.
         *
         * \param des1 The first Design to use in the comparison.
         * \param des2 The second Design to use in the comparison.
         * \return The return is modeled after strcmp.  A return of -1 means
         *         that \a des1 is preferred to \a des2.  A return of 0 means
         *         that neither is preferred, and a return of 1 means that
         *         \a des2 is preferred to \a des1.
         */
        static
        int
        ParetoConstraintCompare(
            const Design& des1,
            const Design& des2
            );

        /**
         * \brief Removes the feasible from \a from and returns them in a new
         *        container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only infeasible Designs.
         *
         * \param from The container from which to separate out all the
         *             feasible into a new container.
         * \return The new container containing only the feasible of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateFeasible(
            Set_T& from
            );

        /**
         * \brief Removes the feasible from the range [first, end) and
         *        returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not evaluate the feasibility of the Designs. It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only infeasible
         * Designs between the locations of the iterators \a first and
         * \a end.
         *
         * \param from The container from which to separate out all the
         *             feasible in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the feasible.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the feasible of the
         *         specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateFeasible(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the feasible from \a from and returns them in a new
         *        container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only infeasible
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             feasible into a new container.
         * \return The new container containing only the feasible of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateFeasible(
            FromSet_T& from
            );

        /**
         * \brief Removes the feasible from the range [first, end) and
         *        returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only infeasible Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             feasible in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the feasible.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the feasible of the
         *         specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateFeasible(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the non-infeasible from \a from and returns them in a
         *        new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only feasible Designs.
         *
         * \param from The container from which to separate out all the
         *             non-feasible into a new container.
         * \return The new container containing only the infeasible of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonFeasible(
            Set_T& from
            );

        /**
         * \brief Removes the non-infeasible from \a from and returns them in a
         *        new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only feasible Designs.
         *
         * \param from The container from which to separate out all the
         *             non-feasible into a new container.
         * \return The new container containing only the infeasible of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonFeasible(
            FromSet_T& from
            );

        /**
         * \brief Removes the non-feasible from the range [first, end) and
         *        returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only feasible Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             infeasible in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the infeasible.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the infeasible of the
         *         specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonFeasible(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the non-feasible from the range [first, end) and
         *        returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * After this operation, \a from will contain only feasible Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             infeasible in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the infeasible.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the infeasible of the
         *         specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonFeasible(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the evaluated from \a from and returns them in a new
         *        container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only non-evaluated
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             evaluated into a new container.
         * \return The new container containing only the evaluted of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateEvaluated(
            Set_T& from
            );

        /**
         * \brief Removes the evaluated from \a from and returns them in a new
         *        container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only non-evaluated
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             evaluated into a new container.
         * \return The new container containing only the evaluted of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateEvaluated(
            FromSet_T& from
            );

        /**
         * \brief Removes the evaluated from the range [first, end) and
         *        returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only non-evaluated
         * Designs between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             evaluated in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the evaluated.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the evaluated of the
         *         specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateEvaluated(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the evaluated from the range [first, end) and
         *        returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only non-evaluated
         * Designs between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             evaluated in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the evaluated.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the evaluated of the
         *         specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateEvaluated(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the non-evaluated from \a from and returns them in a
         *        new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only evaluated Designs.
         *
         * \param from The container from which to separate out all the
         *             non-evaluated into a new container.
         * \return The new container containing only the non-evaluated of
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonEvaluated(
            Set_T& from
            );

        /**
         * \brief Removes the non-evaluated from \a from and returns them in a
         *        new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only evaluated Designs.
         *
         * \param from The container from which to separate out all the
         *             non-evaluated into a new container.
         * \return The new container containing only the non-evaluated of
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonEvaluated(
            FromSet_T& from
            );

        /**
         * \brief Removes the non-evaluated from the range [first, end) and
         *        returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only evaluated Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             non-evaluated in the supplied range into a new
         *             container.
         * \param first The location in \a from at which to begin separating
         *              the non-evaluated.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the evaluated of the
         *         specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonEvaluated(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the non-evaluated from the range [first, end) and
         *        returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * After this operation, \a from will contain only evaluated Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             non-evaluated in the supplied range into a new
         *             container.
         * \param first The location in \a from at which to begin separating
         *              the non-evaluated.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the evaluated of the
         *         specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonEvaluated(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the cloned from \a from and returns them in a new
         *        container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * After this operation, \a from will contain only non-clone Designs.
         *
         * \param from The container from which to separate out all the
         *             clone into a new container.
         * \return The new container containing only the non-clone of
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateCloned(
            Set_T& from
            );

        /**
         * \brief Removes the cloned from \a from and returns them in a new
         *        container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * After this operation, \a from will contain only non-clone Designs.
         *
         * \param from The container from which to separate out all the
         *             clone into a new container.
         * \return The new container containing only the non-clone of
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateCloned(
            FromSet_T& from
            );

        /**
         * \brief Removes the cloned from the range [first, end) and returns
         *        them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only non-clone Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             clones in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the clones.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the clones of
         *         the specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateCloned(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the cloned from the range [first, end) and returns
         *        them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only non-clone Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             clones in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the clones.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the clones of
         *         the specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateCloned(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the non-cloned from \a from and returns them in a new
         *        container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * After this operation, \a from will contain only clone Designs.
         *
         * \param from The container from which to separate out all the
         *             non-clone into a new container.
         * \return The new container containing only the clones in \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonCloned(
            Set_T& from
            );

        /**
         * \brief Removes the non-cloned from \a from and returns them in a new
         *        container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * After this operation, \a from will contain only clone Designs.
         *
         * \param from The container from which to separate out all the
         *             non-clone into a new container.
         * \return The new container containing only the clones in \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonCloned(
            FromSet_T& from
            );

        /**
         * \brief Removes the non-cloned from the range [first, end) and
         *        returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only clone Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             non-clones in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the non-clones.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the non-clones of
         *         the specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonCloned(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the non-cloned from the range [first, end) and
         *        returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only clone Designs
         * between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             non-clones in the supplied range into a new container.
         * \param first The location in \a from at which to begin separating
         *              the non-clones.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the non-clones of
         *         the specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonCloned(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the ill conditioned from \a from and returns them in
         *        a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only non-illconditioned
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             illconditioned into a new container.
         * \return The new container containing only the illconditioned of
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateIllconditioned(
            Set_T& from
            );

        /**
         * \brief Removes the ill conditioned from \a from and returns them in
         *        a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only non-illconditioned
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             illconditioned into a new container.
         * \return The new container containing only the illconditioned of
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateIllconditioned(
            FromSet_T& from
            );

        /**
         * \brief Removes the ill conditioned from the range [first, end) and
         *        returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only non-illconditioned
         * Designs between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             illconditioned in the supplied range into a new
         *             container.
         * \param first The location in \a from at which to begin separating
         *              the illconditioned.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the illconditioned of
         *         the specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateIllconditioned(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the ill conditioned from the range [first, end) and
         *        returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only non-illconditioned
         * Designs between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             illconditioned in the supplied range into a new
         *             container.
         * \param first The location in \a from at which to begin separating
         *              the illconditioned.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the illconditioned of
         *         the specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateIllconditioned(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /**
         * \brief Removes the non-ill conditioned from \a from and returns them
         *        in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only illconditioned
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             non-illconditioned into a new container.
         * \return The new container containing only the illconditioned of
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonIllconditioned(
            Set_T& from
            );

        /**
         * \brief Removes the non-ill conditioned from \a from and returns them
         *        in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only illconditioned
         * Designs.
         *
         * \param from The container from which to separate out all the
         *             non-illconditioned into a new container.
         * \return The new container containing only the illconditioned of
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonIllconditioned(
            FromSet_T& from
            );

        /**
         * \brief Removes the non-ill conditioned from the range [first, end)
         *        and returns them in a new container.
         *
         * Set_T must be an associative container, must hold pointers to
         * Design class objects or derivatives, be forward iteratable in STL
         * style, be default constructable, and support the begin, end, erase,
         * and insert methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only illconditioned
         * Designs between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             non-illconditioned in the supplied range into a new
         *             container.
         * \param first The location in \a from at which to begin separating
         *              the non-illconditioned.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the non-illconditioned of
         *         the specified range of \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        SeparateNonIllconditioned(
            Set_T& from,
            const typename Set_T::iterator& first,
            const typename Set_T::iterator& end
            );

        /**
         * \brief Removes the non-ill conditioned from the range [first, end)
         *        and returns them in a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be an associative container, be forward
         * iteratable in STL style, and support the begin, end, and erase
         * methods.
         *
         * IntoSet_T must be default constructable and support an insert
         * method that takes only a value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsIllconditioned method for each.
         *
         * After this operation, \a from will contain only illconditioned
         * Designs between the locations of the iterators \a first and \a end.
         *
         * \param from The container from which to separate out all the
         *             non-illconditioned in the supplied range into a new
         *             container.
         * \param first The location in \a from at which to begin separating
         *              the non-illconditioned.  \a first may be separated out.
         * \param end One past the last location in \a from at which to stop
         *            separating.  \a end will not be separated out.
         * \return The new container containing only the non-illconditioned of
         *         the specified range of \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        SeparateNonIllconditioned(
            FromSet_T& from,
            const typename FromSet_T::iterator& first,
            const typename FromSet_T::iterator& end
            );

        /// Finds the feasible in \a from and adds them in a new container.
        /**
         * Set_T must hold pointers to Design derivatives, be forward
         * iteratable in STL style, be default constructable, and support the
         * begin, end, and insert methods.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param from The container in which to find feasible Designs.
         * \return A new container holding only the feasible found in \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetFeasible(
            const Set_T& from
            );

        /**
         * \brief Finds the feasible from the range [first, end) and adds them
         *        into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsFeasible method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for feasible.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the feasible found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetFeasible(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /// Finds the feasible in \a from and adds them in a new container.
        /**
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param from The container in which to find feasible Designs.
         * \return A new container holding only the feasible found in \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetFeasible(
            const FromSet_T& from
            );

        /**
         * \brief Finds the feasible from the range [first, end) and adds them
         *        into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsFeasible method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for feasible.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the feasible found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetFeasible(
            FwdIt first,
            const FwdIt& end
            );

        /// Finds the infeasible in \a from and adds them in a new container.
        /**
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param from The container in which to find infeasible Designs.
         * \return A new container holding only the infeasible found in
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonFeasible(
            const Set_T& from
            );

        /**
         * \brief Finds the infeasible from the range [first, end) and adds
         *        them into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsFeasible method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for infeasible.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the infeasible found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonFeasible(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /// Finds the infeasible in \a from and adds them in a new container.
        /**
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param from The container in which to find infeasible Designs.
         * \return A new container holding only the infeasible found in \a
         *         from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonFeasible(
            const FromSet_T& from
            );

        /**
         * \brief Finds the infeasible from the range [first, end) and adds
         *        them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsFeasible method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for infeasible.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the infeasible found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonFeasible(
            FwdIt first,
            const FwdIt& end
            );

        /// Finds the evaluated in \a from and adds them in a new container.
        /**
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param from The container in which to find evaluated Designs.
         * \return A new container holding only the evaluated found in \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetEvaluated(
            const Set_T& from
            );

        /**
         * \brief Finds the evaluated from the range [first, end) and adds them
         *        into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for evaluated.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the evaluated found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetEvaluated(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /// Finds the evaluated in \a from and adds them in a new container.
        /**
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param from The container in which to find evaluated Designs.
         * \return A new container holding only the evaluated found in \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetEvaluated(
            const FromSet_T& from
            );

        /**
         * \brief Finds the evaluated from the range [first, end) and adds
         *        them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for evaluated.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the evaluated found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetEvaluated(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Finds the non-evaluated in \a from and adds them in a new
         *        container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param from The container in which to find non-evaluated Designs.
         * \return A new container holding only the non-evaluated found in
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonEvaluated(
            const Set_T& from
            );

        /**
         * \brief Finds the non-evaluated from the range [first, end) and adds
         *        them into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for non-evaluated.  \a first will be
         *              considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the non-evaluated found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonEvaluated(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /**
         * \brief Finds the non-evaluated in \a from and adds them in a new
         *        container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param from The container in which to find non-evaluated Designs.
         * \return A new container holding only the non-evaluated found in
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonEvaluated(
            const FromSet_T& from
            );

        /**
         * \brief Finds the non-evaluated from the range [first, end) and adds
         *        them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for non-evaluated.  \a first will be
         *              considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the non-evaluated found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonEvaluated(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Finds the illconditioned in \a from and adds them in a new
         *        container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param from The container in which to find illconditioned Designs.
         * \return A new container holding only the illconditioned found in
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetIllconditioned(
            const Set_T& from
            );

        /**
         * \brief Finds the illconditioned from the range [first, end) and adds
         *        them into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for illconditioned.  \a first will be
         *              considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the illconditioned found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetIllconditioned(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /**
         * \brief Finds the illconditioned in \a from and adds them in a new
         *        container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param from The container in which to find illconditioned Designs.
         * \return A new container holding only the illconditioned found in
         *        \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetIllconditioned(
            const FromSet_T& from
            );

        /**
         * \brief Finds the illconditioned from the range [first, end) and adds
         *        them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for illconditioned.  \a first will be
         *              considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the illconditioned found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetIllconditioned(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Finds the non-illconditioned in \a from and adds them in a
         *        new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param from The container in which to find non-illconditioned
         *        Designs.
         * \return A new container holding only the non-illconditioned found in
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonIllconditioned(
            const Set_T& from
            );

        /**
         * \brief Finds the non-illconditioned from the range [first, end) and
         *        adds them into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for non-illconditioned.  \a first will be
         *              considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the non-illconditioned found in
         *         the specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonIllconditioned(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /**
         * \brief Finds the non-illconditioned in \a from and adds them in a
         *        new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param from The container in which to find non-illconditioned
         *             Designs.
         * \return A new container holding only the non-illconditioned found in
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonIllconditioned(
            const FromSet_T& from
            );

        /**
         * \brief Finds the non-illconditioned from the range [first, end) and
         *        adds them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for non-illconditioned.  \a first will be
         *              considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the non-illconditioned found in
         *         the specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonIllconditioned(
            FwdIt first,
            const FwdIt& end
            );

        /// Finds the cloned in \a from and adds them in a new container.
        /**
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param from The container in which to find cloned Designs.
         * \return A new container holding only the cloned found in \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetCloned(
            const Set_T& from
            );

        /**
         * \brief Finds the cloned from the range [first, end) and adds them
         *        into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for cloned.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the cloned found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetCloned(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /// Finds the cloned in \a from and adds them in a new container.
        /**
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param from The container in which to find cloned Designs.
         * \return A new container holding only the cloned found in \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetCloned(
            const FromSet_T& from
            );

        /**
         * \brief Finds the cloned from the range [first, end) and adds
         *        them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for cloned.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the cloned found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetCloned(
            FwdIt first,
            const FwdIt& end
            );

        /// Finds the non-cloned in \a from and adds them in a new container.
        /**
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param from The container in which to find non-cloned Designs.
         * \return A new container holding only the non-cloned found in
         *         \a from.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonCloned(
            const Set_T& from
            );

        /**
         * \brief Finds the non-cloned from the range [first, end) and adds
         *        them into a new container.
         *
         * Set_T must be default constructable and support an insert
         * method taking a single argument of type Design*.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for non-cloned.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the non-cloned found in the
         *         specified range.
         */
        template <typename Set_T>
        inline static
        Set_T
        GetNonCloned(
            typename Set_T::const_iterator first,
            const typename Set_T::const_iterator& end
            );

        /// Finds the non-cloned in \a from and adds them in a new container.
        /**
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param from The container in which to find non-cloned Designs.
         * \return A new container holding only the non-cloned found in
         *         \a from.
         */
        template <typename FromSet_T, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonCloned(
            const FromSet_T& from
            );

        /**
         * \brief Finds the non-cloned from the range [first, end) and adds
         *        them into a new container.
         *
         * Both parameterized container types must hold pointers to Design
         * class objects or derivatives.
         *
         * FromSet_T must be forward iteratable in STL style, and support
         * the begin and end methods. IntoSet_T must be default
         * constructable and support an insert method that takes only a
         * value_type argument.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param first The iterator to the first Design in the range in which
         *              to search for non-cloned.  \a first will be considered.
         * \param end An iterator to one past the last Design in the range in
         *            which to search.  \a end will not be considered.
         * \return A new container holding only the non-cloned found in the
         *         specified range.
         */
        template <typename FwdIt, typename IntoSet_T>
        inline static
        IntoSet_T
        GetNonCloned(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the feasible Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param in The container of Designs in which to count the feasible.
         * \return The number of feasible Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountFeasible(
            const Set_T& in
            );

        /// Counts the feasible Designs in the range [first, end).
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the feasible. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of feasible Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountFeasible(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the infeasible Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param in The container of Designs in which to count the infeasible.
         * \return The number of infeasible Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountNonFeasible(
            const Set_T& in
            );

        /// Counts the infeasible Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not evaluate the feasibility of the Designs.  It
         * simply checks the Design::IsFeasible method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the non-feasible. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of non-feasible Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountNonFeasible(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the evaluated Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param in The container of Designs in which to count the evaluated.
         * \return The number of evaluated Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountEvaluated(
            const Set_T& in
            );

        /// Counts the evaluated Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the evaluated. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of evaluated Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountEvaluated(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the non-evaluated Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param in The container of Designs in which to count the
         *           non-evaluated.
         * \return The number of non-evaluated Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountNonEvaluated(
            const Set_T& in
            );

        /// Counts the non-evaluated Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::IsEvaluated method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the non-evaluated. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of non-evaluated Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountNonEvaluated(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the ill conditioned Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param in The container of Designs in which to count the
         *           illconditioned.
         * \return The number of illconditioned Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountIllconditioned(
            const Set_T& in
            );

        /// Counts the ill conditioned Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the illconditioned. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of illconditioned Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountIllconditioned(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the ill conditioned Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param in The container of Designs in which to count the
         *           non-illconditioned.
         * \return The number of non-illconditioned Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountNonIllconditioned(
            const Set_T& in
            );

        /// Counts the ill conditioned Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not do any evaluation of the Designs.  It simply
         * checks the Design::Isillconditioned method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the non-illconditioned. \a first may be
         *              counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of non-illconditioned Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountNonIllconditioned(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the cloned Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param in The container of Designs in which to count the
         *           cloned.
         * \return The number of cloned Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountCloned(
            const Set_T& in
            );

        /// Counts the cloned Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the clones. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of clone Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountCloned(
            FwdIt first,
            const FwdIt& end
            );

        /// Counts the non-cloned Designs in \a in.
        /**
         * Set_T must hold Design*'s, be forward iteratable in STL
         * style, and support the begin and end methods.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param in The container of Designs in which to count the
         *           non-cloned.
         * \return The number of non-cloned Designs found in \a in.
         */
        template <typename Set_T>
        inline static
        std::size_t
        CountNonCloned(
            const Set_T& in
            );

        /// Counts the non-cloned Designs in \a in.
        /**
         * FwdIt must be a forward iterator that points to Design*'s.
         *
         * This method does not do any clone testing of the Designs.  It simply
         * checks the Design::IsCloned method for each.
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the non-clones. \a first may be counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \return The number of non-clone Designs found in the range.
         */
        template <typename FwdIt>
        inline static
        std::size_t
        CountNonCloned(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Computes the total amount of percentage constraint violation
         *        of \a des.
         *
         * This is a simple summation of the percentage violations of each of
         * the side and non-side constraints.
         *
         * \param des The Design whose total constraint violation percentage is
         *            sought.
         * \return The total constraint violation percentage for the supplied
         *            Design.
         */
        static
        double
        ComputeTotalPercentageViolation(
            const Design& des
            );

        /**
         * \brief Computes the amount of percentage non-side constraint
         *        violation of \a des.
         *
         * This is a simple summation of the percentage violations of each of
         * the non-side constraints.
         *
         * \param des The design for which to compute the non-side constraint
         *            percentage violation.
         * \return The total percentage violation of all non-side constraints
         *         by \a des.
         */
        static
        double
        ComputeNonSidePercentageViolation(
            const Design& des
            );

        /**
         * \brief Computes the amount of percentage side constraint violation
         *        of \a des.
         *
         * This is a simple summation of the percentage violations of each of
         * the side constraints.
         *
         * \param des The Design for which to compute the side constraint
         *            percentage violation.
         * \return The total percentage violation of all side constraints
         *         by \a des.
         */
        static
        double
        ComputeSidePercentageViolation(
            const Design& des
            );

        /**
         * \brief Gathers all Designs contained in the objective function sort
         *        sets of the supplied groups into a single objective function
         *        sort set.
         *
         * \param groups The DesignGroups containing the Designs to be gathered
         *               into a single objective function sorted set.
         * \return A collection of all Designs in all supplied groups objective
         *         function sorted sets.
         */
        static
        DesignOFSortSet
        CollectDesignsByOF(
            const DesignGroupVector& groups
            );

        /**
         * \brief Gathers all Designs contained in the design variable sort
         *        sets of the supplied groups into a single design variable
         *        sort set.
         *
         * \param groups The DesignGroups containing the Designs to be gathered
         *               into a single design variable sorted set.
         * \return A collection of all Designs in all supplied groups design
         *         variable sorted sets.
         */
        static
        DesignDVSortSet
        CollectDesignsByDV(
            const DesignGroupVector& groups
            );

        /**
         * \brief Returns an Extremes object containing the largest and
         *        smallest values found for all objectives found in \a from.
         *
         * This counts all contained Designs including infeasible, etc.
         *
         * \param from The collection of Designs in which to find the objective
         *             function extremes.
         * \return An extremes object containing the largest and smallest
         *         values for each objective represented in \a from.
         */
        template <typename Set_T>
        static
        eddy::utilities::DoubleExtremes
        GetObjectiveFunctionExtremes(
            const Set_T& from
            );

        /**
         * \brief Returns an Extremes object containing the largest and
         *        smallest values found for all design variables found in
         *        \a from.
         *
         * This counts all contained Designs including infeasible, etc.
         *
         * \param from The collection of Designs in which to find the design
         *             variable extremes.
         * \return An extremes object containing the largest and smallest
         *         values for each variable represented in \a from.
         */
        template <typename Set_T>
        static
        eddy::utilities::DoubleExtremes
        GetDesignVariableExtremes(
            const Set_T& from
            );

        /**
         * \brief Returns an Extremes object containing the largest and
         *        smallest values found for all design variables found in
         *        \a from.
         *
         * This counts all contained Designs including infeasible, etc.
         *
         * This specialization is here b/c if we know that this is a design
         * variable sort set that the first dimension is sorted and therefore
         * needn't be iterated.
         *
         * \param from The collection of Designs in which to find the design
         *             variable extremes.
         * \return An extremes object containing the largest and smallest
         *         values for each variable represented in \a from.
         */
        static
        eddy::utilities::DoubleExtremes
        GetDesignVariableExtremes(
            const DesignDVSortSet& from
            );

        /// Finds the first feasible Design in \a cont.
        /**
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.  It must contain pointers to
         * Design derivatives.
         *
         * \param in The container in which to find the first feasible
         *           Design.
         * \return An iterator to the first feasible design found in \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindFeasible(
            const Set_T& in
            );

        /**
         * \brief Finds the first feasible Design in the range [first, end) or
         *        end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding a feasible one.  \a first may be returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding a feasible one.
         * \return Returns an iterator to the first feasible Design in the
         *         range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindFeasible(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Returns an iterator to the first infeasible design found in
         *        \a cont.
         *
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.  It must contain pointers to
         * Design derivatives.
         *
         * \param in The container in which to find the first non-feasible
         *           Design.
         * \return An iterator to the first non-feasible design found in
         *         \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindNonFeasible(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first infeasible design found in
         *        the range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding an infeasible one.  \a first may be returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding an infeasible one.
         * \return Returns an iterator to the first infeasible Design in the
         *         range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindNonFeasible(
            FwdIt first,
            const FwdIt& end
            );

        /// Returns an iterator to the first clone design found in \a cont.
        /**
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.  It must contain pointers to
         * Design derivatives.
         *
         * \param in The container in which to find the first clone Design.
         * \return An iterator to the first clone design found in \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindCloned(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first clone design found in the
         *        range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding a cloned one.  \a first may be returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding a cloned one.
         * \return Returns an iterator to the first clone Design in the
         *         range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindCloned(
            FwdIt first,
            const FwdIt& end
            );

        /// Returns an iterator to the first non-clone design found in \a cont.
        /**
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.
         *
         * \param in The container in which to find the first non-clone Design.
         * \return An iterator to the first non-clone design found in \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindNonCloned(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first non-clone design found in
         *        the range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding a non-cloned one.  \a first may be returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding a non-cloned one.
         * \return Returns an iterator to the first non-clone Design in the
         *         range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindNonCloned(
            FwdIt first,
            const FwdIt& end
            );

        /// Returns an iterator to the first evaluated design found in \a cont.
        /**
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.
         *
         * \param in The container in which to find the first evaluated Design.
         * \return An iterator to the first evaluated design found in \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindEvaluated(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first evaluated design found in
         *        the range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding an evaluated one.  \a first may be returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding an evaluated one.
         * \return Returns an iterator to the first evaluated Design in the
         *         range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindEvaluated(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Returns an iterator to the first non-evaluated design found
         *        in \a cont.
         *
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.
         *
         * \param in The container in which to find the first non-evaluated
         *           Design.
         * \return An iterator to the first non-evaluated design found in
         *         \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindNonEvaluated(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first non-evaluated design found
         *        in the range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding a non-evaluated one.  \a first may be returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding a non-evaluated one.
         * \return Returns an iterator to the first non-evaluated Design in the
         *         range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindNonEvaluated(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Returns an iterator to the first ill conditioned design found
         *        in \a cont.
         *
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.
         *
         * \param in The container in which to find the first illconditioned
         *           Design.
         * \return An iterator to the first illconditioned design found in
         *         \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindIllconditioned(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first ill conditioned design found
         *        in the range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding an illconditioned one.  \a first may be
         *              returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding an illconditioned one.
         * \return Returns an iterator to the first illconditioned Design in
         *         the range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindIllconditioned(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Returns an iterator to the first non-ill conditioned design
         *        found in \a cont.
         *
         * Set_T must be a forward iteratable STL compliant container
         * supplying methods to begin and end.
         *
         * \param in The container in which to find the first
         *           non-illconditioned Design.
         * \return An iterator to the first non-illconditioned design found in
         *         \a cont.
         */
        template <typename Set_T>
        inline static
        typename Set_T::const_iterator
        FindNonIllconditioned(
            const Set_T& in
            );

        /**
         * \brief Returns an iterator to the first non-ill conditioned design
         *        found in the range [first, end) or end if none.
         *
         * FwdIt must be a forward iterator type which produces a Design* when
         * dereferenced.
         *
         * \param first An iterator to the first Design to consider when
         *              finding a non-illconditioned one.  \a first may be
         *              returned.
         * \param end An iterator to one past the last Design to consider when
         *            finding a non-illconditioned one.
         * \return Returns an iterator to the first non-illconditioned Design
         *         in the range [first, end) or end if none.
         */
        template <typename FwdIt>
        inline static
        FwdIt
        FindNonIllconditioned(
            FwdIt first,
            const FwdIt& end
            );

        /**
         * \brief Counts the Designs for which \a predicate returns true in
         *        [first, end).
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the predicate passers. \a first may be
         *              counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \param predicate The predicate to use to test each Design in the
         *                  range.
         * \return The number of predicate passer Designs found in the range.
         */
        template <typename FwdIt, typename Pred>
        inline static
        std::size_t
        CountDesigns(
            FwdIt first,
            const FwdIt& end,
            Pred predicate
            );

        /**
         * \brief Counts the Designs for which \a predicate returns true in
         *        [first, end).
         *
         * \param first An iterator to the first Design in the range in which
         *              to count the predicate passers. \a first may be
         *              counted.
         * \param end An iterator to one past the last Design in the range in
         *            which to count. \a end will not be counted.
         * \param predicate The predicate to use to test each Design in the
         *                  range.
         * \param cutoff The value once reached at which to stop counting.
         *               Supplying a value of -1 has the effect of not having a
         *               cutoff.
         * \return The number of predicate passer Designs found in the range.
         */
        template <typename FwdIt, typename Pred>
        inline static
        std::size_t
        CountDesigns(
            FwdIt first,
            const FwdIt& end,
            Pred predicate,
            eddy::utilities::int32_t cutoff
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Separates the Designs for which \a predicate returns true
         *        from \a from
         *
         * The template parameter \a FromSet_T must be an STL style
         * container that supports the erase(iterator) method and be forward
         * iteratable.  \a end must be reachable by forward iteration from
         * \a first.
         *
         * The template parameter IntoSet_T must be an STL style
         * container that supports the insert(value_type) method and is default
         * constructable.
         *
         * The return will be a container of type IntoSet_T
         * containing the "predicate = true" Designs of \a from. \a from will
         * be left with only "predicate = false" Designs in it.
         *
         * \param from The container from which to separate Designs.
         * \param first The iterator to the first Design in the subrange of
         *              \a from from which to separate Designs.
         * \param end The iterator to one past the last Design in the subrange
         *            of \a from from which to separate Designs.
         * \param predicate The predicate with which to test each Design.
         *                  Those for which \a predicate returns true will be
         *                  separated.
         */
        template <
            typename FromSet_T,
            typename IntoSet_T,
            typename Pred
            >
        static
        IntoSet_T
        SeparateDesigns(
            FromSet_T& from,
            typename FromSet_T::iterator first,
            const typename FromSet_T::iterator& end,
            Pred predicate
            );

        /**
         * \brief Separates the Designs for which \a predicate returns true
         *        from \a from
         *
         * The template parameter \a FromSet_T must be an STL style
         * container that supports the erase(iterator) method and be forward
         * iteratable.  \a end must be reachable by forward iteration from
         * \a first.
         *
         * The template parameter IntoSet_T must be an STL style
         * container that supports the insert(value_type) method and is default
         * constructable.
         *
         * The return will be a container of type IntoSet_T
         * containing the "predicate = true" Designs of the supplied range.
         * The range will be unaltered.
         *
         * \param first The iterator to the first Design in the range from
         *              which to retrieve Designs.
         * \param end The iterator to one past the last Design in the range
         *            from which to retrieve Designs.
         * \param predicate The predicate with which to test each Design.
         *                  Those for which \a predicate returns true will be
         *                  retrieved.
         */
        template <
            typename FwdIt,
            typename IntoSet_T,
            typename Pred
            >
        static
        IntoSet_T
        GetDesigns(
            FwdIt first,
            const FwdIt& end,
            Pred predicate
            );

        /**
         * \brief Counts the elements for which \a predicate returns true in
         *        [first, end).
         *
         * \param first The iterator to the first Design in the range in which
         *              to count.
         * \param end The iterator to one past the last Design in the range in
         *            which to count.
         * \param predicate The predicate with which to test each element in
         *                  the range. Those for which \a predicate returns
         *                  true will be counted.
         */
        template <typename FwdIt, typename Pred>
        static
        std::size_t
        CountIf(
            FwdIt first,
            const FwdIt& end,
            Pred predicate
            );

        /**
         * \brief Counts the elements for which \a predicate returns true in
         *        [first, end).
         *
         * \param first The iterator to the first Design in the range in which
         *              to count.
         * \param end The iterator to one past the last Design in the range in
         *            which to count.
         * \param predicate The predicate with which to test each element in
         *                  the range. Those for which \a predicate returns
         *                  true will be counted.
         * \param cutoff The value once reached at which to stop counting.
         *               Supplying a value of -1 has the effect of not having a
         *               cutoff.
         */
        template <typename FwdIt, typename Pred>
        static
        std::size_t
        CountIf(
            FwdIt first,
            const FwdIt& end,
            Pred predicate,
            eddy::utilities::int32_t cutoff
            );


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

        /// This constructor is private and has no implementation.
        /**
         * This class contains only static methods and therefore should not
         * and cannot be instantiated.
         */
        DesignStatistician(
            );

}; // class DesignStatistician



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
#include "./inline/DesignStatistician.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNSTATISTICIAN_HPP
