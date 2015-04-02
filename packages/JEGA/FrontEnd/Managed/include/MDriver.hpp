/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MDriver.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Feb 09 08:47:20 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MDriver class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MDRIVER_HPP
#define JEGA_FRONTEND_MANAGED_MDRIVER_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>
#include <MProblemConfig.hpp>
#include <MAlgorithmConfig.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace FrontEnd
    {
        class Driver;
    }
}







/*
================================================================================
Namespace Aliases
================================================================================
*/
#pragma managed
#using <mscorlib.dll>








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace Managed {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
MANAGED_CLASS_FORWARD_DECLARE(public, MSolution);
MANAGED_CLASS_FORWARD_DECLARE(public, MAlgorithmConfig);
MANAGED_CLASS_FORWARD_DECLARE(public, MGeneticAlgorithm);





/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A managed wrapper class for the JEGA::FrontEnd::Driver class.
 *
 * This class keeps an instance of a JEGA::FrontEnd::Driver and forwards all
 * work along to it.  It will also provide a reference to that instance if
 * so desired with a call to the Manifest method.
 */
MANAGED_CLASS(public, MDriver) :
    public System::IDisposable
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        static const std::size_t THROW = JEGA::Logging::Logger::THROW;

        static const std::size_t ABORT = JEGA::Logging::Logger::ABORT;

        /// The underlying JEGA::FrontEnd::Driver for which this proxies.
        JEGA::FrontEnd::Driver* _theApp;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /**
         * \brief Allows access to the seed actually provided to the random
         *        number generator.
         *
         * This is not necessarily the same as the value provided to the
         * InitializeJEGA method.  When that method is provided with a sentinal
         * seed value, a seed is generated based on the clock and the time.
         * Therefore, this method provides access to whatever value was
         * actually used.
         *
         * \return The seed value actually used in a call to srand.
         */
        static
        System::UInt32
        GetRandomSeed(
            );

        /**
         * \brief Allows access to the seed file name provided to the global log
         *        for log messages.
         *
         * This will be the same name that was provided to the InitializeJEGA
         * method.  It is provided as a convenience so that that information
         * does not have to be remembered externally.
         *
         * \return The name of the file to which the global log is logging if
         *         file logging is enabled.
         */
        static
        System::String MOH
        GetGlobalLogFilename(
            );

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Indicates whether or not JEGA has been successfully
         *        initialized.
         *
         * \return true if InitializeJEGA was called and completed
         *         successfully and false otherwise.
         */
        static
        bool
        IsJEGAInitialized(
            );

        /**
         * \brief Allows access to the proxied JEGA front end Driver wrapped by
         *        this object.
         *
         * \returm The JEGA::FrontEnd::Driver that is underlying this managed
         *         wrapper.
         */
        JEGA::FrontEnd::Driver&
        Manifest(
            );

        /// Method called to do 1-time initialization of the JEGA project.
        /**
         * This method's primary job is to initialize the global logger and
         * seed the random number generator.  Call this method once before
         * using JEGA and not again.  You can execute as many GA's as you wish
         * having only called this method once.
         *
         * In order to ensure that it only gets called once, your code should
         * look like this:
         *
         * \code
               if(!MDriver::IsJEGAInitialized()) MDriver::InitializeJEGA(...);
           \endcode
         *
         * \param globalLogFilename The name of the file to which the global
         *                          log writes entries.  This is ignored if
         *                          file logging is disabled.
         * \param globalLogDefLevel The default level at which the global log
         *                          will pass entries.  This is ignored if
         *                          logging is disabled.
         * \param rSeed             The seed to supply the random number
         *                          generator.  A supplied value of 0 causes
         *                          the seed to be randomized by time() and
         *                          clock().
         * \return true if initialization completes successfully and false
         *         otherwise (actually always true).
         */
        static
        bool
        InitializeJEGA(
            System::String MOH globalLogFilename,
            JEGA::Logging::LogLevel globalLogDefLevel,
            System::UInt32 rSeed,
            MAlgorithmConfig::FatalBehavior fatalBehavior
            );

        /**
         * \brief Re-seeds the random number generator.
         *
         * This will change the value returned by the GetRandomSeed method so if
         * previous seeds are of interest to you, you must store them yourself.
         * An entry will be logged in the global log for the reseeding.
         *
         * \param rSeed The seed to supply the random number generator.  A
         *              supplied value of 0 causes the seed to be randomized
         *              by time() and clock().
         * \return The seed that was acutally used in re-seeding.  This may be
         *         the value passed in or if 0 is supplied, it will be the value
         *         generated using time() and clock().
         */
        static
        System::UInt32
        ReSeed(
            System::UInt32 rSeed
            );

        static
        void
        ResetGlobalLoggingLevel(
            JEGA::Logging::LogLevel globalLogDefLevel
            );

        static
        void
        FlushGlobalLogStreams(
            );

        /**
         * \brief Creates a new algorithm according to the supplied algorithm
         *        config and runs it.
         *
         * \param config The configuration object describing the algorithm to
         *               run.
         * \return A collection of the final solutions found by the algorithm.
         */
        SolutionVector MOH
        ExecuteAlgorithm(
            MAlgorithmConfig MOH config
            );

        MGeneticAlgorithm MOH
        InitializeAlgorithm(
            MAlgorithmConfig MOH config
            );

        bool
        PerformNextIteration(
            MGeneticAlgorithm MOH theGA
            );

        SolutionVector MOH
        FinalizeAlgorithm(
            MGeneticAlgorithm MOH theGA
            );

        /**
         * \brief Deletes the supplied GA.
         *
         * Use this method to destroy a GA after all iterations have been run.
         * This method knows if the log associated with the GA was created here
         * and needs to be destroyed as well or not.
         *
         * Be sure to use this prior to destroying the algorithm config object
         * which contains the target.  The GA destructor needs the target to
         * be in tact.
         *
         * \param theGA The algorithm that is no longer needed and thus must be
         *              destroyed.
         */
        void
        DestroyAlgorithm(
            MGeneticAlgorithm MOH theGA
            );


    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Implementation of the Dispose method for the IDisposable interface.
        /**
         * This method destroys the underlying JEGA driver.
         * It is safe to call this more than once but it is not safe to use
         * the other methods of this class once this has been called.
         *
         * The destructor should call this method.
         */
        virtual
        void
        MANAGED_DISPOSE(
            );


    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs an MDriver to solve the supplied problem.
        /**
         * \param probConfig The configuration describing the problem that is
         *                   to be solved by this driver.
         */
        MDriver(
            MProblemConfig MOH probConfig
            );

        /// Destructs an MDriver.
        /**
         * The body of this method calls the Dispose method to destroy the
         * JEGA core driver object.
         */
        ~MDriver(
            );

}; // class MDriver



/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
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
#endif // JEGA_FRONTEND_MANAGED_MDRIVER_HPP
