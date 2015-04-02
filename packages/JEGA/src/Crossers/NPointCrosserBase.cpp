/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NPointCrosserBase.

    NOTES:

        See notes of NPointCrosserBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 17 07:11:01 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NPointCrosserBase class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <limits>
#include <utilities/include/Math.hpp>
#include <Crossers/NPointCrosserBase.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;

using eddy::utilities::Math;
using eddy::utilities::RandomNumberGenerator;








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {




/*
================================================================================
Private Template Method Implementations
================================================================================
*/
template <typename CPSet>
CPSet
NPointCrosserBase::_LoadCrossoverPoints(
    eddy::utilities::uint32_t lo,
    eddy::utilities::uint32_t hi,
    eddy::utilities::uint32_t ncpts
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    CPSet cpts;

    // I can't imagine why ncpts could be 0 but this is not the place
    // to make that assertion.
    if(ncpts == 0) return cpts;

    // ncpts is how many we want.
    // hi-lo+1 is how many we can have.
    // ancpts is how many we will actually use.

    // Figure out what ancpts should be.
    eddy::utilities::uint32_t ancpts =
        Math::Min<eddy::utilities::uint32_t>(ncpts, hi-lo+1);

    // if the actual number of crossover points is equal to the number
    // that we can have at max, then copy them all in and forget the
    // random stuff.
    if(ancpts == (hi-lo+1))
    {
        JEGAIFLOG_CF_II(ncpts > ancpts, this->GetLogger(), lquiet(), this,
            ostream_entry(lquiet(), this->GetName() + ": Unable to use the "
                "requested ") << ncpts << " possible crossover point(s).  "
                "Using maximum possible of " << (hi-lo+1) << '.'
            )

        for(eddy::utilities::uint32_t k=lo; k<=hi; ++k) cpts.insert(k);
    }

    // otherwise, keep pumping in crossover points until a unique
    // set exists (which of course will be sorted because of the
    // use of a set).
    else
    {
        while(cpts.size() < ancpts)
            cpts.insert(RandomNumberGenerator::UniformInt(lo, hi));
    }

    // return our nice set.
    return cpts;

}



/*
================================================================================
Static Member Data Definitions
================================================================================
*/
const eddy::utilities::uint32_t NPointCrosserBase::DEFAULT_NUM_CROSS_PTS(2);







/*
================================================================================
Mutators
================================================================================
*/
void
NPointCrosserBase::SetNumCrossPoints(
    eddy::utilities::uint32_t val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_ncpts = val;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Number of crossover points now = ")
            << this->_ncpts
        )

    JEGAIFLOG_CF_II(_ncpts == 0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": The number of crossover "
        "points is 0.  This may be a problem for some crossers.")
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


NPointCrosserBase::CrossPointSet
NPointCrosserBase::GetCrossoverPoints(
    eddy::utilities::uint32_t lo,
    eddy::utilities::uint32_t hi
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CreateCrossoverPointSet(lo, hi, _ncpts);

}

NPointCrosserBase::ReverseCrossPointSet
NPointCrosserBase::GetReverseCrossoverPoints(
    eddy::utilities::uint32_t lo,
    eddy::utilities::uint32_t hi
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CreateReverseCrossoverPointSet(lo, hi, _ncpts);
}

NPointCrosserBase::CrossPointSet
NPointCrosserBase::CreateCrossoverPointSet(
    eddy::utilities::uint32_t lo,
    eddy::utilities::uint32_t hi,
    eddy::utilities::uint32_t ncpts
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_LoadCrossoverPoints<CrossPointSet>(lo, hi, ncpts);
}

NPointCrosserBase::ReverseCrossPointSet
NPointCrosserBase::CreateReverseCrossoverPointSet(
    eddy::utilities::uint32_t lo,
    eddy::utilities::uint32_t hi,
    eddy::utilities::uint32_t ncpts
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_LoadCrossoverPoints<ReverseCrossPointSet>(lo, hi, ncpts);
}


/*
================================================================================
Subclass Overridable Methods
================================================================================
*/

bool
NPointCrosserBase::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const eddy::utilities::uint32_t maxCPs =
        numeric_limits<eddy::utilities::uint32_t>::max();

    size_t ncptsTmp = this->_ncpts;

    bool success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.jega.num_cross_points", ncptsTmp
        );

    // If we did not find the number of crossover points, warn about it and use
    // the default value.  Note that if !success, then ncptsTmp has not been
    // altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The number of crossover "
            "points was not found in the parameter database.  Using the "
            "current value of ") << ncptsTmp
        )

    JEGAIFLOG_CF_II(ncptsTmp > maxCPs, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Requested number of crossover points (")
            << ncptsTmp << ") is too large.  Max is being used instead ("
            << maxCPs << ":)."
            )

    SetNumCrossPoints(
        static_cast<eddy::utilities::uint32_t>(
            Math::Min<eddy::utilities::uint64_t>(ncptsTmp, maxCPs)
            )
        );

    return this->GeneticAlgorithmCrosser::PollForParameters(db);
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


NPointCrosserBase::NPointCrosserBase(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmCrosser(algorithm),
        _ncpts(DEFAULT_NUM_CROSS_PTS)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointCrosserBase::NPointCrosserBase(
    const NPointCrosserBase& copy
    ) :
        GeneticAlgorithmCrosser(copy),
        _ncpts(copy._ncpts)
{
    EDDY_FUNC_DEBUGSCOPE
}

NPointCrosserBase::NPointCrosserBase(
    const NPointCrosserBase& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmCrosser(copy, algorithm),
        _ncpts(copy._ncpts)
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
