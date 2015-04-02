/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class BasicParameterDatabaseImpl.

    NOTES:

        See notes of BasicParameterDatabaseImpl.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Jan 06 07:35:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the BasicParameterDatabaseImpl class.
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
template <typename T>
const T&
BasicParameterDatabaseImpl::GetParamValue(
    const std::string& tag,
    const MY_MAP_T<std::string, T>& from
    ) const
{
    typedef MY_MAP_T<std::string, T> MapT;
    JEGA_IF_THREADSAFE(typename MapT::scoped_lock l(from);)
    typename MapT::const_iterator it(from.find(tag));
    if(it == from.end()) throw JEGA::Utilities::no_such_parameter_error(tag);
    return (*it).second;
}

template <typename T>
bool
BasicParameterDatabaseImpl::AddParamValue(
    const std::string& tag,
    const T& value,
    MY_MAP_T<std::string, T>& theMap
    ) const
{
    typedef MY_MAP_T<std::string, T> MapT;
    JEGA_IF_THREADSAFE(typename MapT::scoped_lock l(theMap);)
    theMap[tag] = value;
    return true;
}

template <typename T>
bool
BasicParameterDatabaseImpl::HasParam(
    const std::string& tag,
    const MY_MAP_T<std::string, T>& in
    ) const
{
    typedef MY_MAP_T<std::string, T> MapT;
    JEGA_IF_THREADSAFE(typename MapT::scoped_lock l(in);)
    return in.find(tag) != in.end();
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

