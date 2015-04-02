/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignTarget.

    NOTES:

        See notes of DesignTarget.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Dec 20 08:11:48 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DesignTarget class.
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
bool
DesignTarget::GetTrackDiscards(
    ) const
{
    return this->_trackDiscards;
}

inline
std::size_t
DesignTarget::GetMaxGuffSize(
    ) const
{
    return this->_maxGuffSize;
}

inline
const DesignVariableInfoVector&
DesignTarget::GetDesignVariableInfos(
    ) const
{
    return this->_dvInfos;
}

inline
const ConstraintInfoVector&
DesignTarget::GetConstraintInfos(
    ) const
{
    return this->_cnInfos;
}

inline
const ObjectiveFunctionInfoVector&
DesignTarget::GetObjectiveFunctionInfos(
    ) const
{
    return this->_ofInfos;
}

inline
std::size_t
DesignTarget::GetNDV(
    ) const
{
    return this->_dvInfos.size();
}

inline
std::size_t
DesignTarget::GetNOF(
    ) const
{
    return this->_ofInfos.size();
}

inline
std::size_t
DesignTarget::GetNCN(
    ) const
{
    return this->_cnInfos.size();
}






/*
================================================================================
Inline Public Methods
================================================================================
*/

template <typename DesCont>
inline
void
DesignTarget::TakeDesigns(
    const DesCont& cont
    )
{
    for(typename DesCont::const_iterator it(cont.begin());
        it!=cont.end(); ++it) TakeDesign(*it);
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

