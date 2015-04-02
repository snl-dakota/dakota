/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MultiObjectiveStatistician.

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

        Thu May 22 08:02:26 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MultiObjectiveStatistician class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_MUTLIOBJECTIVESTATISTICIAN_HPP
#define JEGA_UTILITIES_MUTLIOBJECTIVESTATISTICIAN_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/int_types.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <../Utilities/include/DesignStatistician.hpp>







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
class DesignGroup;
class DesignGroupVector;
class MultiObjectiveStatistician;

template <typename VT>
class DesignValueMap;

template <typename VT>
class DesignDoubleValueMap;


/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/
/// A shorthand for a DesignValueMap mapping designs to 64 bit integers.
typedef
DesignValueMap<eddy::utilities::uint64_t>
DesignCountMap;

/**
 * \brief A shorthand for a DesignDoubleValueMap mapping designs to a pair of
 *        64 bit integers.
 */
typedef
DesignDoubleValueMap<eddy::utilities::uint64_t>
DesignDoubleCountMap;

/*
================================================================================
File Scope Utility Classes
================================================================================
*/
/**
 * \brief A predicate class to test the outcome of a call to
 *        MultiObjectiveStatistician::DominationCompare
 *
 * This predicate will return the result of:
 * \code
    MultiObjectiveStatistician::DominationCompare(des, _des, _infos) == _value;
   \endcode
 *
 * where des is the passed in Design.
 */
class DominationPredicate
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The Design to compare against.
        const Design& _des;

        /// The objective function info vector to use in comparisons.
        const ObjectiveFunctionInfoVector& _infos;

        /**
         * \brief The value that is sought from the DominationCompare method
         *        for a true return from the activation operator.
         */
        const int _value;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// An operator that activates this predicate.
        /**
         * \param des The Design to test using this predicate.
         * \return True if MultiObjectiveStatistician::DominationCompare(
                   \a des, _des, _infos) == _value; and false otherwise.
         */
        inline
        bool
        operator ()(
            const Design& des
            ) const;

        /// An operator that activates this predicate
        /**
         * \param des The Design to test using this predicate.
         * \return True if MultiObjectiveStatistician::DominationCompare(
                   *\a des, _des, _infos) == _value; and false otherwise.
         */
        inline
        bool
        operator ()(
            const Design* des
            ) const;

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a predicate using \a des and \a value.
        /**
         * \param des The Design against which to test all that come into this
         *            prediate.
         * \param value The result of the comparison that causes a return of
         *              true by this predicate.
         */
        DominationPredicate(
            const Design& des,
            int value
            );

        inline
        DominationPredicate(
            const DominationPredicate& copy
            ) :
                _des(copy._des),
                _infos(copy._infos),
                _value(copy._value)
        {}

}; // class DominationPredicate


/**
 * \brief This predicate returns true if the stored Design dominates the
 *        passed in Design.
 */
class DominatesPredicate :
    public DominationPredicate
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a DominatesPredicate to compare Designs to \a des
        /**
         * \param des The Design against which to test all that come into this
         *            prediate.
         */
        inline
        DominatesPredicate(
            const Design& des
            );

        inline
        DominatesPredicate(
            const DominatesPredicate& copy
            );

}; // class DominatesPredicate

/**
 * \brief This predicate returns true if the passed in Design
 *        dominates the stored Design.
 */
class DominatedPredicate :
    public DominationPredicate
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a DominatedPredicate to compare Designs to \a des
        /**
         * \param des The Design against which to test all that come into this
         *            prediate.
         */
        inline
        DominatedPredicate(
            const Design& des
            );

        inline
        DominatedPredicate(
            const DominatedPredicate& copy
            );

}; // class DominatedPredicate





/*
================================================================================
Class Definition
================================================================================
*/

/// A class that computes multi-objective statistics about DesignGroups.
/**
 * This class has the capabilities to compute various mulit-objective
 * statistics about collections of Designs such as DesignGroups.  Examples are:
 *
 * -Calculating the domination counts which computes for each Design the
 *  number of Design's it dominates and the number of Design's that dominate
 *  it.
 * -Determining the "layer" for each Design.  The layer is determined by
 *  removing all Pareto optimal designs and giving them a layer value, then
 *  repeating the process with what is left each time incrementing the layer.
 *
 * This class also has the capabilities of comparing individual designs to
 * groups of designs to find out things like "would this Design be
 * non-dominated in this group", etc.
 */
class MultiObjectiveStatistician
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

        /// A mark used by this statistician to mark designs.
        static const std::size_t MARK;



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

        /// Removes the dominated Designs from \a group.
        /**
         * This method goes through and removes all designs in \a group that
         * are not dominant (in a Pareto sense) with respect to the other
         * members of the group.  The designs are remanded to their
         * DesignTarget.
         *
         * \param group The DesignGroup from which to flush all dominated
         *              Designs.
         * \return The number of Designs flushed.
         */
        static
        std::size_t
        FlushDominatedFrom(
            DesignGroup& group
            );

        /**
         * \brief This method returns true if \a des is found to dominate at
         *        least one member of \a in.  false otherwise.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \return True if at least 1 Design in \a in is dominated by \a des.
         */
        static inline
        bool
        DominatesAtLeast1(
            const Design& des,
            const DesignOFSortSet& in
            );

        /**
         * \brief This method returns true if \a des is found to dominate at
         *        least one member of \a in.  false otherwise.
         *
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the first place to look.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The first location after which to look for a dominated
         *              Design.
         * \return True if at least 1 Design in \a in is dominated by \a des.
         */
        static inline
        bool
        DominatesAtLeast1(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where
            );

        /**
         * \brief Finds the first occurance of a Design dominated by \a des in
         *        \a in and returns the iterator to the found Design.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \return An iterator to the first Design in \a in dominated by \a des
         *         or in.end() if no such design can be found.
         */
        static inline
        DesignOFSortSet::const_iterator
        FindDominatedDesign(
            const Design& des,
            const DesignOFSortSet& in
            );

        /**
         * \brief Finds the first occurance of a Design dominated by \a des in
         *        \a in and returns the iterator to the found Design.
         *
         * This version is good if you know that there are or are not
         * feasible designs in the passed in set.  This will take your
         * word and will not search for them etc.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param hasInfeasible A flag to indicate whether or not \a in
         *                      contains any infeasible Designs.
         * \return An iterator to the first Design in \a in dominated by
         *         \a des or in.end() if no such design can be found.
         */
        static inline
        DesignOFSortSet::const_iterator
        FindDominatedDesign(
            const Design& des,
            const DesignOFSortSet& in,
            bool hasInfeasible
            );

        /**
         * \brief Finds the first occurance of a Design dominated by \a des in
         *        \a in and returns the iterator to the found Design.
         *
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the first place to look.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to begin looking for dominated
         *              Designs.
         * \return An iterator to the first Design in \a in dominated by \a des
         *         or in.end() if no such design can be found.
         */
        static
        DesignOFSortSet::const_iterator
        FindDominatedDesign(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where
            );

        /**
         * \brief Finds the first occurance of a Design dominated by \a des in
         *        \a in and returns the iterator to the found Design.
         *
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the first place to look.
         *
         * This version is good if you know that there are or are not
         * feasible designs in the passed in set.  This will take your
         * word and will not search for them etc.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to begin looking for dominated
         *              Designs.
         * \param hasInfeasible A flag to indicate whether or not \a in
         *                      contains any infeasible Designs.
         * \return An iterator to the first Design in \a in dominated by \a des
         *         or in.end() if no such design can be found.
         */
        static
        DesignOFSortSet::const_iterator
        FindDominatedDesign(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where,
            bool hasInfeasible
            );

        /**
         * \brief This method returns true if \a des is found to be dominated
         *        by at least one member of \a in.  false otherwise.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param n The number of Designs of \a in that must dominated \a des
         *          for a return of true.
         * \return True if \a des is dominated by \a n or more Designs in
         *         \a in.
         */
        static inline
        bool
        IsDominatedByAtLeastN(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::size_type n
            );

        /**
         * \brief This method returns true if \a des is found to be dominated
         *        by at least one member of \a in.  false otherwise.
         *
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the first place to look.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param n The number of Designs of \a in that must dominated \a des
         *          for a return of true.
         * \param where The location at which to begin looking for dominated
         *              Designs.
         * \return True if \a des is dominated by \a n or more Designs in
         *         \a in.
         */
        static inline
        bool
        IsDominatedByAtLeastN(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::size_type n,
            DesignOFSortSet::const_iterator where
            );

        /**
         * \brief This method returns true if \a des is found to be dominated
         *        by at least n members of \a in.  false otherwise.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \return True if \a des is dominated by 1 or more Designs in \a in.
         */
        static inline
        bool
        IsDominatedByAtLeast1(
            const Design& des,
            const DesignOFSortSet& in
            );

        /**
         * \brief This method returns true if \a des is found to be dominated
         *        by at least one member of \a in.  false otherwise.
         *
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the first place to look.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to begin looking for dominated
         *              Designs.
         * \return True if \a des is dominated by 1 or more Designs in \a in.
         */
        static inline
        bool
        IsDominatedByAtLeast1(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where
            );

        /**
         * \brief Finds the first occurance of a Design that dominates \a des
         *        in \a in and returns the iterator to the found Design.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \return The location of a Design in \a in that dominates \a des.
         */
        static inline
        DesignOFSortSet::const_iterator
        FindDominatingDesign(
            const Design& des,
            const DesignOFSortSet& in
            );

        /**
         * \brief Finds the first occurance of a Design that dominates \a des
         *        in \a in and returns the iterator to the found Design.
         *
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the last place to look.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to begin looking for dominating
         *              Designs.
         * \return The location of a Design in \a in that dominates \a des.
         */
        static
        DesignOFSortSet::const_iterator
        FindDominatingDesign(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where
            );

        /**
         * \brief This method determines which of \a des1 and \a des2 is
         *        dominant if either.
         *
         * The return is modeled after strcmp.  A return of -1  means that
         * \a des1 dominates \a des2.  A return of 0 means that neither
         * dominates, and a return of 1 means that \a des2 dominates \a des1.
         *
         * \param des1 The first Design to compare.
         * \param des2 The second Design to compare.
         * \return -1 if \a des1 domiantes \a des2, 1 if \a des2 dominates
         *         \a des1 and 0 if neither dominates the other.
         */
        static inline
        int
        DominationCompare(
            const Design& des1,
            const Design& des2
            );

        /**
         * \brief This method determines which of \a des1 and \a des2 is
         *        dominant if either.
         *
         * This method is the same as it's overload except that it uses the
         * passed in objective function info's rather than getting them for
         * itself.
         *
         * \param des1 The first Design to compare.
         * \param des2 The second Design to compare.
         * \param infos The ObjectiveFunctionInfos used to determine which
         *              Design has preferred objective values.
         * \return -1 if \a des1 domiantes \a des2, 1 if \a des2 dominates
         *         \a des1 and 0 if neither dominates the other.
         */
        static
        int
        DominationCompare(
            const Design& des1,
            const Design& des2,
            const ObjectiveFunctionInfoVector& infos
            );

        /// Finds the Design that is most dominated by \a by in \a in.
        /**
         * This method returns an iterator pointing to the Design in \a in most
         * dominated by \a by.  This is accomplished by traversing the entire
         * group and keeping track of the current worst at all times.
         *
         * \param by The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a by.
         * \return The Design in \a in that is most domianted by \a by.
         */
        static
        DesignOFSortSet::const_iterator
        FindMostDominated(
            const Design& by,
            const DesignOFSortSet& in
            );

        /// Finds the Pareto extremes for the passed in set.
        /**
         * Type SolT must be a type that conforms to the interface of a
         * JEGA::Utilities::Solution.  That includes the Solution class and the
         * Design class.
         *
         * DesignOFSortSet must be an associative container, must hold
         * SolT*'s, be forward iteratable in STL style, be default
         * constructable, support the begin, end, erase, and insert
         * methods, and be sorted by objective function.
         *
         * Finds the extreme values for each objective function
         * considering only the feasible, non-dominated SolTs
         * in \a of.
         *
         * \param of The set of SolTs to find the Pareto extremes of.
         * \return An extremes object containing the "points" of the
         *         non-dominated subset of \a of.
         */
        static
        eddy::utilities::DoubleExtremes
        FindParetoExtremes(
            const DesignOFSortSet& of
            );

        /// Counts how many Designs in \a in are dominated by \a des
        /**
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \return The number of Designs in \a in that are dominated by \a des.
         */
        static inline
        std::size_t
        CountNumDominatedBy(
            const Design& des,
            const DesignOFSortSet& in,
            int cutoff = -1
            );

        /// Counts how many Designs in \a in are dominated by \a des
        /**
         * This version is good if you know that there are or are not
         * feasible designs in the passed in set.  This will take your
         * word and will not search for them etc.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param hasInfeasible A flag to indicate whether or not \a in
         *                      contains any infeasible Designs.
         * \return The number of Designs in \a in that are dominated by \a des.
         */
        static inline
        std::size_t
        CountNumDominatedBy(
            const Design& des,
            const DesignOFSortSet& in,
            bool hasInfeasible,
            int cutoff = -1
            );

        /// Counts how many Designs in \a in are dominated by \a des
        /**
         * The optional iterator argument is for the case where \a des
         * is a member of \a in.  Supplying it can speed up the search.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to begin looking for dominated
         *              Designs.
         * \return The number of Designs in \a in that are dominated by \a des.
         */
        static inline
        std::size_t
        CountNumDominatedBy(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where,
            int cutoff = -1
            );

        /// Counts how many Designs in \a in are dominated by \a des
        /**
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the last place to look.
         *
         * This version is good if you know that there are or are not
         * feasible designs in the passed in set.  This will take your
         * word and will not search for them etc.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to end looking for dominated
         *              Designs.
         * \param hasInfeasible A flag to indicate whether or not \a in
         *                      contains any infeasible Designs.
         * \return The number of Designs in \a in that are dominated by \a des.
         */
        static
        std::size_t
        CountNumDominatedBy(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where,
            bool hasInfeasible,
            int cutoff = -1
            );

        /**
         * \brief Counts how many Designs in \a in dominate \a des subject to
         *        the supplied cutoff.
         *
         * Once cutoff has been reached as a count, the counting is stopped.
         * Therefore the return will never be larger then a non-negative cutoff
         * value.  A cutoff value of -1 is interpreted as no cutoff.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param cutoff The limit after which to stop counting.
         * \return The number of Designs in \a in that dominate \a des.
         */
        static inline
        std::size_t
        CountNumDominating(
            const Design& des,
            const DesignOFSortSet& in,
            eddy::utilities::int32_t cutoff = -1
            );

        /// Counts how many Designs in \a in dominate \a des
        /**
         * The iterator argument is for the case where the proper location for
         * \a des is known within \a in.  This doesn't mean that \a des must be
         * in \a in, only that you know where it would go if it were to be
         * placed in \a in. Supplying it can speed up the search.  It serves as
         * the last place to look.
         *
         * \param des The Design to compare to the Designs of \a in.
         * \param in The set of Designs to compare to \a des.
         * \param where The location at which to end looking for dominating
         *              Designs.
         * \param cutoff The limit after which to stop counting.
         * \return The number of Designs in \a in that dominate \a des.
         */
        static
        std::size_t
        CountNumDominating(
            const Design& des,
            const DesignOFSortSet& in,
            DesignOFSortSet::const_iterator where,
            int cutoff = -1
            );
//
//        /**
//         * \brief Finds the non-dominated in \a from and adds them in a new
//         *        container.
//         *
//         * Type DesignOFSortSet must be an associative container holding a Design
//         * derivative pointer type.  It must support the begin and end methods.
//         *
//         * IntoSet_T must hold support an insert method that takes
//         * a single argument of type DesignOFSortSet::value_type.
//         *
//         * Domination is determined relative to the Designs in \a from only.
//         *
//         * \param from The set from which to retrieve the non-dominated into
//         *             a new collection.
//         * \return A container of type IntoSet_T containing only the
//         *         non-dominated in \a from.
//         */
//        template <typename IntoSet_T>
//        static
//        IntoSet_T
//        GetNonDominated(
//            const DesignOFSortSet& from
//            );

        /**
         * \brief Finds the non-dominated in \a from and adds them in a new
         *        container.
         *
         * Type DesignOFSortSet must be an associative container holding a
         * Design derivative pointer type.  It must support the begin and end
         * methods and an insert method that takes a single argument of type
         * DesignOFSortSet::value_type.
         *
         * Domination is determined relative to the Designs in \a from only.
         *
         * \param from The set from which to retrieve the non-dominated into
         *             a new collection.
         * \return A container of type IntoSet_T containing only the
         *         non-dominated in \a from.
         */
        static inline
        DesignOFSortSet
        GetNonDominated(
            const DesignOFSortSet& from
            );
//
//        /// Finds the non-dominated in \a from and adds them in a new container.
//        /**
//         * IntoSet_T must hold Design*'s and support an insert method that
//         * takes a single argument of type Design*.
//         *
//         * Domination is determined relative to the Designs in from only.
//         *
//         * \param from The set from which to retrieve the dominated into
//         *             a new collection.
//         * \return A container of type IntoSet_T containing only the
//         *         dominated in \a from.
//         */
//        template <typename IntoSet_T>
//        static
//        IntoSet_T
//        GetDominated(
//            const DesignOFSortSet& from
//            );

        /**
         * \brief Finds the non-dominated in \a from and adds them in a new
         *        container.
         *
         * DesignOFSortSet must hold pointers to Design derivatives and support
         * an insert method that takes a single argument of type
         * DesignOFSortSet::value_type.
         *
         * Domination is determined relative to the Designs in \a from only.
         *
         * \param from The set from which to retrieve the dominated into
         *             a new collection.
         * \return A container of type IntoSet_T containing only the
         *         dominated in \a from.
         */
        static inline
        DesignOFSortSet
        GetDominated(
            const DesignOFSortSet& from
            );
//
//        /**
//         * \brief Removes the non-dominated from \a from and returns them in
//         *        a new container.
//         *
//         * \param from The set from which to remove the non-dominated into
//         *             a new collection.
//         * \return A container of type IntoSet_T containing only the
//         *         non-dominated that were in \a from.
//         */
//        template <typename IntoSet_T>
//        static
//        IntoSet_T
//        SeparateNonDominated(
//            DesignOFSortSet& from
//            );

        /**
         * \brief Removes the non-dominated from \a from and returns them in a
         *        new container.
         *
         * \param from The set from which to remove the non-dominated into
         *             a new collection.
         * \return A container of type IntoSet_T containing only the
         *         non-dominated that were in \a from.
         */
        static inline
        DesignOFSortSet
        SeparateNonDominated(
            DesignOFSortSet& from
            );
//
//        /**
//         * \brief Removes the dominated from \a from and returns them in a new
//         *        container.
//         *
//         * \param from The set from which to remove the dominated into
//         *             a new collection.
//         * \return A container of type IntoSet_T containing only the
//         *         dominated that were in \a from.
//         */
//        template <typename IntoSet_T>
//        static
//        IntoSet_T
//        SeparateDominated(
//            DesignOFSortSet& from
//            );

        /**
         * \brief Removes the dominated from \a from and returns them in a new
         *        container.
         *
         * \param from The set from which to remove the dominated into
         *             a new collection.
         * \return A container of type IntoSet_T containing only the
         *         dominated that were in \a from.
         */
        static inline
        DesignOFSortSet
        SeparateDominated(
            DesignOFSortSet& from
            );

        /**
         * \brief Computes the number that each of the Designs in \a designs
         *        is dominated by within \a designs.
         *
         * The resulting Design/number pairs are to be read as:
         * \verbatim
            "Design" is dominated by "number" of the other designs that exist
            in the set \a designs.
           \endverbatim
         *
         * \param designs The set of Designs for which to compute the
         *                "dominated-by" counts.
         * \param cutoff The limit after which to stop counting.
         * \return A map of Designs to count values where the Designs are those
         *         contained in \a designs and the counts are the
         *         "dominated-by" counts.
         */
        static
        DesignCountMap
        ComputeDominatedByCounts(
            const DesignOFSortSet& designs,
            int cutoff = -1
            );

        /**
         * \brief Computes the number that each of the Designs in \a designs
         *        dominates within \a designs.
         *
         * The resulting Design/number pairs are to be read as:
         * \verbatim
            "Design" dominates "number" of the other designs that exist in
            the set \a designs.
           \endverbatim
         *
         * \param designs The set of Designs for which to compute the
         *                "dominates" counts.
         * \param cutoff The limit after which to stop counting.
         * \return A map of Designs to count values where the Designs are those
         *         contained in \a designs and the counts are the
         *         "dominates" counts.
         */
        static
        DesignCountMap
        ComputeDominatingCounts(
            const DesignOFSortSet& designs,
            int cutoff = -1
            );

        /// Computes the domination counts of the Designs in \a designs.
        /**
         * This includes both the number that each design dominates and
         * the number that they are dominated by.
         *
         * The number they dominate is the first count and the number they
         * are dominated by is the second.
         *
         * \param designs The set of Designs for which to compute the
         *                "dominates" and "dominated-by" counts.
         * \return A map of Designs to two count values where the Designs are
         *         those contained in \a designs and the counts are the
         *         "dominates" count and "dominated-by" count respectively.
         */
        static
        DesignDoubleCountMap
        ComputeDominationCounts(
            const DesignOFSortSet& designs
            );

        /// Computes the layers of the Designs in \a designs.
        /**
         * See the description of the MultiObjectiveStatistician class for
         * a description of what a layer is.
         *
         * The resulting Design/number pairs are to be read as:
         * \verbatim
            "Design" has a layer of "number" within the set \a designs.
           \endverbatim
         *
         * \param designs The set of Designs for which to compute the
         *                layers.
         * \return A map of Designs to layer values where the Designs are
         *         those contained in \a designs.
         */
        static
        DesignCountMap
        ComputeLayers(
            const DesignOFSortSet& designs
            );

        /**
         * \brief Tests \a des to see if it is at an extreme according to the
         *        supplied set of extremes.
         *
         * This is intended for use in testing to see if a Design exists at
         * some extreme of the Pareto frontier.  This is the case if the
         * Design has an extreme value (either max or min) for at least all but
         * one of its objective functions.  Of course, if it is extreme for
         * all objectives, it will be considered an extreme Design.
         *
         * \param des The Design to test as an extreme Design.
         * \param paretoExtremes The set of extremes to test \a des against.
         * \return true if \a des is at an extreme according to
         *         \a paretoExtremes and false otherwise.
         */
        static
        bool
        IsExtremeDesign(
            const JEGA::Utilities::Design& des,
            const eddy::utilities::DoubleExtremes& paretoExtremes
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

        /// Default constructs a MultiObjectiveStatistician.
        /**
         * This constructor is private and has no implementation.  This is
         * because all methods of this class are static and thus it
         * should not be instantiated.
         */
        MultiObjectiveStatistician(
            );


}; // class MultiObjectiveStatistician



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
#include "./inline/MultiObjectiveStatistician.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_MUTLIOBJECTIVESTATISTICIAN_HPP
