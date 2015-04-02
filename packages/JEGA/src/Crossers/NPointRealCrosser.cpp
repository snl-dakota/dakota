/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NPointRealCrosser.

    NOTES:

        See notes of NPointRealCrosser.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 10:12:43 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NPointRealCrosser class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithm.hpp>
#include <utilities/include/Math.hpp>
#include <Crossers/NPointRealCrosser.hpp>
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
NPointRealCrosser::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("multi_point_real");
    return ret;
}

const string&
NPointRealCrosser::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This crosser performs n-point real crossover.  This is an "
        "operation that takes place on the entire genome encoded as a "
        "set of real numbers.  Each design variable serves as a real "
        "number in the genome\n\n"
        "The rate is used to determine how many members of the "
        "passed in group (population) should be given an oportunity to "
        "participate in a crossover operation.  Each operation "
        "involves 2 members of the passed in group and creates 2 new "
        "designs.  So the number of operations is round(rate*size/2) "
        "where size is the number of designs in the passed in group.\n\n"
        "The genome is crossed in \"Number of Crossover Points\" "
        "locations (assuming enough exist) which must be at least 1.  The "
        "crossover points are chosen at random.  The operation takes place "
        "as shown below for the case where 2 crossover points are used:\n\n"
        "    P1:   DV1 DV2 | DV3 DV4 DV5 | DV6 DV7 DV8\n"
        "    P2:   dv1 dv2 | dv3 dv4 dv5 | dv6 dv7 dv8\n"
        "          -----------------------------------\n"
        "    C1:   DV1 DV2 | dv3 dv4 dv5 | DV6 DV7 DV8\n"
        "    C2:   dv1 dv2 | DV3 DV4 DV5 | dv6 dv7 dv8\n"
        );

    return ret;
}

GeneticAlgorithmOperator*
NPointRealCrosser::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NPointRealCrosser(algorithm);
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
NPointRealCrosser::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NPointRealCrosser::Name();
}

string
NPointRealCrosser::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NPointRealCrosser::Description();
}

GeneticAlgorithmOperator*
NPointRealCrosser::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NPointRealCrosser(*this, algorithm);
}

void
NPointRealCrosser::Crossover(
    const DesignGroup& from,
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->GetNumCrossPoints() > 0);

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering crossover routine.")
        )

    // check for trivial abort conditions
    if(from.IsEmpty()) return;

    // store the design target for repeated use
    DesignTarget& target = from.GetDesignTarget();
    EDDY_ASSERT(&target == &into.GetDesignTarget());

    // get the crossover rate.
    const double rate = this->GetRate();

    // need the number of des vars.
    const size_t ndv = target.GetNDV();

    // need to know the size of the parent group.
    const DesignDVSortSet::size_type fsize = from.GetSize();

    // compute number of crossover operations that will take place
    const size_t ncross =
        static_cast<size_t>(Math::Round((rate * fsize) / 2.0));

    // if there will be no crossovers, don't continue.
    if(ncross < 1) return;

    // The group sort containers do not support random access so we will
    // copy the contents into a temporary container that does for speed.
    const vector<Design*> temp(from.BeginDV(), from.EndDV());

    // perform the crossovers.  store the results in into.
    for(size_t i=0; i<ncross; ++i)
    {
        // select the parents randomly.
        const Design* prnts[2] = {
            temp[
                RandomNumberGenerator::UniformInt<DesignDVSortSet::size_type>(
                    0, fsize-1
                    )
                ],
            temp[
                RandomNumberGenerator::UniformInt<DesignDVSortSet::size_type>(
                    0, fsize-1
                    )
                ]
        };

        // create the new children
        Design* child0 = this->GetAlgorithm().GetNewDesign();
        Design* child1 = this->GetAlgorithm().GetNewDesign();

        // Get the crossover points.
		CrossPointSet cpts(this->GetCrossoverPoints(
			1, static_cast<eddy::utilities::uint32_t>(ndv-1)
			));

        // these indices will flip to keep track of which parents
        // genetic material goes into which child.
        size_t i1 = 0, i2 = 1;

        // iterate over the crossover points
        // start at the first dv
        size_t dv = 0;
        const CrossPointSet::const_iterator e(cpts.end());
        for(CrossPointSet::const_iterator it(cpts.begin()); it!=e; ++it)
        {
            eddy::utilities::uint32_t ncp = (*it);

            // continue until the next crossover point (i.t.o. dv's).
            for(; dv<ncp; ++dv)
            {
                child0->SetVariableRep(dv, prnts[i1]->GetVariableRep(dv));
                child1->SetVariableRep(dv, prnts[i2]->GetVariableRep(dv));
            }

            // switch the indices to change which parents stuff goes
            // into which child.
            swap(i1, i2);
        }

        // We have to now fill in from the last cross point to the end of the
        // genome.  That was not done in the above loop.
        for(; dv<ndv; ++dv)
        {
            child0->SetVariableRep(dv, prnts[i1]->GetVariableRep(dv));
            child1->SetVariableRep(dv, prnts[i2]->GetVariableRep(dv));
        }

        // insert the children into the "into" structure
        into.Insert(child0);
        into.Insert(child1);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ") << into.GetSize()
            << " children created."
        )
}

bool
NPointRealCrosser::CanProduceInvalidVariableValues(
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



NPointRealCrosser::NPointRealCrosser(
    GeneticAlgorithm& algorithm
    ) :
        NPointCrosserBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointRealCrosser::NPointRealCrosser(
    const NPointRealCrosser& copy
    ) :
        NPointCrosserBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointRealCrosser::NPointRealCrosser(
    const NPointRealCrosser& copy,
    GeneticAlgorithm& algorithm
    ) :
        NPointCrosserBase(copy, algorithm)
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
