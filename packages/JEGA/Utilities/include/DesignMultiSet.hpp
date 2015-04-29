/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignMultiSet.

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

        Wed Apr 07 18:21:54 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignMultiSet class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNMULTISET_HPP
#define JEGA_UTILITIES_DESIGNMULTISET_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <set>
#include <ostream>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>





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
template <typename Pred>
class DesignMultiSet;






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
/// A sorted design container based on the stl multiset class
/**
 * This is a templatized container that requires a predicate for ordering
 * designs.
 *
 * In addition to the inherited STL functionality of the multiset class, this
 * class provides front, back, pop_front, and pop_back STL style methods.
 *
 * This class also provides various methods specific to manipulating a grouping
 * of Design 's.  For example, since it is holding pointers and the predicate
 * searches will look for logical equivolence, methods are available to find
 * exact instances by searching for pointers instead of values.
 */
template <typename Pred>
class JEGA_SL_IEDECL DesignMultiSet :
    public std::multiset<Design*, Pred>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the class which this inherits (short hand).
        typedef
        std::multiset<Design*, Pred>
        base_type;

        /// The type of this class (short hand).
        typedef
        DesignMultiSet<Pred>
        my_type;

        /// The const_iterator type of this set.
        typedef
        typename base_type::const_iterator
        const_iterator;

        /// The iterator type of this set.
        typedef
        typename base_type::iterator
        iterator;

        /// The reference type of this set.
        typedef
        typename base_type::reference
        reference;

        /// The reference type of this set.
        typedef
        typename base_type::const_reference
        const_reference;

        /// The size type of this set.
        typedef
        typename base_type::size_type
        size_type;

        /// The key type of this set.
        typedef
        typename base_type::key_type
        key_type;

        /// The key compare type of this set.
        typedef
        typename base_type::key_compare
        key_compare;

        /// The pointer type of this set.
        typedef
        typename base_type::pointer
        pointer;

        /// The constant pointer type of this set.
        typedef
        typename base_type::const_pointer
        const_pointer;

        /// The allocator pointer type of this set.
        typedef
        typename base_type::allocator_type
        allocator_type;

        /// The constant reverse iterator type of this set.
        typedef
        typename base_type::const_reverse_iterator
        const_reverse_iterator;

        /// The reverse iterator type of this set.
        typedef
        typename base_type::reverse_iterator
        reverse_iterator;

        /// The difference type of this set.
        typedef
        typename base_type::difference_type
        difference_type;

        /// The value type of this set.
        typedef
        typename base_type::value_type
        value_type;

        /// The value compare type of this set.
        typedef
        typename base_type::value_compare
        value_compare;

        /// The return type of the equal_range method when const.
        typedef
        std::pair<const_iterator, const_iterator>
        const_iterator_pair;

        /// The return type of the equal_range method.
        typedef
        std::pair<iterator, iterator>
        iterator_pair;

    protected:


    private:

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The mark that this class uses whenever it must flag a Design.
        static const std::size_t MARK;


    private:




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the last element in the container (const)
        /**
         * Calling this on an empty container will result in a crash or at
         * least undefined behavior.
         *
         * \return An immutable reference to the last element.
         */
        inline
        const_reference
        back(
            ) const;

        /// Returns the first element in the container (const)
        /**
         * Calling this on an empty container will result in a crash or at
         * least undefined behavior.
         *
         * \return An immutable reference to the first element.
         */
        inline
        const_reference
        front(
            ) const;

        /// Returns the last element in the container (const)
        /**
         * Calling this on an empty container will result in a crash or at
         * least undefined behavior.
         *
         * \return A reference to the last element.
         */
        inline
        reference
        back(
            );

        /// Returns the first element in the container (const)
        /**
         * Calling this on an empty container will result in a crash or at
         * least undefined behavior.
         *
         * \return A reference to the first element.
         */
        inline
        reference
        front(
            );

        /// Finds the actual passed in design without regard to the predicate.
        /**
         * If the passed in pointer is not kept in here, the return is end.
         * Otherwise it is an iterator to the location of the Design.
         *
         * The regular find method will find an equivalent Design which may
         * or may not be the actual passed in design.
         *
         * \param key The Design for which an exact match is sought.
         * \return An immutable iterator to the location of "key" or end()
         *         if not found.
         */
        const_iterator
        find_exact(
            const key_type key
            ) const;

        /**
         * \brief Finds a logical equivolent to the passed in Design but
         *        disregards the Design itself.
         *
         * The pointer value of the returned iterator (if not end()) will not
         * be equal to key. If no equivolent matches other than an exact match
         * of key are present, the return is end().  This is just like the
         * regular find method except that a return of the exact same object as
         * is pointed to by "key" is not allowed.
         *
         * The regular find method will find an equivalent Design which may
         * or may not be the actual passed in design and find_exact will find
         * the actual design.
         *
         * \param key The Design for which a duplicate is sought.
         * \return An immutable iterator to the location of a duplicate or
         *         end() if not found.
         */
        const_iterator
        find_not_exact(
            const key_type key
            ) const;

        /**
         * \brief Counts the number of duplicate Design configurations in this
         *        container.
         *
         * For each set of duplicate Design configurations, 1 is considered
         * unique and the rest are considered non-unique.  No tagging of clones
         * takes place. Comparisons are made according to the predicate.
         *
         * \return The number of non-unique Designs found.
         */
        size_type
        count_non_unique(
            ) const;

        /**
         * \brief Prints each Design class object in this container to "stream"
         * using the overload.
         *
         * \param stream The stream into which to write the designs of this
         *               container.
         * \return The supplied \a stream is returned for convenience.
         *         It allows for chaining.
         */
        std::ostream&
        stream_out(
            std::ostream& stream
            ) const;

        /// Writes the Design "des" to "stream".
        /**
         * The design is written in tab-delimited flat file format.  No matter
         * what, all design variables are written.  After that, objective and
         * constraint values are written iff the design has been evaluated and
         * is not illconditioned.
         *
         * \param des The design to write into the supplied "stream".
         * \param stream The stream into which to write the designs of this
         *               container.
         * \return The supplied \a stream is returned for convenience.
         *         It allows for chaining.
         */
        static
        std::ostream&
        stream_out(
            const key_type des,
            std::ostream& stream
            );

        /// Erases the first element in this set.
        /**
         * Calling this on an empty container will result in a crash or at
         * least undefined behavior.
         */
        inline
        void
        pop_front(
            );

        /// Erases the last element in this set.
        /**
         * Calling this on an empty container will result in a crash or at
         * least undefined behavior.
         */
        inline
        void
        pop_back(
            );

        /**
         * \brief Deletes each Design pointer stored in this container then
         *        clears the set.
         */
        void
        flush(
            );

        /**
         * \brief Removes the element at "where" and inserts "key" in a proper
         * location.
         *
         * \param where An iterator to the Design to be removed.
         * \param key The Design to be inserted once "where" is removed.
         * \return An iterator to the new element in it's new location.
         */
        inline
        iterator
        replace(
            iterator where,
            const key_type key
            );

        /// Erases all occurences of the exact key specified.
        /**
         * This does not mean logically equivolent according to the predicate.
         * This erases all occurances of the design pointer specified.  Clones
         * of key may remain in the set.
         *
         * \param key The Design for which instances should be removed.
         * \return The number of occurances found and marked.
         */
        size_type
        erase_exacts(
            const key_type key
            );

        /// Finds the actual passed in design without regard to the predicate.
        /**
         * The pointer value of the returned iterator (if not end()) will not
         * be equal to key. If no equivolent matches other than an exact match
         * of key are present, the return is end().  This is just like the
         * regular find method except that a return of the exact same object as
         * is pointed to by "key" is not allowed.
         *
         * The regular find method will find an equivalent Design which may
         * or may not be the actual passed in design and find_exact will find
         * the actual design.
         *
         * \param key The Design for which a duplicate is sought.
         * \return An iterator to the location of a duplicate or end()
         *         if not found.
         */
        iterator
        find_not_exact(
            const key_type key
            );

        /// Finds the actual passed in design without regard to the predicate.
        /**
         * If the passed in pointer is not kept in here, the return is end.
         * Otherwise it is an iterator to the location of the Design.
         *
         * The regular find method will find an equivalent Design which may
         * or may not be the actual passed in design and the find_not_exact
         * method will find one that is definitely not the same object as key.
         *
         * \param key The Design for which an exact match is sought.
         * \return An iterator to the location of "key" or end()
         *         if not found.
         */
        iterator
        find_exact(
            const key_type key
            );

        /// Copies all elements of "other" into this.  Leaves "other" in tact.
        /**
         * \param other The container from which to copy Designs into this.
         */
        template <typename DesignContainer>
        void
        copy_in(
            const DesignContainer& other
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // look at each member of other in turn.
            const typename DesignContainer::const_iterator oe(other.end());
            for(typename DesignContainer::const_iterator it(other.begin());
                it!=oe; ++it) this->base_type::insert(*it);
        }

        /// Looks for a duplicate of "key" existing within this container.
        /**
         * Comparisons are made according to the predicate.  If found, this
         * method tags the Designs as clones.
         *
         * "key" should not be a member of this set.
         *
         * \param key The Design for which a clone is sought.
         * \return An immutable iterator to the found clone or end() if not
         *         one.
         */
        const_iterator
        test_for_clone(
            const key_type key
            ) const;

        /// Detects and sets dead each non-unique Design configuration.
        /**
         * For each set of duplicate Design configurations, 1 is considered
         * unique and the rest are considered non-unique.  No tagging of clones
         * takes place. Comparisons are made according to the predicate.
         *
         * \return The number of Designs marked.
         */
        size_type
        mark_non_unique(
            std::size_t mark = MARK
            ) const;

        /**
         * \brief Goes through this container and marks all designs for which
         *        the predicate returns true.
         *
         * \param pred The function object to call on each design as it is
         *             considered.
         * \return The number of Designs marked.
         */
        template <typename MarkPred>
        size_type
        mark_if(
            MarkPred pred,
            std::size_t mark = MARK
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            // store the initial size so we can return the removal count.
            size_type nmarked = 0;
            const const_iterator te(this->end());

            // iterate the set and remove all designs for which func evaluates
            // to evaluatesTo.
            for(iterator it(this->begin()); it!=te; ++it)
            {
                bool marked = pred(*it);
                (*it)->Design::ModifyAttribute(mark, marked);
                if(marked) ++nmarked;
            }

            // return the difference between the current size and the old size.
            return nmarked;

        } // DesignMultiSet::mark_if

        /// Checks all the members of "other" for duplicates in this list.
        /**
         * Duplicates found are tagged as clones.
         *
         * Returns the number of times duplicates were detected which is not
         * the same as the number of newly found clones.  It is possible that
         * some clones detected were already clones of other designs and
         * therefore were not "newly" found.
         *
         * Comparisons are made according to the predicate.
         *
         * The design container must be a forward iteratable STL compliant
         * container of Design* 's with begin, end, and empty methods and
         * define the const_iterator type.
         *
         * \param other The other container from which to seek duplicates in
         *              this.
         * \return The number of times duplicates were detected.
         */
        template <typename DesignContainer>
        size_type
        test_for_clones(
            const DesignContainer& other
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE

            // check for the trivial abort conditions
            if(other.empty() || this->empty()) return 0;

            // prepare to store the number of successful clone tests.
            size_type clonecount = 0;
            const typename DesignContainer::const_iterator oe(other.end());
            const const_iterator te(this->end());

            // look at each member of other in turn.
            for(typename DesignContainer::const_iterator it(other.begin());
                it!=oe; ++it)
                    clonecount += (this->test_for_clone(*it)==te) ? 0 : 1;

            // return the number of successful clone tests.
            return clonecount;

        } // test_for_clones

        /**
         * \brief A specialization of the test_for_clones method for containers
         *        of this type.
         *
         * The knowledge that the sorts are the same can be used to expediate
         * this search.  This must be re-implemented in any derived classes and
         * called back on in order for the specialization to work properly.
         *
         * \param other The other container from which to seek duplicates in
         *              this.
         * \return The number of times duplicates were detected.
         */
        size_type
        test_for_clones(
            const my_type& other
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE

            // check for the trivial abort conditions
            if(other.empty() || this->empty()) return 0;

            // Check to be sure that the passed in deque is not this.
            // If it is, this method is not appropriate.
            if(this == &other) return this->test_within_list_for_clones();

            // remember that the complexity of searches in this container is
            // bounded as O(log10(size)).  So it is probably safe to assume
            // that searching in the larger of the lists is nearly as cheap
            // as searching in the smaller.

            // We are going to iterate the smaller list and look at each member
            // but we are going to start with the first possible match and
            // finish with the last.  We can determine those matches using the
            // lower_bound and upper_bound methods.

            // figure out which is the smaller and which is the larger.
            const my_type& smaller =
                (this->size() <= other.size()) ? *this : other;
            const my_type& larger = (&smaller == this) ? other : *this;

            // The lower_bound of the first element of the larger in the
            // smaller will be where we start looking and the upper_bound of
            // the last of larger in the smaller will be where we stop.
            const_iterator s(smaller.lower_bound(*(larger.begin())));
            const const_iterator e(smaller.upper_bound(*(larger.rbegin())));
            const const_iterator le(larger.end());

            // now do our thing with these elements.
            // prepare to store the number of successful clone tests.
            size_type clonecount = 0;

            // look at each member of other in turn.
            for(; s!=e; ++s)
                clonecount += (larger.test_for_clone(*s) == le) ? 0 : 1;

            // return the number of successful clone tests.
            return clonecount;

        } // test_for_clones

        /// Performs clone testing within this list.
        /**
         * This method should be used instead of comparing this container
         * to itself using test_for_clones.
         *
         * Found clones are tagged as such using Desing::TagAsClones.
         * The return is the number of newly found clones (those that were
         * not previously marked as a clone of any other Design).
         *
         * Comparisons are made according to the predicate.
         *
         * \return The number of newly found clones.
         */
        size_type
        test_within_list_for_clones(
            ) const;

        /// Marks all occurences of logical equivolents to the key specified.
        /**
         * This includes the key itself.
         *
         * \param key The Design for which instances should be removed.
         * \return The number of occurances found and marked.
         */
        size_type
        mark(
            const key_type key,
            std::size_t mark = MARK
            );

        /**
         * \brief Marks all occurences of logical equivolents to the key
         *        with the exception of the key itself.
         *
         * \param key The Design for which instances should be removed.
         * \return The number of occurances found and marked.
         */
        size_type
        mark_not_exact(
            const key_type key,
            std::size_t mark = MARK
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
    public:

        /// Constructs a DesignMultiSet using "pred" as the predicate.
        /**
         * \param pred The predicate to use for ordering Designs in this set.
         */
        inline
        DesignMultiSet(
            const key_compare& pred
            );

        /// Copy constructs a DesignMultiSet.
        /**
         * \param copy The set from which properties should be copied.
         */
        inline
        DesignMultiSet(
            const DesignMultiSet& copy
            );

        /**
         * \brief Constructs a DesignMultiSet with initial elements in the
         * range [first, last)
         *
         * \param first The beginning of the sequence of initial Designs for
         *              this set.
         * \param last One past the last of the sequence of initial Designs
         *             for this set.
         * \param pred The predicate to use for ordering Designs in this set.
         */
        template<class InputIterator>
        inline
        DesignMultiSet(
            InputIterator first,
            InputIterator last,
            const key_compare& pred
            ) :
                base_type(first, last, pred)
        {
        } // DesignMultiSet

}; // class DesignMultiSet


/**
 * \brief The class that will serve as a predicate for sorting Designs by
 *        design variable.
 */
class JEGA_SL_IEDECL DVMultiSetPredicate :
    public std::binary_function<Design*, Design*, bool>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The key_type of the set for which this will be the predicate.
        typedef
        Design*
        key_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The activation function for this predicate.
        /**
         * Compares Design 's for a heirarchical sort by design variable such
         * that the Design 's are sorted first by dv0, then by dv1, then dv2,
         * etc.
         *
         * \param d1 The first Design for order comparison.
         * \param d2 The second Design for order comparison.
         * \return True if d1 belongs before d2 and false otherwise.
         */
        inline
        bool
        operator()(
            const key_type d1,
            const key_type d2
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(&d1->GetDesignTarget() == &d2->GetDesignTarget())

            const std::size_t ndv = d1->GetDesignTarget().GetNDV();

            // start iterating the design variables.  Most of the time,
            // it will not be necessary to go past the first one.
            for(std::size_t i=0; i<ndv; ++i)
            {
                // if the current value for design 1 is less than for 2,
                // we know we can exit with a true meaning that design 1
                // belongs before design 2 in the set.
                if(d1->GetVariableRep(i) < d2->GetVariableRep(i)) return true;

                // visa versa is also true.
                if(d1->GetVariableRep(i) > d2->GetVariableRep(i)) return false;
            }

            // if we make it here, the designs are identical (clones of
            // one-another) and we return false in that design 1 does not
            // have to go before design 2 in the list.
            return false;
        }

}; // class DVMultiSetPredicate

/// The type of DesignMultiSet that sorts by design variable.
class JEGA_SL_IEDECL DesignDVSortSet :
    public DesignMultiSet<DVMultiSetPredicate>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the predicate used by this class.
        typedef
        DVMultiSetPredicate
        pred_type;

        /// The type of the class which this inherits (short hand).
        typedef
        DesignMultiSet<pred_type>
        base_type;

        /// The type of this class (short hand).
        typedef
        DesignDVSortSet
        my_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// This is here to ensure proper use of the base class specialization.
        /**
         * \param other The other DesignDVSortSet in which to test for clones.
         * \return The number of times duplicates were detected.
         */
        inline
        size_type
        test_for_clones(
            const my_type& other
            ) const
        {
            return this->base_type::test_for_clones(
                static_cast<const base_type&>(other)
                );
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a DesignDVSortSet
        inline
        DesignDVSortSet(
            ) :
                base_type(pred_type())
        {}

        /// Copy constructs a DesignDVSortSet
        /**
         * \param copy The DesignDVSortSet from which to copy properties into
         *             this.
         */
        inline
        DesignDVSortSet(
            const my_type& copy
            ) :
                base_type(copy)
        {}

        /**
         * \brief Constructs a DesignDVSortSet with initial elements in the
         *        range [first, last)
         *
         * \param first The beginning of the sequence of initial Designs for
         *              this set.
         * \param last One past the last of the sequence of initial Designs
         *             for this set.
         */
        template<class InputIterator>
        inline
        DesignDVSortSet(
            InputIterator first,
            InputIterator last
            ) :
                base_type(first, last, pred_type())
        {}

}; // class DesignDVSortSet

/**
 * \brief The class that will serve as a predicate for sorting Designs by
 *        objective function
 */
class JEGA_SL_IEDECL OFMultiSetPredicate :
    public std::binary_function<Design*, Design*, bool>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The key_type of the set for which this will be the predicate.
        typedef
        Design*
        key_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The activation function for this predicate.
        /**
         * Compares Design 's for a heirarchical sort by objective function
         * such that the Design 's are sorted first by of0, then by of1, then
         * of2, etc.
         *
         * This sort is not strictly ascending or descending based on the
         * real-number values of the objective functions but is instead based
         * on preference for the objectives.  When considering an objective,
         * this predicate tests whether one value is "preferred" to another.
         * for a minimization type objective, preferred will be "lower value".
         * for a maximization objective, preferred is a higher value, etc.
         * The design that is first to have a preferred objective function value
         * goes first.
         *
         * \param d1 The first Design for order comparison.
         * \param d2 The second Design for order comparison.
         * \return True if d1 belongs before d2 and false otherwise.
         */
        inline
        bool
        operator()(
            const key_type d1,
            const key_type d2
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(&d1->GetDesignTarget() == &d2->GetDesignTarget())

            // prepare to iterate those infos.
            const ObjectiveFunctionInfoVector& infos =
                d1->GetDesignTarget().GetObjectiveFunctionInfos();
            const std::size_t nof = infos.size();

            // start iterating the objective functions.  Most of the time,
            // it will not be necessary to go past the first one.
            for(std::size_t i=0; i<nof; ++i)
            {
                // figure out which is the preferred.
                const Design* prefDes = infos[i]->GetPreferredDesign(*d1, *d2);

                // if it is Design 1, we can return true right now.
                if(prefDes == d1) return true;

                // if it is Design 2, we can return false right now.
                if(prefDes == d2) return false;

                // otherwise we have to keep going in the loop.
            }

            // if we make it here, neither was preferred at any point
            // and we return false.
            return false;
        }

}; // class OFMultiSetPredicate;

/// The type of DesignMultiSet that sorts by objective function.
class JEGA_SL_IEDECL DesignOFSortSet :
    public DesignMultiSet<OFMultiSetPredicate>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the predicate used by this class.
        typedef
        OFMultiSetPredicate
        pred_type;

        /// The type of the class which this inherits (short hand).
        typedef
        DesignMultiSet<pred_type>
        base_type;

        /// The type of this class (short hand).
        typedef
        DesignOFSortSet
        my_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// This is here to ensure proper use of the base class specialization.
        /**
         * \param other The other DesignOFSortSet in which to test for clones.
         * \return The number of times duplicates were detected.
         */
        inline
        size_type
        test_for_clones(
            const my_type& other
            ) const
        {
            return this->base_type::test_for_clones(
                static_cast<const base_type&>(other)
                );

        } // test_for_clones

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a DesignOFSortSet
        inline
        DesignOFSortSet(
            ) :
                base_type(pred_type())
        {
        } // DesignOFSortSet

        /// Copy constructs a DesignOFSortSet
        /**
         * \param copy The DesignOFSortSet from which to copy properties into
         *             this.
         */
        inline
        DesignOFSortSet(
            const my_type& copy
            ) :
                base_type(copy)
        {
        } // DesignOFSortSet

        /**
         * \brief Constructs a DesignOFSortSet with initial elements in the
         *        range [first, last)
         *
         * \param first The beginning of the sequence of initial Designs for
         *              this set.
         * \param last One past the last of the sequence of initial Designs
         *             for this set.
         */
        template<class InputIterator>
        inline
        DesignOFSortSet(
            InputIterator first,
            InputIterator last
            ) :
                base_type(first, last, pred_type())
        {
        } // DesignOFSortSet

}; // class DesignOFSortSet

/// The class that will serve as a predicate for sorting Designs by constraint
class JEGA_SL_IEDECL CNMultiSetPredicate :
    public std::binary_function<Design*, Design*, bool>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The key_type of the set for which this will be the predicate.
        typedef
        Design*
        key_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The activation function for this predicate.
        /**
         * Compares Design 's for a heirarchical sort by constraint such
         * that the Design 's are sorted first by cn0, then by cn1, then cn2,
         * etc.
         *
         * \param d1 The first Design for order comparison.
         * \param d2 The second Design for order comparison.
         * \return True if d1 belongs before d2 and false otherwise.
         */
        inline
        bool
        operator()(
            const key_type d1,
            const key_type d2
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(&d1->GetDesignTarget() == &d2->GetDesignTarget())

            // start iterating the constraints.  Most of the time,
            // it will not be necessary to go past the first one.
            for(std::size_t i=0; i<d1->GetDesignTarget().GetNCN(); ++i)
            {
                // if the current value for design 1 is less than for 2,
                // we know we can exit with a true meaning that design 1
                // belongs before design 2 in the set.
                if(d1->GetConstraint(i) < d2->GetConstraint(i)) return true;

                // visa versa is also true.
                if(d1->GetConstraint(i) > d2->GetConstraint(i)) return false;
            }

            // if we make it here, the designs are identical in constraint
            // values and we return false in that design 1 does not
            // have to go before design 2 in the list.
            return false;

        } // operator()

}; // class CNMultiSetPredicate

/// The type of DesignMultiSet that sorts by constraint.
class JEGA_SL_IEDECL DesignCNSortSet :
    public DesignMultiSet<CNMultiSetPredicate>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the predicate used by this class.
        typedef
        CNMultiSetPredicate
        pred_type;

        /// The type of the class which this inherits (short hand).
        typedef
        DesignMultiSet<pred_type>
        base_type;

        /// The type of this class (short hand).
        typedef
        DesignCNSortSet
        my_type;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// This is here to ensure proper use of the base class specialization.
        /**
         * \param other The other DesignCNSortSet in which to test for clones.
         * \return The number of times duplicates were detected.
         */
        inline
        size_type
        test_for_clones(
            const my_type& other
            ) const
        {
            return this->base_type::test_for_clones(
                static_cast<const base_type&>(other)
                );

        } // test_for_clones

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a DesignCNSortSet
        inline
        DesignCNSortSet(
            ) :
                base_type(pred_type())
        {
        } // DesignCNSortSet

        /// Copy constructs a DesignCNSortSet
        /**
         * \param copy The DesignCNSortSet from which to copy properties into
         *             this.
         */
        inline
        DesignCNSortSet(
            const my_type& copy
            ) :
                base_type(copy)
        {
        } // DesignCNSortSet

        /**
         * \brief Constructs a DesignCNSortSet with initial elements in the
         *        range [first, last)
         *
         * \param first The beginning of the sequence of initial Designs for
         *              this set.
         * \param last One past the last of the sequence of initial Designs
         *             for this set.
         */
        template<class InputIterator>
        inline
        DesignCNSortSet(
            InputIterator first,
            InputIterator last
            ) :
                base_type(first, last, pred_type())
        {
        } // DesignCNSortSet

}; // class DesignCNSortSet



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
#include "./inline/DesignMultiSet.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNMULTISET_HPP
