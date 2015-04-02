/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class MaxDesignsNichePressureApplicator.

    NOTES:

        See notes of MaxDesignsNichePressureApplicator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.7.0

    CHANGES:

        Wed Dec 21 16:25:44 2011 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the MaxDesignsNichePressureApplicator
 *        class.
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
const JEGA::DoubleVector&
MaxDesignsNichePressureApplicator::GetDistancePercentages(
    ) const
{
    return this->_distPcts;
}

inline
const std::size_t&
MaxDesignsNichePressureApplicator::GetMaximumDesigns(
    ) const
{
    return this->_maxDesigns;
}

inline
double
MaxDesignsNichePressureApplicator::GetDistancePercentage(
    std::size_t of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(of < this->GetDesignTarget().GetNOF());
    return this->_distPcts[static_cast<JEGA::DoubleVector::size_type>(of)];
}







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

