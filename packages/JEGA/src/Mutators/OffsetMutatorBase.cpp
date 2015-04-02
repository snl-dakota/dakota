/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class OffsetMutatorBase.

    NOTES:

        See notes of OffsetMutatorBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jul 10 07:56:01 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the OffsetMutatorBase class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/OffsetMutatorBase.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
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
const double OffsetMutatorBase::DEFAULT_OFFSET_RANGE(0.1);







/*
================================================================================
Mutators
================================================================================
*/
void
OffsetMutatorBase::SetOffsetRange(
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_offsetRange = val;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Offset range now = ") << this->_offsetRange
        )

    JEGAIFLOG_CF_II((this->_offsetRange < 0.0) || (this->_offsetRange > 1.0),
        this->GetLogger(),
        lquiet(), this, text_entry(lquiet(),
            "Offset Mutator Base: Offset range is not in [0, 1].  This may be "
            "a problem for some offset mutators.")
        )
}








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




void
OffsetMutatorBase::Mutate(
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
    const DesignVariableInfoVector& dvis =
      target.GetDesignVariableInfos();

    // Get the mutation rate.
    const double rate = this->GetRate();

    // determine the number of mutation operations;
    const size_t nmutate =
        static_cast<size_t>(Math::Round(rate * pop.GetSize()));

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

        // store the design being changed.
        Design* chosen = **it;

        // perform the mutation.

        // get the old representation to modify it.
        double varrep = chosen->GetVariableRep(dv);

        // get the offset amount
        double offset = this->GetOffsetAmount(*dvis[dv]);

        // if the offset will not change the design, forget about it.
        if(offset == 0.0) continue;

        // otherwise, apply it.
        varrep += offset;

        // Note that there is a strong likelyhood that this will produce invalid
        // variable representations especially for discrete variables.
        // Just a warning.
        JEGAIFLOG_II(!dvis[dv]->IsValidDoubleRep(varrep),
            this->GetLogger(), lquiet(), this,
            ostream_entry(lquiet(), this->GetName() + ": Produced invalid "
                " variable representation ") << varrep << ". Mutation"
                " performed without corrective action."
            )

        chosen = target.GetNewDesign(*chosen);
        chosen->SetEvaluated(false);
        chosen->SetIllconditioned(false);

        // record the new value
        chosen->SetVariableRep(dv, varrep);
        chosen->RemoveAsClone();
        cldrn.Insert(chosen);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ") << nmutate
            << " total mutations."
        )
}

bool
OffsetMutatorBase::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleFromDB(
        db, "method.mutation_scale", this->_offsetRange
        );

    // If we did not find the offset range, warn about it and use the default
    // value.  Note that if !success, then _offsetRange has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The offset range was "
            "not found in the parameter database.  Using the current value of ")
            << this->_offsetRange
        )

    SetOffsetRange(this->_offsetRange);

    return GeneticAlgorithmMutator::PollForParameters(db);
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




OffsetMutatorBase::OffsetMutatorBase(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(algorithm),
        _offsetRange(DEFAULT_OFFSET_RANGE)
{
    EDDY_FUNC_DEBUGSCOPE
}

OffsetMutatorBase::OffsetMutatorBase(
    const OffsetMutatorBase& copy
    ) :
        GeneticAlgorithmMutator(copy),
        _offsetRange(copy._offsetRange)
{
    EDDY_FUNC_DEBUGSCOPE
}

OffsetMutatorBase::OffsetMutatorBase(
    const OffsetMutatorBase& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(copy, algorithm),
        _offsetRange(copy._offsetRange)
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
