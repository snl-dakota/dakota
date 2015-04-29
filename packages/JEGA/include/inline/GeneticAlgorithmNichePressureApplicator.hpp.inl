/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithmNichePressureApplicator.

    NOTES:

        See notes of GeneticAlgorithmNichePressureApplicator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Jan 05 10:13:06 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the
 *        GeneticAlgorithmNichePressureApplicator class.
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
bool
GeneticAlgorithmNichePressureApplicator::GetCacheDesigns(
    )
{
    return this->_cacheDesigns;
}









/*
================================================================================
Inline Public Methods
================================================================================
*/
inline
bool
GeneticAlgorithmNichePressureApplicator::TestBufferForClone(
    const JEGA::Utilities::Design& ofDes
    )
{
    return this->_cacheDesigns ?
        this->_desBuffer.test_for_clone(
            const_cast<JEGA::Utilities::Design*>(&ofDes)
            ) != this->_desBuffer.end() :
        false;
}

template <typename DesContT>
std::size_t
GeneticAlgorithmNichePressureApplicator::TestBufferForClones(
    const DesContT& against
    )
{
    if(!this->_cacheDesigns) return 0;
    std::size_t cloneCount = 0;
    const typename JEGA::Utilities::DesignDVSortSet::const_iterator le(
        this->_desBuffer.end()
        );
    const typename DesContT::const_iterator e(against.end());
    for(typename DesContT::const_iterator it(against.begin()); it!=e; ++it)
        cloneCount += this->_desBuffer.test_for_clone(*it) == le ? 0 : 1;
    return cloneCount;
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

