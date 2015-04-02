/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class RandomBitMutator.

    NOTES:

        See notes of RandomBitMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 10:12:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RandomBitMutator class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <BitManipulator.hpp>
#include <GeneticAlgorithm.hpp>
#include <utilities/include/Math.hpp>
#include <Mutators/RandomBitMutator.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
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
RandomBitMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("bit_random");
    return ret;
}

const string&
RandomBitMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This mutator performs mutation by randomly selecting a "
        "design variable and converting it to a binary "
        "representation, then randomly choosing a bit from that "
        "string and toggling it.\n\nThe binary representation is "
        "created using a BitManipulator.  The number of mutations is "
        "the rate times the size of the group passed in rounded "
        "to the nearest whole number."
        );
    return ret;
}

GeneticAlgorithmOperator*
RandomBitMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomBitMutator(algorithm);
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
RandomBitMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomBitMutator::Name();
}

string
RandomBitMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomBitMutator::Description();
}

GeneticAlgorithmOperator*
RandomBitMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomBitMutator(*this, algorithm);
}

void
RandomBitMutator::Mutate(
    DesignGroup& pop,
    DesignGroup& cldrn
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing mutation.")
        )

    // store the design target for repeated use
    const DesignTarget& target = pop.GetDesignTarget();

    // We need a BitManipulator to do the bit operations
    BitManipulator maniper(target);

    // get the number of design variables
    const size_t ndv = target.GetNDV();

    // determine the number of mutation operations;
    const size_t nmutate = static_cast<size_t>(
        Math::Round(this->GetRate() * ndv * pop.GetSize())
        );

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

    // Carry out the mutations
    for(vector<DesignDVSortSet::iterator>::iterator it(toMutate.begin());
        it!=toMutate.end(); ++it)
    {
        // pick a random design variable;
        size_t dv = RandomNumberGenerator::UniformInt<size_t>(0, ndv-1);

        // pick a random bit to mutate
        eddy::utilities::uint16_t bit =
            RandomNumberGenerator::UniformInt<eddy::utilities::uint16_t>(
                0, maniper.GetNumberOfBits(dv)-1
                );

        // store the design being changed.
        Design* chosen = *(*it);

        // perform the mutation.

        // get an encoded version of the chosen Design variable
        eddy::utilities::int64_t dvval =
            maniper.ConvertToShiftedInt(chosen->GetVariableRep(dv), dv);

        // flip the chosen bit.
        dvval = maniper.ToggleBit(bit, dvval);

        // record the new variable value (after decoding it).
        chosen = target.GetNewDesign(*chosen);
        chosen->SetEvaluated(false);
        chosen->SetIllconditioned(false);

        chosen->SetVariableRep(dv, maniper.ConvertFromShiftedInt(dvval, dv));
        chosen->RemoveAsClone();
        cldrn.Insert(chosen);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ") << nmutate
            << " total mutations."
        )
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




RandomBitMutator::RandomBitMutator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomBitMutator::RandomBitMutator(
    const RandomBitMutator& copy
    ) :
        GeneticAlgorithmMutator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomBitMutator::RandomBitMutator(
    const RandomBitMutator& copy,
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
