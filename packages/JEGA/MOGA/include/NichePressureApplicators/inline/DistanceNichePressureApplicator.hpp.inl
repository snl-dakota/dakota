/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DistanceNichePressureApplicator.

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
 * \brief Contains the inline methods of the DistanceNichePressureApplicator
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
DistanceNichePressureApplicator::GetDistancePercentages(
    ) const
{
    return this->_distPcts;
}

inline
double
DistanceNichePressureApplicator::GetDistancePercentage(
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

