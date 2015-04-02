/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class UniformOffsetMutator.

    NOTES:

        See notes of UniformOffsetMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jul 10 08:39:30 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the UniformOffsetMutator class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/UniformOffsetMutator.hpp>
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
UniformOffsetMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("offset_uniform");
    return ret;
}

const string&
UniformOffsetMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This mutator does mutation by first randomly selecting a Design.  "
        "It then chooses a random design variable and adds a uniform random "
        "amount to it.  The offset amount is detemined by the offset range "
        "and the total range of the variable being mutated.  The mean is 0.  "
        "The number of mutations is the rate times the size of the group "
        "passed in rounded to the nearest whole number."
        );
    return ret;
}

GeneticAlgorithmOperator*
UniformOffsetMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new UniformOffsetMutator(algorithm);
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
UniformOffsetMutator::GetOffsetAmount(
    const DesignVariableInfo& varInfo
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // compute the actual allowable range
    double halfrange = this->GetOffsetRange()*varInfo.GetDoubleRepRange() / 2;

    // now return a uniform random number in the range [-halfrange, halfrange].
    double ret = RandomNumberGenerator::UniformReal(-halfrange, halfrange);
    for(size_t i=0; i<100 && ret == 0.0; ++i)
        ret = RandomNumberGenerator::UniformReal(-halfrange, halfrange);
    return ret;
}

string
UniformOffsetMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return UniformOffsetMutator::Name();
}

string
UniformOffsetMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return UniformOffsetMutator::Description();
}

GeneticAlgorithmOperator*
UniformOffsetMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new UniformOffsetMutator(*this, algorithm);
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





UniformOffsetMutator::UniformOffsetMutator(
    GeneticAlgorithm& algorithm
    ) :
      OffsetMutatorBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

UniformOffsetMutator::UniformOffsetMutator(
    const UniformOffsetMutator& copy
    ) :
        OffsetMutatorBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

UniformOffsetMutator::UniformOffsetMutator(
    const UniformOffsetMutator& copy,
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
