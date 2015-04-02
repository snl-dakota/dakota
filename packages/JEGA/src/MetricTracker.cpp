/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MetricTracker.

    NOTES:

        See notes of MetricTracker.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jul 22 10:29:01 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MetricTracker class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <iterator>
#include <algorithm>
#include <MetricTracker.hpp>
#include <utilities/include/Math.hpp>
#include <utilities/include/numeric_limits.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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








/*
================================================================================
Mutators
================================================================================
*/


std::size_t
MetricTracker::SetStackMaxDepth(
    size_t depth
    )
{
    EDDY_FUNC_DEBUGSCOPE

    size_t oldSize = this->_stack.size();
    this->_maxDepth = depth;
    PruneTheStack();
    return oldSize < this->_stack.size() ? this->_stack.size()-oldSize : 0;
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


double
MetricTracker::GetPercentDifference(
    size_t elem1,
    size_t elem2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(elem1 < this->_stack.size());
    EDDY_ASSERT(elem2 < this->_stack.size());

    const double v1 = this->_stack[elem1];
    const double v2 = this->_stack[elem2];

    return (v1 == 0.0) ? v2 : Math::Abs((v2 - v1) / v1);
}

MetricTracker::LocPair
MetricTracker::MaxValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    if(this->_stack.empty())
        return LocPair(
            std::numeric_limits<DoubleStack::size_type>::max(),
            eddy::utilities::numeric_limits<double>::smallest()
            );

    DoubleStack::const_iterator it(
        max_element(this->_stack.begin(), this->_stack.end())
        );

    return LocPair(
        // would prefer the distance call but Forte Developer 7 doesn't
        // like it.
//      static_cast<DoubleStack::size_type>(distance(this->_stack.begin(), it)),
        static_cast<DoubleStack::size_type>(it - this->_stack.begin()),
        *it
        );
}

MetricTracker::LocPair
MetricTracker::MinValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    if(this->_stack.empty())
        return make_pair<DoubleStack::size_type>(
            std::numeric_limits<DoubleStack::size_type>::max(),
            std::numeric_limits<double>::max()
            );

    DoubleStack::const_iterator it(
        min_element(this->_stack.begin(), this->_stack.end())
        );

    return LocPair(
//      static_cast<DoubleStack::size_type>(distance(this->_stack.begin(), it)),
        static_cast<DoubleStack::size_type>(it - this->_stack.begin()),
        *it
        );
}





/*
================================================================================
Subclass Visible Methods
================================================================================
*/


void
MetricTracker::PruneTheStack(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(this->_stack.size() > this->_maxDepth)
    {
        // prune the stack to the proper size.
        size_t nerase = this->_stack.size()-this->_maxDepth;
        this->_stack.erase(this->_stack.begin(), this->_stack.begin() + nerase);
    }
}






/*
================================================================================
Subclass Overridable Methods
================================================================================
*/








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


MetricTracker::MetricTracker(
    ) :
        _maxDepth(10)
{
    EDDY_FUNC_DEBUGSCOPE
}

MetricTracker::MetricTracker(
    const MetricTracker& copy
    ) :
        _maxDepth(copy._maxDepth)
{
    EDDY_FUNC_DEBUGSCOPE
}

MetricTracker::MetricTracker(
    size_t maxDepth
    ) :
        _maxDepth(maxDepth)
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
