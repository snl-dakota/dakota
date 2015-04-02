/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Full include specification of the provided level policy compliant
        types.

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

        Mon Feb 07 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the full include specification of the supplied level
 *        policy compliant types.
 */

/*
================================================================================
Create a Group for Level Policy Compliant Classes
================================================================================
*/
/**
 * \defgroup level_class_types Pre-Existing Level Policy Compliant Types
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_LEVEL_CLASSES_HPP
#define EDDY_LOGGING_LEVEL_CLASSES_HPP


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

// include all the custom level policy classes.
#include "../level_classes/cougaar_levels.hpp"
#include "../level_classes/dakota_levels.hpp"
#include "../level_classes/java_util_levels.hpp"
#include "../level_classes/log4j_levels.hpp"
#include "../level_classes/minimal_levels.hpp"
#include "../level_classes/volume_levels.hpp"


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
#endif // EDDY_LOGGING_LEVEL_CLASSES_HPP
