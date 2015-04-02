/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Definition of class AlgorithmConfig.

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

        Tue Feb 07 15:40:43 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the AlgorithmConfig class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_JEGAALGORITHMCONFIG_HPP
#define JEGA_FRONTEND_JEGAALGORITHMCONFIG_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/JEGATypes.hpp>






/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
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
class AlgorithmConfig;
class EvaluatorCreator;







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
 * \brief An object to store the configuration of a JEGA algorithm.
 *
 * This includes information such as which type of algorithm to use, what
 * operators to use, the parameters for the operators, how to get a new
 * evaluator, etc.  The evaluator is managed by the algorithm config so that
 * the same problem can be solved using different evaluators (different
 * fidelities, etc.).
 *
 * The driver can be called with one of these objects to run an algorithm.
 * The same driver can be used to run multiple algorithms on the same
 * problem with different algorithm configs.
 *
 * This class is not thread safe and should therefore be loaded from within
 * a single thread.  It will be read from a single thread by the Driver.
 */
class JEGA_SL_IEDECL AlgorithmConfig
{
    /*
    ============================================================================
    Class Scope Enumerations
    ============================================================================
    */
    public:

        /// The tags for the different available GA specializations.
        enum AlgType
        {
            /// The tag for a MOGA.
            MOGA,

            /// The tag for a SOGA
            SOGA

        }; // enum AlgType

    protected:

    private:

    /*
    ============================================================================
    Class Constants
    ============================================================================
    */
    public:

    protected:

    private:


    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A shorthand for the level type used by the logging project.
        typedef
        JEGA::Logging::LogLevel
        LevelType;

    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /**
         * \brief The parameter database into which data is loaded and from
         *        which it is retreived.
         *
         * This includes all information needed by the algorithm to run.  For
         * example, the names of the operators, the values the operators need,
         * and any input data the algorithms themselves need.
         */
        JEGA::Utilities::ParameterDatabase& _theParamDB;

        /**
         * \brief The object that will create the evaluator that the algorithm
         *        built using this config will use.
         */
        EvaluatorCreator& _theEvalCreator;


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
         * \brief Allows mutable access to the parameter database known by this
         *        config.
         *
         * \return The ParameterDatabase storing all algorithm parameters for
         *         this config.
         */
        JEGA::Utilities::ParameterDatabase&
        GetParameterDB(
            );

        /**
         * \brief Allows immutable access to the parameter database known by
         *        this config.
         *
         * \return The ParameterDatabase storing all algorithm parameters for
         *         this config.
         */
        const JEGA::Utilities::ParameterDatabase&
        GetParameterDB(
            ) const;

        /**
         * \brief Allows mutable access to the evaluator createor known by this
         *        config.
         *
         * \return The EvaluatorCreator used to create an evaluator for this
         *         algorithm.
         */
        EvaluatorCreator&
        GetTheEvaluatorCreator(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Stores the supplied \a name as the algorithm name in the
         *        parameter database.
         *
         * If there is already a name supplied for the algorithm, this
         * operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.jega.algorithm_name", name);
           \endcode
         *
         * \param name The name for the algorithm created using this config.
         * \return True if the algorithm name is stored and false otherwise.
         */
        bool
        SetAlgorithmName(
            const std::string& name
            );

        /**
         * \brief Stores the supplied flag to indicate whether the algorithm
         *        should write each population to a data file or not.
         *
         * If there is already a value stored for this flag,  this
         * operation will fail.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddBooleanParam("method.print_each_pop", doPrint);
           \endcode
         *
         * \param doPrint Whether or not to write each population to a file.
         * \return True if the flag value is stored and false otherwise.
         */
        bool
        SetPrintPopEachGen(
            bool doPrint
            );

        /**
         * \brief Stores the supplied type to indicate what kind of algorithm
         *        is being created.
         *
         * If there is already an algorithm type stored,  this
         * operation will fail.
         *
         * What is actually stored is not the valeu of the parameter but a
         * string that indicates the algorithm type based on \a algType.
         *
         * \param algType The type of algorithm to create.
         * \return True if the algorithm type is stored and false otherwise.
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
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddIntegralParam("method.output", defLevel);
           \endcode
         *
         * \param defLevel The default level for the Logger for the algorithm
         *                 created.
         * \return True if the logging level entry is made to the database and
         *         false otherwise.
         */
        bool
        SetDefaultLoggingLevel(
            const LevelType& defLevel
            );

        /**
         * \brief Sets the name of the file to which the algorithm run as a
         *        result of utilization of this configuration object will log.
         *
         * Leave this empty (do not call this method or call it with an empty
         * string) if you wish for this algorithm to log into the global log.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam("method.log_file", filename);
           \endcode
         *
         * \param filename The name of the file for the created algorithm to
         *                 log into.
         * \return True if the logging file name is successfully set and false
         *         otherwise.
         */
        bool
        SetLoggingFilename(
            const std::string& filename
            );

        /**
         * \brief Sets the pattern for the name of the file to which the
         *        algorithm run as a result of utilization of this
         *        configuration object will write output.
         *
         * All occurances of a # in the pattern will be replaced by the number
         * of the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().AddStringParam(
                "method.jega.final_data_filename", fpattern
                );
           \endcode
         *
         * \param fpattern The pattern for the name of the file for the created
         *                 algorithm to write output to.
         * \return True if the logging file name is successfully set and false
         *         otherwise.
         */
        bool
        SetOutputFilenamePattern(
            const std::string& fpattern
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
            const std::string& name
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
            const std::string& name
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
            const std::string& name
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
            const std::string& name
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
            const std::string& name
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
            const std::string& name
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
                "method.jega.postprocessor_type", name
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
            const std::string& name
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
            const std::string& name
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
            const std::string& name
            );

        /**
         * \brief Retrieves the supplied algorithm name from the parameter
         *        database.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.jega.algorithm_name");
           \endcode
         *
         * \return The current name for the created algorithm.
         */
        std::string
        GetAlgorithmName(
            ) const;

        /**
         * \brief Stores the supplied flag to indicate whether the algorithm
         *        should write each population to a data file or not.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetBooleanParam("method.print_each_pop");
           \endcode
         *
         * \return Whether or not to write each population to a file.
         */
        bool
        GetPrintPopEachGen(
            ) const;

        /**
         * \brief Stores the supplied type to indicate what kind of algorithm
         *        is being created.
         *
         * What is actually stored is not the value of the parameter but a
         * string that indicates the algorithm type based on \a algType.
         *
         * \return The current type of algorithm to create.
         */
        AlgType
        GetAlgorithmType(
            ) const;

        /**
         * \brief Gets the default level at which the Logger created for the
         *        algorithm run as a result of utilization of this
         *        configuration object will log.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetIntegralParam("method.output");
           \endcode
         *
         * \return The current default level for the Logger for the algorithm
         *         created.
         */
        LevelType
        GetDefaultLoggingLevel(
            ) const;

        /**
         * \brief Gets the name of the file to which the algorithm run as a
         *        result of utilization of this configuration object will log.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.log_file");
           \endcode
         *
         * \return The current name of the file for the created algorithm to
         *         log into.
         */
        std::string
        GetLoggingFilename(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the mutator
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.mutation_type");
           \endcode
         *
         * \return The current name of the mutator operator for the algorithm.
         */
        std::string
        GetMutatorName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the converger
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.convergence_type");
           \endcode
         *
         * \return The current name of the converger operator for the
         *         algorithm.
         */
        std::string
        GetConvergerName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the crossover
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.crossover_type");
           \endcode
         *
         * \return The current name of the crossover operator for the
         *         algorithm.
         */
        std::string
        GetCrosserName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the niche pressure
         *        application operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.jega.niching_type");
           \endcode
         *
         * \return The current name of the niche pressure application operator
         *         for the algorithm.
         */
        std::string
        GetNichePressureApplicatorName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the
         *        fitness assessment operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.fitness_type");
           \endcode
         *
         * \return The current name of the fitness assessment operator for the
         *         algorithm.
         */
        std::string
        GetFitnessAssessorName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the selector
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.replacement_type");
           \endcode
         *
         * \return The current name of the selector operator for the algorithm.
         */
        std::string
        GetSelectorName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the post processor
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam(
                "method.jega.postprocessor_type"
                );
           \endcode
         *
         * \return The current name of the post processor operator for the
         *         algorithm.
         */
        std::string
        GetPostProcessorName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the initializer
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.initialization_type");
           \endcode
         *
         * \return The current name of the initializer operator for the
         *         algorithm.
         */
        std::string
        GetInitializerName(
            ) const;

        /**
         * \brief Stores the supplied \a name as the name of the main loop
         *        operator to be used by the algorithm.
         *
         * The result of this method can be equivolently achieved by a call to
         * \code
           GetParameterDB().GetStringParam("method.jega.mainloop_type");
           \endcode
         *
         * \return The current name of the main loop operator for the
         *         algorithm.
         */
        std::string
        GetMainLoopName(
            ) const;

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

        /// Constructs an AlgorithmConfig using the supplied parameters.
        /**
         * \param creator The EvaluatorCreator that will supply the resulting
         *                algorithm with an evaluator instance.
         * \param pdb The ParameterDatabase into which and from which all
         *            configuration data will be placed/stored.
         */
        AlgorithmConfig(
            EvaluatorCreator& creator,
            JEGA::Utilities::ParameterDatabase& pdb
            );




}; // class AlgorithmConfig



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
#include "inline/AlgorithmConfig.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_JEGAALGORITHMCONFIG_HPP
