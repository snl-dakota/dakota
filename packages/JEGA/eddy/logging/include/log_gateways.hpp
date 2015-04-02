/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Full include specification of the provided log gateways.

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

        Wed Feb 09 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the full include specification of the supplied log
 *        gateways.
 */

/*
================================================================================
Create a Group for Log Gateway Classes
================================================================================
*/
/**
 * \defgroup log_gateway_types Pre-Existing Log Gateway Types
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_LOG_MANAGERS_HPP
#define EDDY_LOGGING_LOG_MANAGERS_HPP


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

// include all the custom log gateway classes.
#include "../log_gateways/basic_log_gateway.hpp"
#include "../log_gateways/level_log_gateway.hpp"
#include "../log_gateways/cb_level_log_gateway.hpp"


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
#endif // EDDY_LOGGING_LOG_MANAGERS_HPP
