/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignStatistician.

    NOTES:

        See notes of DesignStatistician.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 07:45:06 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DesignStatistician class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <cfloat>
#include <limits>
#include <algorithm>
#include <utilities/include/extremes.hpp>








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








/*
================================================================================
Inline Public Methods
================================================================================
*/

#define SEPARATE_WHOLE_METHOD(negtag, propname) \
    template <typename Set_T> \
    inline \
    Set_T \
    DesignStatistician::Separate##negtag##propname( \
        Set_T& from \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Separate##negtag##propname<Set_T>( \
            from, from.begin(), from.end() \
            ); \
    }

#define SEPARATE_REGION_METHOD(negtag, propname, truth) \
    template <typename Set_T> \
    inline \
    Set_T \
    DesignStatistician::Separate##negtag##propname( \
        Set_T& from, \
        const typename Set_T::iterator& first, \
        const typename Set_T::iterator& end \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return SeparateDesigns< \
            Set_T, Set_T, Design::BoolFuncRetValPred<Design::Is##propname##Pred> \
            >( \
                from, first, end, \
                Design::BoolFuncRetValPred<Design::Is##propname##Pred>( \
                    Design::Is##propname##Pred(), truth \
                    ) \
                ); \
    }

#define SEPARATE_WHOLE_DIFF_RET_METHOD(negtag, propname) \
    template <typename FromSet_T, typename IntoSet_T> \
    inline \
    IntoSet_T \
    DesignStatistician::Separate##negtag##propname( \
        FromSet_T& from \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Separate##propname<FromSet_T, IntoSet_T>(from); \
    }

#define SEPARATE_REGION_DIFF_RET_METHOD(negtag, propname, truth) \
    template <typename FromSet_T, typename IntoSet_T> \
    inline \
    IntoSet_T \
    DesignStatistician::Separate##negtag##propname( \
        FromSet_T& from, \
        const typename FromSet_T::iterator& first, \
        const typename FromSet_T::iterator& end \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return SeparateDesigns< \
            FromSet_T, IntoSet_T, \
            Design::BoolFuncRetValPred<Design::Is##propname##Pred> \
            >( \
                from, first, end, \
                Design::BoolFuncRetValPred<Design::Is##propname##Pred>( \
                    Design::Is##propname##Pred(), truth \
                    ) \
                ); \
    }

#define FULL_SEPARATE_SUITE(propname) \
    SEPARATE_WHOLE_METHOD(,propname) \
    SEPARATE_REGION_METHOD(,propname, true) \
    SEPARATE_WHOLE_DIFF_RET_METHOD(,propname) \
    SEPARATE_REGION_DIFF_RET_METHOD(,propname, true) \
    SEPARATE_WHOLE_METHOD(Non, propname) \
    SEPARATE_REGION_METHOD(Non, propname, false) \
    SEPARATE_WHOLE_DIFF_RET_METHOD(Non, propname) \
    SEPARATE_REGION_DIFF_RET_METHOD(Non, propname, false)

FULL_SEPARATE_SUITE(Feasible)
FULL_SEPARATE_SUITE(Evaluated)
FULL_SEPARATE_SUITE(Illconditioned)
FULL_SEPARATE_SUITE(Cloned)

#undef FULL_SEPARATE_SUITE
#undef SEPARATE_REGION_DIFF_RET_METHOD
#undef SEPARATE_WHOLE_DIFF_RET_METHOD
#undef SEPARATE_REGION_METHOD
#undef SEPARATE_WHOLE_METHOD


#define GET_WHOLE_METHOD(negtag, propname) \
    template <typename Set_T> \
    inline \
    Set_T \
    DesignStatistician::Get##negtag##propname( \
        const Set_T& from \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Get##negtag##propname<Set_T>(from.begin(), from.end()); \
    } \

#define GET_REGION_METHOD(negtag, propname, truth) \
    template <typename Set_T> \
    inline \
    Set_T \
    DesignStatistician::Get##negtag##propname( \
        typename Set_T::const_iterator first, \
        const typename Set_T::const_iterator& end \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return GetDesigns< \
            typename Set_T::const_iterator, Set_T, \
            Design::BoolFuncRetValPred<Design::Is##propname##Pred> \
            > \
            ( \
                first, end, \
                Design::BoolFuncRetValPred<Design::Is##propname##Pred>( \
                    Design::Is##propname##Pred(), truth \
                    ) \
            ); \
    } \

#define GET_WHOLE_DIFF_RET_METHOD(negtag, propname) \
    template <typename FromSet_T, typename IntoSet_T> \
    inline \
    IntoSet_T \
    DesignStatistician::Get##negtag##propname( \
        const FromSet_T& from \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Get##negtag##propname< \
            typename FromSet_T::const_iterator, IntoSet_T \
            >(from.begin(), from.end()); \
    }

#define GET_REGION_DIFF_RET_METHOD(negtag, propname, truth) \
    template <typename FwdIt, typename IntoSet_T> \
    inline \
    IntoSet_T \
    DesignStatistician::Get##negtag##propname( \
        FwdIt first, \
        const FwdIt& end \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return GetDesigns< \
            FwdIt, IntoSet_T, \
            Design::BoolFuncRetValPred<Design::Is##propname##Pred> \
            >( \
                first, end, \
                Design::BoolFuncRetValPred<Design::Is##propname##Pred>( \
                    Design::Is##propname##Pred(), true \
                    ) \
            ); \
    }

#define FULL_GET_SUITE(propname) \
    GET_WHOLE_METHOD(,propname) \
    GET_REGION_METHOD(,propname, true) \
    GET_WHOLE_DIFF_RET_METHOD(,propname) \
    GET_REGION_DIFF_RET_METHOD(,propname, true) \
    GET_WHOLE_METHOD(Non, propname) \
    GET_REGION_METHOD(Non, propname, false) \
    GET_WHOLE_DIFF_RET_METHOD(Non, propname) \
    GET_REGION_DIFF_RET_METHOD(Non, propname, false)

FULL_GET_SUITE(Feasible)
FULL_GET_SUITE(Evaluated)
FULL_GET_SUITE(Illconditioned)
FULL_GET_SUITE(Cloned)

#undef FULL_GET_SUITE
#undef GET_REGION_DIFF_RET_METHOD
#undef GET_WHOLE_DIFF_RET_METHOD
#undef GET_REGION_METHOD
#undef GET_WHOLE_METHOD

#define COUNT_WHOLE_METHOD(negtag, propname) \
    template <typename Set_T> \
    inline \
    std::size_t \
    DesignStatistician::Count##negtag##propname( \
        const Set_T& from \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Count##negtag##propname<typename Set_T::const_iterator>( \
            from.begin(), from.end() \
            ); \
    } \

#define COUNT_REGION_METHOD(negtag, propname, truth) \
    template <typename FwdIt> \
    inline \
    std::size_t \
    DesignStatistician::Count##negtag##propname( \
        FwdIt first, \
        const FwdIt& end \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return CountDesigns< \
            FwdIt, Design::BoolFuncRetValPred<Design::Is##propname##Pred> \
            >( \
                first, end, \
                Design::BoolFuncRetValPred<Design::Is##propname##Pred>( \
                    Design::Is##propname##Pred(), truth \
                ) \
            ); \
    } \

#define FULL_COUNT_SUITE(propname) \
    COUNT_WHOLE_METHOD(,propname) \
    COUNT_REGION_METHOD(,propname, true) \
    COUNT_WHOLE_METHOD(Non, propname) \
    COUNT_REGION_METHOD(Non, propname, false)

FULL_COUNT_SUITE(Feasible)
FULL_COUNT_SUITE(Evaluated)
FULL_COUNT_SUITE(Illconditioned)
FULL_COUNT_SUITE(Cloned)

#undef FULL_COUNT_SUITE
#undef COUNT_REGION_METHOD
#undef COUNT_WHOLE_METHOD


#define FIND_WHOLE_METHOD(negtag, propname) \
    template <typename Set_T> \
    inline \
    typename Set_T::const_iterator \
    DesignStatistician::Find##negtag##propname( \
        const Set_T& in \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Find##negtag##propname<typename Set_T::const_iterator>( \
            in.begin(), in.end() \
            ); \
    } \

#define FIND_REGION_METHOD(negtag, propname, truth) \
    template <typename FwdIt> \
    inline \
    FwdIt \
    DesignStatistician::Find##negtag##propname( \
        FwdIt first, \
        const FwdIt& end \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return std::find_if< \
            FwdIt, Design::BoolFuncRetValPred<Design::Is##propname##Pred> \
            > ( \
                first, end, \
                Design::BoolFuncRetValPred<Design::Is##propname##Pred>( \
                    Design::Is##propname##Pred(), truth \
                    ) \
                ); \
    } \

#define FULL_FIND_SUITE(propname) \
    FIND_WHOLE_METHOD(,propname) \
    FIND_REGION_METHOD(,propname, true) \
    FIND_WHOLE_METHOD(Non, propname) \
    FIND_REGION_METHOD(Non, propname, false)

FULL_FIND_SUITE(Feasible)
FULL_FIND_SUITE(Evaluated)
FULL_FIND_SUITE(Illconditioned)
FULL_FIND_SUITE(Cloned)

#undef FULL_FIND_SUITE
#undef FIND_REGION_METHOD
#undef FIND_WHOLE_METHOD

template <typename FwdIt, typename Pred>
inline
std::size_t
DesignStatistician::CountDesigns(
    FwdIt first,
    const FwdIt& end,
    Pred predicate
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return CountDesigns(first, end, predicate, -1);
}


template <typename DesignContainer>
std::pair<double, std::vector<typename DesignContainer::const_iterator> >
DesignStatistician::FindLeastViolateDesigns(
    const DesignContainer& cont
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to store the lowest value and associated Designs
    double minViol = std::numeric_limits<double>::max();
    std::vector<typename DesignContainer::const_iterator> bests;

    // prepare to iterate the cont
    typename DesignContainer::const_iterator it(cont.begin());
    for(; it!=cont.end(); ++it)
    {
        // ignore designs that are not evalauted.
        if(!(*it)->IsEvaluated()) continue;

        double viol = ComputeTotalPercentageViolation(**it);
        if(viol < minViol)
        {
            minViol = viol;
            bests.assign(1, it);
        }
        else if(viol == minViol) bests.push_back(it);
    }

    return std::make_pair(minViol, bests);

}

template <typename FwdIt, typename Pred>
inline
std::size_t
DesignStatistician::CountDesigns(
    FwdIt first,
    const FwdIt& end,
    Pred predicate,
    eddy::utilities::int32_t cutoff
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return CountIf(first, end, predicate, cutoff);
}

template <typename Set_T>
eddy::utilities::DoubleExtremes
DesignStatistician::GetObjectiveFunctionExtremes(
    const Set_T& from
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If from is emtpy, we cannot return anything sensible.
    if(from.empty()) return eddy::utilities::DoubleExtremes();

    // retrieve the number of objective functions.
    const std::size_t nof = from.front()->GetNOF();

    // prepare a return object.
    eddy::utilities::DoubleExtremes ret(nof, DBL_MAX, -DBL_MAX);

    // Iterate through and find the maxs/mins for each of the objectives.
    typename Set_T::const_iterator it(from.begin());
    const typename Set_T::const_iterator e(from.end());

    // really what we are doing is storing the highest
    // and lowest values in this set for each objective.
    for(; it!=e; ++it)
        for(std::size_t of=0; of<nof; ++of)
            ret.take_if_either(of, (*it)->GetObjective(of));

    return ret;

}

template <typename Set_T>
eddy::utilities::DoubleExtremes
DesignStatistician::GetDesignVariableExtremes(
    const Set_T& from
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If from is emtpy, we cannot return anything sensible.
    if(from.empty()) return eddy::utilities::DoubleExtremes();

    // retrieve the number of objective functions.
    const std::size_t ndv = from.front()->GetNDV();

    // prepare a return object.
    eddy::utilities::DoubleExtremes ret(ndv, DBL_MAX, -DBL_MAX);

    // Iterate through and find the maxs/mins for each of the objectives.
    const typename Set_T::const_iterator e(from.end());

    // really what we are doing is storing the highest
    // and lowest values in this set for each variable.
    for(typename Set_T::const_iterator it(from.begin());
        it!=e; ++it)
            for(std::size_t dv=0; dv<ndv; ++dv)
                ret.take_if_either(dv, (*it)->GetVariableRep(dv));

    return ret;
}





/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/
template <typename FromSet_T, typename IntoSet_T, typename Pred>
IntoSet_T
DesignStatistician::SeparateDesigns(
    FromSet_T& from,
    typename FromSet_T::iterator first,
    const typename FromSet_T::iterator& end,
    Pred predicate
    )
{
    EDDY_FUNC_DEBUGSCOPE

    IntoSet_T ret;

    while(first!=end)
    {
        if(predicate(*first))
        {
            ret.insert(*first);
            from.erase(first++);
        }
        else ++first;
    }

    return ret;
}

template <typename FwdIt, typename IntoSet_T, typename Pred>
IntoSet_T
DesignStatistician::GetDesigns(
    FwdIt first,
    const FwdIt& end,
    Pred predicate
    )
{
    EDDY_FUNC_DEBUGSCOPE

    IntoSet_T ret;
    for(; first!=end; ++first) if(predicate(*first)) ret.insert(*first);
    return ret;
}

template <typename FwdIt, typename Pred>
std::size_t
DesignStatistician::CountIf(
    FwdIt first,
    const FwdIt& end,
    Pred predicate
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return CountIf(first, end, predicate, -1);
}

template <typename FwdIt, typename Pred>
std::size_t
DesignStatistician::CountIf(
    FwdIt first,
    const FwdIt& end,
    Pred predicate,
    eddy::utilities::int32_t cutoff
    )
{
    EDDY_FUNC_DEBUGSCOPE

    std::size_t ret = 0;

    if(cutoff < 0)
    {
        for(; first!=end; ++first) if(predicate(*first)) ++ret;
    }
    else
    {
        std::size_t uc = static_cast<std::size_t>(cutoff);
        for(; first!=end; ++first)
            if(predicate(*first) && (++ret >= uc)) return ret;
    }

    return ret;
}


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
