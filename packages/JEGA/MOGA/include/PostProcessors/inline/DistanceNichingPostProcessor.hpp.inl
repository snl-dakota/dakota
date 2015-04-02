/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DistanceNichingPostProcessor.

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
 * \brief Contains the inline methods of the DistanceNichingPostProcessor
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
DistanceNichingPostProcessor::GetDistancePercentages(
    ) const
{
    return this->_distPcts;
}

inline
double
DistanceNichingPostProcessor::GetDistancePercentage(
    std::size_t of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(of < GetDesignTarget().GetNOF());
    return _distPcts.at(static_cast<JEGA::DoubleVector::size_type>(of));
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

