/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class UnconstrainedStatistician.

    NOTES:

        See notes of UnconstrainedStatistician.hpp.

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
 * \brief Contains the inline methods of the UnconstrainedStatistician class.
 */


/*
================================================================================
Includes
================================================================================
*/
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {







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
template <typename DesignContainer, typename PFunc>
DesignDoubleMap
UnconstrainedStatistician::ApplyPenalty(
    const DesignContainer& designs,
    PFunc penalizer
    )
{
    // prepare our return object.
    DesignDoubleMap ret(designs.size());

    const typename DesignContainer::const_iterator e(designs.end());
    for(typename DesignContainer::const_iterator it(designs.begin());
        it!=e; ++it) ret.AddValue(*it, penalizer(*it));

    return ret;
}

template <typename PFunc>
DesignDoubleMap
UnconstrainedStatistician::ApplyPenalty(
    const DesignGroup& designs,
    PFunc penalizer
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare our return object.
    DesignDoubleMap ret(designs.SizeDV());

    const typename DesignDVSortSet::const_iterator e(designs.EndDV());
    for(typename DesignDVSortSet::const_iterator it(designs.BeginDV());
        it!=e; ++it) ret.AddValue(*it, penalizer(*it));

    return ret;
}

template <typename PFunc>
DesignDoubleMap
UnconstrainedStatistician::ApplyPenalty(
    const DesignGroupVector& designs,
    PFunc penalizer
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare our return object.
    DesignDoubleMap ret(designs.GetTotalDesignCount());

    for(typename DesignGroupVector::const_iterator it(designs.begin());
        it!=designs.end(); ++it)
            for(typename DesignDVSortSet::const_iterator jt((*it)->BeginDV());
                jt!=(*it)->EndDV(); ++jt)
                    ret.AddValue(*jt, penalizer(*jt));

    return ret;
}

template <typename DesignContainer, typename PFunc>
const DesignDoubleMap&
UnconstrainedStatistician::AddPenalty(
    const DesignContainer& designs,
    PFunc penalizer,
    DesignDoubleMap& to
    )
{
    // go through and do each Design in "group"
    const typename DesignContainer::const_iterator e(designs.end());

    for(typename DesignContainer::const_iterator it(designs.begin());
        it!=e; ++it) to.AddToValue(*it, penalizer(*it));

    return to;
}

template <typename PFunc>
const DesignDoubleMap&
UnconstrainedStatistician::AddPenalty(
    const DesignGroup& designs,
    PFunc penalizer,
    DesignDoubleMap& to
    )
{
    // go through and do each Design in "group"
    const typename DesignDVSortSet::const_iterator e(designs.EndDV());

    for(typename DesignDVSortSet::const_iterator it(designs.BeginDV());
        it!=e; ++it) to.AddToValue(*it, penalizer(*it));

    return to;
}

template <typename PFunc>
const DesignDoubleMap&
UnconstrainedStatistician::AddPenalty(
    const DesignGroupVector& designs,
    PFunc penalizer,
    DesignDoubleMap& to
    )
{
    for(typename DesignGroupVector::const_iterator it(designs.begin());
        it!=designs.end(); ++it)
    {
        // go through and do each Design in "group"
        const typename DesignDVSortSet::const_iterator e((*it)->EndDV());

        for(typename DesignDVSortSet::const_iterator jt((*it)->BeginDV());
            jt!=e; ++jt) to.AddToValue(*jt, penalizer(*jt));
    }

    return to;
}

template <typename DesignContainer>
const DesignDoubleMap&
UnconstrainedStatistician::AddOutOfBoundsPenalties(
    const DesignContainer& designs,
    DesignDoubleMap& to
    )
{
    return AddPenalty<DesignContainer, BoundsPenalizer>(
        designs, BoundsPenalizer(), to
        );

} // UnconstrainedStatistician::AddOutOfBoundsPenalties

template <typename DesignContainer>
DesignDoubleMap
UnconstrainedStatistician::ApplyExteriorPenalty(
    const DesignContainer& designs,
    double multiplier
    )
{
    return ApplyPenalty<DesignContainer, ExteriorPenalizer>(
        designs, ExteriorPenalizer(multiplier)
        );

} // UnconstrainedStatistician::ApplyExteriorPenalty

template <typename DesignContainer>
DesignDoubleMap
UnconstrainedStatistician::ApplyQuadraticPenalty(
    const DesignContainer& designs,
    double multiplier
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ApplyExteriorPenalty<DesignContainer>(designs, multiplier);
}








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








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
