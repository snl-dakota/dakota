/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class ObjectiveFunctionInfo.

    NOTES:

        See notes of ObjectiveFunctionInfo.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 08 12:19:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the ObjectiveFunctionInfo class.
 */




/*
================================================================================
Includes
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {





/*
================================================================================
Inline Mutators
================================================================================
*/





/*
================================================================================
Inline Accessors
================================================================================
*/




inline
ObjectiveFunctionTypeBase&
ObjectiveFunctionInfo::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return *this->_type;
}







/*
================================================================================
Inline Public Methods
================================================================================
*/
inline
double
ObjectiveFunctionInfo::Which(
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return des.GetObjective(this->GetNumber());
}

inline
double
ObjectiveFunctionInfo::WhichForMinimization(
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetValueForMinimization(this->Which(des));
}

inline
const Design*
ObjectiveFunctionInfo::GetPreferredDesign(
    const Design& des1,
    const Design& des2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    eddy::utilities::int8_t pref =
        this->PreferComp(this->Which(des1), this->Which(des2));
    return (pref == -1) ? &des1 : (pref == 1) ? &des2 : 0x0;
}

inline
double
ObjectiveFunctionInfo::GetPreferredAmount(
    const Design& des1,
    const Design& des2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetPreferredAmount(
        this->Which(des1), this->Which(des2)
        );
}

inline
std::string
ObjectiveFunctionInfo::GetTypeString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().ToString();
}





/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

