/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignGroup.

    NOTES:

        See notes of DesignGroup.hpp.

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
 * \brief Contains the inline methods of the DesignGroup class.
 */




/*
================================================================================
Includes
================================================================================
*/




/*
================================================================================
Namespace Using Directives
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
Private Template Method Implementations
================================================================================
*/
template <typename Predicate>
DesignGroup::DVSortContainer::size_type
DesignGroup::Flush(
    Predicate pred
    )
{
    // prepare to count the removals.
    DVSortContainer::size_type nrems = 0;

    for(DVSortContainer::iterator it(this->_dvSort.begin());
        it!=this->_dvSort.end();)
    {
        if( pred(**it) )
        {
            it = this->FlushDesignRetDV(it);
            ++nrems;
        }
        else ++it;
    }
    return nrems;
}






/*
================================================================================
Inline Mutators
================================================================================
*/


inline
void
DesignGroup::SetTitle(
    const std::string& title
    )
{
    this->_title.assign(title);
}








/*
================================================================================
Inline Accessors
================================================================================
*/
inline
DesignTarget&
DesignGroup::GetDesignTarget(
    ) const
{
    return this->_target;
}

inline
const DesignGroup::DVSortContainer&
DesignGroup::GetDVSortContainer(
    ) const
{
    return this->_dvSort;
}

inline
const DesignGroup::OFSortContainer&
DesignGroup::GetOFSortContainer(
    ) const
{
    return this->_ofSort;
}

inline
const std::string&
DesignGroup::GetTitle(
    ) const
{
    return this->_title;
}






/*
================================================================================
Inline Public Methods
================================================================================
*/
template <typename Cont>
void
DesignGroup::AbsorbDesigns(
   const Cont& other
   )
{
    this->AbsorbDesigns(other.begin(), other.end());
}

template <typename It>
void
DesignGroup::AbsorbDesigns(
   It start,
   const It& end
   )
{
    for(; start!=end; ++start) this->Insert(*start);
}

inline
bool
DesignGroup::IsOFSortCurrent(
    ) const
{
    return this->SizeOF() == this->SizeDV();
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::GetSize(
    ) const
{
    return this->_dvSort.size();
}

inline
DesignGroup::DVSortContainer::const_iterator
DesignGroup::BeginDV(
    ) const
{
    return this->_dvSort.begin();
}

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::BeginDV(
    )
{
    return this->_dvSort.begin();
}

inline
DesignGroup::DVSortContainer::const_iterator
DesignGroup::EndDV(
    ) const
{
    return this->_dvSort.end();
}

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::EndDV(
    )
{
    return this->_dvSort.end();
}

inline
DesignGroup::OFSortContainer::const_iterator
DesignGroup::BeginOF(
    ) const
{
    return this->_ofSort.begin();
}

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::BeginOF(
    )
{
    return this->_ofSort.begin();
}

inline
DesignGroup::OFSortContainer::const_iterator
DesignGroup::EndOF(
    ) const
{
    return this->_ofSort.end();
}

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::EndOF(
    )
{
    return this->_ofSort.end();
}

inline
DesignGroup::OFSortContainer::size_type
DesignGroup::SizeOF(
    ) const
{
    return this->_ofSort.size();
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::SizeDV(
    ) const
{
    return this->_dvSort.size();
}

inline
bool
DesignGroup::IsEmpty(
    ) const
{
    return this->_dvSort.empty() && this->_ofSort.empty();
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushCloneDesigns(
    )
{
    return this->FlushIfTrue(Design::IsClonedPred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushEvaluatedDesigns(
    )
{
    return this->FlushIfTrue(Design::IsEvaluatedPred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushFeasibleDesigns(
    )
{
    return this->FlushIfTrue(Design::IsFeasiblePred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushIllconditionedDesigns(
    )
{
    return this->FlushIfTrue(Design::IsIllconditionedPred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushNonCloneDesigns(
    )
{
    return this->FlushIfFalse(Design::IsClonedPred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushNonEvaluatedDesigns(
    )
{
    return this->FlushIfFalse(Design::IsEvaluatedPred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushNonIllconditionedDesigns(
    )
{
    return this->FlushIfFalse(Design::IsIllconditionedPred());
}

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushNonFeasibleDesigns(
    )
{
    return this->FlushIfFalse(Design::IsFeasiblePred());
}

inline
bool
DesignGroup::AllDesignsAreFeasible(
    ) const
{
    return this->CountFeasible() == this->GetSize();
}

inline
bool
DesignGroup::ContainsDesign(
   const Design& des
   ) const
{
    return this->_dvSort.find_exact(const_cast<Design*>(&des)) !=
        this->_dvSort.end();
}

inline
void
DesignGroup::Insert(
   Design* des
   )
{
    this->InsertRetDV(des);

} // DesignGroup::Insert

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::EraseRetDV(
   DVSortContainer::iterator where
   )
{
    this->EraseFromOFContainer(this->_ofSort.find_exact(*where));
    return this->EraseFromDVContainer(where);

} // DesignGroup::EraseRetDV

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::EraseRetOF(
   OFSortContainer::iterator where
   )
{
    this->EraseFromDVContainer(this->_dvSort.find_exact(*where));
    return this->EraseFromOFContainer(where);

} // DesignGroup::EraseRetOF

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushNonUnique(
    )
{
    std::size_t nMarked = this->_dvSort.mark_non_unique();
    return nMarked > 0 ? this->FlushDesigns(DVSortContainer::MARK) : 0;

} // FlushNonUnique

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushDesigns(
    const Design::AttributeMask& attrs,
    bool hasOrNot
    )
{
    Design::AttributePred pred(attrs, hasOrNot);
    return this->Flush(pred);

} // DesignGroup::FlushDesigns

inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushDesigns(
    std::size_t attrs,
    bool hasOrNot
    )
{
    Design::AttributePred pred(attrs, hasOrNot);
    return this->Flush(pred);

} // DesignGroup::FlushDesigns

inline
void
DesignGroup::EraseFromDVContainer(
   const Design* des
   )
{
    this->EraseFromDVContainer(
        this->_dvSort.find_exact(const_cast<Design*>(des))
        );

} // DesignGroup::EraseFromDVContainer

inline
void
DesignGroup::EraseFromOFContainer(
   const Design* des
   )
{
    this->EraseFromOFContainer(
        this->_ofSort.find_exact(const_cast<Design*>(des))
        );

} // DesignGroup::EraseFromOFContainer

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::EraseFromDVContainer(
   DVSortContainer::iterator where
   )
{
    if(where != this->_dvSort.end())
        this->_dvSort.erase(where++);
    return where;

} // DesignGroup::EraseFromDVContainer

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::EraseFromOFContainer(
   OFSortContainer::iterator where
   )
{
    if(where != this->_ofSort.end())
        this->_ofSort.erase(where++);
    return where;

} // DesignGroup::EraseFromOFContainer

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::InsertIntoDVContainer(
   Design* des
   )
{
    return this->InsertIntoDVContainer(des, this->BeginDV());

} // DesignGroup::InsertIntoDVContainer

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::InsertIntoDVContainer(
   Design* des,
   DVSortContainer::iterator first
   )
{
    return this->_dvSort.insert(first, des);

} // DesignGroup::InsertIntoDVContainer

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::InsertIntoOFContainer(
   Design* des
   )
{
    return this->InsertIntoOFContainer(des, this->BeginOF());

} // DesignGroup::InsertIntoOFContainer

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::InsertIntoOFContainer(
   Design* des,
   OFSortContainer::iterator first
   )
{
    return des->IsEvaluated() ?
        this->_ofSort.insert(first, des) : this->_ofSort.end();

} // DesignGroup::InsertIntoOFContainer

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::ReplaceInDVContainer(
   DVSortContainer::iterator where,
   Design* des
   )
{
    return this->_dvSort.replace(where, des);

} // DesignGroup::ReplaceInDVContainer

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::ReplaceInOFContainer(
   OFSortContainer::iterator where,
   Design* des
   )
{
    return this->_ofSort.replace(where, des);

} // DesignGroup::ReplaceInOFContainer

template <typename BoolPredT>
inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushIfTrue(
    const BoolPredT& pred
    )
{
    return this->Flush(pred);

} // DesignGroup::FlushIfTrue

template <typename BoolPredT>
inline
DesignGroup::DVSortContainer::size_type
DesignGroup::FlushIfFalse(
    const BoolPredT& pred
    )
{
    return this->Flush(std::unary_negate<BoolPredT>(pred));

} // DesignGroup::FlushIfFalse

inline
void
DesignGroup::MergeIn(
    DesignGroup& other
    )
{
    this->CopyIn(other);
    other.ClearContainers();

} // DesignGroup::MergeIn

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::InsertRetDV(
   Design* des
   )
{
    this->InsertIntoOFContainer(des);
    return this->InsertIntoDVContainer(des);

} // DesignGroup::InsertRetDV

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::InsertRetOF(
   Design* des
   )
{
    this->InsertIntoDVContainer(des);
    return this->InsertIntoOFContainer(des);

} // DesignGroup::InsertRetOF

inline
DesignGroup::DVSortContainer::iterator
DesignGroup::ReplaceRetDV(
   DVSortContainer::iterator where,
   Design* des
   )
{
    this->EraseRetDV(where);
    return this->InsertRetDV(des);

} // DesignGroup::ReplaceRetDV

inline
DesignGroup::OFSortContainer::iterator
DesignGroup::ReplaceRetOF(
   OFSortContainer::iterator where,
   Design* des
   )
{
    this->EraseRetOF(where);
    return this->InsertRetOF(des);

} // DesignGroup::ReplaceRetOF

/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/


inline
void
DesignGroup::ClearOFContainer(
    )
{
    this->_ofSort.clear();
}

inline
void
DesignGroup::ClearContainers(
    )
{
    this->ClearDVContainer();
    this->ClearOFContainer();
}






/*
================================================================================
Inline Private Methods
================================================================================
*/

inline
void
DesignGroup::ClearDVContainer()
{
    this->_dvSort.clear();
}







/*
================================================================================
Inline Structors
================================================================================
*/
template<typename DesCont>
DesignGroup::DesignGroup(
    DesignTarget& target,
    const DesCont& designs
    ) :
        _target(target),
        _dvSort(),
        _ofSort()
{
    typename DesCont::const_iterator it(designs.begin());
    const typename DesCont::const_iterator e(designs.end());
    for(; it!=e; ++it) this->Insert(*it);
}




/*
================================================================================
File Scope Utility Class Inline Methods
================================================================================
*/







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
