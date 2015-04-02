/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Full include specification of the entry types of the logging project.

    NOTES:

        Includes all entry type header files for the logging project.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Feb 08 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the full include specification of the entry types.
 */

/*
================================================================================
Create a Group for Entry Classes
================================================================================
*/
/**
 * \defgroup entry_types Pre-Existing Entry Types
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_ENTRIES_HPP
#define EDDY_LOGGING_ENTRIES_HPP

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

// include all the custom entry types.
#include "../entries/text_entry.hpp"
#include "../entries/ostream_entry.hpp"



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
#endif // EDDY_LOGGING_ENTRIES_HPP
