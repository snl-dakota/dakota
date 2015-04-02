/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmNichePressureApplicator.

    NOTES:

        See notes of GeneticAlgorithmNichePressureApplicator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Jan 05 10:13:06 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the
 *        GeneticAlgorithmNichePressureApplicator class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <utilities/include/extremes.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/numeric_limits.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;
using namespace eddy::utilities;








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {





/*
================================================================================
Static Member Data Definitions
================================================================================
*/
const bool GeneticAlgorithmNichePressureApplicator::DEFAULT_CACHE_FLAG(true);







/*
================================================================================
Mutators
================================================================================
*/

void
GeneticAlgorithmNichePressureApplicator::SetCacheDesigns(
    bool cache
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_cacheDesigns = cache;

    if(!this->_cacheDesigns)
    {
        JEGAIFLOG_CF_II(!this->_desBuffer.empty(), this->GetLogger(),
            lverbose(), this,
            ostream_entry(lverbose(), this->GetName() + ": Flushing ")
            << this->_desBuffer.size() << " cached designs in response to a "
            "setting of the cache flag to false."
        )

        this->GetDesignTarget().TakeDesigns(this->_desBuffer);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Niched design caching now set to ")
            << (cache ? "true" : "false") << "."
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

void
GeneticAlgorithmNichePressureApplicator::ReAssimilateBufferedDesigns(
    DesignGroup& intoGroup
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // we put them all in and let the selector sort them out.
    intoGroup.AbsorbDesigns(this->_desBuffer);

    // Get them out of here since the desBuffer and intoGroup must be
    // mutually exclusive.
    this->_desBuffer.clear();
}

bool
GeneticAlgorithmNichePressureApplicator::BufferDesign(
    const Design* des
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this->_cacheDesigns) this->_desBuffer.insert(const_cast<Design*>(des));
    return this->_cacheDesigns;
}

DesignOFSortSet
GeneticAlgorithmNichePressureApplicator::GetBest(
    const DesignOFSortSet& of,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    DesignOFSortSet ret;
    double bestFit = eddy::utilities::numeric_limits<double>::smallest();

    for(DesignOFSortSet::const_iterator it(of.begin()); it!=of.end(); ++it)
    {
        double currFit = fitnesses.GetFitness(**it);
        if(currFit == -DBL_MAX) continue;

        if(currFit > bestFit)
        {
            ret.clear();
            ret.insert(*it);
            bestFit = currFit;
        }
        else if(currFit == bestFit) ret.insert(*it);
    }

    return ret;
}






/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
void
GeneticAlgorithmNichePressureApplicator::PreSelection(
    JEGA::Utilities::DesignGroup&
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(
            ldebug(), this->GetName() +
            ": Using default pre-seletion operation."
            )
        )
}

bool
GeneticAlgorithmNichePressureApplicator::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetBooleanFromDB(
        db, "method.jega.cache_niched_designs", this->_cacheDesigns
        );

    // If we did not find the cache flag, warn about it and use the default
    // value.  Note that if !success, then _cacheDesigns has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The cache designs flag "
            "value was not found in the parameter database.  Using the current "
            "value of ") << (this->_cacheDesigns ? "true" : "false")
        )

    this->SetCacheDesigns(this->_cacheDesigns);

    return true;
}

string
GeneticAlgorithmNichePressureApplicator::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Niche Pressure Applicator";
}

bool
GeneticAlgorithmNichePressureApplicator::Finalize(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // There very well may be optimal solutions in the design buffer.  In fact
    // it is more likely than not.  So don't put them in the target and run the
    // risk that the target will simply dispose of them.  Instead, merge them
    // back into the GA's population so that they can be reclaimed and/or
    // filtered out.
    this->GetAlgorithm().GetPopulation().AbsorbDesigns(this->_desBuffer);
    this->_desBuffer.clear();
    return this->GeneticAlgorithmOperator::Finalize();
}






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


GeneticAlgorithmNichePressureApplicator::
GeneticAlgorithmNichePressureApplicator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _cacheDesigns(DEFAULT_CACHE_FLAG),
        _desBuffer()
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmNichePressureApplicator::
GeneticAlgorithmNichePressureApplicator(
    const GeneticAlgorithmNichePressureApplicator& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _cacheDesigns(copy._cacheDesigns),
        _desBuffer(copy._desBuffer)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmNichePressureApplicator::
GeneticAlgorithmNichePressureApplicator(
    const GeneticAlgorithmNichePressureApplicator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm),
        _cacheDesigns(copy._cacheDesigns)
{
    EDDY_FUNC_DEBUGSCOPE
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA

