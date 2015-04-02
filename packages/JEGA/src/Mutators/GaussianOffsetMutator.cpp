/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GaussianOffsetMutator.

    NOTES:

        See notes of GaussianOffsetMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 18 13:39:23 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GaussianOffsetMutator class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/GaussianOffsetMutator.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>




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
GaussianOffsetMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("offset_normal");
    return ret;
}

const string&
GaussianOffsetMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This mutator does mutation by first randomly selecting a Design.  "
        "It then chooses a random design variable and adds a Gaussian random "
        "amount to it.  The amount has a mean of 0 and a standard deviation "
        "that depends on the offset range.  The offset range is interpreted "
        "as a portion of the total range of the variable and that amount is "
        "in turn interpreted as the standard deviation.  The value for offset "
        "range should be in the range [0, 1].  The number of mutations is the "
        "rate times the size of the group passed in rounded to the nearest "
        "whole number."
        );
    return ret;
}

GeneticAlgorithmOperator*
GaussianOffsetMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new GaussianOffsetMutator(algorithm);
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
GaussianOffsetMutator::GetOffsetAmount(
    const DesignVariableInfo& varInfo
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // compute the actual std deviation
    double sigma = this->GetOffsetRange()*varInfo.GetDoubleRepRange();

    // now return a gaussian number with mean 0.0 and stddev
    // of sigma.
    double ret = RandomNumberGenerator::GaussianReal(0.0, sigma);
    for(size_t i=0; i<100 && ret == 0.0; ++i)
        ret = RandomNumberGenerator::GaussianReal(0.0, sigma);
    return ret;
}

string
GaussianOffsetMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GaussianOffsetMutator::Name();
}

string
GaussianOffsetMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GaussianOffsetMutator::Description();
}

GeneticAlgorithmOperator*
GaussianOffsetMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new GaussianOffsetMutator(*this, algorithm);
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


GaussianOffsetMutator::GaussianOffsetMutator(
    GeneticAlgorithm& algorithm
    ) :
      OffsetMutatorBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GaussianOffsetMutator::GaussianOffsetMutator(
    const GaussianOffsetMutator& copy
    ) :
        OffsetMutatorBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

GaussianOffsetMutator::GaussianOffsetMutator(
    const GaussianOffsetMutator& copy,
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
