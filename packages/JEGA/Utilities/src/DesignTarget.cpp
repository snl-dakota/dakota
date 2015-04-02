/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DesignTarget.

    NOTES:

        See notes of DesignTarget.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Dec 20 08:11:48 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DesignTarget class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#ifdef JEGA_THREADSAFE
#include <threads/include/mutex.hpp>
#include <threads/include/mutex_lock.hpp>
#endif

#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/RegionOfSpace.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
JEGA_IF_THREADSAFE(using namespace eddy::threads;)
using namespace JEGA::Logging;






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {



/*
================================================================================
Nested Utility Class Implementations
================================================================================
*/
#ifdef JEGA_THREADSAFE

/// A class housing all mutexes used by the DesignTarget.
/**
 * This gets removed if JEGA_THREADSAFE is not defined.
 */
class DesignTarget::Mutexes
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// A mutext to protect the collection of discards.
        mutable mutex _discardMutex;

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Default constructs a Mutexes object which default constructs
         *        all mutexes.
         */
        Mutexes(
            ) :
                _discardMutex(PTHREAD_MUTEX_RECURSIVE)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // class DesignTarget::Mutexes

#endif // JEGA_THREADSAFE

/*
================================================================================
Static Member Data Definitions
================================================================================
*/

const std::size_t DesignTarget::DEFAULT_MAX_GUFF_SIZE = 1000;




/*
================================================================================
Mutators
================================================================================
*/

void
DesignTarget::SetTrackDiscards(
    bool use
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_trackDiscards = use;

    JEGALOG_II_G(lverbose(), this,
        ostream_entry(
            lverbose(), "Design Target: The track discards flag is now set to "
            // The compiler on sass9000 doesn't like boolalpha.
            //) << std::boolalpha << this->_trackDiscards
            ) << (this->_trackDiscards ? "true" : "false")
        )
}

void
DesignTarget::SetMaxGuffSize(
    std::size_t mgs
    )
{
    EDDY_FUNC_DEBUGSCOPE

    EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
    this->_maxGuffSize = mgs;
    this->_guff.reserve(this->_maxGuffSize);

    // do lazy size decrease.  Don't trim the end here.  Just wait for it to
    // fall below the desired number.

    JEGALOG_II_G(lverbose(), this,
        ostream_entry(
            lverbose(), "Design Target: The maximum Guff size is now set to "
            ) << this->_maxGuffSize << '.'
        )
}









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
const DesignDVSortSet&
DesignTarget::CheckoutDiscards(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_IF_THREADSAFE(this->_mutexes->_discardMutex.lock();)
    return *this->_discards;
}

void
DesignTarget::CheckinDiscards(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_IF_THREADSAFE(this->_mutexes->_discardMutex.unlock();)
}

Design*
DesignTarget::GetNewDesign(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    Design* ret = 0x0;
    {
        EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
        if(this->_guff.empty())
            return new Design(const_cast<DesignTarget&>(*this));
        ret = this->_guff.back();
        const_cast<DesignTarget*>(this)->_guff.pop_back();
    }
    ret->ResetID();
    return ret;
}

Design*
DesignTarget::GetNewDesign(
    const Design& copy
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    Design* ret = 0x0;
    {
        EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
        if(this->_guff.empty()) return new Design(copy);
        ret = this->_guff.back();
        const_cast<DesignTarget*>(this)->_guff.pop_back();
    }
    ret->operator =(copy);
    ret->ResetID();
    return ret;
}

bool
DesignTarget::CheckFeasibility(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    this->CheckSideConstraints(des);
    this->CheckNonSideConstraints(des);
    return des.IsFeasible();
}

bool
DesignTarget::CheckSideConstraints(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // Iterating through the design variables and look for out-of-bounds values.
    for(DesignVariableInfoVector::const_iterator dit(this->_dvInfos.begin());
        dit!=this->_dvInfos.end(); ++dit)
    {
        if(!(*dit)->IsRepInBounds((*dit)->WhichDoubleRep(des)))
        {
            des.SetSatisfiesBounds(false);
            return false;
        }
    }

    // if we make it here, we satisfied them all.
    des.SetSatisfiesBounds(true);
    return true;
}

bool
DesignTarget::CheckNonSideConstraints(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // iterate through the constraint info list and check for violations.
    for(ConstraintInfoVector::const_iterator cit(this->_cnInfos.begin());
        cit!=this->_cnInfos.end(); ++cit)
    {
        if((*cit)->GetViolationAmount(des) != 0.0)
        {
            des.SetSatisfiesConstraints(false);
            return false;
        }
    }

    // if we make it here, we satisfied them all.
    des.SetSatisfiesConstraints(true);
    return true;
}

void
DesignTarget::TakeDesign(
    Design* des
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(des != 0x0);
    EDDY_ASSERT(&des->GetDesignTarget() == this);

    if(this->_trackDiscards && des->IsEvaluated())
    {
        EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
        this->_discards->insert(des);
    }
    else
    {
        {
            EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
            if(this->_guff.size() < this->_maxGuffSize)
            {
                // Must clear prior to unlocking mutex b/c otherwise the design
                // could be pulled out and used prior to clearing, then get
                // cleared, etc.
                des->Dispose();
                this->_guff.push_back(des);
                return;
            }
        }

        // if we make it here, we're not keeping the design at all.  Delete it.
        delete des;
    }
}

bool
DesignTarget::ReclaimDesign(
    const Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
    DesignDVSortSet::iterator it(
        this->_discards->find_exact(const_cast<Design*>(&des))
        );

    if(it == this->_discards->end()) return false;
    this->_discards->erase(it);
    return true;
}

void
DesignTarget::RecordAllConstraintViolations(
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    for(ConstraintInfoVector::const_iterator it(this->_cnInfos.begin());
         it!=this->_cnInfos.end(); ++it)
            (*it)->RecordViolation(des);
}

bool
DesignTarget::AddDesignVariableInfo(
    DesignVariableInfo& info
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&info.GetDesignTarget() == this);
    EDDY_ASSERT(this->_discards->empty());

    if(&info.GetDesignTarget() != this) return false;

    info.SetNumber(this->_dvInfos.size());
    this->_dvInfos.push_back(&info);

    JEGALOG_II_G(lverbose(), this,
        ostream_entry(lverbose(), "Design Target: Design variable " +
            info.GetLabel() + " added.  ") << this->_dvInfos.size()
            << " design variables now in target."
            )

    EDDY_SCOPEDLOCK(l2, this->_mutexes->_discardMutex)
    this->_discards->flush();
    this->FlushTheGuff();
    return true;
}

bool
DesignTarget::AddConstraintInfo(
    ConstraintInfo& info
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&info.GetDesignTarget() == this);
    EDDY_ASSERT(this->_discards->empty());

    if(&info.GetDesignTarget() != this) return false;

    info.SetNumber(this->_cnInfos.size());
    this->_cnInfos.push_back(&info);

    JEGALOG_II_G(lverbose(), this,
        ostream_entry(lverbose(), "Design Target: Constraint " +
            info.GetLabel()) << '(' << info.GetEquation() << ") added.  "
            << this->_cnInfos.size() << " constraints now in target."
            )

    EDDY_SCOPEDLOCK(l2, this->_mutexes->_discardMutex)
    this->_discards->flush();
    this->FlushTheGuff();
    return true;
}

bool
DesignTarget::AddObjectiveFunctionInfo(
    ObjectiveFunctionInfo& info
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&info.GetDesignTarget() == this);
    EDDY_ASSERT(this->_discards->empty());

    if(&info.GetDesignTarget() != this) return false;

    info.SetNumber(this->_ofInfos.size());
    this->_ofInfos.push_back(&info);

    JEGALOG_II_G(lverbose(), this,
        ostream_entry(lverbose(), "Design Target: Objective " +
            info.GetLabel() + " added.  ") << this->_ofInfos.size()
            << " objectives now in target."
            )

    EDDY_SCOPEDLOCK(l2, this->_mutexes->_discardMutex)
    this->_discards->flush();
    this->FlushTheGuff();
    return true;
}

RegionOfSpace
DesignTarget::GetDesignSpace(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    RegionOfSpace ret(this->_dvInfos.size());
    for(DesignVariableInfoVector::const_iterator it(this->_dvInfos.begin());
        it!=this->_dvInfos.end(); ++it)
            ret.SetLimits(
                (*it)->GetNumber(),
                (*it)->GetMinDoubleRep(),
                (*it)->GetMaxDoubleRep()
                );

    return ret;
}


/*
================================================================================
Private Methods
================================================================================
*/

void
DesignTarget::FlushTheGuff(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_SCOPEDLOCK(l, this->_mutexes->_discardMutex)
    for(size_t i=0; i<this->_guff.size(); ++i) delete this->_guff[i];
    this->_guff.clear();
}







/*
================================================================================
Structors
================================================================================
*/
DesignTarget::DesignTarget(
    ) :
        _trackDiscards(true),
        _discards(new DesignDVSortSet()),
        _dvInfos(),
        _ofInfos(),
        _cnInfos(),
        _guff(),
        _maxGuffSize(DEFAULT_MAX_GUFF_SIZE) JEGA_COMMA_IF_THREADSAFE
        JEGA_IF_THREADSAFE(_mutexes(new Mutexes()))
{
    EDDY_FUNC_DEBUGSCOPE
}

DesignTarget::~DesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_THREADSAFE(this->_mutexes->_discardMutex.lock();)
    this->_discards->flush();
    this->FlushTheGuff();
    JEGA_IF_THREADSAFE(this->_mutexes->_discardMutex.unlock();)

    DesignVariableInfoVector::iterator dit(this->_dvInfos.begin());
    for(; dit!=this->_dvInfos.end(); ++dit) delete *dit;

    ConstraintInfoVector::iterator cit(this->_cnInfos.begin());
    for(; cit!=this->_cnInfos.end(); ++cit) delete *cit;

    ObjectiveFunctionInfoVector::iterator oit(this->_ofInfos.begin());
    for(; oit!=this->_ofInfos.end(); ++oit) delete *oit;

    JEGA_IF_THREADSAFE(delete this->_mutexes;)
    delete this->_discards;
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

