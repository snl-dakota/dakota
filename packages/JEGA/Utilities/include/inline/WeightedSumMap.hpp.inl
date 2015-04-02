/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class WeightedSumMap.

    NOTES:

        See notes of WeightedSumMap.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Dec 20 12:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the WeightedSumMap class.
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
    namespace Utilities {





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
WeightedSumMap::GetWeights(
    ) const
{
    return this->_weights;
}







/*
================================================================================
Inline Public Methods
================================================================================
*/

inline
double
WeightedSumMap::GetWeightedSum(
    const Design* of
    ) const
{
    return this->GetValue(of);
}

inline
double
WeightedSumMap::GetWeightedSum(
    const Design& of
    ) const
{
    return this->GetValue(of);
}

inline
bool
WeightedSumMap::AddWeightedSum(
    const Design& of,
    double wsum
    )
{
    return this->AddValue(of, wsum);
}

inline
bool
WeightedSumMap::AddWeightedSum(
    const Design* of,
    double wsum
    )
{
    return this->AddValue(of, wsum);
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
inline
WeightedSumMap::WeightedSumMap(
    const JEGA::DoubleVector& weights,
    std::size_t JEGA_IF_BOOST(initSize)
    ) :
        DesignDoubleMap(JEGA_IF_BOOST(initSize)),
        _weights(weights)
{
}

inline
WeightedSumMap::WeightedSumMap(
    const WeightedSumMap& copy
    ) :
        DesignDoubleMap(copy),
        _weights(copy._weights)
{
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

