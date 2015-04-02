/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class RandomDesignVariableCrosser.

    NOTES:

        See notes of RandomDesignVariableCrosser.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 07:12:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RandomDesignVariableCrosser class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <vector>
#include <GeneticAlgorithm.hpp>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Crossers/RandomDesignVariableCrosser.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
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
const size_t RandomDesignVariableCrosser::DEFAULT_NOCPM(2);
const size_t RandomDesignVariableCrosser::DEFAULT_NOPPM(2);








/*
================================================================================
Mutators
================================================================================
*/
void
RandomDesignVariableCrosser::SetNumChildrenPerMating(
    size_t to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_nocpm = to;

    JEGALOG_II(GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Number of children per mating now = "
            ) << this->_nocpm
        )

    JEGAIFLOG_CF_II(this->_nocpm == 0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": The number of children "
            "per mating is 0.  No crossover will occur.")
        )
}

void
RandomDesignVariableCrosser::SetNumParentsPerMating(
    size_t to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_noppm = to;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Number of parents per mating now = "
            ) << this->_noppm
        )

    JEGAIFLOG_CF_II(this->_noppm == 0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": The number of parents per "
            "mating is 0.  No crossover will occur.")
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
RandomDesignVariableCrosser::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("shuffle_random");
    return ret;
}

const string&
RandomDesignVariableCrosser::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This crosser randomly selects a design variable donor "
        "from among the parents and assigns that variable value "
        "into a child.  In this way, it creates a random shuffling of "
        "the design variables from the parents for each child.  The "
        "number of mutations is the rate times the size of the group "
        "passed in rounded to the nearest whole number."
        );
    return ret;
}

GeneticAlgorithmOperator*
RandomDesignVariableCrosser::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomDesignVariableCrosser(algorithm);
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
RandomDesignVariableCrosser::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomDesignVariableCrosser::Name();
}

string
RandomDesignVariableCrosser::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomDesignVariableCrosser::Description();
}

GeneticAlgorithmOperator*
RandomDesignVariableCrosser::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomDesignVariableCrosser(*this, algorithm);
}

void
RandomDesignVariableCrosser::Crossover(
    const DesignGroup& from,
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering crossover routine.")
        )

    // get the number of children and parents for each mating.
    const size_t nocpm = this->GetNumChildrenPerMating();
    const size_t noppm = this->GetNumParentsPerMating();

    // need to use the size of the "from" group a lot
    // so we will store it here.
    const DesignDVSortSet::size_type fsize = from.GetSize();

    // check for simple abort conditions
    if((noppm <= 0) || (nocpm <= 0) || (fsize == 0)) return;

    // compute number of crossover operations that will take place
    const size_t ncross =
        static_cast<size_t>(Math::Round((this->GetRate() * fsize) / noppm));

    // if there will be no crossovers, don't continue.
    if(ncross < 1) return;

    // store the design target for repeated use
    const DesignTarget& target = from.GetDesignTarget();
    EDDY_ASSERT(&target == &into.GetDesignTarget());

    // need the number of des vars.
    const size_t ndv = target.GetNDV();

    // The group sort containers do not support random access so we will
    // copy the contents into a temporary container that does for speed.
    const vector<Design*> temp(from.BeginDV(), from.EndDV());

    // perform the crossovers.  store the results in "into".
    for(size_t i=0; i<ncross; ++i)
    {
        // prepare to store the parents for each mating
        vector<const Design*> prnts(noppm, 0x0);

        // select the parents by storing pointers to noppm random Designs
        for(size_t j=0; j<noppm; ++j)
            prnts[j] = temp[
                RandomNumberGenerator::UniformInt<DesignDVSortSet::size_type>(
                    0, fsize-1
                    )
                ];

        // use the parents to create the children.
        for(size_t j=0; j<nocpm; ++j)
        {
            // create a new child
            Design* child = this->GetAlgorithm().GetNewDesign();

            // randomly assign the design variables of
            // the child from the parents.
            for(size_t k=0; k<ndv; ++k)
            {
                // randomly select a parent to donate
                const size_t prnt =
                    RandomNumberGenerator::UniformInt<size_t>(0, noppm-1);

                // copy the selected parents genetics into the
                // child being built.
                child->SetVariableRep(k, prnts[prnt]->GetVariableRep(k));
            }

            // insert the child into the "into" structure
            into.Insert(child);
        }
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ") << into.GetSize()
            << " children created."
        )
}

bool
RandomDesignVariableCrosser::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.jega.num_offspring", this->_nocpm
        );

    // If we did not find the nocpm, warn about it and use the default
    // value.  Note that if !success, then _nocpm has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The number of children "
            "per mating was not found in the parameter database.  Using the "
            "current value of ") << this->_nocpm
        )

    this->SetNumChildrenPerMating(this->_nocpm);

    success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.jega.num_parents", this->_noppm
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The number of parents "
            "per mating was not found in the parameter database.  Using the "
            "current value of ") << this->_noppm
        )

    this->SetNumParentsPerMating(this->_noppm);

    return this->GeneticAlgorithmCrosser::PollForParameters(db);
}

bool
RandomDesignVariableCrosser::CanProduceInvalidVariableValues(
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

RandomDesignVariableCrosser::RandomDesignVariableCrosser(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmCrosser(algorithm),
        _nocpm(DEFAULT_NOCPM),
        _noppm(DEFAULT_NOPPM)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomDesignVariableCrosser::RandomDesignVariableCrosser(
    const RandomDesignVariableCrosser& copy
    ) :
        GeneticAlgorithmCrosser(copy),
        _nocpm(copy._nocpm),
        _noppm(copy._noppm)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomDesignVariableCrosser::RandomDesignVariableCrosser(
    const RandomDesignVariableCrosser& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmCrosser(copy, algorithm),
        _nocpm(copy._nocpm),
        _noppm(copy._noppm)
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
