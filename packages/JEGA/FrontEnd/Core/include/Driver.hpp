/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Definition of class Driver.

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

        Fri Jan 06 07:40:17 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the Driver class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_JEGA_FEAPP_HPP
#define JEGA_FRONTEND_JEGA_FEAPP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <map>
#include <string>
#include <utility>
#include <../Utilities/include/Logging.hpp>





/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Algorithms
    {
        class GeneticAlgorithm;
        class GeneticAlgorithmMutator;
        class GeneticAlgorithmCrosser;
        class GeneticAlgorithmOperator;
        class GeneticAlgorithmMainLoop;
        class GeneticAlgorithmSelector;
        class GeneticAlgorithmConverger;
        class GeneticAlgorithmEvaluator;
        class GeneticAlgorithmInitializer;
        class GeneticAlgorithmOperatorSet;
        class GeneticAlgorithmPostProcessor;
        class GeneticAlgorithmFitnessAssessor;
        class GeneticAlgorithmNichePressureApplicator;
    }

    namespace Utilities
    {
        class DesignOFSortSet;
        class ParameterDatabase;
    }
}




/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class Driver;
class ProblemConfig;
class AlgorithmConfig;







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
 * \brief The main driver class of the JEGA front end project.
 *
 * This class serves as the driver of JEGA.  It allows a user to initalize JEGA
 * and execute algorithms.  An instance of this class is intimately linked to
 * a problem configuration.  Therefore, a single instance of a Driver can only
 * solve a single problem but can do it multiple times using different
 * algorithms and evaluators.  To solve multiple problems, you must create
 * multiple Drivers.
 */
class JEGA_SL_IEDECL Driver
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:

        /// A map to link a GA the logger that was created for it.
        typedef
        std::map<JEGA::Algorithms::GeneticAlgorithm*, JEGA::Logging::Logger*>
        GALoggerMap;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /**
         * \brief A flag that indicates whether or not InitializeJEGA has been
         *        called and returned successfully.
         */
        static bool _initialized;

        /**
         * \brief The value that was ultimately used as the random seed when
         *        InitializeJEGA was called.
         *
         * This may not be the same as the value supplied to the method.  If
         * the sentry value of 0 is passed to InitializeJEGA, then this will
         * store the value determined using time and clock.
         */
        static unsigned int _rSeed;

        /**
         * \brief The description of the problem that is solved whenever
         *        the ExecuteAlgorithm method is called.
         */
        const ProblemConfig& _probConfig;

        /**
         * \brief A mapping of the GA's created by this Driver and the
         *        associated logs it is responsible for destroying in turn.
         */
        GALoggerMap _myLogs;

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
        unsigned int
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
        const std::string&
        GetGlobalLogFilename(
            );

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

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

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
               if(!Driver::IsJEGAInitialized()) Driver::InitializeJEGA(...);
           \endcode
         *
         * \param globalLogFilename The name of the file to which the global
         *                          log writes entries.  This is ignored if
         *                          file logging is disabled.
         * \param globalLogDefLevel The default level at which the global log
         *                          will pass entries.  This is ignored if
         *                          logging is disabled.
         * \param rSeed             The seed to supply the random number
         *                          generator.  The default value of 0 causes
         *                          the seed to be randomized by time() and
         *                          clock().
         * \return true if initialization completes successfully and false
         *         otherwise (actually always true).
         */
        static
        bool
        InitializeJEGA(
            const std::string& globalLogFilename = "JEGAGlobal.log",
            const JEGA::Logging::LogLevel& globalLogDefLevel =
                JEGA::Logging::LevelClass::Default,
            unsigned int rSeed = 0,
            JEGA::Logging::Logger::FatalBehavior onFatal =
                JEGA::Logging::Logger::ABORT
            );

        /**
         * \brief Re-seeds the random number generator.
         *
         * This will change the value returned by the GetRandomSeed method so if
         * previous seeds are of interest to you, you must store them yourself.
         * An entry will be logged in the global log for the reseeding.
         *
         * \param rSeed The seed to supply the random number generator.  The
         *              default value of 0 causes the seed to be randomized
         *              by time() and clock().
         * \return The seed that was acutally used in re-seeding.  This may be
         *         the value passed in or if the default is supplied, it will be
         *         the value generated using time() and clock().
         */
        static
        unsigned int
        ReSeed(
            unsigned int rSeed = 0
            );

        static
        void
        ResetGlobalLoggingLevel(
            const JEGA::Logging::LogLevel& globalLogDefLevel
            );

        static
        void
        FlushGlobalLogStreams(
            );

        /**
         * \brief Creates a new algorithm according to the supplied algorithm
         *        config and runs it.
         *
         * The final solutions are not accounted for by this algorithm and will
         * have to be destroyed when you no longer need them using a call to
         * flush() on the returned DesignOFSortSet.  Failure to do this will
         * constitute a memory leak.
         *
         * \param algConfig The configuration object describing the algorithm
         *                  to run.
         * \return A collection of the final solutions found by the algorithm.
         */
        JEGA::Utilities::DesignOFSortSet
        ExecuteAlgorithm(
            const AlgorithmConfig& algConfig
            );

        JEGA::Algorithms::GeneticAlgorithm*
        InitializeAlgorithm(
            const AlgorithmConfig& algConfig
            );

        bool
        PerformNextIteration(
            JEGA::Algorithms::GeneticAlgorithm* theGA
            );

        JEGA::Utilities::DesignOFSortSet
        FinalizeAlgorithm(
            JEGA::Algorithms::GeneticAlgorithm* theGA
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
            JEGA::Algorithms::GeneticAlgorithm* theGA
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Reads all required data from the problem description database
         *        stored in the supplied algorithm config.
         *
         * The returned GA is fully configured and ready to be run.  It must
         * also be destroyed at some later time.  You MUST call
         * DestroyAlgorithm for this purpose.  Failure to do so could result
         * in a memory leak and an eventual segmentation fault!  Be sure to
         * call DestroyAlgorithm prior to destroying the algorithm config that
         * was used to create it!
         *
         * \param algConfig The fully loaded configuration object containing
         *                  the database of parameters for the algorithm to be
         *                  run on the known problem.
         * \return The fully configured and loaded GA ready to be run using
         *         the PerformIterations method.
         */
        JEGA::Algorithms::GeneticAlgorithm*
        ExtractAllData(
            const AlgorithmConfig& algConfig
            );

        /**
         * \brief Performs the required iterations on the supplied GA.
         *
         * This includes the calls to AlgorithmInitialize and AlgorithmFinalize
         * and logs some information if appropriate.
         *
         * \param theGA The GA on which to perform iterations.  This parameter
         *              must be non-null.
         * \return The final solutions reported by the supplied GA after all
         *         iterations and call to AlgorithmFinalize.
         */
        JEGA::Utilities::DesignOFSortSet
        PerformIterations(
            JEGA::Algorithms::GeneticAlgorithm* theGA
            );

        /// Creates a new algorithm of the type indicated in the configuration.
        /**
         * This does nothing more than instantiate an algorithm of the correct
         * type.  It does not endow it with properties, operators, etc.  That
         * must be done in a subsequent step.
         *
         * \param algConfig The configuration object from which to detect the
         *                  desired algorithm type.
         * \param logger The logger object that the GA is to know and log to.
         * \return The newly created but not loaded algorithm of the desired
         *         type.
         */
        JEGA::Algorithms::GeneticAlgorithm*
        CreateNewAlgorithm(
            const AlgorithmConfig& algConfig,
            JEGA::Logging::Logger& logger
            );

        /**
         * \brief Calls srand with the supplied random seed value.
         *
         * This method also logs a message indicating the seed value to the
         * global logger.
         *
         * \param seed The seed for the random number generator.
         */
        static
        void
        SeedRandomGenerator(
            unsigned int seed
            );

        /**
         * \brief Endows the supplied algorithm with properties according to
         *        information in the supplied configuration.
         *
         * This includes the setting of all algorithm specific parameters,
         * installation of all algorithm operators, and endowment of those
         * operators with parameter values.
         *
         * \param theGA The genetic algorithm to load.
         * \param algConfig The configuration object from which to obtain the
         *                  information necessary to load the GA.
         */
        static
        void
        LoadAlgorithm(
            JEGA::Algorithms::GeneticAlgorithm& theGA,
            const AlgorithmConfig& algConfig
            );

        /// This method verifies that \a op is not null.
        /**
         * If it is, this method logs a fatal error to the global logger.
         *
         * \param op The operator to verify as a valid one.
         * \param type The name of the type of this operator for display in
         *             the error message.
         * \param name The name by which the operator was retrieved for display
         *             in the error message.
         */
        static
        void
        VerifyValidOperator(
            JEGA::Algorithms::GeneticAlgorithmOperator* op,
            const std::string& type,
            const std::string& name
            );

        /**
         * \brief Retrieves a Mutator by the supplied name and sets it as the
         *        Mutator in the supplied operator set.
         *
         * \param name The name of the Mutator to retrieve.
         * \param into The operator set in which to put the Mutator after
         *             successful retrieval.
         * \return A pointer to the newly created Mutator or null if failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmMutator*
        SetMutator(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a Converger by the supplied name and sets it as the
         *        Converger in the supplied operator set.
         *
         * \param name The name of the Converger to retrieve.
         * \param into The operator set in which to put the Converger after
         *             successful retrieval.
         * \return A pointer to the newly created Converger or null if failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmConverger*
        SetConverger(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a Crosser by the supplied name and sets it as the
         *        Crosser in the supplied operator set.
         *
         * \param name The name of the Crosser to retrieve.
         * \param into The operator set in which to put the Crosser after
         *             successful retrieval.
         * \return A pointer to the newly created Crosser or null if failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmCrosser*
        SetCrosser(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a NichePressureApplicator by the supplied name and
         *        sets it as the NichePressureApplicator in the supplied
         *        operator set.
         *
         * \param name The name of the NichePressureApplicator to retrieve.
         * \param into The operator set in which to put the
         *             NichePressureApplicator after successful retrieval.
         * \return A pointer to the newly created NichePressureApplicator or
         *         null if failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmNichePressureApplicator*
        SetNichePressureApplicator(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a FitnessAssessor by the supplied name and sets it
         *        as the FitnessAssessor in the supplied operator set.
         *
         * \param name The name of the FitnessAssessor to retrieve.
         * \param into The operator set in which to put the FitnessAssessor
         *             after successful retrieval.
         * \return A pointer to the newly created FitnessAssessor or null if
         *         failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmFitnessAssessor*
        SetFitnessAssessor(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a Selector by the supplied name and sets it as the
         *        Selector in the supplied operator set.
         *
         * \param name The name of the Selector to retrieve.
         * \param into The operator set in which to put the Selector after
         *             successful retrieval.
         * \return A pointer to the newly created Selector or null if failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmSelector*
        SetSelector(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a post processor by the supplied name and sets it
         *        as the post processor in the supplied operator set.
         *
         * \param name The name of the post processor to retrieve.
         * \param into The operator set in which to put the post processor
         *             after successful retrieval.
         * \return A pointer to the newly created post processor or null if
         *         failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmPostProcessor*
        SetPostProcessor(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a Initializer by the supplied name and sets it as
         *        the Initializer in the supplied operator set.
         *
         * \param name The name of the Initializer to retrieve.
         * \param into The operator set in which to put the Initializer after
         *             successful retrieval.
         * \return A pointer to the newly created Initializer or null if
         *         failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmInitializer*
        SetInitializer(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /**
         * \brief Retrieves a MainLoop by the supplied name and sets it as the
         *        MainLoop in the supplied operator set.
         *
         * \param name The name of the MainLoop to retrieve.
         * \param into The operator set in which to put the MainLoop after
         *             successful retrieval.
         * \return A pointer to the newly created MainLoop or null if failure.
         */
        static
        JEGA::Algorithms::GeneticAlgorithmMainLoop*
        SetMainLoop(
            const std::string& name,
            JEGA::Algorithms::GeneticAlgorithmOperatorSet& into
            );

        /// Retrieves the algorithm name stored in \a from.
        /**
         * This is the value mapped to the method.jega.algorithm_name key
         * in the ParameterDatabase of \a from.
         *
         * \param from The loaded algorithm configuration from which to
         *             retrieve the algorithm name.
         * \return The algorithm name stored in the parameter database of
         *         \a from.
         */
        static
        std::string
        GetAlgorithmName(
            const AlgorithmConfig& from
            );

        /// Retrieves the algorithm type stored in \a from.
        /**
         * This is the value mapped to the method.algorithm key
         * in the ParameterDatabase of \a from.
         *
         * \param from The loaded algorithm configuration from which to
         *             retrieve the algorithm type.
         * \return The algorithm type stored in the parameter database of
         *         \a from.
         */
        static
        std::string
        GetAlgorithmType(
            const AlgorithmConfig& from
            );

        /**
         * \brief Retrieves the name of the log file that will be written to by
         *        the algorithm.
         *
         * This is the value mapped to the method.log_file key
         * in the ParameterDatabase of \a from.
         *
         * \param from The loaded algorithm configuration from which to
         *             retrieve the algorithm's log file name.
         * \return The log file name stored in the parameter database of
         *         \a from.
         */
        static
        std::string
        GetAlgorithmLogFilename(
            const AlgorithmConfig& from
            );

        /**
         * \brief Retrieves the level at which the Logger created for the
         *        algorithm will log by default.
         *
         * This is the value mapped to the method.output key
         * in the ParameterDatabase of \a from.
         *
         * \param from The loaded algorithm configuration from which to
         *             retrieve the algorithm's log's default logging level.
         * \return The log level stored in the parameter database of
         *         \a from.
         */
        static
        JEGA::Logging::LogLevel
        GetAlgorithmDefaultLogLevel(
            const AlgorithmConfig& from
            );

        /**
         * \brief Does a deep copy of the supplied set of Designs.
         *
         * \param from A set of designs on which to perform a deep copy.
         * \return A new set of clones of the Designs in \a from.
         */
        static
        JEGA::Utilities::DesignOFSortSet
        DeepDuplicate(
            const JEGA::Utilities::DesignOFSortSet& from,
            bool moveTags = true
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:


        /// This method issues a logging message describing the signal.
        /**
         * The logging message is not issued at the fatal level because it is
         * desirable to allow the program to exit in whatever way it was about
         * to (or not).
         *
         * \param val The value of the signal that was caught and sent here.
         */
        static
        void
        handle_signal(
            int val
            ) throw();




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a Driver
        /**
         * The problem can be solved in multiple ways by multiple algorithms
         * even using multiple different evaluators by issuing multiple calls
         * to ExecuteAlgorithm with different AlgorithmConfigs.
         *
         * \param probConfig The definition of the problem to be solved by this
         *                   Driver whenever ExecuteAlgorithm is called.
         */
        Driver(
            const ProblemConfig& probConfig
            );

        virtual
        ~Driver(
            );


}; // class Driver



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/Driver.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_JEGA_FEAPP_HPP
