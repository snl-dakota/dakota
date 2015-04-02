/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTSH:

        Implementation of class DesignGroup.

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
 * \brief Contains the implementation of the DesignGroup class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignStatistician.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace eddy::utilities;







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








/*
================================================================================
Mutators
================================================================================
*/








/*
================================================================================
Accessors
================================================================================
*/






/*
================================================================================
Public Methods
================================================================================
*/

DoubleExtremes
DesignGroup::ComputeDVExtremes(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DesignStatistician::GetDesignVariableExtremes(
        this->GetDVSortContainer()
        );
}

DoubleExtremes
DesignGroup::ComputeOFExtremes(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DesignStatistician::GetObjectiveFunctionExtremes(
        this->GetOFSortContainer()
        );
}

bool
DesignGroup::AnyDesignsAreFeasible(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // use a DesignStatistician to see if we can find a feasible Design
    return DesignStatistician::FindFeasible(this->_dvSort) !=
           this->_dvSort.end();

} // DesignGroup::AnyDesignsAreFeasible



DesignGroup::DVSortContainer::iterator
DesignGroup::FlushDesignRetDV(
    DVSortContainer::iterator it
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(it == this->EndDV()) return this->EndDV();

    Design* des = *it;
    DVSortContainer::iterator ret(this->EraseRetDV(it));
    this->_target.TakeDesign(des);
    return ret;

} // DesignGroup::FlushDesignRetDV

DesignGroup::OFSortContainer::iterator
DesignGroup::FlushDesignRetOF(
    OFSortContainer::iterator it
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(it == this->EndOF()) return this->EndOF();

    Design* des = *it;
    OFSortContainer::iterator ret(this->EraseRetOF(it));
    this->_target.TakeDesign(des);
    return ret;

} // DesignGroup::FlushDesignRetOF


std::size_t
DesignGroup::CountFeasible(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    std::size_t ret = 0;

    // go through and count all evaluated, feasible Designs
    for(DVSortContainer::const_iterator it(this->_dvSort.begin());
        it!=this->_dvSort.end(); ++it) if((*it)->IsFeasible()) ++ret;

    return ret;
}

void
DesignGroup::FlushAll(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // send all Designs to the target.
    if(!this->IsEmpty())
    {
        while(!this->_dvSort.empty())
        {
            this->_target.TakeDesign(this->_dvSort.front());
            this->_dvSort.pop_front();
        }

        this->ClearContainers();
    }
}

bool
DesignGroup::SynchronizeOFAndDVContainers(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // if the objective sort is current, we needn't do anything.
    if(this->IsOFSortCurrent()) return true;

    DesignGroup* ucthis = const_cast<DesignGroup*>(this);
    ucthis->ClearOFContainer();
    const DVSortContainer::const_iterator e(this->_dvSort.end());

    for(DVSortContainer::const_iterator it(this->_dvSort.begin()); it!=e; ++it)
        ucthis->InsertIntoOFContainer(*it);

    return this->IsOFSortCurrent();
}

const DesignGroup&
DesignGroup::operator = (
   const DesignGroup& right
   )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&this->_target == &right._target);

    if(this== &right) return *this;

    this->_dvSort.clear();
    this->_ofSort.clear();

    this->_dvSort.insert(right._dvSort.begin(), right._dvSort.end());
    this->_ofSort.insert(right._ofSort.begin(), right._ofSort.end());

    return *this;
}

void
DesignGroup::CopyIn(
    const DesignGroup& other
    )
{
    EDDY_FUNC_DEBUGSCOPE

    EDDY_ASSERT(&this->_target == &other.GetDesignTarget());

    // do the DVSortContainer first.
    DVSortContainer::const_iterator dvit(other.BeginDV());
    const DVSortContainer::const_iterator dve(other.EndDV());

    DVSortContainer::iterator dvloc(this->BeginDV());
    for(; dvit!=dve; ++dvit) dvloc = this->InsertIntoDVContainer(*dvit, dvloc);

    // now do the OFSortContainer.
    OFSortContainer::const_iterator ofit(other.BeginOF());
    const OFSortContainer::const_iterator ofe(other.EndOF());

    OFSortContainer::iterator ofloc(this->BeginOF());
    for(; ofit!=ofe; ++ofit) ofloc = this->InsertIntoOFContainer(*ofit, ofloc);

} // DesignGroup::CopyIn

bool
DesignGroup::Erase(
   const Design* des
   )
{
    EDDY_FUNC_DEBUGSCOPE
    DesignDVSortSet::iterator where(
        this->_dvSort.find_exact(const_cast<Design*>(des))
        );

    bool ret = (where != this->_dvSort.end());
    if(ret) this->EraseRetDV(where);
    return ret;

} // DesignGroup::Erase

/*
================================================================================
Subclass Visible Methods
================================================================================
*/





/*
================================================================================
Subclass Overridable Methods
================================================================================
*/



/*
================================================================================
Private Methods
================================================================================
*/





/*
================================================================================
Structors
================================================================================
*/
DesignGroup::DesignGroup(
    DesignTarget& target
    ) :
        _dvSort(),
        _ofSort(),
        _target(target),
        _title()
{
    EDDY_FUNC_DEBUGSCOPE

} // DesignGroup::DesignGroup

DesignGroup::DesignGroup(
    const DesignGroup& copy
    ) :
        _dvSort(copy._dvSort),
        _ofSort(copy._ofSort),
        _target(copy._target),
        _title(copy._title)
{
    EDDY_FUNC_DEBUGSCOPE

} // DesignGroup::DesignGroup

DesignGroup::~DesignGroup(
    )
{
    EDDY_FUNC_DEBUGSCOPE
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
