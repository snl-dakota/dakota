/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Configuration File Front End

    CONTENTS:

        Definition of class ConfigFileFrontEndApp.

    NOTES:

        See notes under Class Definition section of this file.

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
 * \brief Contains the definition of the ConfigFileFrontEndApp class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_CONFIGFILE_CONFIGFILEFRONTENDAPP_HPP
#define JEGA_FRONTEND_CONFIGFILE_CONFIGFILEFRONTENDAPP_HPP





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#ifndef JEGA_HAVE_BOOST
#error A boost implementation must be available to use the \
configuration file front end to JEGA.  Please specify the location of \
a suitable version of boost and define the JEGA_HAVE_BOOST preprocessor \
symbol in order to use this feature.  This message will likely be \
followed by many compiler errors.  If you do not wish to specify a boost \
implementation, please remove this file from the compilation process. \
The remainder of JEGA will compile successfully into a library but the \
configuration file front end code will not be available.
#endif

#include <map>
#include <string>
#include <iosfwd>
#include <vector>
#include <../Utilities/include/Logging.hpp>



/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace boost
{
    namespace program_options
    {
        class variables_map;
        class variable_value;
        class options_description;
    }
}

namespace JEGA
{
    namespace FrontEnd
    {
        class ProblemConfig;
        class AlgorithmConfig;
        class EvaluatorCreator;
    }

    namespace Utilities
    {
        class ConstraintInfo;
        class DesignVariableInfo;
        class ObjectiveFunctionInfo;
        class BasicParameterDatabaseImpl;
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
        namespace ConfigFile {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class ConfigFileFrontEndApp;







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
 * \brief The main application that handles all data input for the JEGA
 *        configuration file front end.
 *
 * The majority of what this class does is the reading in, validation,
 * and subsequent marshaling of information in a configuration file.
 *
 * By default, this class supplies an external evaluator with information
 * obtained from the configuration file.  This class can be subclassed and the
 * GetEvaluatorCreator method can be overridden to obtain any desired evaluator
 * while still maintaining the configuration file input for all other JEGA
 * functionality.
 */
class ConfigFileFrontEndApp
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
    Nested Inner Class Forward Declares
    ============================================================================
    */
    public:

        /**
         * \brief A nested class that holds all the data that describes the
         *        various allowable inputs.
         *
         * This includes the string tags, the type map indices, and the default
         * values.
         */
        class InputDescriptors;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /**
         * \brief This is the variables map retrieved from the program_options
         *        project for all specified input.
         */
        boost::program_options::variables_map* _vMap;

        /**
         * \brief The parameter database loaded up from the data read in from
         *        the configuration file.
         */
        JEGA::Utilities::BasicParameterDatabaseImpl* _theParamDB;

        /**
         * \brief The problem configuration object loaded up from the data read
         *        in from the configuration file.
         */
        JEGA::FrontEnd::ProblemConfig* _theProbConfig;



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


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief The main activation method of this application class.
         *
         * \param argc The command line argument count.  This is typically
         *             passed directly from the main function.
         * \param argv The command line arguments.  This is typically passed
         *             directly from the main function.
         */
        int
        Run(
            int argc,
            char* argv[]
            );

        /**
         * \brief Returns the description object housing those input options
         *        that may only be specified on the command line for this app.
         *
         * \return The description object for the command line only input
         *         options.
         */
        static
        const boost::program_options::options_description&
        CommandLineOnlyInputOptions(
            );

        /**
         * \brief Returns the description object housing those input options
         *        that may or must be specified on the command line for this
         *        app.
         *
         * Some of these options may be shared.
         *
         * \return The description object for all the possible command line
         *         input options.
         */
        static
        const boost::program_options::options_description&
        AllCommandLineInputOptions(
            );

        /**
         * \brief Returns the description object housing those input options
         *        that may only be specified in a configuration file for this
         *        app.
         *
         * \return The description object for the config file only input
         *         options.
         */
        static
        const boost::program_options::options_description&
        ConfigFileOnlyInputOptions(
            );

        /**
         * \brief Returns the description object housing those input options
         *        that may or must be specified in a configuration file for
         *        this app.
         *
         * Some of these options may be shared.
         *
         * \return The description object for all the possible config file
         *         input options.
         */
        static
        const boost::program_options::options_description&
        AllConfigFileInputOptions(
            );

        /**
         * \brief Returns the description object housing those input options
         *        that may specified on the command line or in a configuration
         *        file for this app.
         *
         * \return The description object for all the input options that can
         *         be specified on the command line and/or in a config file.
         */
        static
        const boost::program_options::options_description&
        SharedInputOptions(
            );

        /**
         * \brief Returns the description object housing all input options
         *        for this app.
         *
         * \return The description object for all the input options.
         */
        static
        const boost::program_options::options_description&
        AllInputOptions(
            );

        /**
         * \brief Returns true if the supplied parameter was input either on
         *        the command line or in a configuration file.
         *
         * This says nothing about the validity of that input.
         *
         * \param param The string associated with the variable of interest.
         * \return True if an input exists and false otherwise (may be a
         *         default).
         */
        bool
        HasInputValueFor(
            const std::string& param
            ) const;

        /// Prints all supplied arguments to standard out.
        /**
         * \param stream The output stream into which to print all the supplied
         *               input arguments.
         */
        void
        PrintAllInputArgs(
            std::ostream& stream
            ) const;

        /// Prints all supplied command line args to standard out.
        /**
         * Some of these may simply be default values and not explicitly
         * supplied.
         *
         * \param stream The output stream into which to print the supplied
         *               command line input arguments.
         */
        void
        PrintSuppliedCommandLineArgs(
            std::ostream& stream
            ) const;

        /// Prints all supplied configuration file args to standard out.
        /**
         * Some of these may simply be default values and not explicitly
         * supplied.
         *
         * \param stream The output stream into which to print the supplied
         *               configuration file input arguments.
         */
        void
        PrintSuppliedConfigFileArgs(
            std::ostream& stream
            ) const;

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Outputs the help for the input of this application to standard out.
        /**
         * \param stream The output stream into which to write the help
         *               message.
         */
        void
        PrintHelpMessage(
            std::ostream& into
            ) const;

        /// Outputs the version of JEGA to standard out.
        /**
         * \param stream The output stream into which to write the version
         *               message.
         */
        void
        PrintVersionMessage(
            std::ostream& into
            ) const;

        /// Returns the variable_value for the supplied parameter.
        /**
         * Be certain to make sure that such a value exists by first
         * calling HasInputValueFor with param.
         *
         * \param param The string associated with the variable of interest.
         * \return The generic variable_value associated with "param".
         */
        const boost::program_options::variable_value&
        GetVariableValue(
            const std::string& param
            ) const;

        /**
         * \brief Transfers information stored in the variables_map by the
         *        program_options code into the known parameter database.
         */
        void
        LoadParameterDatabase(
            );

        /**
         * \brief Transfers information stored in the variables_map by the
         *        program_options code into the known problem configuration
         *        object.
         */
        void
        LoadProblemConfig(
            );

        /**
         * \brief Loads the algorithm parameters read and stored by the program
         *        options code into the supplied algorithm config object.
         *
         * \param aConfig The algorithm configuration object into which to
         *                load the supplied parameters.
         */
        void
        LoadAlgorithmParameters(
            JEGA::FrontEnd::AlgorithmConfig& aConfig
            );

        /**
         * \brief Records the design variable information supplied by the user
         *        into the known problem configuration object.
         */
        void
        LoadDesignVariables(
            );

        /**
         * \brief Records the objective function information supplied by the
         *        user into the known problem configuration object.
         */
        void
        LoadObjectiveFunctions(
            );

        /**
         * \brief Records the constraint information supplied by the user into
         *        the known problem configuration object.
         */
        void
        LoadConstraints(
            );

        /**
         * \brief Builds all double matrices from the supplied input.
         *
         * This is carried out by concatenating all double matrix entries with
         * the same identifer as rows into a single matrix.  The resulting
         * matrices are added to the known parameter database.
         */
        void
        LoadDoubleMatrices(
            );

        /**
         * \brief Verifes that the input algorithm type string is one of MOGA
         *        or SOGA.
         *
         * If the input string is not one of MOGA or SOGA, then a runtime_error
         * is thrown.
         */
        void
        ValidateAlgorithmType(
            );

        /**
         * \brief Converts the input logging level from the supplied string to
         *        an actual LogLevel value.
         *
         * If the supplied level string is not recongized, then the default
         * logging level as defined by the dakota_levels level class will be
         * used.
         *
         * \param input The string to be converted into a logging level.
         * \return The logging level associated with \a input or the default
         *         level if input is not recognized.
         */
        JEGA::Logging::LogLevel
        ResolveLogLevel(
            const std::string& input
            ) const;

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:

        /// Retrieves all user input from both command line and config file.
        /**
         * This method will end the program if an unrecoverable error occurs.
         *
         * \param argc The number of command line arguments supplied.
         * \param argv The text of each command line argument in a vector.
         */
        virtual
        void
        RetrieveAllInput(
            int argc,
            char* argv[]
            );

        /// Parses and stores all command line input arguments.
        /**
         * This method will end the program if an unrecoverable error occurs.
         *
         * \param argc The number of command line arguments supplied.
         * \param argv The text of each command line argument in a vector.
         */
        virtual
        void
        RetrieveCommandLineInput(
            int argc,
            char* argv[]
            );

        /// Parses and stores all configuration file input arguments.
        /**
         * This method will end the program if an unrecoverable error occurs.
         */
        virtual
        void
        RetrieveConfigFileInput(
            );

        /**
         * \brief Performs Validation of all input data using individual
         *        validate methods.
         */
        virtual
        void
        ValidateAllInput(
            );

        /**
         * \brief Returns an evaluator creator that can be used to create the
         *        evaluator to be used by JEGA.
         *
         * The framework will assume ownership of the creator and destroy
         * it when appropriate.
         *
         * \return A new EvaluatorCreator to be used in the creation of a new
         *         evaluator for use by JEGA.
         */
        virtual
        JEGA::FrontEnd::EvaluatorCreator*
        GetEvaluatorCreator(
            );

    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Returns a description object housing those input options
         *        that may only be specified on the command line for this app.
         *
         * \return A description object for the command line only input
         *         options.
         */
        static
        boost::program_options::options_description
        CreateCommandLineOnlyInputOptions(
            );

        /**
         * \brief Returns a description object housing those input options
         *        that may or must be specified on the command line for this
         *        app.
         *
         * Some of these options may be shared.
         *
         * \return A description object for all the possible command line
         *         input options.
         */
        static
        boost::program_options::options_description
        CreateAllCommandLineInputOptions(
            );

        /**
         * \brief Returns a description object housing those input options
         *        that may only be specified in a configuration file for this
         *        app.
         *
         * \return A description object for the config file only input
         *         options.
         */
        static
        boost::program_options::options_description
        CreateConfigFileOnlyInputOptions(
            );

        /**
         * \brief Returns a description object housing those input options
         *        that may or must be specified in a configuration file for
         *        this app.
         *
         * Some of these options may be shared.
         *
         * \return A description object for all the possible config file
         *         input options.
         */
        static
        boost::program_options::options_description
        CreateAllConfigFileInputOptions(
            );

        /**
         * \brief Returns a description object housing those input options
         *        that may specified on the command line or in a configuration
         *        file for this app.
         *
         * \return A description object for all the input options that can
         *         be specified on the command line and/or in a config file.
         */
        static
        boost::program_options::options_description
        CreateSharedInputOptions(
            );

        /**
         * \brief Returns a description object housing all input options
         *        for this app.
         *
         * \return A description object for all the input options.
         */
        static
        boost::program_options::options_description
        CreateAllInputOptions(
            );

        /// Returns the value of the variable supplied for param.
        /**
         * This attempts to resolve the return of GetVariableValue as the
         * supplied type.
         *
         * \param param The string associated with the variable of interest.
         * \return The value supplied for the variable associated with "param".
         */
        template <typename T>
        const T&
        GetValueOf(
            const std::string& param
            ) const;

        /**
         * \brief Inserts a supplied info object into the supplied map keyed
         *        on the user supplied index.
         *
         * \param theMap The map into which to insert the info and its index.
         * \param index The index on which to key the inserted information
         *              object.
         * \param theInfo The information object to be inserted into the map.
         * \param type The strign name of the type of the information object.
         *             This is used for error reporting.
         */
        template <typename InfoT>
        static
        void
        AddInfoToMap(
            std::map<std::size_t, InfoT*>& theMap,
            std::size_t index,
            InfoT* theInfo,
            const std::string& type
            );

        /**
         * \brief Gathers all elements of all vectors input using the supplied
         *        \a tag into a single vector and returns it.
         *
         * The input vectors are appended to the return vector in the order
         * in which they are returned from the program_options library which
         * will be the order in which they were specified by the user.
         *
         * \param tag The tag by which to retrieve the vectors to be
         *            concatenated.
         * \return A vector composed of all the elments of all the vectors
         *         supplied for the given \a tag.
         */
        template <typename T, const char I>
        std::vector<T>
        ConcatenateVector(
            const std::string& tag
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a new ConfigFileFrontEndApp
        ConfigFileFrontEndApp(
            );

        /// Destructs a new ConfigFileFrontEndApp
        virtual
        ~ConfigFileFrontEndApp(
            );

}; // class ConfigFileFrontEndApp



/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace ConfigFile
    } // namespace FrontEnd
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/ConfigFileFrontEndApp.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_CONFIGFILE_CONFIGFILEFRONTENDAPP_HPP
