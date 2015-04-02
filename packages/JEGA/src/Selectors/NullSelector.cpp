/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullSelector.

    NOTES:

        See notes of NullSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:27:40 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullSelector class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Selectors/NullSelector.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>








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
NullSelector::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_selection");
    return ret;
}

const string&
NullSelector::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This selection does nothing.  It is intended to serve as "
        "a null object for selectors.  Use it if you do not wish "
        "to perform any selection."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullSelector::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullSelector(algorithm);
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
NullSelector::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullSelector::Name();
}

string
NullSelector::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullSelector::Description();
}

GeneticAlgorithmOperator*
NullSelector::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullSelector(*this, algorithm);
}


void
NullSelector::Select(
    DesignGroupVector&,
    DesignGroup&,
    size_t,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(GetLogger(), ldebug(), this, text_entry(ldebug(),
        GetName() + ": in use."))
}


DesignOFSortSet
NullSelector::SelectNBest(
    DesignGroupVector& from,
    size_t n,
    const FitnessRecord& ftns
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(ldebug(), this->GetName() + ": identifying the ") <<
        n << " best designs."
        )

    return DesignOFSortSet();
}

bool
NullSelector::CanSelectSameDesignMoreThanOnce(
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
NullSelector::NullSelector(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullSelector::NullSelector(
    const NullSelector& copy
    ) :
        GeneticAlgorithmSelector(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullSelector::NullSelector(
    const NullSelector& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(copy, algorithm)
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
