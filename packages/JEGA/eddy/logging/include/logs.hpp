/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Full include specification of the Eddy logging project.

    NOTES:

        Includes all log type header files for the logging project.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Feb 07 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the full include specification of the log types.
 */


/*
================================================================================
Create a Group for Log Classes
================================================================================
*/
/**
 * \defgroup log_types Pre-Existing Log Types
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_LOGS_HPP
#define EDDY_LOGGING_LOGS_HPP


/*
================================================================================
Conditional Inclusion
================================================================================
*/
#ifdef EDDY_LOGGING_ON


/*
================================================================================
Establish the Namespace
================================================================================
*/
namespace eddy { namespace logging {} }

/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"

// include all the custom log types.
#include "../logs/appending_log.hpp"
#include "../logs/decorator_log.hpp"
#include "../logs/file_log.hpp"
#include "../logs/file_ring_log.hpp"
#include "../logs/list_log.hpp"
#include "../logs/null_log.hpp"
#include "../logs/ostream_log.hpp"



/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif


/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_LOGS_HPP
