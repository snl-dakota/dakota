/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class BitManipulator.

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
 * \brief Contains the inline methods of the BitManipulator class.
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
    namespace Algorithms {







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
const JEGA::Utilities::DesignTarget&
BitManipulator::GetDesignTarget(
    ) const
{
    return this->_target;
}






/*
================================================================================
Inline Public Methods
================================================================================
*/

inline
eddy::utilities::uint32_t
BitManipulator::GetTotalNumOfBits(
    ) const
{
    return this->_tnBits;
}

inline
double
BitManipulator::ConvertFromShiftedInt(
    eddy::utilities::int64_t val,
    std::size_t dv
    ) const
{
    return val/this->_mults[dv] - this->_mins[dv];
}

inline
eddy::utilities::uint16_t
BitManipulator::GetNumberOfBits(
    std::size_t dv
    ) const
{
    return this->_nbits[dv];
}

inline
eddy::utilities::int64_t
BitManipulator::ToggleBit(
    eddy::utilities::uint16_t bit,
    eddy::utilities::int64_t val
    )
{
    return val ^ static_cast<eddy::utilities::uint32_t>(1<<bit);
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
    } // namespace Algorithms
} // namespace JEGA
