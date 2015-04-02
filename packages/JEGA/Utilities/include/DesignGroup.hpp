/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignGroup.

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

        Fri May 16 12:05:31 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignGroup class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNGROUP_HPP
#define JEGA_UTILITIES_DESIGNGROUP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>
#include <functional>
#include <utilities/include/extremes.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>



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

/// Class used to keep a collection of Design's in sorted order.
/**
 * This class provides the capability of storing Design's sorted by both
 * design variable and objective function value.  It also provides some
 * functionality for finding out information about those Design's.
 *
 * The philisophy surrounding the sorted containers that the sort is useful
 * for fast finding, insertion, removal, etc.  The design variable sort
 * provides a means of storing any design sorted heirchically by design
 * variables.  The objective function sort does the same but for objective
 * function. The objective function sort exists because many methods and
 * algorithms can take advantage of this property when operating.
 *
 * The catch with having the two different sorts is that they are not
 * necessarily in sync with one another.  Designs can only be inserted into
 * the objective function sort once they have been evaluated.  Otherwise, if
 * they were inserted with invalid objective values which later changed, they
 * would no longer be in sorted order.  As such, this group object has methods
 * that allow the user to test the synchronicity of the sorts and a method
 * to synchronize them.
 */
class JEGA_SL_IEDECL DesignGroup
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /**
         * \brief The type of the container for keeping designs sorted by
         *        design variable.
         */
        typedef
        DesignDVSortSet
        DVSortContainer;

        /**
         * \brief The type of the container for keeping designs sorted by
         *        objective function.
         */
        typedef
        DesignOFSortSet
        OFSortContainer;

        /**
         * \brief The type of the container for keeping designs sorted by
         *        objective function.
         */
        typedef
        DesignCNSortSet
        CNSortContainer;

    protected:


    private:

    /*
    ============================================================================
    Enumerators
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

        /**
         * \brief A list of all Designs that are part of this group
         *        sorted heirarchically by design variable.
         */
        DVSortContainer _dvSort;

        /**
         * \brief A list of all Designs that were evaluated at the
         *        time the insertion request was made that are part of this
         *        group. This list is sorted hierarchically by Objective
         *        Function.
         */
        OFSortContainer _ofSort;

        /// The target being used by this DesignGroup.
        DesignTarget& _target;

        /// A name for this group
        std::string _title;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the _title data member to "title".
        /**
         * \param title The new title for this group.
         */
        inline
        void
        SetTitle(
            const std::string& title
            );





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the Design target being used by this DesignGroup
        /**
         * \return The target for which this is a group of Designs.
         */
        inline
        DesignTarget&
        GetDesignTarget(
            ) const;

        /// Gets a const reference to the DVSortContainer for this group
        /**
         * \return An immutable reference to the container of Designs sorted
         *         by design variable.
         */
        inline
        const DVSortContainer&
        GetDVSortContainer(
            ) const;

        /// Gets a const reference to the OFSortContainer for this group
        /**
         * \return An immutable reference to the container of Designs sorted
         *         by objective function.
         */
        inline
        const OFSortContainer&
        GetOFSortContainer(
            ) const;

        /// Returns the title of this group.
        /**
         * \return The title given to this group if any.
         */
        inline
        const std::string&
        GetTitle(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Computes and returns the maxs and mins for each design
         *        variable in this group.
         *
         * \return An object containing the extreme values (min and max) for
         *         each design variable of the Designs contained in this group.
         */
        eddy::utilities::DoubleExtremes
        ComputeDVExtremes(
            ) const;

        /**
         * \brief Computes and returns the maxs and mins for each objective
         *        function in this group.
         *
         * \return An object containing the extreme values (min and max) for
         *         each objective function of the Designs contained in this
         *         group.
         */
        eddy::utilities::DoubleExtremes
        ComputeOFExtremes(
            ) const;

        /// Reads Designs in from "filename" and makes them part of this group.
        /**
         * \param filename The name of the file from which to read Designs and
         *        add them to this group.
         * \return The number of Designs read in and added.
         */
        DVSortContainer::size_type
        LoadDesignsFromFile(
            const std::string& filename
            );

        /**
         * \brief returns true if the OFSortContainer is the same size as the
         *        DVSortContainer.
         *
         * It is assumed that if they are the same size that they also contain
         * the same Designs.  This is generally a very safe assumption.
         *
         * The ofsort is often not current because designs cannot be added to
         * it until they are evaluated.  They can however be added to the
         * dvsort at any time.
         *
         * \return True if the ofsort is current and false otherwise.
         */
        inline
        bool
        IsOFSortCurrent(
            ) const;

        /// Returns the size of this group (actually size of DVSortContainer)
        /**
         * \return The number of Designs in this group according to the design
         *         variable sorted container.
         */
        inline
        DVSortContainer::size_type
        GetSize(
            ) const;

        /// Returns an iterator to the first member of the DVSortContainer
        /**
         * \return An immutable iterator to the first element of the design
         *         variable sorted container of this group.
         */
        inline
        DVSortContainer::const_iterator
        BeginDV(
            ) const;

        /// Returns an iterator to the first member of the DVSortContainer
        /**
         * \return A mutable iterator to the first element of the design
         *         variable sorted container of this group.
         */
        inline
        DVSortContainer::iterator
        BeginDV(
            );

        /**
         * \brief Returns an iterator to one past the last member of the
         *        DVSortContainer
         *
         * \return An immutable iterator to one past the last element of the
         *         design variable sorted container of this group.
         */
        inline
        DVSortContainer::const_iterator
        EndDV(
            ) const;

        /**
         * \brief Returns an iterator to one past the last member of the
         *        DVSortContainer
         *
         * \return A mutable iterator to one past the last element of the
         *         design variable sorted container of this group.
         */
        inline
        DVSortContainer::iterator
        EndDV(
            );

        /// Returns an iterator to the first member of the OFSortContainer
        /**
         * \return An immutable iterator to the first element of the objective
         *         function sorted container of this group.
         */
        inline
        OFSortContainer::const_iterator
        BeginOF(
            ) const;

        /// Returns an iterator to the first member of the OFSortContainer
        /**
         * \return A mutable iterator to the first element of the objective
         *         function sorted container of this group.
         */
        inline
        OFSortContainer::iterator
        BeginOF(
            );

        /**
         * \brief Returns an iterator to one past the last member of the
         *        OFSortContainer
         *
         * \return An immutable iterator to one past the last element of the
         *         objective function sorted container of this group.
         */
        inline
        OFSortContainer::const_iterator
        EndOF(
            ) const;

        /**
         * \brief Returns an iterator to one past the last member of the
         *        OFSortContainer
         *
         * \return A mutable iterator to one past the last element of the
         *         objective function sorted container of this group.
         */
        inline
        OFSortContainer::iterator
        EndOF(
            );

        /// Returns the number of Designs in the DVSortContainer
        /**
         * \return The number of design stored in the design variable sorted
         *         container of this group.
         */
        inline
        DVSortContainer::size_type
        SizeDV(
            ) const;

        /// Returns the number of Designs in the OFSortContainer
        /**
         * \return The number of design stored in the objective function sorted
         *         container of this group.
         */
        inline
        OFSortContainer::size_type
        SizeOF(
            ) const;

        /// Counts the number of designs in the group labeled feasible.
        /**
         * This method ignores designs that have not been evaluated.
         * They do not contribute to the count in any way.
         *
         * \return The number of designs in this group for which
         *         Design::IsFeasible returns true.
         */
        std::size_t
        CountFeasible(
            ) const;

        /**
         * \brief Returns true if all the Designs in this group are
         *        evaluated and feasible and false otherwise.
         *
         * \return True if all designs in the design variable sort of this
         *         container are evaluated and feasible.
         */
        inline
        bool
        AllDesignsAreFeasible(
            ) const;

        /**
         * \brief Returns true if any the evaluated Designs in this group are
         *        feasible and false otherwise.
         *
         * This method searches the design variable sorted set for a feasible
         * and evaluated Design and stops when it finds one.
         *
         * \return true if at least 1 evaluated and feasible design exists in
         *         this group.
         */
        bool
        AnyDesignsAreFeasible(
            ) const;

        /**
         * \brief Removes the Design pointed to by "des" from all lists of this
         *        group.
         *
         * This is for use when you don't care about the fact that iterators
         * will become invalidated.  If you are iterating, use the EraseRetDV
         * or EraseRetOF methods.  They are faster anyway b/c this method
         * requires two searches whereas they require only one.
         *
         * \param des The Design to completely remove from this group.
         * \return True if the design was found and removed and false otherwise.
         */
        bool
        Erase(
            const Design* des
            );

        /**
         * \brief Removes the element pointed to by "where" from all lists of
         *        this group.
         *
         * "where" must be an iterator from the DVSortContainer.   The return
         * is the iterator of the next element of the DVSortContainer after
         * the one removed or the EndDV() if none.
         *
         * \param where The iterator pointing to the design in the design
         *              variable sorted container that is to be removed.
         * \return The iterator to the next Design after where once it has been
         *            removed.
         */
        inline
        DVSortContainer::iterator
        EraseRetDV(
            DVSortContainer::iterator where
            );

        /**
         * \brief Removes the element pointed to by "where" from all lists of
         *        this group.
         *
         * "where" must be an iterator from the OFSortContainer.  The return
         * is the iterator of the next element of the OFSortContainer after
         * the one removed or the EndDV() if none.
         *
         * \param where The iterator pointing to the design in the objective
         *              function sorted container that is to be removed.
         * \return The iterator to the next Design after where once it has been
         *         removed.
         */
        inline
        OFSortContainer::iterator
        EraseRetOF(
            OFSortContainer::iterator where
            );

        /**
         * \brief Inserts the Design pointed to by "des" into all lists of this
         *        group if allowable.
         *
         * It would not be allowable for instance to insert a Design that has
         * not been evaluated into the OFSortContainer.  That would compromise
         * the sort at evaluation time.  There is no return value so use this
         * method if you don't care where the Design winds up in the lists.
         *
         * \param des The Design to insert to all lists of this group.
         */
        inline
        void
        Insert(
            Design* des
            );

        /**
         * \brief Inserts the Design pointed to by "des" into all lists of this
         *        group if allowable.
         *
         * It would not be allowable for instance to insert a Design that has
         * not been evaluated into the OFSortContainer.  That would compromise
         * the sort at evaluation time.
         *
         * \param des The Design to insert to all lists of this group.
         * \return The iterator pointing to the inserted Design in the
         *         DVSortContainer.
         */
        inline
        DVSortContainer::iterator
        InsertRetDV(
            Design* des
            );

        /**
         * \brief Inserts the Design pointed to by "des" into all lists of this
         *        group if allowable.
         *
         * It would not be allowable for instance to insert a Design that has
         * not been evaluated into the OFSortContainer.  That would compromise
         * the sort at evaluation time.
         *
         * The return is the iterator pointing to the inserted Design in the
         * OFSortContainer or the EndOF() if insertion there fails (although
         * it will have been successfully inserted into the DVSortContainer no
         * matter what so use SynchronizeOFAndDVContainers to recover when the
         * time comes).
         *
         * \param des The Design to insert to all lists of this group.
         * \return The iterator pointing to the inserted Design in the
         *         OFSortContainer or EndOF if it could not be inserted there.
         */
        inline
        OFSortContainer::iterator
        InsertRetOF(
            Design* des
            );

        /**
         * \brief Removes the element pointed to by "where" from all lists of
         *        this group and Inserts "des".
         *
         * "where" must be an iterator from the DVSortContainer.  It returns
         * the return value of the InsertRetDV method (see InsertRetDV for
         * description).
         *
         * \param where The location of the Design to be removed.
         * \param des The Design to be inserted.
         * \return The iterator to the location in the design variable sorted
         *         container of this group at which "des" was placed.
         */
        inline
        DVSortContainer::iterator
        ReplaceRetDV(
            DVSortContainer::iterator where,
            Design* des
            );

        /**
         * \brief Removes the element pointed to by "where" from all lists of
         *        this group and Inserts "des".
         *
         * "where" must be an iterator from the OFSortContainer.  It returns
         * the return value of the InsertRetOF method (see InsertRetOF for
         * description).
         *
         * \param where The location of the Design to be removed.
         * \param des The Design to be inserted.
         * \return The iterator to the location in the objective function
         *         sorted container of this group at which "des" was placed
         *         or EndOF if "des" could not be put in the objective sort.
         */
        inline
        OFSortContainer::iterator
        ReplaceRetOF(
            OFSortContainer::iterator where,
            Design* des
            );

        /**
         * \brief Brings the OFSortContainer up to date with the Designs
         *        contained in the DVSortContainer.
         *
         * This requires no action if IsOFSortCurrent returns true.
         *
         * This method does not stop when unsuccessful inserts occur so the
         * resulting OFSortContainer will contain as many Designs as possible.
         * Inserts into the objective function sort container will fail if some
         * designs are not evaluated.
         *
         * \return true if every Design was successfully loaded as stipulated
         *         by the insert methods, false otherwise.  The return is the
         *         value of IsOFSortCurrent after all loading is done.
         */
        bool
        SynchronizeOFAndDVContainers(
            ) const;

        /**
         * \brief Clears both the Design Variable and Objective Function Sort
         *        Containers.
         *
         * Uses ClearDVContainer and ClearOFContainer respectively.  The
         * cleared Design pointers are neither deleted nor sent to be stored
         * elsewhere so be sure that they are referenced in other places before
         * calling this method.  It is meant to be useful when a set of designs
         * is known to exist in more than one group.
         */
        inline
        void
        ClearContainers(
            );

        /// Equates this group to "right".
        /**
         * \param right The group from which to assign properties into this.
         * \return This group after assignment.
         */
        const DesignGroup&
        operator = (
            const DesignGroup& right
            );

        /**
         * \brief Removes every Design in the group and sends them to be stored
         *        by their DesignTarget.
         */
        void
        FlushAll(
            );

        /**
         * \brief Completely removes "it" from this group and passes it to the
         *        target.
         *
         * The design will be removed from all sorted sets.
         * "it" must be a valid iterator from this group's DVSortContainer.
         * The return is the iterator to the first element after the removed
         * Design in the DVSortContainer.
         *
         * \param it The iterator to the design to flush from this group.
         * \return An iterator to the next Design after the one removed or
         *         EndDV if none.
         */
        DVSortContainer::iterator
        FlushDesignRetDV(
            DVSortContainer::iterator it
            );

        /**
         * \brief Completely removes "it" from this group and passes it to the
         *        target.
         *
         * The design will be removed from all sorted sets.
         * "it" must be a valid iterator from this group's OFSortContainer.
         * The return is the iterator to the first element after the removed
         * Design in the OFSortContainer.
         *
         * \param it The iterator to the design to flush from this group.
         * \return An iterator to the next Design after the one removed or
         *         EndOF if none.
         */
        OFSortContainer::iterator
        FlushDesignRetOF(
            OFSortContainer::iterator it
            );

        /**
         * \brief Removes every Design in the group that returns true on
         *        IsCloned().
         *
         * This method sends all clone Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushCloneDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns true on
         *        IsEvaluated().
         *
         * This method sends all evaluated Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushEvaluatedDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns true on
         *        IsFeasible().
         *
         * This method sends all feasible Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushFeasibleDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns true on
         *        IsIllconditioned().
         *
         * This method sends all illconditioned Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushIllconditionedDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns false on
         *        IsCloned().
         *
         * This method sends all non-clone Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushNonCloneDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns false on
         *        IsEvaluated().
         *
         * This method sends all non-evaluated Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushNonEvaluatedDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns false on
         *        IsIllconditioned().
         *
         * This method sends all non-illconditioned Designs to be stored by
         * their DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushNonIllconditionedDesigns(
            );

        /**
         * \brief Removes every Design in the group that returns false on
         *        IsFeasible().
         *
         * This method sends all infeasible Designs to be stored by their
         * DesignTarget.
         *
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushNonFeasibleDesigns(
            );

        /// Tells whether this group is empty of Designs or not.
        /**
         * This is implemented by checking the sorted set to be sure
         * that they are both empty.
         *
         * \return true if there are no designs in this group and false
         *         otherwise.
         */
        inline
        bool
        IsEmpty(
            ) const;

        /// Returns true if the Design can be found in this group.
        /**
         * Only the design variable sort is searched because it should not
         * be possible for designs to exist in the objective sort set that do
         * not also exist in the design variable sort set.
         *
         * \param "des" the design to find in this group.
         * \return true if "des" is found in this group and false otherwise.
         */
        inline
        bool
        ContainsDesign(
            const Design& des
            ) const;

        /// Copies the designs in "other" into this group.
        /**
         * This method leaves other in tact.  The Designs are not duplicated
         * but will henceforth exists as is in each group.
         *
         * \param other The other group from which to adopt Designs.
         */
        void
        CopyIn(
            const DesignGroup& other
            );

        /// Inserts the designs in the passed in container into this group.
        /**
         * The container type must support forward iteration.  Appropriate
         * iterators must be attainable using the first class container syntax
         * of other.begin() and other.end().
         *
         * The passed in container is not altered in any way.
         *
         * \param other The container from which to adopt Designs.
         */
        template <typename Cont>
        void
        AbsorbDesigns(
            const Cont& other
            );

        /// Inserts the designs in the range [start, end) into this group.
        /**
         * The iterators must be to Design pointers and must be forward
         * iterators and support the pre-increment operation.  "end" must be
         * achievable by forward iteration from "start" and all Design pointers
         * must be valid.
         *
         * The passed in range is not altered in any way.
         *
         * \param start The first iterator in the range of Designs to adopt.
         *              "start" will be adopted.
         * \param end One past the last iterator in the range to be adopted.
         *            "end" will not be adopted.
         */
        template <typename It>
        void
        AbsorbDesigns(
            It start,
            const It& end
            );

        /**
         * \brief Copies the designs in "other" into this group and then clears
         *        "other".
         *
         * \param other The container from which to adopt Designs.
         */
        inline
        void
        MergeIn(
            DesignGroup& other
            );

        /**
         * \brief Detects and removes each non-unique Design configuration of
         *        this container.
         *
         * For each set of duplicate Design configurations, 1 is considered
         * unique and the rest are considered non-unique.  Those removed are
         * sent to the target.
         *
         * \return The number of designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushNonUnique(
            );

        /**
         * \brief Goes through and removes all designs that have the supplied
         *        attributes.
         *
         * \param attrs The attributes on which to base the flush operations.
         * \param hasOrNot Whether to flush those with the attributes (true)
         *                 or those without (false).
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushDesigns(
            const Design::AttributeMask& attrs,
            bool hasOrNot = true
            );

        /**
         * \brief Goes through and removes all designs that have the supplied
         *        attribute.
         *
         * \param attrIndex The index of the attribute on which to base the
         *                  flush operations.
         * \param hasOrNot Whether to flush those with the attribute (true)
         *                 or those without (false).
         * \return The number of Designs removed.
         */
        inline
        DVSortContainer::size_type
        FlushDesigns(
            std::size_t attrIndex,
            bool hasOrNot = true
            );


    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Removes the Design pointed to by "des" from the OFSort of
         *        this group.
         *
         * The Design will remain in the design variable sorted group.  If
         * "des" is not in the set, this does nothing.
         *
         * \param des The Design to be removed from the objective function
         *            sorted set of this group.
         */
        inline
        void
        EraseFromOFContainer(
            const Design* des
            );

        /**
         * \brief Removes the Design pointed to by "des" from the DVSort of
         *        this group.
         *
         * The Design will remain in the objective function sorted group if it
         * was in there to begin with.  If "des" is not in the set, this does
         * nothing.
         *
         * \param des The Design to be removed from the design variable
         *            sorted set of this group.
         */
        inline
        void
        EraseFromDVContainer(
            const Design* des
            );

        /**
         * \brief Removes the element pointed to by "where" from only the
         *        DVSortContainer of this group.
         *
         * "where" must be an iterator from this groups DVSortContainer.
         *
         * \param where The location in the DV sort to remove.
         * \return The iterator of the next element of the DVSortContainer
         *         after the removed one or the EndDV if none.
         */
        inline
        DVSortContainer::iterator
        EraseFromDVContainer(
            DVSortContainer::iterator where
            );

        /**
         * \brief Removes the element pointed to by "where" from only the
         *        OFSortContainer of this group.
         *
         * "where" must be an iterator from this groups OFSortContainer.
         *
         * \param where The location in the OF sort to remove.
         * \return The iterator of the next element of the OFSortContainer
         *         after the removed one or the EndOF if none.
         */
        inline
        OFSortContainer::iterator
        EraseFromOFContainer(
            OFSortContainer::iterator where
            );

        /**
         * \brief Inserts the Design pointed to by "des" into only the
         *        DVSortContainer of this group.
         *
         * \param des The design to insert to only the design variable sorted
         *            set of this group.
         * \return The iterator pointing to the inserted Design in the
         *         DVSortContainer.
         */
        inline
        DVSortContainer::iterator
        InsertIntoDVContainer(
            Design* des
            );

        /**
         * \brief Inserts the Design pointed to by "des" into only the
         *         DVSortContainer of this group.
         *
         * The "first" iterator parameter may be supplied if it is known that
         * the Design is to be inserted belongs after "first".
         *
         * \param des The design to insert to only the design variable sorted
         *            set of this group.
         * \param first An iterator to an element in this groups design
         *              variable sort after which des is to be inserted.
         * \return The iterator pointing to the inserted Design in the
         *         DVSortContainer.
         */
        inline
        DVSortContainer::iterator
        InsertIntoDVContainer(
            Design* des,
            DVSortContainer::iterator first
            );

        /**
         * \brief Inserts the Design pointed to by "des" into only the
         *        OFSortContainer of this group if allowable.
         *
         * It would not be allowable for instance to insert a Design that has
         * not been evaluated into the OFSortContainer.  This would compromise
         * the sort at evaluation time.
         *
         * \param des The design to insert to only the objective function
         *            sorted set of this group.
         * \return The iterator pointing to the inserted Design in the
         *         OFSortContainer or EndOF if insertion fails.
         */
        inline
        OFSortContainer::iterator
        InsertIntoOFContainer(
            Design* des
            );

        /**
         * \brief Inserts the Design pointed to by "des" into only the
         *        OFSortContainer of this group if allowable.
         *
         * It would not be allowable for instance to insert a Design that has
         * not been evaluated into the OFSortContainer.  This would compromise
         * the sort at evaluation time.
         *
         * The "first" iterator parameter may be supplied if it is known that
         * the Design to be inserted belongs after "first".
         *
         * \param des The design to insert to only the objective function
         *            sorted set of this group.
         * \param first An iterator to an element in this groups objective
         *              function sort after which des is to be inserted.
         * \return The iterator pointing to the inserted Design in the
         *         OFSortContainer or EndOF if insertion fails.
         */
        inline
        OFSortContainer::iterator
        InsertIntoOFContainer(
            Design* des,
            OFSortContainer::iterator first
            );

        /**
         * \brief Removes the element pointed to by "where" from only the
         *        DVSortContainer of this group and Inserts "des".
         *
         * "where" must be an iterator from this groups DVSortContainer.
         *
         * \param where An iterator pointing to the Design that should be
         *              removed from the design variable sort container of this
         *              group.
         * \param des The design that is to be inserted into the design
         *            variable sort container of this group.
         * \return The return value of the InsertIntoDVContainer method
         *         (see InsertIntoDVContainer for description).
         */
        inline
        DVSortContainer::iterator
        ReplaceInDVContainer(
            DVSortContainer::iterator where,
            Design* des
            );

        /**
         * \brief Removes the element pointed to by "where" from only the
         *        OFSortContainer of this group and Inserts "des".
         *
         * "where" must be an iterator from this groups OFSortContainer.
         *
         * \param where An iterator pointing to the Design that should be
         *              removed from the objective function sort container of
         *              this group.
         * \param des The design that is to be inserted into the objective
         *            function sort container of this group.
         * \return The return value of the InsertIntoFOContainer method
         *         (see InsertIntoFOContainer for description).
         */
        inline
        OFSortContainer::iterator
        ReplaceInOFContainer(
            OFSortContainer::iterator where,
            Design* des
            );

        /// Clears only the Objective Function Sort Container of this group.
        inline
        void
        ClearOFContainer(
            );

        /**
         * \brief Goes through and removes all designs on which the supplied
         *        predicate evaluates to true.
         *
         * The removed Designs are removed from all sets in this group.
         *
         * \param pred The function object to execute for each Design as a
         *             predicate for removal from this group.
         * \return The number of Designs removed.
         */
        template <typename BoolPredT>
        inline
        DVSortContainer::size_type
        FlushIfTrue(
            const BoolPredT& pred
            );

        /**
         * \brief Goes through and removes all designs on which the supplied
         *        predicate evaluates to false.
         *
         * The removed Designs are removed from all sets in this group.
         *
         * \param pred The function object to execute for each Design as a
         *             predicate for removal from this group.
         * \return The number of Designs removed.
         */
        template <typename BoolPredT>
        inline
        DVSortContainer::size_type
        FlushIfFalse(
            const BoolPredT& pred
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

        /**
         * \brief Goes through and removes all designs for which "pred"
         *        evaluates to true.
         *
         * \param pred The predictate to execute with each design in this
         *             group.
         * \return The number of Designs removed.
         */
        template <typename Predicate>
        DVSortContainer::size_type
        Flush(
            Predicate pred
            );

        /// Clears only the Design Variable Sort Container of this group.
        /**
         * The cleared Design pointers are neither deleted nor sent to be
         * stored elsewhere so be sure that they are referenced in other places
         * before calling this method.  It is meant to be useful when a set of
         * designs is known to exist in more than one group.
         */
        inline
        void
        ClearDVContainer(
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a DesignGroup knowing target.
        /**
         * \param target The DesignTarget for which this group is being
         *               created.
         */
        DesignGroup(
            DesignTarget& target
            );

        /// Copy constructs a DesignGroup.
        /**
         * \param copy The existing DesignGroup from which to copy properties
         *             into this.
         */
        DesignGroup(
            const DesignGroup& copy
            );

        /**
         * \brief This constructor will build a group containing the Design's
         *        in "designs".
         *
         * It is templatized so that any stl compliant container of Design*'s
         * can be used as the argument.  The passed in collection of Designs
         * will be unmodified.
         *
         * \param target The DesignTarget for which this group is being
         *               created.
         * \param designs The existing container of Designs to put into this
         *                new group.
         */
        template<typename DesCont>
        DesignGroup(
            DesignTarget& target,
            const DesCont& designs
            );

        /// Destructs a DesignGroup.
        ~DesignGroup(
            );

}; // class DesignGroup



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
#include "./inline/DesignGroup.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNGROUP_HPP
