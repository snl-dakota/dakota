/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullEvaluator.

    NOTES:

        See notes of NullEvaluator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:26:48 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullEvaluator class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Evaluators/NullEvaluator.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;








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
NullEvaluator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_evaluation");
    return ret;
}

const string&
NullEvaluator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This evaluator does nothing.  It is intended to serve as "
        "a null object for evaluators.  Use it if you do not wish "
        "to perform any evaluation."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullEvaluator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullEvaluator(algorithm);
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
string
NullEvaluator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return NullEvaluator::Name();
}

string
NullEvaluator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return NullEvaluator::Description();
}

GeneticAlgorithmOperator*
NullEvaluator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return new NullEvaluator(*this, algorithm);
}

bool
NullEvaluator::Evaluate(
    DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this, text_entry(ldebug(),
        this->GetName() + ": in use."))

    this->IncrementNumberEvaluations(group.SizeDV());

    // return as though everything is ok.
    return true;
}

bool
NullEvaluator::Evaluate(
    Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this, text_entry(ldebug(),
        this->GetName() + ": in use."))

    this->IncrementNumberEvaluations();

    // return as though everything is ok.
    return true;
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
NullEvaluator::NullEvaluator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmEvaluator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullEvaluator::NullEvaluator(
    const NullEvaluator& copy
    ) :
        GeneticAlgorithmEvaluator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullEvaluator::NullEvaluator(
    const NullEvaluator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmEvaluator(copy, algorithm)
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
