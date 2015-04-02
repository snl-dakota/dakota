/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class LayerFitnessAssessor.

    NOTES:

        See notes of LayerFitnessAssessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 24 14:32:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the LayerFitnessAssessor class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <FitnessRecord.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignValueMap.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../MOGA/include/FitnessAssessors/LayerFitnessAssessor.hpp>



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
LayerFitnessAssessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("layer_rank");
    return ret;
}

const string&
LayerFitnessAssessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This fitness assessor computes fitness according to the algorithm "
        "proposed in:\n\n"
        "Goldberg, D.E., Korb, B., and Deb, K. (1989). \"Messy Genetic "
        "Algorithms: Motivation, Analysis, and First Results\". Complex "
        "Systems, 3:493-530\n\nAs presented in:\n\n"
        "Coello Coello, C.A., Van Veldhuizen, D.A., and Lamont, G.B., "
        "\"Evolutionary Algorithms for Solving Multi-Objective Problems\", "
        " Kluwer Academic/PlenumPublishers, New York, NY., 2002.\n\n"
        "In the above references, the idea of rank is akin to a "
        "MultiObjectiveStatistician's idea of layer."
        );
    return ret;
}

GeneticAlgorithmOperator*
LayerFitnessAssessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new LayerFitnessAssessor(algorithm);
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
LayerFitnessAssessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return LayerFitnessAssessor::Name();
}

string
LayerFitnessAssessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return LayerFitnessAssessor::Description();
}

GeneticAlgorithmOperator*
LayerFitnessAssessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new LayerFitnessAssessor(*this, algorithm);
}


const FitnessRecord*
LayerFitnessAssessor::AssessFitness(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(GetLogger(), ldebug(), this,
        text_entry(ldebug(), GetName() + ": Assessing fitness.")
        )

    size_t gSize = groups.GetTotalDesignCount();

    // prepare our resulting fitness record.
    FitnessRecord* ret = new FitnessRecord(gSize);

    // look for the abort conditions.
    if(gSize == 0) return ret;

    typedef eddy::utilities::uint64_t ui64;

    // We'll use a MultiObjectiveStatistician to compute the layers.
    DesignValueMap<ui64> layers(
        MultiObjectiveStatistician::ComputeLayers(
            DesignStatistician::CollectDesignsByOF(groups)
            )
        );

    // Now go through and fill up our basic fitness record.  Recall that
    // the higher fitnesses are considered better so we will negate our
    // counts to make smaller better.
    for(DesignValueMap<ui64>::const_iterator it(layers.begin());
        it!=layers.end(); ++it)
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



LayerFitnessAssessor::LayerFitnessAssessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

LayerFitnessAssessor::LayerFitnessAssessor(
    const LayerFitnessAssessor& copy
    ) :
        GeneticAlgorithmFitnessAssessor(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

LayerFitnessAssessor::LayerFitnessAssessor(
    const LayerFitnessAssessor& copy,
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
