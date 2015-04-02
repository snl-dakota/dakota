/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DistanceNichePressureApplicator.

    NOTES:

        See notes of DistanceNichePressureApplicator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Fri Apr 28 10:45:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DistanceNichePressureApplicator
 *        class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <utilities/include/extremes.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/numeric_limits.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../MOGA/include/NichePressureApplicators/DistanceNichePressureApplicator.hpp>



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
const double DistanceNichePressureApplicator::DEFAULT_DIST_PCT(0.01);







/*
================================================================================
Mutators
================================================================================
*/
void
DistanceNichePressureApplicator::SetDistancePercentages(
    const JEGA::DoubleVector& pcts
    )
{
    EDDY_FUNC_DEBUGSCOPE

    std::size_t nof = this->GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II(nof < pcts.size(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(),
            this->GetName() + ": Received more percentages than there are "
            "objective functions.  Extras will be ignored.")
        )

    JEGAIFLOG_CF_II(nof > pcts.size() && pcts.size() > 1, this->GetLogger(),
        lquiet(), this, ostream_entry(lquiet(),
            this->GetName() + ": Received fewer percentages (") << pcts.size()
            << ") than there are objective functions (" << nof << ").  "
            "Using default value of " << DEFAULT_DIST_PCT << " to fill in."
        )

    JEGAIFLOG_CF_II(nof > pcts.size() && pcts.size() == 1, this->GetLogger(),
        lquiet(), this, ostream_entry(lquiet(),
            this->GetName() + ": Received a single distance percentage for a ")
            << nof << " objective function problem.  Using the supplied value "
            "of " << pcts[0] << " for all objectives."
        )

    this->_distPcts = pcts;

    double fill_val =
        (this->_distPcts.size() == 1) ? this->_distPcts[0] : DEFAULT_DIST_PCT;

    if(nof > this->_distPcts.size())
        this->_distPcts.resize(
            static_cast<JEGA::DoubleVector::size_type>(nof), fill_val
            );

    // now go through and set each one individually so that they can be checked
    // for legitimacy.
    for(JEGA::DoubleVector::size_type i=0; i<nof; ++i)
        this->SetDistancePercentage(i, this->_distPcts[i]);
}

void
DistanceNichePressureApplicator::SetDistancePercentages(
    double pct
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        static const double minPct = std::numeric_limits<double>::min();
        )

    std::size_t nof = this->GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II(pct < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Distance percentages must be at least ")
            << minPct << " Supplied value of " << pct
            << " will be replaced by the minimum."
        )

    JEGAIFLOG_CF_II(pct > 1.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Distance percentages cannot exceed 100%.  "
            "Supplied value of ") << pct << " will be replaced by 100%."
        )

    pct = Math::Max(0.0, Math::Min(pct, 1.0));

    this->_distPcts.assign(
        static_cast<JEGA::DoubleVector::size_type>(nof), pct
        );

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": All distance percentages now = ") << pct
        )
}

void
DistanceNichePressureApplicator::SetDistancePercentage(
    std::size_t of,
    double pct
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        static const double minPct = std::numeric_limits<double>::min();
        )

    const DesignTarget& target = GetDesignTarget();
    std::size_t nof = target.GetNOF();
    JEGA::DoubleVector::size_type dvsof =
        static_cast<JEGA::DoubleVector::size_type>(of);

    // make sure we have enough locations in the percentages vector.
    this->_distPcts.resize(
        static_cast<JEGA::DoubleVector::size_type>(nof), DEFAULT_DIST_PCT
        );

    // now verify the supplied objective function index.
    JEGAIFLOG_CF_II_F(of >= nof, this->GetLogger(), this,
        ostream_entry(lfatal(),
            this->GetName() + ": Request to change objective with index #")
            << of << ".  Valid indices are 0 through " << (nof-1) << "."
        )

    // now verify the supplied value.
    JEGAIFLOG_CF_II(pct < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Distance percentages must be at least ")
            << minPct << " Supplied value of " << pct << " for objective \""
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" will be replaced by the minimum."
        )

    JEGAIFLOG_CF_II(pct < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Distance percentages cannot exceed 100%.  "
            "Supplied value of ") << pct << " for objective \""
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" will be replaced by 100%."
        )

    pct = Math::Max(0.0, Math::Min(pct, 1.0));

    this->_distPcts[dvsof] = pct;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Distance for objective \"")
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
DistanceNichePressureApplicator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("distance");
    return ret;
}

const string&
DistanceNichePressureApplicator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This niche pressure applicator enforces a minimum distance between "
        "any two points in the performance space along each dimension "
        "individually.  Any point that is too close to another along all "
        "dimensions is removed.  This begins with the first design in the "
        "objective list and proceeds in order removing designs.  One "
        "exception to the distance rules is that any extreme designs are kept "
        "regardless.  An extreme design is one that has an extreme value "
        "for at least N-1 objectives where N is the number of objectives."
        );
    return ret;
}

GeneticAlgorithmOperator*
DistanceNichePressureApplicator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new DistanceNichePressureApplicator(algorithm);
}








/*
================================================================================
Subclass Visible Methods
================================================================================
*/

JEGA::DoubleVector
DistanceNichePressureApplicator::ComputeCutoffDistances(
    const DoubleExtremes& paretoExtremes
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // the cutoff distance is a percentage of the range of the objective
    // considering only the non-dominated designs.
    std::size_t nof = this->GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II_F(nof != paretoExtremes.size(), GetLogger(), this,
        ostream_entry(lfatal(), this->GetName() + ": Extremes contain "
            "record of ") << paretoExtremes.size() << " objectives for an "
            << nof << " objective problem."
        )

    // Prepare a vector for return.
    JEGA::DoubleVector ret(nof);

    for(DoubleExtremes::size_type i=0; i<nof; ++i)
        ret[i] = Math::Abs(
            this->GetDistancePercentage(i) * paretoExtremes.get_range(i)
            );

    // return the square route of the sum of squares.
    return ret;
}

double
DistanceNichePressureApplicator::ComputeObjectiveDistance(
    const Design& des1,
    const Design& des2,
    size_t of
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::Abs(des1.GetObjective(of) - des2.GetObjective(of));
}


size_t
DistanceNichePressureApplicator::ComputePointCountBound(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    double sum = 0.0;
    size_t nof = this->GetDesignTarget().GetNOF();

    for(size_t i=0; i<nof; ++i)
    {
        double prod = 1.0;
        for(size_t j=0; j<nof; ++j)
        {
            if(j == i) continue;
            prod *= this->GetDistancePercentage(j);
        }
        sum += 1.0/((nof-1)*prod);
    }

    return static_cast<size_t>(Math::Round(sum));
}


/*
================================================================================
Subclass Overridable Methods
================================================================================
*/


string
DistanceNichePressureApplicator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DistanceNichePressureApplicator::Name();
}

string
DistanceNichePressureApplicator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DistanceNichePressureApplicator::Description();
}

GeneticAlgorithmOperator*
DistanceNichePressureApplicator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new DistanceNichePressureApplicator(*this, algorithm);
}

bool
DistanceNichePressureApplicator::PollForParameters(
    const ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleVectorFromDB(
        db, "method.jega.niche_vector", this->_distPcts
        );

    // If we did not find the distance percentages, warn about it and use the
    // default values.  Note that if !success, then _distPcts has not been
    // altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The distance percentages "
            "were not found in the parameter database.  Using the current "
            "values.")
        )

    this->SetDistancePercentages(this->_distPcts);

    return this->GeneticAlgorithmNichePressureApplicator::PollForParameters(db);
}

void
DistanceNichePressureApplicator::PreSelection(
    DesignGroup& population
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if we are not caching designs, we needn't do anything here.
    if(!this->GetCacheDesigns()) return;

    // Sychronize the lists just in case.
    population.SynchronizeOFAndDVContainers();

    JEGA_LOGGING_IF_ON(
        const DesignOFSortSet::size_type initPSize = population.SizeOF();
        )

    // Re-assimilate the bufferred designs into the population so that they
    // can be considered when making the initial selection.  We will cull
    // them out again when ApplyNichePressure is called later if appropriate.
    this->ReAssimilateBufferedDesigns(population);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Returned ")
            << (population.SizeOF() - initPSize) << " designs during "
               "pre-selection phase of niche pressure application."
        )
}


void
DistanceNichePressureApplicator::ApplyNichePressure(
    DesignGroup& population,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If the population is empty, we needn't go any further.
    if(population.IsEmpty()) return;

    // in case we are not caching, we will need the target below.
    DesignTarget& target = this->GetDesignTarget();

    // we will need the number of objectives for a few things here.
    size_t nof = target.GetNOF();

    // Sychronize the lists just in case.
    population.SynchronizeOFAndDVContainers();

    // we are only going to consider the "best" (which should be the
    // non-dominated) designs as defined by the fitnesses.  We will call them
    // the pareto even though they may not be.
    DesignOFSortSet pareto(
        GetBest(population.GetOFSortContainer(), fitnesses)
        );

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Current population has ")
            << pareto.size() << " \"best fitness\" designs."
        )

    JEGA_LOGGING_IF_ON(std::size_t prevParetoSize = pareto.size();)

    // Now continue by extracting the pareto extremes
    DoubleExtremes paretoExtremes(
        DesignStatistician::GetObjectiveFunctionExtremes(pareto)
        );

    // We will figure which designs are too close by normalized
    // distance from the current design.  Fortunately, we are using
    // the same cutoff distance for all designs and so we can compute that
    // now for all designs.
    JEGA::DoubleVector dists(this->ComputeCutoffDistances(paretoExtremes));

    // now, we start with the first design and search the range for all
    // those that are too close.  We remove those that are too close but, if we
    // are caching, put them in a temporary storage container so that they are
    // not lost.  We also never remove an extreme Design.  Since there can only
    // be as many extreme designs as there are objectives, keep the count of
    // the number encountered to possibly reduce the cost of testing for
    // extremes.
    size_t nExtremesFound = 0;

    // prepare to output the number of designs cut out.
    JEGA_LOGGING_IF_ON(prevParetoSize = pareto.size();)

    // We can bound our search range by noting that once the first objective
    // distance is too big, we needn't look any further.  That is true because
    // of the sorting of the DesignOFSortSet.
    for(DesignOFSortSet::iterator curr(pareto.begin());
        curr!=pareto.end(); ++curr)
    {
        // Iterate all designs beyond curr until we are done and see if we keep
        // or discard them.
        DesignOFSortSet::iterator next(curr);
        for(++next; next!=pareto.end();)
        {
            double obj0Dist = this->ComputeObjectiveDistance(**curr, **next, 0);

            // If the distance at obj0 is large enough, we can get out of this
            // inner loop and move onto the next "curr".
            if(obj0Dist > dists[0]) break;

            // if next is an extreme design, we keep it no matter what.
            if(nExtremesFound < nof &&
               MultiObjectiveStatistician::IsExtremeDesign(
                **next, paretoExtremes
                )){ ++next; ++nExtremesFound; continue; }

            // prepare to store whether or not we will be keeping next.
            bool keep = false;

            // We need to see if the distances are all too small
            for(size_t of=1; of<nof; ++of)
                // if the distance at this objective is large enough, we will
                // be keeping the design so we can continue on our way with
                // the loop iterating curr.
                if(this->ComputeObjectiveDistance(
                    **curr, **next, of
                    ) > dists[of]) { keep = true; break; }

            // If we are keeping it, we can go to the next design.
            if(keep) { ++next; continue; }

            // If we make it here, it is too close on all dimensions so we kill
            // off next by removing it from the population and putting it into
            // our design buffer.  It also comes out of pareto so that we don't
            // consider it anymore.
            population.Erase(*next);

            // if we are caching, put design in our buffer.  If not,
            // give it back to the target.
            if(!this->BufferDesign(*next)) target.TakeDesign(*next);
            pareto.erase(next++);
        }
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Removed ")
            << (prevParetoSize - pareto.size()) << " of " << prevParetoSize
            << " \"best fitness\" designs during the application of niche "
               "pressure."
        )

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Final population size "
            "after niching is ") << population.GetSize() << "."
        )

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



DistanceNichePressureApplicator::DistanceNichePressureApplicator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmNichePressureApplicator(algorithm),
        _distPcts(GetDesignTarget().GetNOF(), DEFAULT_DIST_PCT)
{
    EDDY_FUNC_DEBUGSCOPE
}

DistanceNichePressureApplicator::DistanceNichePressureApplicator(
    const DistanceNichePressureApplicator& copy
    ) :
        GeneticAlgorithmNichePressureApplicator(copy),
        _distPcts(copy._distPcts)
{
    EDDY_FUNC_DEBUGSCOPE
}

DistanceNichePressureApplicator::DistanceNichePressureApplicator(
    const DistanceNichePressureApplicator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmNichePressureApplicator(copy, algorithm),
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
