/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class RandomDesignVariableMutator.

    NOTES:

        See notes of RandomDesignVariableMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 04 12:07:44 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RandomDesignVariableMutator class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Mutators/RandomDesignVariableMutator.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;
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
RandomDesignVariableMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("replace_uniform");
    return ret;
}

const string&
RandomDesignVariableMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This mutator does mutation by first randomly selecting a Design.  "
        "It then chooses a random design variable and reassigns that variable "
        "to a random valid value.\n\nThe number of mutations is the rate "
        "times the size of the group passed in rounded to the nearest whole "
        "number."
        );
    return ret;
}

GeneticAlgorithmOperator*
RandomDesignVariableMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomDesignVariableMutator(algorithm);
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
RandomDesignVariableMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomDesignVariableMutator::Name();
}

string
RandomDesignVariableMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomDesignVariableMutator::Description();
}

GeneticAlgorithmOperator*
RandomDesignVariableMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomDesignVariableMutator(*this, algorithm);
}

void
RandomDesignVariableMutator::Mutate(
    DesignGroup& pop,
    DesignGroup& cldrn
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing mutation.")
        )

    // store the design target for repeated use
    DesignTarget& target = pop.GetDesignTarget();

    // Get the number of design variables per Design
    const size_t ndv = target.GetNDV();

    // extract the design variable information
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

    // determine the number of mutation operations;
    const size_t nmutate =
        static_cast<size_t>(Math::Round(this->GetRate() * pop.GetSize()));

    // if there will be no mutations, don't continue.
    if(nmutate < 1)
    {
        JEGALOG_II(this->GetLogger(), ldebug(), this,
            text_entry(ldebug(), this->GetName() + ": Rate and group size are "
                       "such that no mutation will occur.")
            )
        return;
    }

    // The DesignGroup sort containers do not support random access.  Therefore,
    // for efficiency, we will pre-select the designs to mutate and iterate
    // through to collect them.  Then we will mutate them.
    vector<DesignDVSortSet::iterator> toMutate(
        ChooseDesignsToMutate(nmutate, pop)
        );

    for(vector<DesignDVSortSet::iterator>::iterator it(toMutate.begin());
        it!=toMutate.end(); ++it)
    {
        size_t dv = RandomNumberGenerator::UniformInt<size_t>(0, ndv-1);

        Design* chosen = **it;

        // perform the mutation.
        double oldRep = chosen->GetVariableRep(dv);
        double newRep = dvis[dv]->GetRandomDoubleRep();

        // See to it that the old and new reps are not the same if possible.
        for(int i=0; i<100 && oldRep == newRep; ++i)
            newRep = dvis[dv]->GetRandomDoubleRep();

        // If we could not find a new value different from the old, don't bother
        // with the rest of this loop.
        if(newRep == oldRep) continue;

        chosen = target.GetNewDesign(*chosen);
        chosen->SetEvaluated(false);
        chosen->SetIllconditioned(false);
        chosen->SetVariableRep(dv, dvis[dv]->GetRandomDoubleRep());
        chosen->RemoveAsClone();
        cldrn.Insert(chosen);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ") << nmutate
            << " total mutations."
        )
}

bool
RandomDesignVariableMutator::CanProduceInvalidVariableValues(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
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
RandomDesignVariableMutator::RandomDesignVariableMutator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomDesignVariableMutator::RandomDesignVariableMutator(
    const RandomDesignVariableMutator& copy
    ) :
        GeneticAlgorithmMutator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomDesignVariableMutator::RandomDesignVariableMutator(
    const RandomDesignVariableMutator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(copy, algorithm)
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
