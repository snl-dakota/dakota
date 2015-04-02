/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Declarations of the default types used in the logging project.

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

        Fri Jan 28 15:40:08 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the declarations of default types used in the logging
 *        project.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_DEFAULT_TYPES_HPP
#define EDDY_LOGGING_DEFAULT_TYPES_HPP


/*
================================================================================
Type Definitions
================================================================================
*/

// Define the default character type if it is not already defined.
#ifndef EDDY_LOGGING_DEF_CHAR_TYPE
#define EDDY_LOGGING_DEF_CHAR_TYPE char
#endif

// Define the default character traits type using the default character type.
#ifndef EDDY_LOGGING_DEF_CHAR_TRAITS
#include <iosfwd>
#define EDDY_LOGGING_DEF_CHAR_TRAITS                                       \
    std::char_traits< EDDY_LOGGING_DEF_CHAR_TYPE >
#endif


// Define the default level type if it is not already defined.
#ifndef EDDY_LOGGING_DEF_LEVEL_CLASS

#   ifdef EDDY_LOGGING_ON
#       include <../level_classes/cougaar_levels.hpp>

        /// A macro to expand to the default level class type.
        /**
         * The default is to use the cougaar_levels with the default char_t and
         * char_traits whatever they are.
         */
#       define EDDY_LOGGING_DEF_LEVEL_CLASS eddy::logging::cougaar_levels< >
#   else
#       define EDDY_LOGGING_DEF_LEVEL_CLASS
#   endif

#endif


/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_DEFAULT_TYPES_HPP
