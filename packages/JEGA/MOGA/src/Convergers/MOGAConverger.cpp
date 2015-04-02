/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MOGAConverger.

    NOTES:

        See notes of MOGAConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Jul 07 09:24:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MOGAConverger class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cmath>
#include <cfloat>
#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/numeric_limits.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../MOGA/include/Convergers/MOGAConverger.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace eddy::utilities;
using namespace JEGA::Utilities;







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

JEGA_IF_MESSAGE_BOARD(
    MessageInfo MOGAConverger::DENSITY_MSG_INFO(
        MessageBoard::MessageIdentifier(0x0, "metric_tracker", "density")
        );

    MessageInfo MOGAConverger::EXPANSE_MSG_INFO(
        MessageBoard::MessageIdentifier(0x0, "metric_tracker", "expanse")
        );

    MessageInfo MOGAConverger::DEPTH_MSG_INFO(
        MessageBoard::MessageIdentifier(0x0, "metric_tracker", "depth")
        );
)







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


const string&
MOGAConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("metric_tracker");
    return ret;
}

const string&
MOGAConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger computes various metrics for the "
        "population and determines if sufficient improvement "
        "is being made.  If not, this converger returns true."
        );
    return ret;
}

GeneticAlgorithmOperator*
MOGAConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new MOGAConverger(algorithm);
}


/*
================================================================================
Subclass Visible Methods
================================================================================
*/

double
MOGAConverger::GetMaxRangeChange(
    const eddy::utilities::DoubleExtremes& newParExtremes
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(newParExtremes.size() == this->_prevParExtremes.size());

    JEGAIFLOG_CF_II_F(newParExtremes.size() != this->_prevParExtremes.size(),
        this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Dimensional disagreement "
            "between old Pareto extremes and new Pareto extremes.")
        );

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(
            ldebug(), this->GetName() + ": Pareto objective extremes now:"
            )
        )

#ifdef JEGA_LOGGING_ON

    const ObjectiveFunctionInfoVector& ofInfos =
        this->GetDesignTarget().GetObjectiveFunctionInfos();

    if(this->GetLogger().Gate().will_log(this, ldebug()))
    {
        for(size_t i=0; i<newParExtremes.size(); ++i)
        {
            JEGALOG_II(this->GetLogger(), ldebug(), this,
                ostream_entry(ldebug(), ofInfos[i]->GetLabel() + ": ") <<
                newParExtremes.get_min(i) << ", " << newParExtremes.get_max(i)
                )
        }
    }

#endif

    // compute the percentage change in range along each dimension as the
    // difference in the ranges divided by the old range.  Return the maximum
    // of them.
    double maxChng = 0.0;
    DoubleExtremes::size_type of = 0;
    DoubleExtremes::size_type size = newParExtremes.size();

    for(DoubleExtremes::size_type i=0; i<size; ++i)
    {
        const double overallRange = this->_prevParExtremes.get_range(i);
        const double currChng = overallRange == 0.0 ?
            newParExtremes.get_range(i) :
            (newParExtremes.get_range(i) - overallRange) / overallRange;

        const double currChngAbs = std::fabs(currChng);

        JEGALOG_II(this->GetLogger(), ldebug(), this,
            ostream_entry(
                ldebug(), this->GetName() + ": " + ofInfos[i]->GetLabel() +
                (currChng >= 0.0 ? " expanded " : " contracted ") +
                "by ") << currChngAbs*100 << "%."
            )

        if(currChngAbs > std::fabs(maxChng)) { maxChng = currChng; of = i; }
    }

    // do a little outputting if appropriate
    JEGAIFLOG_CF_II(maxChng != 0.0, this->GetLogger(), lverbose(), this,
        ostream_entry(
            lverbose(), this->GetName() + ": Max \"best fitness\" expanse "
            "change of "
            ) << Math::Round(maxChng*100.0, this->GetNumDP())
              << "% found for objective \"" << ofInfos[of]->GetLabel() << "\"."
        )

    JEGAIFLOG_CF_II(maxChng == 0.0, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": No \"best fitness\" "
            "expansion or contraction occured this generation.")
        )

    return maxChng;
}

double
MOGAConverger::GetDensityChange(
    const JEGA::Utilities::DesignOFSortSet& newPop,
    const eddy::utilities::DoubleExtremes& newPopExtremes
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // This is computed as the percentage change in density where density
    // is npts/volume.

    // start with our volumes.
    const double oldVol = ComputeVolume(this->_prevPopExtremes);
    const double newVol = ComputeVolume(newPopExtremes);

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(ldebug(), this->GetName() + ": Pareto volume now = ")
            << newVol << '.'
        )

    // now do our densities.
    const double oldDen = this->_prevPopSize / oldVol;
    const double newDen = newPop.size() / newVol;

    // now compute and return our percent change.
    const double fracChng = (newDen - oldDen) / oldDen;

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(ldebug(), this->GetName() + ": Population density now = ")
            << newDen << '.'
        )

    JEGAIFLOG_CF_II(fracChng != 0.0, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() +
            ": Population density " +
            (newDen < oldDen ? "decreased by " : "increased by ")
            ) << Math::Round(std::fabs(fracChng)*100.0, this->GetNumDP())
              << "%."
        )

    JEGAIFLOG_CF_II(fracChng == 0.0, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() +
            ": Population density did not change this generation.")
        )

    return fracChng;
}

double
MOGAConverger::GetFractionDominated(
    const JEGA::Utilities::DesignOFSortSet& curr
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // if the previous set has zero size, we cannot compute.
    if(this->_prevParSet.empty()) return 0.0;

    // go through in and count how many are dominated by designs in "by".
    // The return will be whatever percentage of in.size() count is.
    eddy::utilities::uint64_t numDom = 0;

    const DesignOFSortSet::const_iterator e(this->_prevParSet.end());
    for(DesignOFSortSet::const_iterator it(this->_prevParSet.begin()); it!=e; ++it)
        numDom += MultiObjectiveStatistician::IsDominatedByAtLeast1(**it, curr)
                  ? 1 : 0;

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(ldebug(), this->GetName()) << ": " << numDom << " of the "
            "designs from the previous \"best fitness\" population are "
            "dominated by designs in the current \"best fitness\" population."
        )

    const double fracDom = static_cast<double>(numDom)/this->_prevParSet.size();

    JEGAIFLOG_CF_II(fracDom != 0.0, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ")
            << Math::Round(fracDom*100.0, this->GetNumDP())
            << "% of the previous \"best fitness\" population is dominated by "
               "designs in the current \"best fitness\" population."
        )

    JEGAIFLOG_CF_II(fracDom == 0.0, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": No \"best fitness\" "
            "designs from the previous population are dominated by \"best "
            "fitness\" designs in the current population.")
        )

    return fracDom;
}


void
MOGAConverger::UpdatePreviousParetoSet(
    const DesignOFSortSet& newSet
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // we have to go through and destroy all the designs in the
    // current set because we created them.  They do not have to go
    // to the target because they are duplicates of mainstream Designs.
    this->_prevParSet.flush();

    // now create new designs that are duplicates of those in the new set
    // and put them into our previous set.
    const DesignOFSortSet::const_iterator e(newSet.end());

    for(DesignOFSortSet::const_iterator it(newSet.begin()); it!=e; ++it)
        this->_prevParSet.insert(this->GetAlgorithm().GetNewDesign(*(*it)));
}

DesignOFSortSet
MOGAConverger::GetBest(
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


string
MOGAConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MOGAConverger::Name();
}

string
MOGAConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MOGAConverger::Description();
}

GeneticAlgorithmOperator*
MOGAConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MOGAConverger(*this, algorithm);
}

double
MOGAConverger::GetMetricValue(
    const DesignGroup& group,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We will track our various metrics and return the one that
    // is the "best" (most improvement).  We only care about our
    // best designs as defined by the fitnesses so start by extracting
    // those.  Ideally, they are the non-dominated so we will call them
    // the pareto here.
    const DesignOFSortSet& popByOF = group.GetOFSortContainer();

    DesignOFSortSet pareto(this->GetBest(popByOF, fitnesses));

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Operating on ")
            << popByOF.size() << " total designs and "
            << pareto.size() << " \"best fitness\" designs."
        )

    // Start by getting our newest pareto extremes.  We don't have to
    // use the multi objective statistician because we know all our
    // designs are non dominated.
    DoubleExtremes newParExt(
        DesignStatistician::GetObjectiveFunctionExtremes(pareto)
        );

    DoubleExtremes newPopExt(
        DesignStatistician::GetObjectiveFunctionExtremes(popByOF)
        );

    double maxFrac = 0.0;

    // The first thing to check is that this is not the first generation.
    // If it is, then we set our previous data and return 0.0 as our metric.
    if(this->GetAlgorithm().GetGenerationNumber() > 1)
    {
        // Start with our maximum change in range along any Pareto dimension.
        const double maxExp = this->GetMaxRangeChange(newParExt);
        const double maxExpAbs = std::fabs(maxExp);

        // If posting, only post the specific.  Those interested in the
        // general should be using appropriate predicates to get this post.
        JEGA_IF_MESSAGE_BOARD(
            if(EXPANSE_MSG_INFO.WillPost() || this->_expanseMsgInfo.WillPost())
                this->_expanseMsgInfo.Post(maxExp);
        )


        // Now get our change in density.
        const double denInc = this->GetDensityChange(popByOF, newPopExt);
        const double denIncAbs = std::fabs(denInc);

        JEGA_IF_MESSAGE_BOARD(
            if(DENSITY_MSG_INFO.WillPost() || this->_densityMsgInfo.WillPost())
                this->_densityMsgInfo.Post(denInc);
        )

        // Finally, get the percentage of Designs in the old group that
        // are dominated by designs in the new group.
        const double fracDom = this->GetFractionDominated(pareto);
        const double fracDomAbs = std::fabs(fracDom);

        JEGA_IF_MESSAGE_BOARD(
            if(DEPTH_MSG_INFO.WillPost() || this->_depthMsgInfo.WillPost())
                this->_depthMsgInfo.Post(fracDom);
        )

        // The return will be the sum of all these metrics.
        maxFrac = maxExpAbs + denIncAbs + fracDomAbs;

        JEGAIFLOG_CF_II(maxExpAbs >= denIncAbs && maxExpAbs >= fracDomAbs,
            this->GetLogger(), ldebug(), this,
            text_entry(ldebug(), this->GetName() + ": Worst convergence "
                "offender is the expansion metric.")
            )

        JEGAIFLOG_CF_II(denIncAbs >= maxExpAbs && denIncAbs >= fracDomAbs,
            this->GetLogger(), ldebug(), this,
            text_entry(ldebug(), this->GetName() + ": Worst convergence "
                "offender is the density metric.")
            )

        JEGAIFLOG_CF_II(fracDomAbs >= maxExpAbs && fracDomAbs >= denIncAbs,
            this->GetLogger(), ldebug(), this,
            text_entry(ldebug(), this->GetName() + ": Worst convergence "
                "offender is the domination fraction metric.")
            )
    }

    // Since we have them now, take our new extremes as our old ones in prep
    // for the next generation.
    this->UpdateParetoExtremes(newParExt);
    this->UpdatePopulationExtremes(newPopExt);

    // record the current as the previous set
    this->UpdatePreviousParetoSet(pareto);
    this->_prevPopSize = popByOF.size();

    return maxFrac;
}

bool
MOGAConverger::CheckConvergence(
    const DesignGroup& group,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // make sure the sets of group are synchronized.
    JEGA_LOGGING_IF_ON(bool synced = ) group.SynchronizeOFAndDVContainers();

    JEGAIFLOG_CF_II(!synced, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Group synchronization "
            "failed.  Proceeding with group in current state.")
        )

    return this->MetricTrackerConvergerBase::CheckConvergence(group, fitnesses);
}






/*
================================================================================
Private Methods
================================================================================
*/
double
MOGAConverger::ComputeVolume(
    const DoubleExtremes& extremes
    )
{
    EDDY_FUNC_DEBUGSCOPE

    DoubleExtremes::size_type size = extremes.size();
    double vol = 1.0;
    for(DoubleExtremes::size_type i=0; i<size; ++i)
    {
        const double r = extremes.get_range(i);
        if(r != 0.0) vol *= r;
    }

    return vol;
}








/*
================================================================================
Structors
================================================================================
*/


MOGAConverger::MOGAConverger(
    GeneticAlgorithm& algorithm
    ) :
        MetricTrackerConvergerBase(algorithm, true),
        _prevParSet(),
        _prevParExtremes(
            algorithm.GetDesignTarget().GetNOF(), DBL_MAX, -DBL_MAX
            ),
        _prevPopExtremes(
            algorithm.GetDesignTarget().GetNOF(), DBL_MAX, -DBL_MAX
            ),
        _prevPopSize(0)
#ifdef JEGA_MESSAGE_BOARD
        ,
        _densityMsgInfo(),
        _expanseMsgInfo(),
        _depthMsgInfo()
#endif
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_MESSAGE_BOARD(
        this->_densityMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "density")
            );

        this->_expanseMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "expanse")
            );

        this->_depthMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "depth")
            );
        )
}

MOGAConverger::MOGAConverger(
    const MOGAConverger& copy
    ) :
        MetricTrackerConvergerBase(copy),
        _prevParSet(copy._prevParSet),
        _prevParExtremes(copy._prevParExtremes),
        _prevPopExtremes(copy._prevPopExtremes),
        _prevPopSize(copy._prevPopSize)
#ifdef JEGA_MESSAGE_BOARD
        ,
        _densityMsgInfo(),
        _expanseMsgInfo(),
        _depthMsgInfo()
#endif
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_MESSAGE_BOARD(
        this->_densityMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "density")
            );

        this->_expanseMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "expanse")
            );

        this->_depthMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "depth")
            );
        )
}

MOGAConverger::MOGAConverger(
    const MOGAConverger& copy,
    GeneticAlgorithm& algorithm
    ) :
        MetricTrackerConvergerBase(copy, algorithm),
        _prevParSet(copy._prevParSet),
        _prevParExtremes(copy._prevParExtremes),
        _prevPopExtremes(copy._prevPopExtremes),
        _prevPopSize(copy._prevPopSize)
#ifdef JEGA_MESSAGE_BOARD
        ,
        _densityMsgInfo(),
        _expanseMsgInfo(),
        _depthMsgInfo()
#endif
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_MESSAGE_BOARD(
        this->_densityMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "density")
            );

        this->_expanseMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "expanse")
            );

        this->_depthMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(this, "metric_tracker", "depth")
            );
        )
}

MOGAConverger::~MOGAConverger(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_prevParSet.flush();
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
