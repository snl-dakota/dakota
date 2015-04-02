/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Contains configuration for the JEGA project.

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

        Thu Feb 02 09:32:21 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains configuration for the JEGA project.
 *
 * This includes any platform and compiler specific configuration.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGACONFIG_HPP
#define JEGACONFIG_HPP

/*
================================================================================
Configuration Includes
================================================================================
*/
// First we must include the jega_config.h file created by autoconf if using
// autoconf (ex. with dakota).  If not, this will be ignored and will cause
// no harm.
#ifdef HAVE_CONFIG_H
#include "jega_config.h"
#endif /* HAVE_CONFIG_H */


/*
================================================================================
Prepare Underlying Configuration Macros
================================================================================
*/
#ifdef JEGA_SL
#   ifndef EDDY_SL
#       define EDDY_SL
#   endif
#endif

#ifdef JEGA_EXPORTING
#   ifndef EDDY_EXPORTING
#       define EDDY_EXPORTING
#   endif
#endif

#ifdef JEGA_THREADSAFE
#   ifndef EDDY_THREADSAFE
#       define EDDY_THREADSAFE
#   endif
#endif

#ifdef JEGA_HAVE_BOOST
#   ifndef EDDY_HAVE_BOOST
#       define EDDY_HAVE_BOOST
#   endif
#endif

/*
================================================================================
Post Configuration Includes
================================================================================
*/
#include <utilities/include/config.hpp>

// This is to prevent warnings in the case where eddy_config.h has
// previously been included.
#ifndef EDDY_OPTION_DEBUG
#   ifdef JEGA_OPTION_DEBUG
#       define EDDY_OPTION_DEBUG
#   endif
#endif

// Even if not threadsafe, must have threads config file so that macros
// expand properly.  This probably already came in from utilities/config.hpp.
#include <threads/include/config.hpp>
#include <logging/include/config.hpp>


/*
================================================================================
Version Information
================================================================================
*/
// Ignore any previous definition of JEGA_VERSION.
#ifdef JEGA_VERSION
#undef JEGA_VERSION
#endif

// Ignore any previous definition of JEGA_PACKAGE.
#ifdef JEGA_PACKAGE
#undef JEGA_PACKAGE
#endif

#define JEGA_VERSION "2.7.0"
#define JEGA_PACKAGE "JEGA v"JEGA_VERSION


/*
================================================================================
Utility Macros
================================================================================
*/

#ifdef EDDY_WINDOWS
#   define JEGA_WINDOWS
#endif

/// See EDDY_COMMA_IF_THREADSAFE.
#define JEGA_COMMA_IF_THREADSAFE EDDY_COMMA_IF_THREADSAFE

/// See EDDY_IF_THREADSAFE.
#define JEGA_IF_THREADSAFE(a) EDDY_IF_THREADSAFE(a)

/// See EDDY_IF_NO_THREADSAFE.
#define JEGA_IF_NO_THREADSAFE(a) EDDY_IF_NO_THREADSAFE(a)

/// See EDDY_IF_BOOST.
#define JEGA_IF_BOOST(a) EDDY_IF_BOOST(a)

/// See EDDY_IF_NO_BOOST.
#define JEGA_IF_NO_BOOST(a) EDDY_IF_NO_BOOST(a)

/*
================================================================================
Shared Library Specifications
================================================================================
*/

/// See EDDY_SL_EXPORT.
#define JEGA_SL_EXPORT EDDY_SL_EXPORT

/// See EDDY_SL_IMPORT.
#define JEGA_SL_IMPORT EDDY_SL_IMPORT

/// See EDDY_SL_IEDECL.
#define JEGA_SL_IEDECL EDDY_SL_IEDECL



/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {




/*
================================================================================
End Namespace
================================================================================
*/
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGACONFIG_HPP
