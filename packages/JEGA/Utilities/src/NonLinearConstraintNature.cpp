/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NonLinearConstraintNature.

    NOTES:

        See notes of NonLinearConstraintNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 10:23:56 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NonLinearConstraintNature class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintNatureBase.hpp>
#include <../Utilities/include/NonLinearConstraintNature.hpp>




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {







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
NonLinearConstraintNature::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Non-Linear");
    return name;

} // NonLinearConstraintNature::GetName





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
NonLinearConstraintNature::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();
}

ConstraintNatureBase*
NonLinearConstraintNature::Clone(
    ConstraintTypeBase& forType
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NonLinearConstraintNature(*this, forType);
}

bool
NonLinearConstraintNature::EvaluateConstraint(
    Design&
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II_G(lquiet(), this,
        text_entry(lquiet(), "Attempt to evaluate a non-linear constraint "
                    "using ConstraintInfo Heirarchy failed.")
        )
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


NonLinearConstraintNature::NonLinearConstraintNature(
    ConstraintTypeBase& type
    ) :
        ConstraintNatureBase(type)
{
    EDDY_FUNC_DEBUGSCOPE
}

NonLinearConstraintNature::NonLinearConstraintNature(
    const NonLinearConstraintNature& copy,
    ConstraintTypeBase& type
    ) :
        ConstraintNatureBase(copy, type)
{
    EDDY_FUNC_DEBUGSCOPE
}

NonLinearConstraintNature::~NonLinearConstraintNature(
    )
{
    EDDY_FUNC_DEBUGSCOPE
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
