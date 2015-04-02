/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class Design.

    NOTES:

        See notes of Design.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 15 11:04:05 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the Design class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <../Utilities/include/DesignTarget.hpp>








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {


/*
================================================================================
Nested Utility Class Implementations
================================================================================
*/

inline
bool
Design::AttributePred::operator ()(
    const Design& des
    ) const
{
    return operator ()(&des);
}

inline
bool
Design::AttributePred::operator ()(
    const Design* des
    ) const
{
    return des->HasAttributes(this->_attributes) == this->_hasOrNot;
}

inline
Design::AttributePred::AttributePred(
    const AttributeMask& attrs,
    bool hasOrNot
    ) :
        _attributes(attrs),
        _hasOrNot(hasOrNot)
{
}

inline
Design::AttributePred::AttributePred(
    std::size_t attrIndex,
    bool hasOrNot
    ) :
        _attributes(1 << attrIndex),
        _hasOrNot(hasOrNot)
{
}

template <typename BoolFuncPredT>
inline
bool
Design::BoolFuncRetValPred<BoolFuncPredT>::operator ()(
    const Design& des
    ) const
{
    return this->_func(des) == this->_evaluatesTo;
}

template <typename BoolFuncPredT>
inline
bool
Design::BoolFuncRetValPred<BoolFuncPredT>::operator ()(
    const Design* des
    ) const
{
    return operator ()(*des);
}

template <typename BoolFuncPredT>
inline
Design::BoolFuncRetValPred<BoolFuncPredT>::BoolFuncRetValPred(
    const BoolFuncPredT& func,
    bool evaluatesTo
    ) :
        _func(func),
        _evaluatesTo(evaluatesTo)
{
}



/*
================================================================================
Inline Mutators
================================================================================
*/
inline
void
Design::SetNextClone(
    Design* clone
    )
{
    this->_nClone = clone;
}

inline
void
Design::SetPreviousClone(
    Design* clone
    )
{
    this->_pClone = clone;
}

inline
void
Design::SetRawAttributes(
    const AttributeMask& attr
    )
{
    this->_attributes = attr;
}

inline
void
Design::SetObjective(
    std::size_t num,
    double val
    )
{
    this->_objectives[num] = val;
}

inline
void
Design::SetConstraint(
    std::size_t num,
    double val
    )
{
    this->_constraints[num] = val;
}

inline
void
Design::SetVariableRep(
    std::size_t num,
    double rep
    )
{
    this->_variables[num] = rep;
}

void
Design::SetVariableValue(
    std::size_t num,
    double val
    )
{
    this->_variables[num] = this->_target.GetDesignVariableInfos(
        )[num]->GetDoubleRepOf(val);
}

inline
void
Design::SetTag(
    void* tag
    )
{
    this->_tag = tag;
}




/*
================================================================================
Inline Accessors
================================================================================
*/
inline
std::size_t
Design::GetID(
    ) const
{
    return this->_id;
}

inline
const Design::AttributeMask&
Design::GetRawAttributes(
    ) const
{
    return this->_attributes;
}

inline
DesignTarget&
Design::GetDesignTarget(
    ) const
{
    return this->_target;
}

inline
Design*
Design::GetNextClone(
    ) const
{
    return this->_nClone;
}

inline
Design*
Design::GetPreviousClone(
    ) const
{
    return this->_pClone;
}

inline
double
Design::GetVariableRep(
    std::size_t num
    ) const
{
    return this->_variables[num];
}

inline
double
Design::GetObjective(
    std::size_t num
    ) const
{
    return this->_objectives[num];
}

inline
double
Design::GetConstraint(
    std::size_t num
    ) const
{
    return this->_constraints[num];
}

inline
size_t
Design::GetNDV(
    ) const
{
    return this->_target.GetNDV();

} // Design::GetNDV

inline
size_t
Design::GetNOF(
    ) const
{
    return this->_target.GetNOF();

} // Design::GetNOF

inline
size_t
Design::GetNCN(
    ) const
{
    return this->_target.GetNCN();

} // Design::GetNCN

double
Design::GetVariableValue(
    std::size_t num
    ) const
{
    return this->_target.GetDesignVariableInfos()[num]->GetValueOf(
        this->_variables[num]
        );
}

inline
void*
Design::GetTag(
    ) const
{
    return this->_tag;
}

/*
================================================================================
Inline Public Methods
================================================================================
*/
inline
bool
Design::HasAttributes(
    const AttributeMask& attributes
    ) const
{
    return this->_attributes.contains(attributes);
}

inline
bool
Design::HasAttribute(
    std::size_t attribute_index
    ) const
{
    return this->_attributes.get_bit(attribute_index);
}

inline
bool
Design::IsEvaluated(
    ) const
{
    return this->HasAttribute(Evaluated);
}

inline
bool
Design::IsFeasible(
    ) const
{
    return this->IsEvaluated() &&
           !this->IsIllconditioned() &&
           this->HasAttribute(FeasibleConstraints) &&
           this->HasAttribute(FeasibleBounds);
}

inline
bool
Design::SatisfiesConstraints(
    ) const
{
    return this->IsEvaluated() &&
           !this->IsIllconditioned() &&
           this->HasAttribute(FeasibleConstraints);
}

inline
bool
Design::SatisfiesBounds(
    ) const
{
    return this->IsEvaluated() &&
           !this->IsIllconditioned() &&
           this->HasAttribute(FeasibleBounds);
}

inline
bool
Design::IsIllconditioned(
    ) const
{
    return this->HasAttribute(Illconditioned);
}

inline
bool
Design::IsCloned(
    ) const
{
    return (this->_pClone != 0x0 || this->_nClone != 0x0);
}

inline
void
Design::ModifyAttributes(
    const AttributeMask& attributes,
    bool on
    )
{
    on ? (this->_attributes |= attributes) : (this->_attributes &= ~attributes);
}

inline
void
Design::ModifyAttribute(
    std::size_t attribute_index,
    bool on
    )
{
    this->_attributes.set_bit(attribute_index, on);
}

inline
void
Design::SetEvaluated(
    bool lval
    )
{
    this->ModifyAttribute(Evaluated, lval);
}

inline
void
Design::SetSatisfiesBounds(
    bool lval
    )
{
    this->ModifyAttribute(FeasibleBounds, lval);
}

inline
void
Design::SetSatisfiesConstraints(
    bool lval
    )
{
    this->ModifyAttribute(FeasibleConstraints, lval);
}

inline
void
Design::SetIllconditioned(
    bool lval
    )
{
    this->ModifyAttribute(Illconditioned, lval);
}

inline
void
Design::ResetAttributes(
    )
{
    this->SetRawAttributes(0);
}

inline
void
Design::SetDisposeCallback(
    Design::DisposeCallback dispCbk
    )
{
    DISP_CBK = dispCbk;
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
inline
std::size_t
Design::ResetID(
    )
{
    return this->_id = NextID();
}

inline
std::size_t
Design::NextID(
    )
{
    EDDY_SCOPEDLOCK(l, ID_MUTEX)
    return NEXT_ID++;
}








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
