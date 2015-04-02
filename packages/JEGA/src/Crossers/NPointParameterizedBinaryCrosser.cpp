/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NPointParameterizedBinaryCrosser.

    NOTES:

        See notes of NPointParameterizedBinaryCrosser.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 10:13:08 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NPointParameterizedBinaryCrosser
 *        class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <limits>
#include <BitManipulator.hpp>
#include <GeneticAlgorithm.hpp>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>
#include <Crossers/NPointParameterizedBinaryCrosser.hpp>

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

void
NPointParameterizedBinaryCrosser::SetNumCrossPoints(
    const IntVector& ncpts
    )
{
    EDDY_FUNC_DEBUGSCOPE

    size_t ndv = this->GetDesignTarget().GetNDV();

    JEGAIFLOG_CF_II(ndv < ncpts.size(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(),
            this->GetName() + ": Received more crossover point counts than "
            "there are design variables.  Extras will be ignored.")
        )

    JEGAIFLOG_CF_II(ndv > ncpts.size() && ncpts.size() != 1, this->GetLogger(),
        lquiet(), this, ostream_entry(lquiet(),
            this->GetName() + ": Received fewer crossover point counts (")
            << ncpts.size() << ") than there are design variables ("
            << ndv << ").  Using the default value of "
            << DEFAULT_NUM_CROSS_PTS << " to fill in."
        )

    JEGAIFLOG_CF_II(ndv > ncpts.size() && ncpts.size() == 1, this->GetLogger(),
        lquiet(), this, ostream_entry(lquiet(),
            this->GetName() + ": Received a single crossover point count for "
            "a ") << ndv << " design variable problem.  Using the supplied "
            "value of " << ncpts[0] << " for all variables."
        )

    this->_numCrossPts = ncpts;

    eddy::utilities::int32_t fill_val = (this->_numCrossPts.size() == 1) ?
        this->_numCrossPts[0] : DEFAULT_NUM_CROSS_PTS;

    this->_numCrossPts.resize(ndv, fill_val);

    // now go through and set each one individually so that they can be checked
    // for legitimacy.
    for(JEGA::IntVector::size_type i=0; i<ndv; ++i)
        this->SetNumCrossPoints(i, this->_numCrossPts[i]);
}

void
NPointParameterizedBinaryCrosser::SetNumCrossPoints(
    eddy::utilities::int32_t ncpts
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->SetNumCrossPoints(IntVector(this->GetDesignTarget().GetNDV(), ncpts));
}

void
NPointParameterizedBinaryCrosser::SetNumCrossPoints(
    size_t dv,
    eddy::utilities::int32_t ncpts
    )
{
    EDDY_FUNC_DEBUGSCOPE

    const DesignTarget& target = this->GetDesignTarget();

    size_t ndv = target.GetNDV();

    // make sure we have enough locations in the cross pt ct vector.
    this->_numCrossPts.resize(ndv, DEFAULT_NUM_CROSS_PTS);

    // now verify the supplied design variable index.
    JEGAIFLOG_CF_II_F(dv >= ndv, this->GetLogger(), this,
        ostream_entry(lfatal(),
            this->GetName() + ": Request to change number of crossover points "
            "for variable with index #") << dv << ".  Valid indices are 0 "
            "through " << (ndv-1) << "."
        )

    // now verify the supplied value.
    JEGAIFLOG_CF_II(ncpts < 1, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Numbers of crossover points must be at least "
                "1 Supplied value of ") << ncpts << " for variable \""
            << target.GetDesignVariableInfos()[dv]->GetLabel()
            << "\" will be replaced by the minimum."
        )

    ncpts = Math::Max<eddy::utilities::int32_t>(1, ncpts);

    this->_numCrossPts[dv] = ncpts;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Number of crossover points for variable \"")
            << target.GetDesignVariableInfos()[dv]->GetLabel()
            << "\" now = " << ncpts << "."
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

const string&
NPointParameterizedBinaryCrosser::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("multi_point_parameterized_binary");
    return ret;
}

const string&
NPointParameterizedBinaryCrosser::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This crosser performs n-point parameterized binary crossover.  "
        "This is a binary operation that takes place on each "
        "variable individually.  Each variable has its own desired number of "
        "crossover locations.\n\n"
        "The rate is used to determine how many members of the "
        "passed in group (population) should be given an oportunity to "
        "participate in a crossover operation.  Each operation "
        "involves 2 members of the passed in group and creates 2 new "
        "designs.  So the number of operations is round(rate*size/2) "
        "where size is the number of designs in the passed in group.\n\n"
        "Each variable of the parent designs is crossed independently "
        "of each other.\n\n"
        "To carry this out, each variable is converted into a binary "
        "representation using a BitManipulator.  The binary strings are "
        "then crossed in the appropriate number of locations "
        "(assuming enough exist) which must be at least 1.  The "
        "crossover points are chosen at random.  The operation takes place "
        "as shown below for the case where 1 crossover point is used for each "
        "variable:\n\n"
        "           dv1     dv2     dv3\n"
        "    P1:   000|00  00|000  0000|0\n"
        "    P2:   111|11  11|111  1111|1\n"
        "          ------  ------  ------   ....\n"
        "    C1:   000|11  00|111  0000|1\n"
        "    C2:   111|00  11|000  1111|0\n"
        );

    return ret;
}

GeneticAlgorithmOperator*
NPointParameterizedBinaryCrosser::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NPointParameterizedBinaryCrosser(algorithm);
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
NPointParameterizedBinaryCrosser::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NPointParameterizedBinaryCrosser::Name();
}

string
NPointParameterizedBinaryCrosser::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NPointParameterizedBinaryCrosser::Description();
}

GeneticAlgorithmOperator*
NPointParameterizedBinaryCrosser::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NPointParameterizedBinaryCrosser(*this, algorithm);
}

void
NPointParameterizedBinaryCrosser::Crossover(
    const DesignGroup& from,
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_numCrossPts.size() > 0);

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering crossover routine.")
        )

    // check for trivial abort conditions
    if(from.IsEmpty()) return;

    // store the design target for repeated use
    DesignTarget& target = from.GetDesignTarget();
    EDDY_ASSERT(&target == &into.GetDesignTarget());

    // Prepare a shortcut name for the vector of design variable infos.
    typedef DesignVariableInfoVector DVIV;

    // Now store the DV infos for repeated use.
    const DVIV& dvinfos = target.GetDesignVariableInfos();

    // We need a BitManipulator to do the bit operations
    BitManipulator maniper(target);

    // get the crossover rate.
    const double rate = this->GetRate();

    // need to know the size of the parent group.
    const DesignDVSortSet::size_type fsize = from.GetSize();

    // compute number of crossover operations that will take place
    const size_t ncross =
        static_cast<size_t>(Math::Round((rate * fsize) / 2.0));

    // if there will be no crossovers, don't continue.
    if(ncross < 1) return;

    // The group sort containers do not support random access so we will
    // do a-priori selection of the parents for the crossover operations.
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

        // do the uniform parameterized crossover.
        for(DVIV::const_iterator it(dvinfos.begin()); it!=dvinfos.end(); ++it)
        {
            // store the dv for repeated use.
            const size_t dv = (*it)->GetNumber();

            // store the number of bits for this dv.
            const eddy::utilities::uint16_t nbits =
                maniper.GetNumberOfBits(dv);

            // figure out if this dv can be crossed like this at all.
            JEGAIFLOG_CF_II_F(nbits < 2, this->GetLogger(), this,
                ostream_entry(lfatal(), this->GetName() + ": Minimum number of "
                "bits-per-dv required to perform crossover is 2.  Design "
                "variable \"" + (*it)->GetLabel() + "\" uses only ")
                << nbits << " bit(s) for representation.  Cannot continue.  "
                "You must either choose a different crossover scheme or "
                "modify your problem."
                )

            // do the encoding for each parent.
            eddy::utilities::int64_t p1dj =
                maniper.ConvertToShiftedInt(*prnts[0], dv);
            eddy::utilities::int64_t p2dj =
                maniper.ConvertToShiftedInt(*prnts[1], dv);

            // Get the crossover points
            ReverseCrossPointSet cpts(CreateReverseCrossoverPointSet(
                1, nbits-1, this->_numCrossPts[dv]
                ));

            // create the bit extraction operators which will require
            // iteration of the reversed cpts.
            ReverseCrossPointSet::const_iterator rit(cpts.begin());

            // prepare the bit extraction operator.
            eddy::utilities::int64_t p1_op = 0;
            for(; rit!=cpts.end(); ++rit) p1_op ^= ((1 << (*rit)) - 1);

            // perform the crossovers.
            child0->SetVariableRep(dv,
                maniper.ConvertFromShiftedInt((p1dj&p1_op)|(p2dj&~p1_op), dv));
            child1->SetVariableRep(dv,
                maniper.ConvertFromShiftedInt((p1dj&~p1_op)|(p2dj&p1_op), dv));
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
NPointParameterizedBinaryCrosser::PollForParameters(
    const Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetIntVectorFromDB(
        db, "method.jega.num_cross_points", this->_numCrossPts
        );

    // If we did not find the number of crossover points, warn about it and use
    // the default value.  Note that if !success, then _numCrossPts has not
    // been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The number of crossover "
            "points for each variable were not found in the parameter "
            "database.  Checking for a single value.")
        )

    bool ret = this->NPointCrosserBase::PollForParameters(db);

    if(success) this->SetNumCrossPoints(this->_numCrossPts);
    else this->SetNumCrossPoints(this->NPointCrosserBase::GetNumCrossPoints());

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


NPointParameterizedBinaryCrosser::NPointParameterizedBinaryCrosser(
    GeneticAlgorithm& algorithm
    ) :
        NPointCrosserBase(algorithm),
        _numCrossPts(
            algorithm.GetDesignTarget().GetNDV(),
            NPointCrosserBase::DEFAULT_NUM_CROSS_PTS
            )
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointParameterizedBinaryCrosser::NPointParameterizedBinaryCrosser(
    const NPointParameterizedBinaryCrosser& copy
    ) :
        NPointCrosserBase(copy),
        _numCrossPts(copy._numCrossPts)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointParameterizedBinaryCrosser::NPointParameterizedBinaryCrosser(
    const NPointParameterizedBinaryCrosser& copy,
    GeneticAlgorithm& algorithm
    ) :
        NPointCrosserBase(copy, algorithm),
        _numCrossPts(copy._numCrossPts)
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
