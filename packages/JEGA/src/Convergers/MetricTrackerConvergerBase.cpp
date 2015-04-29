/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MetricTrackerConvergerBase.

    NOTES:

        See notes of MetricTrackerConvergerBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jul 22 15:43:27 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MetricTrackerConvergerBase class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cmath>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/numeric_limits.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Convergers/MetricTrackerConvergerBase.hpp>
#include <../Utilities/include/ParameterDatabase.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
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
const DoubleStack::size_type MetricTrackerConvergerBase::DEFAULT_NUM_GENS(10);

const double MetricTrackerConvergerBase::DEFAULT_CHNG(0.1);

JEGA_IF_MESSAGE_BOARD(
    MessageInfo MetricTrackerConvergerBase::METRIC_MSG_INFO(
        MessageBoard::MessageIdentifier(0x0, "metric_track_converger", "metric")
        );

    MessageInfo MetricTrackerConvergerBase::CURR_PCT_CHNG_INFO(
        MessageBoard::MessageIdentifier(
            0x0, "metric_track_converger", "curr_pct_chng"
            )
        );
)


/*
================================================================================
Mutators
================================================================================
*/
void
MetricTrackerConvergerBase::SetPercentChange(
    double change
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_change = change;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Max allowable % change now = "
            ) << this->_change
        )
}

void
MetricTrackerConvergerBase::SetNumGenerations(
    size_t numGen
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->SetMetricStackMaxDepth(numGen);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Number of generations tracked now = ")
                << this->_metricTracker.GetStackMaxDepth()
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


int
MetricTrackerConvergerBase::GetNumDP(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // If the percent change is 0, then we use 2.
    if(this->_change == 0.0) return 2;

    return static_cast<int>(
        Math::Max(Math::Ceil(Math::Abs(
            Math::Log10(this->_change*100.0)
            ))+1, 2.0)
        );
}






/*
================================================================================
Subclass Overridable Methods
================================================================================
*/

bool
MetricTrackerConvergerBase::CheckConvergence(
    const DesignGroup& group,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(this->MaxGenEvalTimeConverger::CheckConvergence(group, fitnesses))
        return true;

    // now get the metric to push onto the stack.
    double metric = this->GetMetricValue(group, fitnesses);

    // If posting, only post the specific.  Those interested in the general
    // should be using appropriate predicates to get this post.
    JEGA_IF_MESSAGE_BOARD(
        if(METRIC_MSG_INFO.WillPost() || this->_metricMsgInfo.WillPost())
            this->_metricMsgInfo.Post(metric);
        )

    this->_metricTracker.Push(metric);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Pushed metric value of ")
            << metric << " onto the convergence stack."
        )

    // If we have not yet stored the required number of metric
    // values, we certainly cannot be done.
    if(!this->_metricTracker.IsFull()) return false;

    // Otherwise, we must compare the new value to all previous values to
    // determine if we converged.  The new value is at the top of the stack and
    // is "metric".

    // Iterate the number of entries minus the last one and if we can determine
    // that we don't converge, break.
    const double minVal = this->_metricTracker.MinValue().second;
    const double maxVal = this->_metricTracker.MaxValue().second;
    bool converged = true;

    // If this is an absolute checker, then the max absolute value of the metric
    // tracker is the value of interest.
    double pctChng = 0;
    if(this->_absolute)
    {
        pctChng = Math::Max(fabs(minVal), fabs(maxVal));
    }
    else
    {
        if(maxVal == minVal)
        {
            pctChng = 0.0;
        }
        else if(maxVal == 0.0)
        {
            pctChng = (maxVal - minVal) / ((maxVal + minVal) * 0.5);
        }
        else
        {
            pctChng = (maxVal - minVal) / fabs(maxVal);
        }
    }

    // report to 1 more decimal place than necessary to cover the _change
    // using at least 2.
    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Current max percentage progress is ")
            << Math::Round(pctChng * 100.0, this->GetNumDP()) << "%."
        )

    converged = (pctChng <= this->_change);

    // If posting, only post the specific.  Those interested in the general
    // should be using appropriate predicates to get this post.
    JEGA_IF_MESSAGE_BOARD(
        if(CURR_PCT_CHNG_INFO.WillPost() || this->_pctChngMsgInfo.WillPost())
            this->_pctChngMsgInfo.Post(pctChng);
        )

    // Otherwise we are done if converged is true
    this->SetConverged(converged);

    return this->GetConverged();
}

bool
MetricTrackerConvergerBase::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    size_t ngens = DEFAULT_NUM_GENS;
    bool success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.jega.num_generations", ngens
        );

    // If we did not find the num gens, warn about it and use the default
    // value.  Note that if !success, then ngens has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The number of "
            "generations was not found in the parameter database.  Using the "
            "current value of ") << ngens
        )

    this->SetNumGenerations(ngens);

    success = ParameterExtractor::GetDoubleFromDB(
        db, "method.jega.percent_change", this->_change
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The percent change "
            "was not found in the parameter database.  Using the current "
            "value of ") << this->_change
        )

    // Otherwise, finish recording and output at the verbose level.
    this->SetPercentChange(this->_change);

    return this->MaxGenEvalTimeConverger::PollForParameters(db);
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




MetricTrackerConvergerBase::MetricTrackerConvergerBase(
    GeneticAlgorithm& algorithm,
    bool absolute
    ) :
        MaxGenEvalTimeConverger(algorithm),
        _metricTracker(DEFAULT_NUM_GENS),
        _change(DEFAULT_CHNG),
        _absolute(absolute)
#ifdef JEGA_MESSAGE_BOARD
        ,
        _metricMsgInfo(),
        _pctChngMsgInfo()
#endif
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_MESSAGE_BOARD(
        this->_metricMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(
                this, "metric_track_converger", "metric"
                )
            );

        this->_pctChngMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(
                this, "metric_track_converger", "curr_pct_chng"
                )
            );
        )
}

MetricTrackerConvergerBase::MetricTrackerConvergerBase(
    const MetricTrackerConvergerBase& copy
    ) :
        MaxGenEvalTimeConverger(copy),
        _metricTracker(copy._metricTracker),
        _change(copy._change),
        _absolute(copy._absolute)
#ifdef JEGA_MESSAGE_BOARD
        ,
        _metricMsgInfo(),
        _pctChngMsgInfo()
#endif
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_MESSAGE_BOARD(
        this->_metricMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(
                this, "metric_track_converger", "metric"
                )
            );

        this->_pctChngMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(
                this, "metric_track_converger", "curr_pct_chng"
                )
            );
        )
}

MetricTrackerConvergerBase::MetricTrackerConvergerBase(
    const MetricTrackerConvergerBase& copy,
    GeneticAlgorithm& algorithm
    ) :
        MaxGenEvalTimeConverger(copy, algorithm),
        _metricTracker(copy._metricTracker),
        _change(copy._change),
        _absolute(copy._absolute)
#ifdef JEGA_MESSAGE_BOARD
        ,
        _metricMsgInfo(),
        _pctChngMsgInfo()
#endif
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_MESSAGE_BOARD(
        this->_metricMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(
                this, "metric_track_converger", "metric"
                )
            );

        this->_pctChngMsgInfo.ResetMessageIdentifier(
            MessageBoard::MessageIdentifier(
                this, "metric_track_converger", "curr_pct_chng"
                )
            );
        )
}




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
