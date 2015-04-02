/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignDoubleValueMap.

    NOTES:

        See notes of DesignDoubleValueMap.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Dec 20 12:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DesignDoubleValueMap class.
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
Static Member Initialization
================================================================================
*/
template <typename VT>
const typename DesignDoubleValueMap<VT>::value_type
    DesignDoubleValueMap<VT>::MAX_POSSIBLE =
        DesignDoubleValueMap<VT>::my_limits::max();

template <typename VT>
const typename DesignDoubleValueMap<VT>::value_type
    DesignDoubleValueMap<VT>::MIN_POSSIBLE =
        DesignDoubleValueMap<VT>::my_limits::smallest();


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

template <typename VT>
const typename DesignDoubleValueMap<VT>::DoubleValue&
DesignDoubleValueMap<VT>::DoubleValueMax(
    )
{
    static const DoubleValue max(
        std::make_pair(my_type::MAX_POSSIBLE, my_type::MAX_POSSIBLE)
        );
    return max;
}

template <typename VT>
const typename DesignDoubleValueMap<VT>::DoubleValue&
DesignDoubleValueMap<VT>::DoubleValueMin(
    )
{
    static const DoubleValue min(
        std::make_pair(my_type::MIN_POSSIBLE, my_type::MIN_POSSIBLE)
        );
    return min;
}

template <typename VT>
inline
const typename DesignDoubleValueMap<VT>::DoubleValue&
DesignDoubleValueMap<VT>::GetValues(
    const Design* of
    ) const
{
    typename base_type::const_iterator it(this->find(of));
    return it == this->end() ? my_type::DoubleValueMax() : (*it).second;
}

template <typename VT>
inline
const typename DesignDoubleValueMap<VT>::DoubleValue&
DesignDoubleValueMap<VT>::GetValues(
    const Design& of
    ) const
{
    return this->GetValues(&of);
}

template <typename VT>
inline
const typename DesignDoubleValueMap<VT>::value_type&
DesignDoubleValueMap<VT>::GetFirstValue(
    const Design* of
    ) const
{
    return this->GetValues(of).first;
}

template <typename VT>
inline
const typename DesignDoubleValueMap<VT>::value_type&
DesignDoubleValueMap<VT>::GetFirstValue(
    const Design& of
    ) const
{
    return this->GetValues(of).first;
}

template <typename VT>
inline
const typename DesignDoubleValueMap<VT>::value_type&
DesignDoubleValueMap<VT>::GetSecondValue(
    const Design* of
    ) const
{
    return this->GetValues(of).second;
}

template <typename VT>
inline
const typename DesignDoubleValueMap<VT>::value_type&
DesignDoubleValueMap<VT>::GetSecondValue(
    const Design& of
    ) const
{
    return this->GetValues(of).second;
}

template <typename VT>
inline
bool
DesignDoubleValueMap<VT>::AddValues(
    const Design& of,
    const value_type& ct1,
    const value_type& ct2
    )
{
    return this->AddValues(&of, ct1, ct2);
}

template <typename VT>
inline
bool
DesignDoubleValueMap<VT>::AddValues(
    const Design* of,
    const value_type& ct1,
    const value_type& ct2
    )
{
    return this->AddValues(of, my_type::MakeDoubleValue(ct1, ct2));
}

template <typename VT>
inline
bool
DesignDoubleValueMap<VT>::AddValues(
    const Design& of,
    const DoubleValue& dc
    )
{
    return this->AddValues(&of, dc);
}

template <typename VT>
inline
bool
DesignDoubleValueMap<VT>::AddValues(
    const Design* of,
    const DoubleValue& dc
    )
{
    return this->insert(typename base_type::value_type(of, dc)).second;
}

template <typename VT>
inline
typename DesignDoubleValueMap<VT>::DoubleValue
DesignDoubleValueMap<VT>::MakeDoubleValue(
    const value_type& ct1,
    const value_type& ct2
    )
{
    return std::make_pair(ct1, ct2);
}

template <typename VT>
void
DesignDoubleValueMap<VT>::AddToValues(
    const Design* of,
    const value_type& amt1,
    const value_type& amt2
    )
{
    typename base_type::iterator it(this->find(of));

    if(it == this->end()) this->AddValues(of, amt1, amt2);

    else
    {
        my_type::_AddToValue((*it).second.first, amt1);
        my_type::_AddToValue((*it).second.second, amt2);
    }
}

template <typename VT>
void
DesignDoubleValueMap<VT>::AddToFirstValue(
    const Design* of,
    const value_type& amt
    )
{
    EDDY_FUNC_DEBUGSCOPE

    typename base_type::iterator it(this->find(of));
    if(it == this->end()) this->AddValues(of, amt, value_type(0));
    else my_type::_AddToValue((*it).second.first, amt);
}

template <typename VT>
void
DesignDoubleValueMap<VT>::AddToSecondValue(
    const Design* of,
    const value_type& amt
    )
{
    EDDY_FUNC_DEBUGSCOPE

    typename base_type::iterator it(this->find(of));
    if(it == this->end()) this->AddValues(of, value_type(0), amt);
    else my_type::_AddToValue((*it).second.second, amt);
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

template <typename VT>
inline
void
DesignDoubleValueMap<VT>::_AddToValue(
    value_type& ct,
    const value_type& amt
    )
{
    ct += amt;
}






/*
================================================================================
Inline Structors
================================================================================
*/


template <typename VT>
DesignDoubleValueMap<VT>::DesignDoubleValueMap(
    std::size_t JEGA_IF_BOOST(initSize)
    ) :
        base_type(JEGA_IF_BOOST(initSize))
{
}

template <typename VT>
DesignDoubleValueMap<VT>::~DesignDoubleValueMap(
    )
{
}





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

