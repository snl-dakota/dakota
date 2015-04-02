/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Implementation of methods defined in thread_exceptions.hpp.

    NOTES:

        See notes of thread_exceptions.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Oct 12 18:11:58 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementations of methods defined in
 *        thread_exceptions.hpp.
 */




/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <string>
#include <sstream>
#include "../include/thread_exceptions.hpp"




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace threads {






/*
================================================================================
File Scope Utility Functions
================================================================================
*/
/// Converts a value of type T to a string.
/**
 * This is a relatively unsafe implementation but since it is only used locally
 * and is only called with integers, it is good enough.
 *
 * \param val The value to convert into a string.
 * \return The string representation of val.
 */
template <typename T>
string
ToString(
    const T& val
    )
{
    ostringstream ostr;
    ostr << val;
    return ostr.str();
}

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


lock_error::lock_error(
    const std::string& msg,
    int err_code
    ) throw() :
        logic_error(msg + "- error code is: " + ToString(err_code))
{
}




/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy

