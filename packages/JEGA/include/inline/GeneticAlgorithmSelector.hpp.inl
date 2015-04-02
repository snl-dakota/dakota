/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithmSelector.

    NOTES:

        See notes of GeneticAlgorithmSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 21 15:26:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the GeneticAlgorithmSelector class.
 */


/*
================================================================================
Includes
================================================================================
*/
#include <FitnessRecord.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JU = JEGA::Utilities;

namespace JEGA {
    namespace Algorithms {





inline
bool
GeneticAlgorithmSelector::FitnessPred::operator ()(
    const JU::Design* des1,
    const JU::Design* des2
    ) const
{
    return this->_ftns.GetFitness(*des1) >
           this->_ftns.GetFitness(*des2);
}

inline
bool
GeneticAlgorithmSelector::FitnessPred::operator ()(
    const DesignGroupInfo& d1Info,
    const DesignGroupInfo& d2Info
    ) const
{
    return this->operator ()(*d1Info.second, *d2Info.second);
}


inline
GeneticAlgorithmSelector::FitnessPred::FitnessPred(
    const FitnessRecord& ftns
    ) :
        _ftns(ftns)
{}


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








/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/

template <typename CompT>
void
GeneticAlgorithmSelector::SelectNBest(
    JEGA::Utilities::DesignGroupVector& from,
    JEGA::Utilities::DesignGroup& into,
    std::size_t n,
    const FitnessRecord& fitnesses,
    CompT comp
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if n is zero, we don't need to do anything at all.
    if(n == 0) return;

    std::size_t desCt = from.GetTotalDesignCount();

    // check for trivial abort conditions.
    if(desCt == 0) return;

    std::size_t fsze = from.size();

    if(desCt <= n)
    {
        for(std::size_t i=0; i<fsze; ++i)
        {
            const JU::DesignOFSortSet::const_iterator eof(from[i]->EndOF());
            for(JU::DesignOFSortSet::iterator dt(from[i]->BeginOF());
                dt!=eof; ++dt) into.Insert(*dt);
            from[i]->ClearContainers();
        }
        return;
    }

    std::vector<DesignGroupInfo> allDes;
    allDes.reserve(desCt);

    for(std::size_t i=0; i<fsze; ++i)
    {
        const JU::DesignOFSortSet::const_iterator eof(from[i]->EndOF());
        for(JU::DesignOFSortSet::iterator dt(from[i]->BeginOF());
            dt!=eof; ++dt) allDes.push_back(DesignGroupInfo(i, dt));
    }

    std::sort(allDes.begin(), allDes.end(), comp);

    // now choose from them as long as there are some and we don't have enough.
    for(std::size_t i=0; i<n && i<desCt; ++i)
    {
        into.Insert(*allDes[i].second);
        from[allDes[i].first]->EraseRetOF(allDes[i].second);
    }
}

template <typename CompT>
JU::DesignOFSortSet
GeneticAlgorithmSelector::SelectNBest(
    JU::DesignGroupVector& from,
    std::size_t n,
    CompT comp
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if n is zero, we don't need to do anything at all.
    if(n == 0) return JU::DesignOFSortSet();

    std::size_t desCt = from.GetTotalDesignCount();

    // check for trivial abort conditions.
    if(desCt == 0) return JU::DesignOFSortSet();

    JU::DesignOFSortSet ret;

    std::size_t fsze = from.size();

    if(desCt <= n)
    {
        for(std::size_t i=0; i<fsze; ++i)
        {
            const JU::DesignOFSortSet::const_iterator eof(from[i]->EndOF());
            for(JU::DesignOFSortSet::iterator dt(from[i]->BeginOF());
                dt!=eof; ++dt) ret.insert(*dt);
        }
        return ret;
    }

    std::vector<JU::Design*> allDes;
    allDes.reserve(desCt);

    for(std::size_t i=0; i<fsze; ++i)
    {
        const JU::DesignOFSortSet::const_iterator eof(from[i]->EndOF());
        for(JU::DesignOFSortSet::iterator dt(from[i]->BeginOF());
            dt!=eof; ++dt) allDes.push_back(*dt);
    }

    std::sort(allDes.begin(), allDes.end(), comp);

    // now choose from them as long as there are some and we don't have enough.
    for(std::size_t i=0; i<n && i<desCt; ++i) ret.insert(allDes[i]);

    return ret;
}







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
    } // namespace Algorithms
} // namespace JEGA
