/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignMultiSet.

    NOTES:

        See notes of DesignMultiSet.hpp.

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
 * \brief Contains the inline methods of the DesignMultiSet class.
 */




/*
================================================================================
Includes
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
Static Member Data Definitions
================================================================================
*/
template <typename Pred>
const std::size_t DesignMultiSet<Pred>::MARK = 7;

/*
================================================================================
Inline Mutators
================================================================================
*/








/*
================================================================================
Inline Accessors
================================================================================
*/






/*
================================================================================
Inline Public Methods
================================================================================
*/

template <typename Pred>
inline
typename DesignMultiSet<Pred>::const_reference
DesignMultiSet<Pred>::back(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return *this->rbegin();

} // DesignMultiSet::back

template <typename Pred>
inline
typename DesignMultiSet<Pred>::const_reference
DesignMultiSet<Pred>::front(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return *this->begin();

} // DesignMultiSet::front

template <typename Pred>
inline
typename DesignMultiSet<Pred>::reference
DesignMultiSet<Pred>::back(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    // workaround for compilation on IBM with MIPSpro 7.4.2m
    return const_cast<reference>(static_cast<const_reference>(*this->rbegin()));

} // DesignMultiSet::back

template <typename Pred>
inline
typename DesignMultiSet<Pred>::reference
DesignMultiSet<Pred>::front(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    // workaround for compilation on IBM with MIPSpro 7.4.2m
    return const_cast<reference>(static_cast<const_reference>(*this->begin()));

} // DesignMultiSet::front

template <typename Pred>
typename DesignMultiSet<Pred>::const_iterator
DesignMultiSet<Pred>::find_exact(
    const key_type key
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // start by bounding the search region.
    const_iterator_pair bounds(this->equal_range(key));

    // now, look at every member in that range until key is found.
    for(; bounds.first!=bounds.second; ++bounds.first)
        if(*(bounds.first) == key) return bounds.first;

    // if we make it here, we didn't find it.
    return this->end();

} // DesignMultiSet::find_exact

template <typename Pred>
typename DesignMultiSet<Pred>::const_iterator
DesignMultiSet<Pred>::find_not_exact(
    const key_type key
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // start by bounding the search region.
    const_iterator_pair bounds(this->equal_range(key));

    // now, look at every member in that range until
    // a non-match to key is found.
    for(; bounds.first!=bounds.second; ++bounds.first)
        if(*bounds.first != key) return bounds.first;

    // if we make it here, we didn't find it.
    return this->end();

} // DesignMultiSet::find_not_exact

template <typename Pred>
typename DesignMultiSet<Pred>::size_type
DesignMultiSet<Pred>::count_non_unique(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // check for the trivial abort conditions
    if(this->size() < 2) return 0;

    // prepare to store the duplicate count.
    size_type ndups = 0;

    // prepare some iterators for use in the following loops.
    const_iterator iit, jit, e(this->end());

    // Get the predicate for repeated use.
    Pred predicate(this->key_comp());

    // Start with the first Design and check each one after it until a
    // non-duplicate Design is found.  Then start again where we left
    // off and repeat the process until the entire container has been checked.
    for(iit=this->begin(); iit!=e; iit=jit)
    {
        jit = iit;
        for(++jit; jit!=e; ++jit)
        {
            if(!predicate(*iit, *jit)) ++ndups;
            else break;
        }
    }

    // return the number of duplicates counted.
    return ndups;

} // DesignMultiSet::count_non_unique

template <typename Pred>
std::ostream&
DesignMultiSet<Pred>::stream_out(
    std::ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // check for the trivial abort conditions
    if(this->empty()) return stream;

    // prepare some iterators for use below.
    // cannot use --end() on some platforms.
    const_iterator it(this->begin());
    const_iterator e(this->end());

    // stream out each design
    for(--e; it!=e; ++it) this->stream_out(*it, stream) << '\n';

    // do the last one separately without a newline
    return this->stream_out(*e, stream);

} // DesignMultiSet::stream_out

template <typename Pred>
std::ostream&
DesignMultiSet<Pred>::stream_out(
    const key_type des,
    std::ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE

    stream.precision(12);
    const std::size_t ndv = des->GetNDV();

    if(ndv > 0)
    {
        // Print the design variable values no matter what so long as there are
        // some.  Start with the first ndv-1 each followed by a tab.
        for(std::size_t i=0; i<(ndv-1); ++i)
            stream << des->GetVariableValue(i) << '\t';

        // now put out the last design variable without a tab
        // so that if the responses don't get written, there is
        // no hanging tab.
        stream << des->GetVariableValue(ndv-1);
    }

    // only print out responses if the Design has been
    // evaluated and is well conditioned.
    if(des->IsEvaluated() && !des->IsIllconditioned())
    {
        // these must be signed because we do subtraction.
        const std::size_t nof = des->GetNOF();
        const std::size_t ncn = des->GetNCN();

        // put out each objective after a tab character
        for(std::size_t j=0; j<nof; ++j)
            stream << '\t' << des->GetObjective(j);

        // now do the same for any constraints.
        for(std::size_t j=0; j<ncn; ++j)
            stream << '\t' << des->GetConstraint(j);
    }

    return stream;

} // DesignMultiSet::stream_out

template <typename Pred>
typename DesignMultiSet<Pred>::const_iterator
DesignMultiSet<Pred>::test_for_clone(
    const key_type key
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    const_iterator clone = this->find_not_exact(key);
    if(clone != this->end()) Design::TagAsClones(*key, **clone);
    return clone;

} // DesignMultiSet::test_for_clone

template <typename Pred>
typename DesignMultiSet<Pred>::size_type
DesignMultiSet<Pred>::mark_non_unique(
    std::size_t mark
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // check for the trivial abort conditions
    if(this->size() < 2) return 0;

    // prepare to store the duplicate count.
    size_type nmarked = 0;

    // Store the iterator to the last design of interest for repeated use.
    // The last Design is the last one in the set which is --end();
    const const_iterator e(this->end());

    for(const_iterator curr(this->begin()); curr!=e;)
    {
        // mark the first of all non-unique with the false tag.
        (*curr)->ModifyAttribute(mark, false);

        // store the Design associated with curr for use below.
        Design* currDes = *curr;

        // if curr is the last one, we don't have to do anything further.
        if(++curr == e) break;

        // Otherwise, iterate all duplicates and mark them true.
        for(const_iterator last(this->upper_bound(currDes));
            curr!=last; ++curr)
        {
            (*curr)->ModifyAttribute(mark, true);
            ++nmarked;
        }
    }

    // return the number of duplicates marked as non-unique.
    return nmarked;

} // DesignMultiSet::mark_non_unique

template <typename Pred>
typename DesignMultiSet<Pred>::size_type
DesignMultiSet<Pred>::test_within_list_for_clones(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // check for the trivial abort conditions
    if(this->size() < 2) return 0;

    // prepare to store the duplicate count.
    size_type nclones = 0;

    // prepare some iterators for use in the following loops.
    const_iterator jit, e(this->end());

    // Get the predicate for repeated use.
    Pred predicate(this->key_comp());

    // Start with the first Design and check each one after it until a
    // non-duplicate Design is found.  Then start again where we left
    // off and repeat the process until the entire container has been checked.
    for(const_iterator iit(this->begin()); iit!=e; iit=jit)
    {
        jit = iit;
        for(++jit; jit!=e; ++jit)
        {
            if(!predicate(*iit, *jit))
                nclones += Design::TagAsClones(**jit, **iit);
            else break;
        }
    }

    // return the number of duplicates counted.
    return nclones;

} // DesignMultiSet::test_within_list_for_clones

template <typename Pred>
typename DesignMultiSet<Pred>::size_type
DesignMultiSet<Pred>::mark(
    const key_type key,
    std::size_t mark
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to count the number marked.
    size_type nmarked = 0;

    // start by bounding the search region.
    iterator_pair bounds(this->equal_range(key));

    // unmark all those before the range
    for(iterator it(this->begin()); it!=bounds.first; ++it)
        (*it)->ModifyAttribute(mark, false);

    // now mark all those in the range.  Also track our number marked.
    for(; bounds.first!=bounds.second; ++bounds.first)
    {
        (*bounds.first)->ModifyAttribute(mark, true);
        ++nmarked;
    }

    // now unmark all those after the range.
    const const_iterator e(this->end());
    for(; bounds.second!=e; ++bounds.second)
        (*bounds.second)->ModifyAttribute(mark, false);

    return nmarked;
}

template <typename Pred>
typename DesignMultiSet<Pred>::size_type
DesignMultiSet<Pred>::mark_not_exact(
    const key_type key,
    std::size_t mark
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to count the number marked.
    size_type nmarked = 0;

    // start by bounding the search region.
    iterator_pair bounds(this->equal_range(key));

    // unmark all those before the range
    for(iterator it(this->begin()); it!=bounds.first; ++it)
        (*it)->ModifyAttribute(mark, false);

    // now mark all those in the range that are not the same exact
    // object as key.  Also track our number marked.
    for(; bounds.first!=bounds.second; ++bounds.first)
    {
        bool mark = *bounds.first != key;
        (*bounds.first)->ModifyAttribute(mark, mark);
        if(mark) ++nmarked;
    }

    // now unmark all those after the range.
    const const_iterator e(this->end());
    for(; bounds.second!=e; ++bounds.second)
        (*bounds.second)->ModifyAttribute(mark, false);

    return nmarked;
}

template <typename Pred>
inline
void
DesignMultiSet<Pred>::pop_front(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!this->empty())
    this->base_type::erase(this->base_type::begin());

} // DesignMultiSet::pop_front

template <typename Pred>
inline
void
DesignMultiSet<Pred>::pop_back(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!this->empty())

    // cannot use --end() on some platforms.
    iterator it(this->base_type::end());
    this->base_type::erase(--it);

} // DesignMultiSet::pop_back

template <typename Pred>
inline
typename DesignMultiSet<Pred>::iterator
DesignMultiSet<Pred>::replace(
    iterator where,
    const key_type key
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(where != this->end())

    this->base_type::erase(where);
    return this->base_type::insert(key);

} // DesignMultiSet::replace

template <typename Pred>
typename DesignMultiSet<Pred>::size_type
DesignMultiSet<Pred>::erase_exacts(
    const key_type key
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store the initial size so we can easily return the number erased.
    size_type isize = this->size();

    // start by bounding the search region.
    iterator_pair bounds(this->equal_range(key));

    // now, look at every member in that range.
    // when any exact matches to key are found, erase them.
    for(; bounds.first!=bounds.second;)
    {
        if(*bounds.first == key) this->erase(bounds.first++);
        else ++bounds.first;
    }

    // return the difference in size from the beginning.
    return isize - this->size();

} // DesignMultiSet::erase_exacts

template <typename Pred>
void
DesignMultiSet<Pred>::flush(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // iterate the set and destroy all designs.
    const_iterator e(this->end());
    for(const_iterator it(this->begin()); it!=e; ++it) delete (*it);

    // now clear the set.
    this->base_type::clear();

} // DesignMultiSet::flush

template <typename Pred>
typename DesignMultiSet<Pred>::iterator
DesignMultiSet<Pred>::find_exact(
    const key_type key
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // start by bounding the search region.
    iterator_pair bounds(this->base_type::equal_range(key));

    // now, look at every member in that range until key is found.
    for(; bounds.first!=bounds.second; ++bounds.first)
        if(*(bounds.first) == key) return bounds.first;

    // if we make it here, we didn't find it.
    return this->base_type::end();

} // DesignMultiSet::find_exact

template <typename Pred>
typename DesignMultiSet<Pred>::iterator
DesignMultiSet<Pred>::find_not_exact(
    const key_type key
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // start by bounding the search region.
    iterator_pair bounds(this->base_type::equal_range(key));

    // now, look at every member in that range until
    // a non-match to key is found.
    for(; bounds.first!=bounds.second; ++bounds.first)
        if(*bounds.first != key) return bounds.first;

    // if we make it here, we didn't find it.
    return this->base_type::end();

} // DesignMultiSet::find_not_exact





/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/


template <typename Pred>
inline
DesignMultiSet<Pred>::DesignMultiSet(
    const key_compare& pred
    ) :
        base_type(pred)
{
} // DesignMultiSet::DesignMultiSet

template <typename Pred>
inline
DesignMultiSet<Pred>::DesignMultiSet(
    const my_type& copy
    ) :
        base_type(copy)
{
} // DesignMultiSet::DesignMultiSet






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

