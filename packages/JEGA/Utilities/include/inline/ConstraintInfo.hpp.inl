/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class ConstraintInfo.

    NOTES:

        See notes of ConstraintInfo.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Jun 09 16:16:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the ConstraintInfo class.
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
ConstraintTypeBase&
ConstraintInfo::GetType(
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
ConstraintInfo::Which(
   const Design& des
   ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return des.GetConstraint(this->GetNumber());
}

inline
double
ConstraintInfo::GetViolationAmount(
    const Design& of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetViolationAmount(this->Which(of));
}

inline
double
ConstraintInfo::GetViolationAmount(
    double of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetViolationAmount(of);
}

inline
double
ConstraintInfo::GetViolationPercentage(
    const Design& of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetViolationPercentage(this->Which(of));
}


inline
double
ConstraintInfo::GetPreferredAmount(
    const Design& des1,
    const Design& des2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetPreferredAmount(des1, des2);
}

inline
std::string
ConstraintInfo::GetTypeString() const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().ToString();
}

inline
std::string
ConstraintInfo::GetNatureString() const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetNatureString();
}

inline
bool
ConstraintInfo::SetNature(
    ConstraintNatureBase* nature
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().SetNature(nature);
}

inline
ConstraintNatureBase&
ConstraintInfo::GetNature() const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetNature();
}

inline
void
ConstraintInfo::RecordViolation(
    const Design& of
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->RecordViolation(this->Which(of));
}

inline
bool
ConstraintInfo::EvaluateConstraint(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().EvaluateConstraint(des);
}

inline
std::string
ConstraintInfo::GetEquation(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetEquation();
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
