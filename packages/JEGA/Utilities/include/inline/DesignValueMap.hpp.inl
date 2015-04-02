/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignValueMap.

    NOTES:

        See notes of DesignValueMap.hpp.

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
 * \brief Contains the inline methods of the DesignValueMap class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <utilities/include/Math.hpp>
#include <utilities/include/numeric_limits.hpp>





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
const VT DesignValueMap<VT>::MAX_POSSIBLE =
    eddy::utilities::numeric_limits<VT>::max();

template <typename VT>
const VT DesignValueMap<VT>::MIN_POSSIBLE =
    eddy::utilities::numeric_limits<VT>::smallest();



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
template<typename VT>
inline
const typename DesignValueMap<VT>::value_type&
DesignValueMap<VT>::GetMaxValue(
    ) const
{
    return this->_maxValue;
}

template<typename VT>
inline
const typename DesignValueMap<VT>::value_type&
DesignValueMap<VT>::GetMinValue(
    ) const
{
    return this->_minValue;
}

template<typename VT>
inline
const typename DesignValueMap<VT>::value_type&
DesignValueMap<VT>::GetTotalValue(
    ) const
{
    return this->_totalValue;
}







/*
================================================================================
Inline Public Methods
================================================================================
*/

template<typename VT>
inline
void
DesignValueMap<VT>::SuspendStatistics(
    )
{
    this->_statsSuspended = true;
}

template<typename VT>
inline
void
DesignValueMap<VT>::ResumeStatistics(
    bool performUpdate
    )
{
    this->_statsSuspended = false;
    if(performUpdate) this->UpdateTotals();
}

template<typename VT>
inline
bool
DesignValueMap<VT>::UpdatingStatistics(
    )
{
    return this->_statsSuspended;
}

template<typename VT>
inline
typename DesignValueMap<VT>::value_type
DesignValueMap<VT>::GetValue(
    const Design* of
    ) const
{
    typename base_type::const_iterator it(this->find(of));
    return (it == this->end()) ? MAX_POSSIBLE : (*it).second;
}

template<typename VT>
inline
typename DesignValueMap<VT>::value_type
DesignValueMap<VT>::GetValue(
    const Design& of
    ) const
{
    return this->GetValue(&of);
}

template<typename VT>
inline
bool
DesignValueMap<VT>::AddValue(
    const Design& of,
    const value_type& val
    )
{
    return this->AddValue(&of, val);
}

template<typename VT>
inline
const typename DesignValueMap<VT>::value_type&
DesignValueMap<VT>::AddToValue(
    const Design& of,
    const value_type& amt
    )
{
    return this->AddToValue(&of, amt);
}

template<typename VT>
inline
bool
DesignValueMap<VT>::RemoveValue(
    const Design& of
    )
{
    return this->RemoveValue(&of);
}


template<typename VT>
bool
DesignValueMap<VT>::AddValue(
    const Design* of,
    const value_type& val
    )
{
    if(!this->insert(typename base_type::value_type(of, val)).second)
        return false;

    if(this->_statsSuspended) return true;
    if(val < this->_minValue) this->_minValue = val;
    if(val > this->_maxValue) this->_maxValue = val;
    this->_totalValue += val;
    return true;
}

template<typename VT>
inline
typename DesignValueMap<VT>::value_type
DesignValueMap<VT>::GetAverageValue(
    ) const
{
    return this->empty() ? MAX_POSSIBLE : this->_totalValue/this->size();
}

template<typename VT>
const typename DesignValueMap<VT>::value_type&
DesignValueMap<VT>::AddToValue(
    const Design* of,
    const value_type& amt
    )
{
    typename base_type::iterator it(this->find(of));
    if(it == this->end())
    {
        this->AddValue(of, amt);
        return amt;
    }
    else if(amt == value_type(0))
    {
        return (*it).second;
    }
    else
    {
        VT& value = (*it).second;
        const bool wasMax = (value == this->_maxValue);
        const bool wasMin = (value == this->_minValue);

        value += amt;
        if(this->_statsSuspended) return value;

        this->_totalValue += amt;

        // If it was the max and we are adding a positive value, then we don't
        // need to update totals.
        if(wasMax && (amt > value_type(0)))
        {
            this->_maxValue = value;
        }
        // Similar logic works for the min.
        else if(wasMin && (amt < value_type(0)))
        {
            this->_minValue = value;
        }
        // If it was the max or min but we reduce/increased it respectively,
        // then we don't know where the new max and min are and must do a full
        // update of the statistics.
        else if(wasMax || wasMin)
        {
            this->UpdateTotals();
        }
        // If it wasn't the max or min then we can just see if it is now and
        // we are all set.
        else
        {
            // don't need to check for empty here b/c if it were empty
            // we wouldn't be here.
            if(value > this->_maxValue) this->_maxValue = value;
            if(value < this->_minValue) this->_minValue = value;
        }
        return value;
    }
}

template<typename VT>
bool
DesignValueMap<VT>::RemoveValue(
    const Design* of
    )
{
    typename base_type::iterator it(this->find(of));
    if(it == this->end()) return false;
    if(this->_statsSuspended) { this->erase(it); return true; }

    const VT& val = (*it).second;
    bool wasMax = (val == this->_maxValue);
    bool wasMin = (val == this->_minValue);
    this->erase(it);

    if(wasMax || wasMin) this->UpdateTotals();
    else this->_totalValue -= val;
    return true;
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
void
DesignValueMap<VT>::UpdateTotals(
    )
{
    this->_maxValue = MIN_POSSIBLE;
    this->_minValue = MAX_POSSIBLE;
    this->_totalValue = value_type(0);

    for(typename base_type::const_iterator it(this->begin());
        it!=this->end(); ++it)
    {
        const VT& value = (*it).second;
        if(value > this->_maxValue) this->_maxValue = value;
        if(value < this->_minValue) this->_minValue = value;
        this->_totalValue += value;
    }
}







/*
================================================================================
Inline Structors
================================================================================
*/


template <typename VT>
DesignValueMap<VT>::DesignValueMap(
    std::size_t JEGA_IF_BOOST(initSize)
    ) :
        base_type(JEGA_IF_BOOST(initSize)),
        _minValue(MAX_POSSIBLE),
        _maxValue(MIN_POSSIBLE),
        _totalValue(value_type(0)),
        _statsSuspended(false)
{
}

template <typename VT>
DesignValueMap<VT>::DesignValueMap(
    const my_type& copy
    ) :
        base_type(copy),
        _minValue(copy._minValue),
        _maxValue(copy._maxValue),
        _totalValue(copy._totalValue),
        _statsSuspended(copy._statsSuspended)
{
}

template <typename VT>
DesignValueMap<VT>::~DesignValueMap(
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

