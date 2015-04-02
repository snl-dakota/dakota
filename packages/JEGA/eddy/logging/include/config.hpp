/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Logging project configuration code.

    NOTES:



    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Feb 10 15:45:44 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains various Logging project configuration code.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_CONFIG_HPP
#define EDDY_LOGGING_CONFIG_HPP





/*
================================================================================
Conditional Inclusion
================================================================================
*/
#ifdef EDDY_LOGGING_ON


/*
================================================================================
Includes
================================================================================
*/
// This will bring in the config config.hpp as well.
#include "../../threads/include/config.hpp"


/*
================================================================================
Version Information
================================================================================
*/
/// Expands to a string identifying the current version of the logging project.
#define EDDY_LOGGING_VERSION "1.0.0"

/// Expands to a string identifying the loggin package.
#define EDDY_LOGGING_PACKAGE "Eddy Logging v"EDDY_LOGGING_VERSION






/*
================================================================================
Shared Library Specifications
================================================================================
*/



/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace logging {




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace logging
} // namespace eddy



/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif // EDDY_LOGGING_ON

/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_DETAIL_THREADSAFE_HPP
