/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NPointBinaryCrosser.

    NOTES:

        See notes of NPointBinaryCrosser.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 10:12:55 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NPointBinaryCrosser class.
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
#include <../Utilities/include/Design.hpp>
#include <Crossers/NPointBinaryCrosser.hpp>
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
NPointBinaryCrosser::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("multi_point_binary");
    return ret;
}

const string&
NPointBinaryCrosser::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This crosser performs n-point binary crossover.  This is a binary "
        "operation that takes place on the entire genome encoded as a "
        "binary string.\n\n"
        "The rate is used to determine how many members of the "
        "passed in group (population) should be given an oportunity to "
        "participate in a crossover operation.  Each operation "
        "involves 2 members of the passed in group and creates 2 new "
        "designs.  So the number of operations is round(rate*size/2) "
        "where size is the number of designs in the passed in group.\n\n"
        "The binary string is crossed in \"Number of Crossover Points\" "
        "locations (assuming enough exist) which must be at least 1.  The "
        "crossover points are chosen at random.  The operation takes place "
        "as shown below for the case where 2 crossover points are used:\n\n"
        "    P1:   00000 | 00000 | 00000\n"
        "    P2:   11111 | 11111 | 11111\n"
        "          ---------------------\n"
        "    C1:   00000 | 11111 | 00000\n"
        "    C2:   11111 | 00000 | 11111\n"
        );

    return ret;
}

GeneticAlgorithmOperator*
NPointBinaryCrosser::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NPointBinaryCrosser(algorithm);
}

VariableBitLocPair
NPointBinaryCrosser::GetSplitVariable(
    eddy::utilities::uint32_t ofcpt,
    const BitManipulator& maniper
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(ofcpt > 0);
    EDDY_ASSERT(ofcpt < maniper.GetTotalNumOfBits());

    // get the number of design variables.
    const size_t ndv = this->GetDesignTarget().GetNDV();

    // Check to see if "ofcpt" makes sense
    if(ofcpt < 1)
        return make_pair<size_t, eddy::utilities::uint16_t>(0, 0);

    else if(ofcpt >= maniper.GetTotalNumOfBits())
        return VariableBitLocPair(ndv, 0);

    // figure this out by adding bits for design variable
    // encodings until we surpass ofcpt.
    size_t dv = 0;

    // iterate over the number of design variables.
    for(eddy::utilities::uint32_t sum=0; dv<ndv; ++dv)
    {
        // find out how many bits belong to this variable.
        eddy::utilities::uint16_t nbits = maniper.GetNumberOfBits(dv);

        // if we have surpassed the "ofcpt", we know our dv and bit location.
        if((sum+=nbits) > ofcpt)
            return VariableBitLocPair(
                dv, nbits-static_cast<eddy::utilities::uint16_t>(sum-ofcpt)
                );

        // if we are equal to "ofcpt", we have split exactly at the boundary
        // of two variables.  We will consider the split variable to be the
        // next one and the bit location to be 0.
        if(sum == ofcpt)
            return VariableBitLocPair(++dv, 0);
    }

    // if we make it here, there is some problem.
    JEGALOG_II_F(this->GetLogger(), this,
        text_entry(lfatal(), "Crossover bit location is beyond end of genome.")
        )

    return VariableBitLocPair(ndv, 0);
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
NPointBinaryCrosser::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NPointBinaryCrosser::Name();
}

string
NPointBinaryCrosser::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NPointBinaryCrosser::Description();
}

GeneticAlgorithmOperator*
NPointBinaryCrosser::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NPointBinaryCrosser(*this, algorithm);
}

void
NPointBinaryCrosser::Crossover(
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

    // We need a BitManipulator to do the bit operations
    BitManipulator maniper(target);

    // Store the number of design variables for repeated use.
    const size_t ndv = target.GetNDV();

    // get the crossover rate.
    const double rate = this->GetRate();

    // need to know the size of the parent group.
    const DesignDVSortSet::size_type fsize = from.GetSize();

    // compute number of crossover operations that will take place
    const size_t ncross =
        static_cast<size_t>(Math::Round((rate * fsize) / 2.0));

    // if there will be no crossovers, don't continue.
    if(ncross < 1) return;

    // store the highest legitimate crossover point there can be.
    const eddy::utilities::uint32_t hicpt = maniper.GetTotalNumOfBits()-1;

    // The DesignGroup sort containers do not support random access so we will
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

        // Determine the crossover points (which must be unique for this to
        // make sense). Also prepare the crossover point container.
        CrossPointSet cpts(this->GetCrossoverPoints(1, hicpt));

        JEGAIFLOG_CF_II_F(cpts.empty(), this->GetLogger(), this,
            text_entry(lfatal(), this->GetName() + ": The number of crossover "
            "points is 0.  Cannot perform any crossover.")
            )

        // now we go through and find out which variables are actually
        // being split.  Do a raw copy of each unsplit variable and
        // perform parameterized crossover on the split ones.

        // iterate over the crossover points
        CrossPointSet::const_iterator it(cpts.begin());
        const CrossPointSet::const_iterator e(cpts.end());

        // dv keeps track of the current design variable and the "i#" indices
        // will flip to keep track of which parents genetic material goes into
        // which child.
        size_t dv = 0, i1 = 0, i2 = 1;

        for(; it!=e; ++dv)
        {
            // now prepare a list of crossover points at which to cross the
            // current variable.  They must be done in reverse order.
            ReverseCrossPointSet vcpts;

            // To get the elements of vcpts, we need to iterate
            // the global crossover points until we have collected all that
            // specify the same variable as the first one.
            VariableBitLocPair ccv(this->GetSplitVariable(*it, maniper));

            // put in the first value to vcpts.
            vcpts.insert(ccv.second);

            // Store the index of the acted upon variable.
            size_t csv = ccv.first;

            // now iterate until we find a crossover point that does not
            // specify csv
            for(; it!=e; ++it)
            {
                ccv = this->GetSplitVariable(*it, maniper);
                if(ccv.first == csv) vcpts.insert(ccv.second);
                else break;
            }

            // do raw copy up to the split variable.
            for(; dv<csv; ++dv)
            {
                child0->SetVariableRep(dv, prnts[i1]->GetVariableRep(dv));
                child1->SetVariableRep(dv, prnts[i2]->GetVariableRep(dv));
            }

            // Now do the parameterized crossover scheme on the split variable.
            // create the bit extraction operators which will require
            // iteration of the reversed cpts.
            ReverseCrossPointSet::const_iterator rit(vcpts.begin());

            // do the encoding for each parent (note that dv==csv right now).
            eddy::utilities::int64_t p1dj =
                maniper.ConvertToShiftedInt(*prnts[i1], dv);
            eddy::utilities::int64_t p2dj =
                maniper.ConvertToShiftedInt(*prnts[i2], dv);

            // prepare the bit extraction operator.
            eddy::utilities::int64_t p1_op = 0;
            for(; rit!=vcpts.end(); ++rit) p1_op ^= ((1 << (*rit)) - 1);

            // perform the crossovers.
            child0->SetVariableRep(dv,
                maniper.ConvertFromShiftedInt((p1dj&~p1_op)|(p2dj&p1_op), dv)
                );
            child1->SetVariableRep(dv,
                maniper.ConvertFromShiftedInt((p1dj&p1_op)|(p2dj&~p1_op), dv)
                );

            // switch the indices to change which parents stuff goes
            // into which child.
            i1 = (i1 == 1) ? 0 : 1;
            i2 = (i1 == 1) ? 0 : 1;
        }

        // now do raw copy of any remaining variables.
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

NPointBinaryCrosser::NPointBinaryCrosser(
    GeneticAlgorithm& algorithm
    ) :
        NPointCrosserBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointBinaryCrosser::NPointBinaryCrosser(
    const NPointBinaryCrosser& copy
    ) :
        NPointCrosserBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointBinaryCrosser::NPointBinaryCrosser(
    const NPointBinaryCrosser& copy,
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
