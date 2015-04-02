/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MAlgorithmConfig.

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

        Mon Feb 20 07:30:18 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MAlgorithmConfig class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MALGORITHMCONFIG_HPP
#define JEGA_FRONTEND_MANAGED_MALGORITHMCONFIG_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>

#pragma unmanaged
#include <../FrontEnd/Core/include/AlgorithmConfig.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/







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
MANAGED_CLASS_FORWARD_DECLARE(public, MEvaluator);
MANAGED_INTERFACE_FORWARD_DECLARE(public, MParameterDatabase);






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
 * \brief A managed wrapper for the JEGA::FrontEnd::AlgorithmConfig object.
 *
 * This class uses a JEGA::FrontEnd::AlgorithmConfig instance internally and
 * exposes the required methods in a managed compliant manner for use from
 * .NET languages.
 */
MANAGED_CLASS(public, MAlgorithmConfig) :
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
    Class Constants
    ============================================================================
    */
    public:

        /// An enum to mirror the one in JEGA::FrontEnd::AlgorithmConfig.
        MANAGED_ENUM(AlgType)
        {
            /// The tag for a MOGA
            MOGA = JEGA::FrontEnd::AlgorithmConfig::MOGA,

            /// The tag for a SOGA
            SOGA = JEGA::FrontEnd::AlgorithmConfig::SOGA

        }; // enum AlgType

        /// An enum to mirror the one in the JEGA::Logging::Logger class.
        MANAGED_ENUM(FatalBehavior)
        {
            /// The tag for abort behavior
            ABORT = JEGA::Logging::Logger::ABORT,

            /// The tag for throw behavior
            THROW = JEGA::Logging::Logger::THROW

        }; // enum FatalBehavior

        /**
         * \brief A representation of the debug logging level for JEGA for use
         *        in managed projects.
         */
        static const JEGA::Logging::LogLevel Debug = 0;
 //           JEGA::Logging::ldebug();

        /**
         * \brief A representation of the verbose logging level for JEGA for
         *        use in managed projects.
         */
        static const JEGA::Logging::LogLevel Verbose = 1;
//            JEGA::Logging::lverbose();

        /**
         * \brief A representation of the normal logging level for JEGA for use
         *        in managed projects.
         */
        static const JEGA::Logging::LogLevel Normal = 2;

        /**
         * \brief A representation of the quiet logging level for JEGA for use
         *        in managed projects.
         */
        static const JEGA::Logging::LogLevel Quiet = 3;
//            JEGA::Logging::lquiet();

        /**
         * \brief A representation of the fatal logging level for JEGA for use
         *        in managed projects.
         */
        static const JEGA::Logging::LogLevel Fatal = UCHAR_MAX;
//            JEGA::Logging::lfatal();

        /**
         * \brief A representation of the silent logging level for JEGA for use
         *        in managed projects.
         */
        static const JEGA::Logging::LogLevel Silent = 4;
//            JEGA::Logging::lsilent();

        /**
         * \brief A representation of the default logging level for JEGA for
         *        use in managed projects.
         *
         * The default is currently quiet.
         */
        static const JEGA::Logging::LogLevel Default = 2;
//            JEGA::Logging::lquiet();


    protected:


    private:

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// A pointer to the actual underlying run configuration object.
        JEGA::FrontEnd::AlgorithmConfig* _theConfig;

        /**
         * \brief A pointer to the ParameterDatabase created by this class for
         *        the underlying JEGA algorithm config.
         */
        MParameterDatabase MOH _thePDB;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /**
         * \brief Sets the name of the algorithm for which this is the
         *        configuration.
         *
         * This is a convenience method.  The effect of this call can be
         * obtained by a call to
         * \code
           GetParameterDB().AddStringParam("method.jega.algorithm_name", name);
           \endcode
         *
         * \param name The name for the algorithm for which this is the
         *             configuration.
         * \return True if the name is successfully loaded into the parameter
         *         database and false otherwise.
         */
        bool
        SetAlgorithmName(
            System::String MOH name
            );

        /**
         * \brief Sets the flag that determines whether or not each population
         *        should be printed to a file.
         *
         * \param doPrint True if each population should be printed to a file
         *                and false otherwise.
         */
        bool
        SetPrintPopEachGen(
            bool doPrint
            );

        /**
         * \brief Sets the flag that determines what type of algorithm will be
         *        run.
         *
         * Choices are one of MOGA and SOGA from the AlgType enumeration.
         *
         * \param algType The type of algorithm to run.
         */
        bool
        SetAlgorithmType(
            AlgType algType
            );

        /**
         * \brief Sets the default level at which the Logger created for the
         *        algorithm run as a result of utilization of this
         *        configuration object will log.
         *
         * This is ignored if the logging filename is empty.
         *
         * \param defLevel The default level for the Logger for the algorithm
         *                 created.
         */
        bool
        SetDefaultLoggingLevel(
            JEGA::Logging::LogLevel defLevel
            );

        /**
         * \brief Sets the name of the file to which the algorithm run as a
         *        result of utilization of this configuration object will log.
         *
         * Leave this empty (do not call this method or call it with an empty
         * string) if you wish for this algorithm to log into the global log.
         *
         * \param filename The name of the file for the created algorithm to
         *                 log into.
         */
        bool
        SetLoggingFilename(
            System::String MOH filename
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /**
         * \brief Allows access to the parameter database for loading in
         *        parameter values.
         *
         * \return The parameter database from which configuration of the
         *         algorithm will be extracted by JEGA.
         */
        MParameterDatabase MOH
        GetParameterDB(
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
         * \brief Allows access to the JEGA core algorithm configuration object
         *        wrapped by this managed type.
         *
         * \return The JEGA front end algortihm config that will actually be
         *         used by JEGA to configure an algorithm to be run.
         */
        const JEGA::FrontEnd::AlgorithmConfig&
        Manifest(
            );

        /**
         * \brief Stores the supplied \a name as the name of the mutator
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the mutator operator, this
         * operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.mutation_type", name);
           \endcode
         *
         * \param name The name of the mutator operator for the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetMutatorName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the converger
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the converger operator, this
         * operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.convergence_type", name);
           \endcode
         *
         * \param name The name of the converger operator for the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetConvergerName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the crossover
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the crossover operator, this
         * operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.crossover_type", name);
           \endcode
         *
         * \param name The name of the crossover operator for the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetCrosserName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the niche pressure
         *        application operator to be used by the algorithm.
         *
         * If there is already a name supplied for the niche pressure
         * application operator, this operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.jega.niching_type", name);
           \endcode
         *
         * \param name The name of the niche pressure application operator for
         *             the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetNichePressureApplicatorName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the
         *        fitness assessment operator to be used by the algorithm.
         *
         * If there is already a name supplied for the fitness assessment
         * operator, this operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.fitness_type", name);
           \endcode
         *
         * \param name The name of the fitness assessment operator for the
         *             algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetFitnessAssessorName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the selector
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the selector operator, this
         * operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.replacement_type", name);
           \endcode
         *
         * \param name The name of the selector operator for the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetSelectorName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the post processor
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the post processor operator,
         * this operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam(
                "method.jega.postprocessing_type", name
                );
           \endcode
         *
         * \param name The name of the post processor operator for the
         *             algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetPostProcessorName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the initializer
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the initializer operator,
         * this operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.initialization_type", name);
           \endcode
         *
         * \param name The name of the initializer operator for the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetInitializerName(
            System::String MOH name
            );

        /**
         * \brief Stores the supplied \a name as the name of the main loop
         *        operator to be used by the algorithm.
         *
         * If there is already a name supplied for the main loop operator,
         * this operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.jega.mainloop_type", name);
           \endcode
         *
         * \param name The name of the main loop operator for the algorithm.
         * \return true if the name gets stored in the ParameterDatabase and
         *         false otherwise.
         */
        bool
        SetMainLoopName(
            System::String MOH name
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
         * This method destroys the underlying JEGA algorithm configuration.
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


        /**
         * \brief Constructs an MAlgorithmConfig to use the supplied database
         *        and evaluator.
         *
         * \param thePDB The parameter database that is to be used by this
         *               algorithm configuration object.
         * \param theEvaluator The managed evaluator through which evaluations
         *                     will be performed.
         */
        MAlgorithmConfig(
            MParameterDatabase MOH thePDB,
            MEvaluator MOH theEvaluator
            );

        /**
         * \brief Constructs an MAlgorithmConfig to use the supplied evaluator.
         *
         * The default database is a new instance of the
         * MBasicParameterDatabaseImpl.
         *
         * \param theEvaluator The managed evaluator through which evaluations
         *                     will be performed.
         */
        MAlgorithmConfig(
            MEvaluator MOH theEvaluator
            );

        /// Finalizes an MAlgorithmConfig.
        /**
         * The body of this method calls the Dispose method to destroy the
         * JEGA core algorithm configuration object.
         */
        ~MAlgorithmConfig(
            );




}; // class MAlgorithmConfig



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
#endif // JEGA_FRONTEND_MANAGED_MALGORITHMCONFIG_HPP
