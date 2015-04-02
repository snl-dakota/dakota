/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class CauchyOffsetMutator.

    NOTES:

        See notes of CauchyOffsetMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jul 10 09:23:33 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the CauchyOffsetMutator class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/CauchyOffsetMutator.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
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




const string&
CauchyOffsetMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("offset_cauchy");
    return ret;
}

const string&
CauchyOffsetMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This mutator does mutation by first randomly selecting a Design.  "
        "It then chooses a random design variable and adds a Cauchy random "
        "amount to it.  The amount has a mean of 0 and a standard deviation "
        "that depends on the offset range.  The offset range is interpreted "
        "as a precentage of the total range of the variable and that amount "
        "is in turn interpreted as the standard deviation.  The number of "
        "mutations is the rate times the size of the group passed in rounded "
        "to the nearest whole number."
        );
    return ret;
}

GeneticAlgorithmOperator*
CauchyOffsetMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new CauchyOffsetMutator(algorithm);
}






/*
================================================================================
Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Subclass Overridable Methods
================================================================================
*/




double
CauchyOffsetMutator::GetOffsetAmount(
    const DesignVariableInfo& varInfo
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // compute the actual std deviation
    double sigma = this->GetOffsetRange()*varInfo.GetDoubleRepRange();

    // now return a gaussian number with mean 0.0 and stddev
    // of sigma.
    double ret = RandomNumberGenerator::CauchyReal(0.0, sigma);
    for(size_t i=0; i<100 && ret == 0.0; ++i)
        ret = RandomNumberGenerator::CauchyReal(0.0, sigma);
    return ret;
}

string
CauchyOffsetMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return CauchyOffsetMutator::Name();
}

string
CauchyOffsetMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return CauchyOffsetMutator::Description();
}

GeneticAlgorithmOperator*
CauchyOffsetMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new CauchyOffsetMutator(*this, algorithm);
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




CauchyOffsetMutator::CauchyOffsetMutator(
    GeneticAlgorithm& algorithm
    ) :
        OffsetMutatorBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

CauchyOffsetMutator::CauchyOffsetMutator(
    const CauchyOffsetMutator& copy
    ) :
        OffsetMutatorBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

CauchyOffsetMutator::CauchyOffsetMutator(
    const CauchyOffsetMutator& copy,
    GeneticAlgorithm& algorithm
    ) :
        OffsetMutatorBase(copy, algorithm)
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
