/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Configuration File Front End

    CONTENTS:

        The application entry point.

    NOTES:

        

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Mon Oct 30 14:23:13 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the entry point for the configuration file front end to
 *        JEGA.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../FrontEnd/ConfigFile/include/ConfigFileFrontEndApp.hpp>





/*
================================================================================
Application Entry Point
================================================================================
*/
/**
 * \brief The entry point for the configuration file front end to JEGA.
 *
 * This method does nothing but instantiate a ConfigFileFrontEndApp and call
 * Run on it passing the supplied command line arguments.
 *
 * \param argc The number of command line arguments.
 * \param argv The vector of command line arguments.
 */
int
main(
    int argc,
    char* argv[]
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Create the main application class.
    JEGA::FrontEnd::ConfigFile::ConfigFileFrontEndApp app;
    return app.Run(argc, argv);

} // main
