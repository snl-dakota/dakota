/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class BitManipulator.

    NOTES:

        See notes of BitManipulator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 10:11:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the BitManipulator class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cmath>
#include <ostream>
#include <BitManipulator.hpp>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
using namespace eddy::utilities;







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


eddy::utilities::int64_t
BitManipulator::ConvertToShiftedInt(
    const Design& des,
    std::size_t dv
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->ConvertToShiftedInt(des.GetVariableRep(dv), dv);
}

eddy::utilities::int64_t
BitManipulator::ConvertToShiftedInt(
    double val,
    std::size_t dv
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_mins.size() > dv);
    EDDY_ASSERT(this->_mults.size() > dv);

    return static_cast<eddy::utilities::int64_t>(
        Math::Round((val + this->_mins[dv]) * this->_mults[dv])
        );
}

eddy::utilities::uint8_t
BitManipulator::CountBits(
    eddy::utilities::int64_t of,
    bool withVal
    )
{
    EDDY_FUNC_DEBUGSCOPE

    eddy::utilities::uint8_t ret = 0;
    eddy::utilities::uint8_t want = withVal ? 1 : 0;

    for(int i=0; i<64; ++i)
        ret += (((of >> i) & 1) == want) ? 1 : 0;

    return ret;
}

void
BitManipulator::PrintAllBits(
    ostream& stream,
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    const std::size_t ndv = this->_target.GetNDV();

    if(ndv == 0) return;

    const DesignVariableInfoVector& dvinfos =
        this->_target.GetDesignVariableInfos();

    for(std::size_t dv=0; dv<(ndv-1); ++dv)
    {
        this->PrintBits(stream, des, dv);
        stream << ' ';
    }

    // now do the last one without the space.
    this->PrintBits(stream, des, ndv-1);
}

void
BitManipulator::PrintBits(
    ostream& stream,
    const JEGA::Utilities::Design& des,
    std::size_t dv
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(dv < des.GetNDV());

    BitManipulator::PrintBits(
        stream, this->ConvertToShiftedInt(des.GetVariableRep(dv), dv), 0,
        this->GetNumberOfBits(dv)-1
        );
}


void
BitManipulator::PrintBits(
    ostream& stream,
    eddy::utilities::int64_t val,
    eddy::utilities::uint16_t lobit,
    eddy::utilities::uint16_t hibit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(hibit < 64);
    EDDY_ASSERT(lobit <= hibit);

    for(eddy::utilities::int32_t i=hibit;
        i>=static_cast<eddy::utilities::int32_t>(lobit); --i)
            stream << ((val>>i)&1);
}

bool
BitManipulator::AreContentsCurrent(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // get the needed info items
    const DesignVariableInfoVector& dvinfos =
        this->_target.GetDesignVariableInfos();

    // now loop through the design variable infos and extract
    // the needed information.
    DesignVariableInfoVector::const_iterator it(dvinfos.begin());

    for(DoubleVector::size_type i=0; it!=dvinfos.end(); ++it, ++i)
    {
        // compute and store the multiplier / divider.
        double mult = std::pow(10.0, (*it)->GetPrecision());
        if(mult != this->_mults[i]) return false;

        // store the minimum representation.
        double mini = (*it)->GetMinDoubleRep();
        if(mini != this->_mins[i]) return false;

        // compute and store the number of bits for this variable.
        double temp = ((*it)->GetMaxDoubleRep()-mini) * mult;

        eddy::utilities::uint16_t nbits =
            static_cast<eddy::utilities::uint16_t>(
                Math::Floor(Math::Log(temp, 2.0)) + 1
                );

        if(nbits != this->_nbits[i]) return false;
    }
    return true;
}

void
BitManipulator::ReValidateContents(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // clear the current data.
    this->_nbits.clear();
    this->_tnBits = 0;
    this->_mults.clear();
    this->_mins.clear();

    // get the needed info items
    const DesignVariableInfoVector& dvinfos =
        this->_target.GetDesignVariableInfos();

    // now loop through the design variable infos and extract
    // the needed information.
    DesignVariableInfoVector::const_iterator it(dvinfos.begin()),
        e(dvinfos.end());

    for(; it!=e; ++it)
    {
        // compute and store the multiplier / divider.
        double mult = std::pow(10, static_cast<double>((*it)->GetPrecision()));
        this->_mults.push_back(mult);

        // store the minimum representation.
        double mini = (*it)->GetMinDoubleRep();
        this->_mins.push_back(mini);

        // compute and store the number of bits for this variable.
        double temp = ((*it)->GetMaxDoubleRep()-mini) * mult;

        eddy::utilities::uint16_t nbits =
            static_cast<eddy::utilities::uint16_t>(
                Math::Floor(Math::Log(temp, 2.0)) + 1.0
                );

        EDDY_DEBUG(
            nbits > 64, "Variable span too large for 64 bit representation."
            );

        JEGAIFLOG_CF_II_G_F(nbits > 64, this,
            text_entry(lfatal(), "BitManipulator: Variable \"" +
               (*it)->GetLabel() + "\"'s range is too large to "
               "be represented by 64 bits.")
            )

        this->_nbits.push_back(nbits);
        this->_tnBits += nbits;
    }
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








/*
================================================================================
Private Methods
================================================================================
*/
BitManipulator::BinaryString
BitManipulator::ToBinaryString(
    double v
    )
{
    EDDY_FUNC_DEBUGSCOPE

	if(v == 0.0) return BinaryString();
    v = Math::Truncate(v);
	if(v == 0.0) return BinaryString();

    // figure out how many bits we are going to need.
    const size_t nbits =
		static_cast<size_t>(Math::Floor(Math::Log(v, 2.0))) + 1;

    // now prepare our return object.
    BinaryString ret(nbits, 0);

    // now load up the string.
    for(size_t b=nbits-1; b>0 && v!=0.0; --b)
    {
        eddy::utilities::uintmax_t bv = (1 << b);
        if(v >= bv)
        {
            ret.set_bit(b);
            v -= bv;
        }
    }

    // now do the last bit separately.
    if(v >= 1) ret.set_bit(0);

    return ret;
}


double
BitManipulator::ToDouble(
    const BinaryString& bstr
    )
{
    EDDY_FUNC_DEBUGSCOPE

    double ret = 0.0;

    for(size_t i=0; i<bstr.size(); ++i)
        if(bstr.get_bit(i)) ret += (1 << i);

    return ret;
}






/*
================================================================================
Structors
================================================================================
*/


BitManipulator::BitManipulator(
    const DesignTarget& target
    ) :
        _nbits(),
        _tnBits(0),
        _mults(),
        _mins(),
        _target(target)
{
    EDDY_FUNC_DEBUGSCOPE
    this->ReValidateContents();
}





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
