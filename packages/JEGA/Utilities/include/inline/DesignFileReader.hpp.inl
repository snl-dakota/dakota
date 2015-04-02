/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignFileReader.

    NOTES:

        See notes of DesignFileReader.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Fri Mar 03 13:23:45 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DesignFileReader class.
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
const std::string&
DesignFileReader::Result::GetFilename(
    ) const
{
    return _filename;
}

inline
const DesignDVSortSet&
DesignFileReader::Result::GetDesigns(
    ) const
{
    return _designs;
}

inline
eddy::utilities::uint64_t
DesignFileReader::Result::GetNumFullReads(
    ) const
{
    return _fullReads;
}

inline
eddy::utilities::uint64_t
DesignFileReader::Result::GetNumPartialReads(
    ) const
{
    return _partReads;
}

inline
eddy::utilities::uint64_t
DesignFileReader::Result::GetTotalReads(
    ) const
{
    return _partReads + _fullReads;
}

inline
eddy::utilities::uint64_t
DesignFileReader::Result::GetTotalAttemptedReads(
    ) const
{
    return _attempts;
}

inline
eddy::utilities::uint64_t
DesignFileReader::Result::GetNumFailures(
    ) const
{
    return _failures;
}

inline
eddy::utilities::uint64_t
DesignFileReader::Result::GetNumEmptyLines(
    ) const
{
    return _emptyLines;
}

inline
bool
DesignFileReader::Result::GetErrorFlag(
    ) const
{
    return _error;
}







/*
================================================================================
Inline Public Methods
================================================================================
*/








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

