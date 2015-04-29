/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DiscreteDesignVariableNature.

    NOTES:

        See notes of DiscreteDesignVariableNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 08:55:37 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DiscreteDesignVariableNature
 *        class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <algorithm>
#include <functional>
#include <utilities/include/Math.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>
#include <../Utilities/include/DiscreteDesignVariableNature.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace eddy::utilities;







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
/**
 * \brief A base class for the Min and Max predicates.
 */
template <typename Comp>
class DiscreteDesignVariableNature::CutoffPred :
    public std::unary_function<double, bool>
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    protected:

        /// Cutoff value
        argument_type _value;

        /// The comparison operator to use on the passed in values.
        Comp _comp;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Comparison operator
        /**
         * \param val The value to test against the stored value.
         * \return True if \a val is greater than or equal to the stored value.
         */
        result_type
        operator ()(
            argument_type val
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->_comp(val, this->_value);
        };

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a CutoffPred to compare passed in values to \a val.
        /**
         * \param val The value for this operator to compare passed in values
         *            to.
         */
        CutoffPred(
            argument_type val
            ) :
                _value(val),
                _comp()
        {
            EDDY_FUNC_DEBUGSCOPE
        };

}; // class DiscreteDesignVariableNature::MaxPred

/**
 * \brief A class to test passed in values to see if they are less
 *        than or equal to a stored value.
 */
class DiscreteDesignVariableNature::MaxPred :
	public DiscreteDesignVariableNature::CutoffPred<greater_equal<double> >
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a MaxPred to compare passed in values to \a val.
        /**
         * \param val The value for this operator to compare passed in values
         *            to using greater than or equal to.
         */
        MaxPred(
            argument_type val
            ) :
                DiscreteDesignVariableNature::CutoffPred<
                    greater_equal<double>
                    >(val)
        {
            EDDY_FUNC_DEBUGSCOPE
        };

}; // class DiscreteDesignVariableNature::MaxPred


/**
 * \brief A class to test passed in values to see if they are greater
 *        than or equal to a stored value.
 */
class DiscreteDesignVariableNature::MinPred :
    public DiscreteDesignVariableNature::CutoffPred<less_equal<double> >
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a MinPred to compare passed in values to \a val.
        /**
         * \param val The value for this operator to compare passed in values
         *            to using less than or equal to.
         */
        MinPred(
            argument_type val
            ) :
                DiscreteDesignVariableNature::CutoffPred<
                    less_equal<double>
                    >(val)
        {
            EDDY_FUNC_DEBUGSCOPE
        };

}; // class DiscreteDesignVariableNature::MinPred


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
DiscreteDesignVariableNature::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Discrete");
}

DesignVariableNatureBase*
DiscreteDesignVariableNature::Clone(
    DesignVariableTypeBase& forType
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new DiscreteDesignVariableNature(*this, forType);
}

double
DiscreteDesignVariableNature::GetMaxDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return static_cast<double>(this->_disVals.size()-1);
}

double
DiscreteDesignVariableNature::GetMinDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_disVals.empty() ? -1 : 0.0;
}

double
DiscreteDesignVariableNature::GetRandomDoubleRep(
    double lb,
    double ub
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return static_cast<double>(
        RandomNumberGenerator::UniformInt<DoubleVector::size_type>(
                // if anything, shrink the range.
                static_cast<size_t>(Math::Ceil(lb)),
                static_cast<size_t>(Math::Floor(ub))
             )
        );
}

double
DiscreteDesignVariableNature::GetDoubleRepOf(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    DoubleVector::const_iterator it(
        find(this->_disVals.begin(), this->_disVals.end(), value)
        );
    return (it==this->_disVals.end()) ?
        -DBL_MAX : static_cast<double>(it-this->_disVals.begin());
}

double
DiscreteDesignVariableNature::GetRandomValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    DoubleVector::size_type elem =
        static_cast<DoubleVector::size_type>(
            this->DesignVariableNatureBase::GetRandomDoubleRep()
            );
    return this->_disVals[elem];
}

double
DiscreteDesignVariableNature::GetMaxValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_disVals.back();
}

double
DiscreteDesignVariableNature::GetMinValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_disVals.front();
}

double
DiscreteDesignVariableNature::GetValueOf(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->IsValidDoubleRep(rep));
    DoubleVector::size_type loc =
        static_cast<DoubleVector::size_type>(Math::Round(rep));
    return this->IsValidDoubleRep(rep) ? this->_disVals[loc] : -DBL_MAX;
}

double
DiscreteDesignVariableNature::GetNearestValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // go through the values (which are in sorted order)
    // and find the closest one.

    // prepare to use the beginning and end of the vector over
    // and over again.
    DoubleVector::const_iterator b(this->_disVals.begin());
    DoubleVector::const_iterator e(this->_disVals.end());

    // bound the passed in value
    pair<DoubleVector::const_iterator, DoubleVector::const_iterator> p =
        equal_range(b, e, value);

    // if the value actually exists in the list, the lowerbound will be
    // the correct value.
    if(*(p.first) == value) return value;

    // otherwise, if the upperbound is the beginning of the vector, then
    // the value passed in is smaller than all others and we return
    // the lowerbound
    if(p.second == b) return this->_disVals.front();

    // otherwise, if the lowerbound is equal to the end of the vector,
    // then our value is larger than all _disVals and we return the
    // last (largest) value.
    if(p.first == e) return this->_disVals.back();

    // if we make it here, we have a value that is in-between
    // two legitimate values.  To handle this, we have to back up the
    // lowerbound and see which is furthest from the value (lb or ub).
    EDDY_ASSERT(p.first != b);
    --p.first;

    double ubd = Math::Abs(*(p.second) - value);
    double lbd = Math::Abs(*(p.first) - value);

    return (ubd > lbd) ? *(p.first) : *(p.second);
}

double
DiscreteDesignVariableNature::GetNearestValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    if(rep == -DBL_MAX) return rep;

    const double temp = Math::Round(rep);

    return Math::Max(Math::Min(
        this->GetMaxDoubleRep(), temp
        ), this->GetMinDoubleRep());
}

double
DiscreteDesignVariableNature::GetDistanceBetweenDoubleReps(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return 1.0;
}

bool
DiscreteDesignVariableNature::AddDiscreteValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    DoubleVector::const_iterator it(
        find(this->_disVals.begin(), this->_disVals.end(), value)
        );

    EDDY_DEBUG(it!=this->_disVals.end(),
          "Attempt to add duplicate discrete value failed");

    if(it == this->_disVals.end())
    {
        this->_disVals.insert(lower_bound(
            this->_disVals.begin(), this->_disVals.end(), value
            ), value);
        return true;
    }
    return false;
}

void
DiscreteDesignVariableNature::ClearDiscreteValues(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_disVals.clear();
}

bool
DiscreteDesignVariableNature::RemoveDiscreteValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE

    DoubleVector::size_type osize = this->_disVals.size();
    this->_disVals.erase(remove(
        this->_disVals.begin(), this->_disVals.end(), value
        ), this->_disVals.end());
    return this->_disVals.size() != osize;
}

void DiscreteDesignVariableNature::SetMaxValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
     this->_disVals.erase(
         remove_if(
            this->_disVals.begin(), this->_disVals.end(), MaxPred(value)
            ),
         this->_disVals.end()
         );
    this->AddDiscreteValue(value);
}

void DiscreteDesignVariableNature::SetMinValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_disVals.erase(
        remove_if(this->_disVals.begin(), this->_disVals.end(), MinPred(value)),
        this->_disVals.end()
        );
    this->AddDiscreteValue(value);
}

bool
DiscreteDesignVariableNature::IsDiscreteValueLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

bool
DiscreteDesignVariableNature::IsValueInBounds(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return find(this->_disVals.begin(), this->_disVals.end(), value) !=
           this->_disVals.end();
}

bool
DiscreteDesignVariableNature::IsRepInBounds(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::IsWhole(rep) && (rep >= 0.0) && (rep < this->_disVals.size());
}

bool
DiscreteDesignVariableNature::IsOutOfBoundsDefined(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

bool
DiscreteDesignVariableNature::IsPrecisionLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

bool
DiscreteDesignVariableNature::IsValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DesignVariableNatureBase::IsValidValue(value) &&
           this->IsValueInBounds(value);
}

bool
DiscreteDesignVariableNature::IsValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DesignVariableNatureBase::IsValidDoubleRep(rep) &&
           this->IsRepInBounds(rep);
}

bool
DiscreteDesignVariableNature::IsDiscrete(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

bool
DiscreteDesignVariableNature::IsContinuum(
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
DiscreteDesignVariableNature::DiscreteDesignVariableNature(
    DesignVariableTypeBase& type
    ) :
        DesignVariableNatureBase(type)
{
    EDDY_FUNC_DEBUGSCOPE
}

DiscreteDesignVariableNature::DiscreteDesignVariableNature(
    const DiscreteDesignVariableNature& copy,
    DesignVariableTypeBase& type
    ) :
        DesignVariableNatureBase(copy, type),
        _disVals(copy._disVals)
{
    EDDY_FUNC_DEBUGSCOPE
}

DiscreteDesignVariableNature::~DiscreteDesignVariableNature(
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
