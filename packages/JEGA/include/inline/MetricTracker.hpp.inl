/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class MetricTracker.

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
 * \brief Contains the inline methods of the MetricTracker class.
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
std::size_t
MetricTracker::GetStackDepth(
    ) const
{
    return this->_stack.size();
}

inline
std::size_t
MetricTracker::GetStackMaxDepth(
    ) const
{
    return this->_maxDepth;
}

inline
const DoubleStack&
MetricTracker::GetStack(
    ) const
{
    return this->_stack;
}






/*
================================================================================
Inline Public Methods
================================================================================
*/


inline
double
MetricTracker::Top(
    ) const
{
    return this->_stack.back();
}

inline
double
MetricTracker::Bottom(
    ) const
{
    return this->_stack.front();
}

inline
bool
MetricTracker::IsFull(
    ) const
{
    return this->_stack.size() == this->_maxDepth;
}

inline
void
MetricTracker::Push(
    double val
    )
{
    this->_stack.push_back(val);
    this->PruneTheStack();
}

inline
void
MetricTracker::Pop(
    )
{
    this->_stack.pop_back();
}

inline
void
MetricTracker::PopBottom(
    )
{
    this->_stack.erase(this->_stack.begin());
}

inline
double
MetricTracker::GetPercentDifference(
    ) const
{
    return this->GetPercentDifference(0, this->_stack.size()-1);
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
