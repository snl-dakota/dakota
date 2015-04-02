/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DominationCountFitnessAssessor.

    NOTES:

        See notes of DominationCountFitnessAssessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Dec 22 09:49:30 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DominationCountFitnessAssessor
 *        class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <FitnessRecord.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignValueMap.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../MOGA/include/FitnessAssessors/DominationCountFitnessAssessor.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;








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
DominationCountFitnessAssessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("domination_count");
    return ret;
}


const string&
DominationCountFitnessAssessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This fitness assessor assigns the negative of the number of designs "
        "in the passed in groups that dominate the given design as the "
        "fitness."
        );
    return ret;
}


GeneticAlgorithmOperator*
DominationCountFitnessAssessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new DominationCountFitnessAssessor(algorithm);
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
DominationCountFitnessAssessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DominationCountFitnessAssessor::Name();
}

string
DominationCountFitnessAssessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DominationCountFitnessAssessor::Description();
}

GeneticAlgorithmOperator*
DominationCountFitnessAssessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new DominationCountFitnessAssessor(*this, algorithm);
}


const FitnessRecord*
DominationCountFitnessAssessor::AssessFitness(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Assessing fitness.")
        )

    size_t gSize = groups.GetTotalDesignCount();

    // prepare our resulting fitness record.
    FitnessRecord* ret = new FitnessRecord(gSize);

    // look for the abort conditions.
    if(gSize == 0) return ret;

    // Now use a MultiObjectiveStatistician to compute the domination counts.
    DesignCountMap res(
        MultiObjectiveStatistician::ComputeDominatedByCounts(
            DesignStatistician::CollectDesignsByOF(groups)
            )
        );

    // Now go through and fill up our basic fitness record.  Recall that the
    // higher fitnesses are considered better so we will negate our counts to
    // make smaller better.
    for(DesignCountMap::const_iterator it(res.begin()); it!=res.end(); ++it)
        ret->AddFitness((*it).first, -static_cast<double>((*it).second));

    // finally return our result.
    return ret;

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



DominationCountFitnessAssessor::DominationCountFitnessAssessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

DominationCountFitnessAssessor::DominationCountFitnessAssessor(
    const DominationCountFitnessAssessor& copy
    ) :
        GeneticAlgorithmFitnessAssessor(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

DominationCountFitnessAssessor::DominationCountFitnessAssessor(
    const DominationCountFitnessAssessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(copy, algorithm)
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

