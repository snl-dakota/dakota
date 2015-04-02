/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class RadialNichePressureApplicator.

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
 * \brief Contains the inline methods of the RadialNichePressureApplicator
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
RadialNichePressureApplicator::GetRadiusPercentages(
    ) const
{
    return _radPcts;
}

inline
double
RadialNichePressureApplicator::GetRadiusPercentage(
    size_t of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(of < this->GetDesignTarget().GetNOF());
    return this->_radPcts[static_cast<JEGA::DoubleVector::size_type>(of)];
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

