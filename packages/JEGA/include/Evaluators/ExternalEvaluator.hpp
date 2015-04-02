/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ExternalEvaluator.

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

        Thu Nov 09 14:54:09 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ExternalEvaluator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_CONFIGFILE_EXTERNALEVALUATOR_HPP
#define JEGA_FRONTEND_CONFIGFILE_EXTERNALEVALUATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmEvaluator.hpp>
#include <../Utilities/include/JEGATypes.hpp>






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








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class ExternalEvaluator;







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
 * \brief This evaluator operates by calling an external program and reading
 *        results in from a text file.
 *
 * The executable can be anything that can be called via the command line such
 * as an actual assembly, a script or batch file, etc.  The command line will
 * include two arguments.  The first is the name of the file from which the
 * design variable values can be read.  The second is the name of the file that
 * this evaluator will look for in order to read in the results of the
 * evaluation.  The following is an example of such a command line string.
 *
 * \verbatim
    myevaler.exe params3.in results3.out
   \endverbatim
 *
 * The design variables will be written as floating point values, 1 per line
 * in the params file.  The results must be written in the same fashion.  The
 * objective function values must be written followed by constraint values if
 * there are any.  These must be written in the order in which the objectives
 * and constraints were described to the DesignTarget.
 *
 * If JEGA has been built in thread safe mode, this operator will respect the
 * concurrency as defined and implemented in the base class.
 *
 * The name of the executable, the pattern by which to create parameter file
 * names, and the pattern by which to create response file names are extracted
 * from the parameter database using the strings "method.jega.exe_name",
 * "method.jega.out_file_pat", and "method.jega.in_file_pat" respectively.
 * The number of concurrent evaluations to run is extracted using the string
 * "method.jega.eval_concurrency". The first 3 are all extracted as strings.
 * The concurrency is extracted as a size type variable.  If the output file
 * pattern and input file pattern are not supplied, then the default values as
 * defined by DEFAULT_OUT_PATTERN and DEFAULT_IN_PATTERN are used.  If the
 * executable name is not supplied, then a warning is issued.  If a name is not
 * supplied by the time an evaluation is requested, then a fatal error occurs.
 * There is no default for this.  If a concurrency value is not supplied, then
 * the default value as defined by DEFAULT_EVAL_CONCUR is used.  These are
 * required in addition to any requirements of the base class.
 */
class JEGA_SL_IEDECL ExternalEvaluator :
    public GeneticAlgorithmEvaluator
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
    public:

        /// The default number of generations over which to track the metric.
        static const std::string DEFAULT_OUT_PATTERN;

        /// The default percent change in the metric being sought.
        static const std::string DEFAULT_IN_PATTERN;

    private:

        /// The name of the executable program to be run for evaluations.
        std::string _exeName;

        /// The pattern by which parameter file names are written.
        /**
         * These are the files into which design variables get written to be
         * read in by analysis code.
         */
        std::string _outPattern;

        /// The pattern by which response file names are written.
        /**
         * These are the files into which objective function and constraint
         * values get written to be read back in by this evaluator.
         */
        std::string _inPattern;

        std::size_t _evalNum;

        /// The mutext that protects the _evalNum variable in threadsafe mode.
        EDDY_DECLARE_MUTABLE_MUTEX(_currEvalMutex)

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Allows mutation of the executable name known by this evaluator.
        /**
         * This method issues a verbose level log entry to inform of the new
         * executable name.
         *
         * \param exeName The new name of the program to run for evaluations.
         */
        void
        SetExecutableName(
            const std::string& exeName
            );

        /// Allows mutation of the output file name pattern.
        /**
         * This method issues a verbose level log entry to inform of the new
         * pattern.
         *
         * \param outPattern The new pattern by which output parameter files
         *                   are named.
         */
        void
        SetOutputFilenamePattern(
            const std::string& outPattern
            );

        /// Allows mutation of the input file name pattern.
        /**
         * This method issues a verbose level log entry to inform of the new
         * pattern.
         *
         * \param inPattern The new pattern by which input response files
         *                  are named.
         */
        void
        SetInputFilenamePattern(
            const std::string& inPattern
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Allows access to the name of the executable used by this evaluator.
        /**
         * \return The current executable name used in creating command lines
         *         for performing executions.
         */
        inline
        const std::string&
        GetExecutableName(
            ) const;

        /// Allows access to the parameter output file name pattern.
        /**
         * \return The pattern used to create parameter output file names.
         */
        inline
        const std::string&
        GetOutputFilenamePattern(
            ) const;

        /// Allows access to the response input file name pattern.
        /**
         * \return The pattern used to create response input file names.
         */
        inline
        const std::string&
        GetInputFilenamePattern(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "external".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        static
        const std::string&
        Description(
            );

        /**
         * \brief Returns a new instance of this operator class for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which the new evaluator is to be used.
         * \return A new, default instance of a ExternalEvaluator.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Creates a new output file name whereby all #'s in
         *        \a _outPattern are replaced by the number \a lbRepl.
         *
         * \param lbRepl The number to put in place of all #'s in the returned
         *               output file name.
         * \return The filename resulting from replacing all #'s in
         *         \a _outPattern by \a lbRepl.
         */
        std::string
        GetOutputFilename(
            std::size_t lbRepl
            ) const;

        /**
         * \brief Creates a new input file name whereby all #'s in
         *        \a _inPattern are replaced by the number \a lbRepl.
         *
         * \param lbRepl The number to put in place of all #'s in the returned
         *               input file name.
         * \return The filename resulting from replacing all #'s in
         *         \a _inPattern by \a lbRepl.
         */
        std::string
        GetInputFilename(
            std::size_t lbRepl
            ) const;

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Does evaluation of each Design in \a group.
        /**
         * This method is overridden only to do some sanity checking with the
         * names of the input files and output files in the case of a >1
         * evaluation concurrency.  If the concurrency is >1, then the names
         * must have wildcards.  After the check, the base class implementation
         * is invoked.
         *
         * \param group The group containing the Designs to be evaluated.
         * \return True if all Designs evaluate successfully and false
         *         otherwise.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::DesignGroup& group
            );

        /**
         * \brief Overridden to perform evaluation of the Design provided.
         *
         * This is the method from which an evaluation is performed.  This
         * method will issue a call to the external evaluation program and
         * read in the results.
         *
         * \param des The design to be evaluted.
         * \return true if the Design is successfully evaluated and the results
         *              read in and false otherwise.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::Design& des
            );

        ///**
        // * \brief Overridden to perform evaluation of the Design provided in
        // *        the supplied EvaluationJob.
        // *
        // * This is the method from which an evaluation is performed.  This
        // * method will issue a call to the external evaluation program and
        // * read in the results.
        // *
        // * \param evalJob The object containing the information needed to
        // *                perform the evaluation of a Design.
        // * \return true if the Design is successfully evaluated and the results
        // *              read in and false otherwise.
        // */
        //virtual
        //bool
        //Evaluate(
        //    EvaluationJob& evalJob
        //    );

        /// Returns the proper name of this operator.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const;

        /// Returns a full description of what this operator does and how.
        /**
         * \return See Description().
         */
        virtual
        std::string
        GetDescription(
            ) const;

        /**
         * \brief Creates and returns a pointer to an exact duplicate of this
         *        operator.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            ) const;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the output file name pattern, input file name
         * pattern, and the name of the executable to call.
         *
         * \param db The database of parameters from which the configuration
         *           information can be retrieved.
         * \return true if the extraction completed successfully and false
         *         otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Replaces all occurances of the supplied character in the
         *        supplied string with a string representation of the supplied
         *        value.
         *
         * \param of The character to replace.
         * \param in The string in which to do the replacing.
         * \param with The number to put in place of all \a of's.
         */
        static
        std::string
        ReplaceAllOccurrances(
            char of,
            const std::string& in,
            eddy::utilities::uint64_t with
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a ExternalEvaluator for use by \a algorithm.
        /**
         * If you use this constructor, it will be necessary for you to supply
         * an evaluation functor via the SetEvaluationFunctor method prior to
         * use.
         *
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        ExternalEvaluator(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            );

        /// Copy constructs an ExternalEvaluator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        ExternalEvaluator(
            const ExternalEvaluator& copy
            );

        /// Copy constructs an ExternalEvaluator for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        ExternalEvaluator(
            const ExternalEvaluator& copy,
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            );

}; // class ExternalEvaluator



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/ExternalEvaluator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_CONFIGFILE_EXTERNALEVALUATOR_HPP
