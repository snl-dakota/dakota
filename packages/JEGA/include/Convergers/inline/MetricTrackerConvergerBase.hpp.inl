/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class MetricTrackerConvergerBase.

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
 * \brief Contains the inline methods of the MetricTrackerConvergerBase class.
 */




/*
================================================================================
Includes
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {








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

inline
double
MetricTrackerConvergerBase::GetPercentChange(
    ) const
{
    return this->_change;
}

inline
std::size_t
MetricTrackerConvergerBase::GetNumGenerations(
    ) const
{
    return this->_metricTracker.GetStackMaxDepth();
}

inline
MetricTracker&
MetricTrackerConvergerBase::GetMetricTracker(
    )
{
    return this->_metricTracker;
}

inline
const MetricTracker&
MetricTrackerConvergerBase::GetMetricTracker(
    ) const
{
    return this->_metricTracker;
}





/*
================================================================================
Inline Public Methods
================================================================================
*/




inline
void
MetricTrackerConvergerBase::AddMetricValue(
    double value
    )
{
    this->_metricTracker.Push(value);
}

inline
double
MetricTrackerConvergerBase::GetProgressPercentage(
    std::size_t older,
    std::size_t newer
    ) const
{
    return this->_metricTracker.GetPercentDifference(older, newer);
}

inline
void
MetricTrackerConvergerBase::SetMetricStackMaxDepth(
    std::size_t maxDepth
    )
{
    this->_metricTracker.SetStackMaxDepth(maxDepth);
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
    } // namespace Algorithms
} // namespace JEGA
