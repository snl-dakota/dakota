/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DistanceNichingPostProcessor.

    NOTES:

        See notes of DistanceNichingPostProcessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Sep 05 14:31:05 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DistanceNichingPostProcessor
 *        class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <limits>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../MOGA/include/PostProcessors/DistanceNichingPostProcessor.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
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
const double DistanceNichingPostProcessor::DEFAULT_DIST_PCT(0.01);








/*
================================================================================
Mutators
================================================================================
*/
void
DistanceNichingPostProcessor::SetDistancePercentages(
    const JEGA::DoubleVector& pcts
    )
{
    EDDY_FUNC_DEBUGSCOPE

    size_t nof = GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II(nof < pcts.size(), GetLogger(), lquiet(), this,
        text_entry(lquiet(),
            GetName() + ": Received more percentages than there are objective "
            "functions.  Extras will be ignored.")
        )

    JEGAIFLOG_CF_II(nof > pcts.size() && pcts.size() > 1, GetLogger(), lquiet(),
        this, ostream_entry(lquiet(),
            GetName() + ": Received fewer percentages (") << pcts.size()
            << ") than there are objective functions (" << nof << ").  "
            "Using default value of " << DEFAULT_DIST_PCT << " to fill in."
        )

    JEGAIFLOG_CF_II(nof > pcts.size() && pcts.size() == 1, GetLogger(), lquiet(),
        this, ostream_entry(lquiet(),
            GetName() + ": Received a single distance percentage for a ")
            << nof << " objective function problem.  Using the supplied value "
            "of " << pcts[0] << " for all objectives."
        )

    _distPcts = pcts;

    double fill_val =
        (_distPcts.size() == 1) ? _distPcts[0] : DEFAULT_DIST_PCT;

    if(nof > _distPcts.size())
        _distPcts.resize(
            static_cast<JEGA::DoubleVector::size_type>(nof), fill_val
            );

    // now go through and set each one individually so that they can be checked
    // for legitimacy.
    for(JEGA::DoubleVector::size_type i=0; i<nof; ++i)
        this->SetDistancePercentage(i, _distPcts[i]);
}

void
DistanceNichingPostProcessor::SetDistancePercentages(
    double pct
    )
{
    EDDY_FUNC_DEBUGSCOPE
    SetDistancePercentages(DoubleVector(GetDesignTarget().GetNOF(), pct));
}

void
DistanceNichingPostProcessor::SetDistancePercentage(
    size_t of,
    double pct
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        static const double minPct = numeric_limits<double>::min();
        )

    const DesignTarget& target = GetDesignTarget();
    size_t nof = target.GetNOF();
    JEGA::DoubleVector::size_type dvsof =
        static_cast<JEGA::DoubleVector::size_type>(of);

    // make sure we have enough locations in the percentages vector.
    _distPcts.resize(
        static_cast<JEGA::DoubleVector::size_type>(nof), DEFAULT_DIST_PCT
        );

    // now verify the supplied objective function index.
    JEGAIFLOG_CF_II_F(of >= nof, GetLogger(), this,
        ostream_entry(lfatal(),
            GetName() + ": Request to change objective with index #") << of
            << ".  Valid indices are 0 through " << (nof-1) << "."
        )

    // now verify the supplied value.
    JEGAIFLOG_CF_II(pct < 0.0, GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            GetName() + ": Distance percentages must be at least ") << minPct
            << " Supplied value of " << pct << " for objective \""
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" will be replaced by the minimum."
        )

    JEGAIFLOG_CF_II(pct > 1.0, GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            GetName() + ": Distance percentages cannot exceed 100%.  Supplied "
            "value of ") << pct << " for objective \""
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" will be replaced by 100%."
        )

    pct = Math::Max(0.0, Math::Min(pct, 1.0));

    _distPcts[dvsof] = pct;

    JEGALOG_II(GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            GetName() + ": Distance for objective \"")
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" now = " << pct << "."
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


const string&
DistanceNichingPostProcessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("distance_postprocessor");
    return ret;
}

const string&
DistanceNichingPostProcessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        ""
        );
    return ret;
}

GeneticAlgorithmOperator*
DistanceNichingPostProcessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new DistanceNichingPostProcessor(algorithm);
}







/*
================================================================================
Subclass Visible Methods
================================================================================
*/

JEGA::DoubleVector
DistanceNichingPostProcessor::ComputeCutoffDistances(
    const DoubleExtremes& objExtremes
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // the cutoff distance is a percentage of the range of the objective
    // considering only the non-dominated designs.
    size_t nof = GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II_F(nof != objExtremes.size(), GetLogger(), this,
        ostream_entry(lfatal(), GetName() + ": Extremes contain "
            "record of ") << objExtremes.size() << " objectives for an "
            << nof << " objective problem."
        )

    // Prepare a vector for return.
    JEGA::DoubleVector ret(nof);

    for(DoubleExtremes::size_type i=0; i<nof; ++i)
        ret[i] = Math::Abs(
            this->GetDistancePercentage(i) * objExtremes.get_range(i)
            );

    // return the square route of the sum of squares.
    return ret;
}

double
DistanceNichingPostProcessor::ComputeObjectiveDistance(
    const Design& des1,
    const Design& des2,
    size_t of
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::Abs(des1.GetObjective(of) - des2.GetObjective(of));
}





/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
string
DistanceNichingPostProcessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DistanceNichingPostProcessor::Name();
}

string
DistanceNichingPostProcessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DistanceNichingPostProcessor::Description();
}

GeneticAlgorithmOperator*
DistanceNichingPostProcessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new DistanceNichingPostProcessor(*this, algorithm);
}


void
DistanceNichingPostProcessor::PostProcess(
    DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(GetLogger(), ldebug(), this, text_entry(ldebug(),
        GetName() + ": post processing."))

    // we will need the number of objectives for a few things here.
    size_t nof = GetDesignTarget().GetNOF();

    // If the group is empty, we needn't go any further.
    if(group.IsEmpty()) return;

    // Sychronize the lists just in case.
    group.SynchronizeOFAndDVContainers();

    // Store our candidate designs for use below and so we don't screw things
    // up by operating directly on the group.
    DesignOFSortSet designs(group.GetOFSortContainer());

    // Now continue by extracting the overall extremes
    DoubleExtremes objExtremes(
        DesignStatistician::GetObjectiveFunctionExtremes(designs)
        );

    // We will figure which designs are too close by normalized
    // distance from the current design.  Fortunately, we are using
    // the same cutoff distance for all designs and so we can compute that
    // now for all designs.
    JEGA::DoubleVector dists(ComputeCutoffDistances(objExtremes));

    // we will need the target below.
    DesignTarget& target = this->GetDesignTarget();

    // now, we start with the first design and search the range for all
    // those that are too close.  We never extract an extreme Design.

    // prepare to output the number of designs cut out.
    JEGA_LOGGING_IF_ON(DesignOFSortSet::size_type prevSize = designs.size();)

    // We can bound our search range by noting that once the first objective
    // distance is too big, we needn't look any further.  That is true because
    // of the sorting of the DesignOFSortSet.
    for(DesignOFSortSet::iterator curr(designs.begin());
        curr!=designs.end(); ++curr)
    {
        // Iterate all designs beyond curr until we are done and see if we keep
        // or discard them.
        DesignOFSortSet::iterator next(curr);
        for(++next; next!=designs.end();)
        {
            double obj0Dist = ComputeObjectiveDistance(**curr, **next, 0);

            // If the distance at obj0 is large enough, we can get out of this
            // inner loop and move onto the next "curr".
            if(obj0Dist > dists.at(0)) break;

            // if next is an extreme design, we keep it no matter what.
            if(MultiObjectiveStatistician::IsExtremeDesign(
                **next, objExtremes
                )){ ++next; continue; }

            // prepare to store whether or not we will be keeping next.
            bool keep = false;

            // We need to see if the distances are all too small
            for(size_t of=1; of<nof; ++of)
                // if the distance at this objective is large enough, we will
                // be keeping the design so we can continue on our way with
                // the loop iterating curr.
                if(ComputeObjectiveDistance(**curr, **next, of) > dists[of])
                { keep = true; break; }

            // If we are keeping it, we can go to the next design.
            if(keep) { ++next; continue; }

            // If we make it here, it is too close on all dimensions so we kill
            // off next by removing it from the group and giving it to the
            // design target.  It also comes out of designs so that we don't
            // consider it anymore.
            group.Erase(*next);

            // if we are caching, put design in our buffer.  If not,
            // give it back to the target.
            target.TakeDesign(*next);
            designs.erase(next++);
        }
    }

    JEGALOG_II(GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), GetName() + ": Removed ")
            << (prevSize - designs.size()) << " of " << prevSize
            << " designs during post processing."
        )

    JEGALOG_II(GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), GetName() + ": Final group size after "
            "post processing is ") << group.GetSize() << "."
        )

}

bool
DistanceNichingPostProcessor::PollForParameters(
    const ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleVectorFromDB(
        db, "method.jega.niche_vector", _distPcts
        );

    // If we did not find the crossover rate, warn about it and use the default
    // value.  Note that if !success, then _distPcts has not been altered.
    JEGAIFLOG_CF_II(!success, GetLogger(), lverbose(), this,
        text_entry(lverbose(), GetName() + ": The distance percentages were "
            "not found in the parameter database.  Using the current values.")
        )

    SetDistancePercentages(_distPcts);

    return GeneticAlgorithmPostProcessor::PollForParameters(db);
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

DistanceNichingPostProcessor::DistanceNichingPostProcessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmPostProcessor(algorithm),
        _distPcts(GetDesignTarget().GetNOF(), DEFAULT_DIST_PCT)
{
    EDDY_FUNC_DEBUGSCOPE
}

DistanceNichingPostProcessor::DistanceNichingPostProcessor(
    const DistanceNichingPostProcessor& copy
    ) :
        GeneticAlgorithmPostProcessor(copy),
        _distPcts(copy._distPcts)
{
    EDDY_FUNC_DEBUGSCOPE
}

DistanceNichingPostProcessor::DistanceNichingPostProcessor(
    const DistanceNichingPostProcessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmPostProcessor(copy, algorithm),
        _distPcts(copy._distPcts)
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

