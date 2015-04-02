/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class RadialNichePressureApplicator.

    NOTES:

        See notes of RadialNichePressureApplicator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Wed Jan 18 10:45:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RadialNichePressureApplicator
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
#include <utilities/include/extremes.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../MOGA/include/NichePressureApplicators/RadialNichePressureApplicator.hpp>




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
const double RadialNichePressureApplicator::DEFAULT_RAD_PCT(0.01);







/*
================================================================================
Mutators
================================================================================
*/
void
RadialNichePressureApplicator::SetRadiusPercentages(
    const JEGA::DoubleVector& pcts
    )
{
    EDDY_FUNC_DEBUGSCOPE

    size_t nof = this->GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II(nof < pcts.size(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(),
            this->GetName() + ": Received more percentages than there are "
            "objective functions.  Extras will be ignored.")
        )

    JEGAIFLOG_CF_II(nof > pcts.size() && pcts.size() > 1, this->GetLogger(),
        lquiet(), this, ostream_entry(lquiet(),
            this->GetName() + ": Received fewer percentages (") << pcts.size()
            << ") than there are objective functions (" << nof << ").  "
            "Using default value of " << DEFAULT_RAD_PCT << " to fill in."
        )

    JEGAIFLOG_CF_II(nof > pcts.size() && pcts.size() == 1, this->GetLogger(),
        lquiet(), this, ostream_entry(lquiet(),
            this->GetName() + ": Received a single radius percentage for a ")
            << nof << " objective function problem.  Using the supplied value "
            "of " << pcts[0] << " for all objectives."
        )

    this->_radPcts = pcts;

    double fill_val = (this->_radPcts.size() == 1) ?
        this->_radPcts[0] : DEFAULT_RAD_PCT;

    if(nof > this->_radPcts.size())
        this->_radPcts.resize(
            static_cast<JEGA::DoubleVector::size_type>(nof), fill_val
            );

    // now go through and set each one individually so that they can be checked
    // for legitimacy.
    for(JEGA::DoubleVector::size_type i=0; i<nof; ++i)
        this->SetRadiusPercentage(i, this->_radPcts[i]);
}

void
RadialNichePressureApplicator::SetRadiusPercentages(
    double pct
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        static const double minPct = numeric_limits<double>::min();
        )

    size_t nof = this->GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II(pct < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Radius percentages must be at least ")
            << minPct << " Supplied value of " << pct << " will be replaced by "
               "the minimum."
        )

    JEGAIFLOG_CF_II(pct > 1.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Radius percentages cannot exceed 100%. "
            " Supplied value of ") << pct << " will be replaced by 100%."
        )

    pct = Math::Max(0.0, Math::Min(pct, 1.0));

    this->_radPcts.assign(static_cast<JEGA::DoubleVector::size_type>(nof), pct);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": All radius percentages now = ") << pct
        )
}

void
RadialNichePressureApplicator::SetRadiusPercentage(
    size_t of,
    double pct
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        static const double minPct = numeric_limits<double>::min();
        )

    const DesignTarget& target = this->GetDesignTarget();
    size_t nof = target.GetNOF();
    JEGA::DoubleVector::size_type dvsof =
        static_cast<JEGA::DoubleVector::size_type>(of);

    // make sure we have enough locations in the percentages vector.
    this->_radPcts.resize(
        static_cast<JEGA::DoubleVector::size_type>(nof), DEFAULT_RAD_PCT
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
            this->GetName() + ": Radius percentages must be at least ")
            << minPct << " Supplied value of " << pct << " for objective \""
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" will be replaced by the minimum."
        )

    JEGAIFLOG_CF_II(pct < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Radius percentages cannot exceed 100%. "
            " Supplied value of ") << pct << " for objective \""
            << target.GetObjectiveFunctionInfos()[dvsof]->GetLabel()
            << "\" will be replaced by 100%."
        )

    pct = Math::Max(0.0, Math::Min(pct, 1.0));

    this->_radPcts[dvsof] = pct;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Radius for objective \"")
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
RadialNichePressureApplicator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("radial");
    return ret;
}

const string&
RadialNichePressureApplicator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This operator provides niche pressure by enforcing a minimum\n"
        "distance between non-dominated designs in the performance space.\n"
        "It requires a percentage value for each objective, each of which is\n"
        "interpreted as a percentage of the range observed for that\n"
        "objective within the subset of non-dominated designs.  The\n"
        "resulting distances are merged using a square-root-sum-of-squares\n"
        "into the radius about each design within which another design may\n"
        "not exist."
        );
    return ret;
}

GeneticAlgorithmOperator*
RadialNichePressureApplicator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new RadialNichePressureApplicator(algorithm);
}








/*
================================================================================
Subclass Visible Methods
================================================================================
*/

double
RadialNichePressureApplicator::ComputeCutoffDistance(
    const DoubleExtremes& paretoExtremes
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // the cutoff distance is sqrt-sos of the allowable distances for each
    // objective normalized by the Pareto range for that objective.
    size_t nof = this->GetDesignTarget().GetNOF();

    JEGAIFLOG_CF_II_F(nof != paretoExtremes.size(), this->GetLogger(), this,
        ostream_entry(lfatal(), this->GetName() + ": Extremes contain "
            "record of ") << paretoExtremes.size() << " objectives for an "
            << nof << " objective problem."
        )

    // prepare to track the sum-of-squares.
    double sos = 0.0;

    for(DoubleExtremes::size_type i=0; i<nof; ++i)
    {
        // The cutoff distance along this dimension is the percentage of the
        // range along this dimension.
        const double rp = this->GetRadiusPercentage(i);
        sos += rp * rp;
    }

    // return the square route of the sum of squares.
    return Math::Sqrt(sos);
}

double
RadialNichePressureApplicator::GetNormalizedDistance(
    const Design& des1,
    const Design& des2,
    const DoubleExtremes& paretoExtremes
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II_F(!des1.IsEvaluated() || !des2.IsEvaluated(),
        this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unevaluated designs detected "
            "when calculating objective function distances."
            )
        )

    size_t nof = des1.GetNOF();

    // we will compute the distance as sqrt s-o-s of the individual normalized
    // objective distances.
    double sos = 0.0;

    for(size_t i=0; i<nof; ++i)
    {
        const double dist = this->GetNormalizedObjectiveDistance(
            des1, des2, i,
            paretoExtremes.get_range(static_cast<DoubleExtremes::size_type>(i))
            );

        sos += (dist * dist);
    }

    return Math::Sqrt(sos);
}

double
RadialNichePressureApplicator::GetNormalizedObjectiveDistance(
    const Design& des1,
    const Design& des2,
    size_t of,
    double range
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::Abs(des1.GetObjective(of)-des2.GetObjective(of)) / range;
}

/*
================================================================================
Subclass Overridable Methods
================================================================================
*/


string
RadialNichePressureApplicator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RadialNichePressureApplicator::Name();
}

string
RadialNichePressureApplicator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RadialNichePressureApplicator::Description();
}

GeneticAlgorithmOperator*
RadialNichePressureApplicator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new RadialNichePressureApplicator(*this, algorithm);
}

bool
RadialNichePressureApplicator::PollForParameters(
    const ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleVectorFromDB(
        db, "method.jega.niche_vector", this->_radPcts
        );

    // If we did not find the crossover rate, warn about it and use the default
    // value.  Note that if !success, then _radPcts has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The radius percentages "
            "were not found in the parameter database.  Using the current "
            "values.")
        )

    this->SetRadiusPercentages(this->_radPcts);

    return GeneticAlgorithmNichePressureApplicator::PollForParameters(db);
}

void
RadialNichePressureApplicator::PreSelection(
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
RadialNichePressureApplicator::ApplyNichePressure(
    DesignGroup& population,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If the population is empty, we needn't go any further.
    if(population.IsEmpty()) return;

    // we will need the number of objectives for a few things here.
    size_t nof = this->GetDesignTarget().GetNOF();

    // Sychronize the lists just in case.
    population.SynchronizeOFAndDVContainers();

    // we are only going to consider the "best" (which should be the
    // non-dominated) designs as defined by the fitnesses.  We will
    // call them the pareto even though they may not be.
    DesignOFSortSet pareto(
        this->GetBest(population.GetOFSortContainer(), fitnesses)
        );

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Current population has ")
            << pareto.size() << " \"best fitness\" designs."
        )

    JEGA_LOGGING_IF_ON(size_t prevParetoSize = pareto.size();)

    // Now continue by extracting the pareto extremes
    DoubleExtremes paretoExtremes(
        DesignStatistician::GetObjectiveFunctionExtremes(pareto)
        );

    // We will figure which designs are too close by normalized
    // distance from the current design.  Fortunately, we are using
    // the same cutoff distance for all designs and so we can compute that
    // now for all designs.
    double cutoffDist = this->ComputeCutoffDistance(paretoExtremes);

    // in case we are not caching, we will need the target below.
    DesignTarget& target = this->GetDesignTarget();

    // now, we start with the first design and search the range for all
    // those that are too close.  We remove those that are too close but
    // put them in a temporary storage container so that they are not lost.
    // We also never remove an extreme Design.  Since there can only
    // be as many extreme designs as there are objectives, keep the count of
    // the number encountered to possibly reduce the cost of testing for
    // extremes.
    size_t nExtremesFound = 0;

    // prepare to output the number of designs cut out.
    JEGA_LOGGING_IF_ON(prevParetoSize = pareto.size();)

    // We can bound our search range by noting that once the first objective
    // distanceis too big, we needn't look any further.  That is true because
    // of the sorting of the DesignOFSortSet.
    for(DesignOFSortSet::iterator curr(pareto.begin());
        curr!=pareto.end(); ++curr)
    {
        // Iterate all designs beyond curr until we are done and see if we keep
        // or discard them.
        DesignOFSortSet::iterator next(curr);
        for(++next; next!=pareto.end();)
        {
            double obj0Dist = this->GetNormalizedObjectiveDistance(
                **curr, **next, 0, paretoExtremes.get_range(0)
                );

            // If the distance at obj0 is large enough, we can get out of this
            // inner loop and move onto the next "curr".
            if(obj0Dist > cutoffDist) break;

            // if next is an extreme design, we keep it no matter what.
            if(nExtremesFound < nof &&
               MultiObjectiveStatistician::IsExtremeDesign(
                **next, paretoExtremes
                )){ ++next; ++nExtremesFound; continue; }

            // Otherwise, we need to see if the overall distance is too small
            double dist =
                this->GetNormalizedDistance(**curr, **next, paretoExtremes);

            // If it is too small, we kill off next by removing it from the
            // population and putting it into our design buffer.  It also
            // comes out of pareto so that we don't consider it anymore.
            if(dist < cutoffDist)
            {
                population.Erase(*next);

                // if we are caching, put design in our buffer.  If not,
                // give it back to the target.
                if(!this->BufferDesign(*next)) target.TakeDesign(*next);
                pareto.erase(next++);
            }

            // otherwise we go onto the next design.
            else ++next;
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



RadialNichePressureApplicator::RadialNichePressureApplicator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmNichePressureApplicator(algorithm),
        _radPcts(GetDesignTarget().GetNOF(), DEFAULT_RAD_PCT)
{
    EDDY_FUNC_DEBUGSCOPE
}

RadialNichePressureApplicator::RadialNichePressureApplicator(
    const RadialNichePressureApplicator& copy
    ) :
        GeneticAlgorithmNichePressureApplicator(copy),
        _radPcts(copy._radPcts)
{
    EDDY_FUNC_DEBUGSCOPE
}

RadialNichePressureApplicator::RadialNichePressureApplicator(
    const RadialNichePressureApplicator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmNichePressureApplicator(copy, algorithm),
        _radPcts(copy._radPcts)
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
