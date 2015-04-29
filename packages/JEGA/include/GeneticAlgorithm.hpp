/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithm

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 15 08:25:23 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithm class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHM_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHM_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <memory> // for auto_ptr
#include <string>
#include <FitnessRecord.hpp>
#include <utilities/include/int_types.hpp>
#include <GeneticAlgorithmOperatorSet.hpp>
#include <GeneticAlgorithmOperatorGroup.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/keyed_registry.hpp>
#include <../Utilities/include/RegionOfSpace.hpp>








/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
        class DesignOFSortSet;
        class DesignGroupVector;
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
    namespace Algorithms {







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class GeneticAlgorithm;
class GeneticAlgorithmMutator;
class GeneticAlgorithmCrosser;
class GeneticAlgorithmSelector;
class GeneticAlgorithmMainLoop;
class GeneticAlgorithmEvaluator;
class GeneticAlgorithmConverger;
class GeneticAlgorithmInitializer;
class GeneticAlgorithmOperatorSet;
class GeneticAlgorithmPostProcessor;
class GeneticAlgorithmFitnessAssessor;










/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/

/**
 * \brief Registry used by a GeneticAlgorith to keep track of allowed operator
 *        groups.
 */
typedef
eddy::utilities::keyed_registry<
    std::string, GeneticAlgorithmOperatorGroupInstanceFunc
    >
GeneticAlgorithmOperatorGroupRegistry;






/*
================================================================================
Class Definition
================================================================================
*/

/// The base class for all genetic algorithms.
/**
 * This base class provides most of the needed functionality of a genetic
 * algorithm.  Only a small number of methods must be overridden and they
 * generally require little code.
 *
 * The algorithm is conceptually separate from the operators used to implement
 * it.  The operators are therefore "plugable" and could in theory be changed
 * at any time.  This class keeps track of what operators are currently in
 * use by containing a GeneticAlgorithmOperatorSet as a member data.  In order
 * to ensure that the operators in use are compatable with one another, this
 * class also keeps a current operator group.  The operator group is
 * automatically chosen from all groups registered with this algorithm as
 * obtained from a call the required override GetOperatorGroupRegistry.
 * The first operator group that contains all the operators in the current
 * operator set is used as the current group.  If none are found then an
 * error occurs.
 */
class JEGA_SL_IEDECL GeneticAlgorithm
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

        /// Current operator group being used by this.
        /**
         * A pointer to the group of all currently allowable operators
         * for this algorithm.
         */
        const GeneticAlgorithmOperatorGroup* _opGroup;

        /// Current operators being used by this.
        /**
         * The actual set of operators being used by the
         * algorithm right now.
         */
        GeneticAlgorithmOperatorSet* _opSet;

        /// Stores the current population of Designs.
        JEGA::Utilities::DesignGroup _pop;

        /// Stores the set of children after variation and prior to selection.
        JEGA::Utilities::DesignGroup _cldrn;

        /// The DesignTarget used by this algorithm for Design information.
        JEGA::Utilities::DesignTarget& _target;

        /// The logger class object into which this algorithm is to log.
        JEGA::Logging::Logger& _log;

        /// The name given to this instance of a GA.
        std::string _name;

        /// The name of the file to which the final data is written.
        std::string _finalDataFile;

        /// The instance number of this GA.
        eddy::utilities::uint64_t _instanceNum;

        /**
         * \brief A flag that tell the algorithm whether or not to print the
         *        population to a numbered data file after each generation or
         *        not.
         */
        bool _printPopEachGen;

        bool _printFinalData;

        bool _printDiscards;

        /// An instance counter for the GA class.
        /**
         * This is a strictly increasing number.  It is never decremented.
         * The first GA created will be number 1, the second number 2 and so
         * on.
         */
        static eddy::utilities::uint64_t _instanceCt;

        /**
         * \brief The region of the design space in which this GA is
         *        constrained to operate
         */
        JEGA::Utilities::RegionOfSpace _myDesignSpace;

        /// A flag set once AlgorithmFinalize has been called.
        bool _isFinalized;

        /// A flag set once AlgorithmInitialize has been called.
        bool _isInitialized;

        std::auto_ptr<const FitnessRecord> _lastFtns;

        std::string _dataDir;

        std::size_t _startTime;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the name of this algorithm to the supplied new name.
        /**
         * The new name cannot be empty.  If it is and there is an existing
         * name, the new one is ignored.  If it is empty and there is not an
         * existing name, the default name is installed.
         *
         * This method prints out a verbose level log entry indicating the
         * new name.
         *
         * \param name The new name for this genetic algorithm.
         */
        void
        SetName(
            const std::string& name
            );

        /// Sets the name of the file to which the final data will be written.
        /**
         * The name cannot be empty.  If it is, the default value is used.
         *
         * This method replaces all occurances of the # character with the
         * instance number of this GA.
         *
         * This method prints out a verbose level log entry indicating the
         * new name.
         *
         * \param name The new name for the final datafile name.
         */
        void
        SetFinalDataFilename(
            const std::string& name
            );

        void
        SetPrintFinalData(
            bool print
            );

        void
        SetDataDirectory(
            const std::string& dir
            );

        void
        SetPrintDiscards(
            bool print
            );

        void
        SetPrintEachPopulation(
            bool print
            );

        void
        SetCurrentFitnesses(
            const FitnessRecord* ftns
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns this GA's instance number.
        /**
         * \return The number given to this instance of a GA.
         */
        inline
        eddy::utilities::uint64_t
        GetInstanceNumber(
            ) const;

        /// Returns the name given to this instance of a GA.
        /**
         * \return The name of this genetic algorithm.
         */
        inline
        const std::string&
        GetName(
            ) const;

        /// Returns the name of the file to which final data will be written.
        /**
         * \return The name of the file to which final data will be written.
         */
        inline
        const std::string&
        GetFinalDataFilename(
            ) const;

        /// Returns the directory into which all data files are to be written.
        /**
         * \return The directory into which all data files are to be written.
         */
        inline
        const std::string&
        GetDataDirectory(
            ) const;

        /// Returns the DesignTarget known by this algorithm (non-const).
        /**
         * \return A mutable reference to the know DesignTarget.
         */
        inline
        JEGA::Utilities::DesignTarget&
        GetDesignTarget(
            );

        /// Returns the DesignTarget known by this algorithm (const).
        /**
         * \return An immutable reference to the know DesignTarget.
         */
        inline
        const JEGA::Utilities::DesignTarget&
        GetDesignTarget(
            ) const;

        /// Returns the current operator set for this algorithm (non-const)
        /**
         * \return A mutable reference to the contained operator set.
         */
        inline
        GeneticAlgorithmOperatorSet&
        GetOperatorSet(
            );

        /// Returns the current operator set for this algorithm (const)
        /**
         * \return An immutable reference to the contained operator set.
         */
        inline
        const GeneticAlgorithmOperatorSet&
        GetOperatorSet(
            ) const;

        /// Returns the operator group being used by this algorithm (const).
        /**
         * \return An immutable reference to the operator group currently in
         *         use.
         */
        inline
        const GeneticAlgorithmOperatorGroup&
        GetOperatorGroup(
            ) const;

        /// Returns the current population of Designs (non-const)
        /**
         * \return A mutable reference to the current population of designs.
         */
        inline
        JEGA::Utilities::DesignGroup&
        GetPopulation(
            );

        /// Returns the current population of Designs (const)
        /**
         * \return An immutable reference to the current population of designs.
         */
        inline
        const JEGA::Utilities::DesignGroup&
        GetPopulation(
            ) const;

        /// Returns the current set of offspring Designs (non-const)
        /**
         * \return A mutable reference to the current set of children designs.
         */
        inline
        JEGA::Utilities::DesignGroup&
        GetOffspring(
            );

        /// Returns the current set of offspring Designs (const)
        /**
         * \return An immutable reference to the current set of children
         *         designs.
         */
        inline
        const JEGA::Utilities::DesignGroup&
        GetOffspring(
            ) const;

        const FitnessRecord&
        GetCurrentFitnesses(
            );

        inline
        const std::size_t&
        GetStartTime(
            ) const;

        /**
         * \brief Indicates whether or not this algorithm is writing the
         *        current population to a data file at the end of each
         *        generation.
         *
         * \return The value of the \a _printPopEachGen flag.
         */
        inline
        bool
        IsWritingPopEachGen(
            ) const;

        inline
        bool
        IsWritingFinalData(
            ) const;

        inline
        bool
        IsWritingDiscards(
            ) const;

        /**
         * \brief Allows access to the region of space in which this GA is
         *        constrained to operate.
         *
         * \return The region of space in which this GA operates.
         */
        inline
        const JEGA::Utilities::RegionOfSpace&
        GetMyRegion(
            ) const;

        /**
         * \brief Returns true if AlgorithmFinalize has been called and false
         *        otherwise.
         *
         * \return True if AlgorithmFinalize has been called and false
         *         otherwise.
         */
        inline
        bool
        IsFinalized(
            ) const;

        /**
         * \brief Returns true if AlgorithmInitialize has been called and false
         *        otherwise.
         *
         * \return True if AlgorithmInitialize has been called and false
         *         otherwise.
         */
        inline
        bool
        IsInitialized(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Used to call PollForParameters.
        /**
         * This method issues a call to PollForParameters at which time derived
         * algorithm types can ask for specific parameters by string.  This is
         * the method that should be used by whomever creates algorithms.
         *
         * \param pdb The database of parameters from which algorithms can
         *            extract their needed configuration information.
         * \return True if polling for parameters succeeds and false otherwise.
         */
        bool
        ExtractParameters(
            const JEGA::Utilities::ParameterDatabase& pdb
            );

        /**
         * \brief Allows access to the Logger being used by this algorithm.
         *
         * The logger should not be used directly except in the JEGA_LOGGING
         * macros.  If logging from a static method, either include the
         * logger as a parameter or use the global log via the _G macros.
         *
         * \return The Logger class object that should be used by this
         *         algorithm and all it's components.
         */
        inline
        JEGA::Logging::Logger&
        GetLogger(
            ) const;

        /// Returns the mutator being used by this algorithm (const).
        /**
         * \return An immutable reference to the current mutator.
         */
        inline
        const GeneticAlgorithmMutator&
        GetMutator(
            ) const;

        /// Returns the converger being used by this algorithm (const).
        /**
         * \return An immutable reference to the current converger.
         */
        inline
        const GeneticAlgorithmConverger&
        GetConverger(
            ) const;

        /// Returns the crosser being used by this algorithm (const).
        /**
         * \return An immutable reference to the current crosser.
         */
        inline
        const GeneticAlgorithmCrosser&
        GetCrosser(
            ) const;

        /// Returns the fitness assessor being used by this algorithm (const).
        /**
         * \return An immutable reference to the current fitness assessor.
         */
        inline
        const GeneticAlgorithmFitnessAssessor&
        GetFitnessAssessor(
            ) const;

        /// Returns the initializer being used by this algorithm (const).
        /**
         * \return An immutable reference to the current initializer.
         */
        inline
        const GeneticAlgorithmInitializer&
        GetInitializer(
            ) const;

        /// Returns the selector being used by this algorithm (const).
        /**
         * \return An immutable reference to the current selector.
         */
        inline
        const GeneticAlgorithmSelector&
        GetSelector(
            ) const;

        /// Returns the post processor being used by this algorithm (const).
        /**
         * \return An immutable reference to the current post processor.
         */
        inline
        const GeneticAlgorithmPostProcessor&
        GetPostProcessor(
            ) const;

        /// Returns the evaluator being used by this algorithm (const).
        /**
         * \return An immutable reference to the current evaluator.
         */
        inline
        const GeneticAlgorithmEvaluator&
        GetEvaluator(
            ) const;

        /**
         * \brief Returns the niche pressure applicator being used by this
         *        algorithm (const).
         *
         * \return An immutable reference to the current niche pressure
         *         applicator.
         */
        inline
        const GeneticAlgorithmNichePressureApplicator&
        GetNichePressureApplicator(
            ) const;

        /// Returns the main loop being used by this algorithm (const).
        /**
         * \return An immutable reference to the current main loop.
         */
        inline
        const GeneticAlgorithmMainLoop&
        GetMainLoop(
            ) const;

        /// Returns a properly allocated Design for use with this algorithm.
        /**
         * This job is simply passed along to the DesignTarget.
         *
         * \return A new, fully allocated but empty Design object.
         */
        JEGA::Utilities::Design*
        GetNewDesign(
            ) const;

        /// Returns a properly allocated Design which is a copy of \a copy.
        /**
         * This job is simply passed along to the DesignTarget.
         *
         * \param copy The Design object to copy when creating the new one.
         * \return A new, fully allocated but Design object which is a copy of
         *         \a copy.
         */
        JEGA::Utilities::Design*
        GetNewDesign(
            const JEGA::Utilities::Design& copy
            ) const;

        /// Sets the current operator group.
        /**
         * Used to change the current operator group being used by this
         * algorithm.  If it is unable to match the current operator set
         * to the passed in group, it clears it out meaning that they resort
         * back to default operators.
         *
         * The set may fail if \a to is not a registered group of this
         * GeneticAlgorithm in which case the method returns false.
         *
         * \param to The new group for this algorithm to use.
         * \return True if the new group is set and false otherwise.
         */
        bool
        SetOperatorGroup(
            const GeneticAlgorithmOperatorGroup& to
            );

        /// Sets the current operator set.
        /**
         * Used to change the current operator set being used by this
         * algorithm.  It checks to be sure that a valid group is either
         * currently installed or can be found based on the operators in \a to.
         * If it finds such a group, it sets it and accepts the new set.  If
         * not, it returns false and does not accept the new set.
         *
         * \param to The new operator set for this algorithm to use.
         * \return True if the new set is accepted and false otherwise.
         */
        bool
        SetOperatorSet(
            const GeneticAlgorithmOperatorSet& to
            );

        /// Sets the current mutator.
        /**
         * Used to change the current mutator being used by this algorithm.
         * The change only occurs if the passed in mutator is in the current
         * operator group or if a group can be found that contains the current
         * operator set with the new mutator type.  If the set ultimately
         * succeeds, the return is true.
         *
         * \param to The new mutator for this algorithm to use.
         * \return True if the mutator is accepted and false otherwise.
         */
        bool
        SetMutator(
            GeneticAlgorithmMutator* to
            );

        /// Sets the current converger.
        /**
         * Used to change the current converger being used by this algorithm.
         * The change only occurs if the passed in converger is in the current
         * operator group or if a group can be found that contains the current
         * operator set with the new converger type.  If the set ultimately
         * succeeds, the return is true.
         *
         * \param to The new converger for this algorithm to use.
         * \return True if the converger is accepted and false otherwise.
         */
        bool
        SetConverger(
            GeneticAlgorithmConverger* to
            );

        /// Sets the current crosser.
        /**
         * Used to change the current crosser being used by this algorithm.
         * The change only occurs if the passed in crosser is in the current
         * operator group or if a group can be found that contains the current
         * operator set with the new crosser type.  If the set ultimately
         * succeeds, the return is true.
         *
         * \param to The new crosser for this algorithm to use.
         * \return True if the crosser is accepted and false otherwise.
         */
        bool
        SetCrosser(
            GeneticAlgorithmCrosser* to
            );

        /// Sets the current fitness assessor.
        /**
         * Used to change the current fitness assessor being used by this
         * algorithm. The change only occurs if the passed in fitness assessor
         * is in the current operator group or if a group can be found that
         * contains the current operator set with the new fitness assessor
         * type.  If the set ultimately succeeds, the return is true.
         *
         * \param to The new fitness assessor for this algorithm to use.
         * \return True if the fitness assessor is accepted and false
         *         otherwise.
         */
        bool
        SetFitnessAssessor(
            GeneticAlgorithmFitnessAssessor* to
            );

        /// Sets the current initializer.
        /**
         * Used to change the current initializer being used by this algorithm.
         * The change only occurs if the passed in initializer is in the
         * current operator group or if a group can be found that contains the
         * current operator set with the new initializer type.  If the set
         * ultimately succeeds, the return is true.
         *
         * \param to The new initializer for this algorithm to use.
         * \return True if the initializer is accepted and false otherwise.
         */
        bool
        SetInitializer(
            GeneticAlgorithmInitializer* to
            );

        /// Sets the current main loop.
        /**
         * Used to change the current main loop being used by this algorithm.
         * The change only occurs if the passed in main loop is in the
         * current operator group or if a group can be found that contains the
         * current operator set with the new main loop type.  If the set
         * ultimately succeeds, the return is true.
         *
         * \param to The new main loop for this algorithm to use.
         * \return True if the main loop is accepted and false otherwise.
         */
        bool
        SetMainLoop(
            GeneticAlgorithmMainLoop* to
            );

        /// Sets the current niche pressure applicator.
        /**
         * Used to change the current niche pressure applicator being used by
         * this algorithm. The change only occurs if the passed in niche
         * pressure applicator is in the current operator group or if a group
         * can be found that contains the current operator set with the new
         * niche pressure applicator type.  If the set ultimately succeeds, the
         * return is true.
         *
         * \param to The new niche pressure applicator for this algorithm to
         *           use.
         * \return True if the niche pressure applicator is accepted and false
         *         otherwise.
         */
        bool
        SetNichePressureApplicator(
            GeneticAlgorithmNichePressureApplicator* to
            );

        /// Sets the current selector.
        /**
         * Used to change the current selector being used by this algorithm.
         * The change only occurs if the passed in selector is in the current
         * operator group or if a group can be found that contains the current
         * operator set with the new selector type.  If the set ultimately
         * succeeds, the return is true.
         *
         * \param to The new selector for this algorithm to use.
         * \return True if the selector is accepted and false otherwise.
         */
        bool
        SetSelector(
            GeneticAlgorithmSelector* to
            );

        /// Sets the current post processor.
        /**
         * Used to change the current post processor being used by this
         * algorithm. The change only occurs if the passed in post processor is
         * in the current operator group or if a group can be found that
         * contains the current operator set with the new post processor type.
         * If the set ultimately succeeds, the return is true.
         *
         * \param to The new post processor for this algorithm to use.
         * \return True if the post processor is accepted and false otherwise.
         */
        bool
        SetPostProcessor(
            GeneticAlgorithmPostProcessor* to
            );

        /// Sets the current evaluator.
        /**
         * Used to change the current evaluator being used by this algorithm.
         * The change only occurs if the passed in evaluator is in the current
         * operator group or if a group can be found that contains the current
         * operator set with the new evaluator type.  If the set ultimately
         * succeeds, the return is true.
         *
         * \param to The new evaluator for this algorithm to use.
         * \return True if the evaluator is accepted and false otherwise.
         */
        bool
        SetEvaluator(
            GeneticAlgorithmEvaluator* to
            );

        /// Returns the number of evaluations that have been executed.
        /**
         * The return value indicates the number of evaluations carried out
         * BY THE CURRENT EVALUATOR ONLY on behalf of this algorithm.
         *
         * \return The number of evaluations reported by the current evaluator.
         */
        eddy::utilities::uint64_t
        GetNumberEvaluations(
            ) const;

        /// Returns the number of generations that have been executed.
        /**
         * The return value indicates the number of generations carried out
         * BY THE CURRENT MAIN LOOP ONLY on behalf of this algorithm.
         *
         * \return The number of generations reported by the current main loop.
         */
        eddy::utilities::uint64_t
        GetGenerationNumber(
            ) const;

        /// Verifies that all design variable values are valid.
        /**
         * This method calls ValidateVariableValues for each DesignGroup
         * in \a groups.
         *
         * \param groups The Design groups whose Designs are to be corrected.
         * \return The number of designs for which an invalid value could not
         *         be corrected.  Such designs will be marked ill-conditioned.
         */
        std::size_t
        ValidateVariableValues(
            const JEGA::Utilities::DesignGroupVector& groups
            ) const;

        /// Verifies that all design variable values are valid.
        /**
         * This method checks to be sure that all variable values are valid as
         * defined by their info object.  This generally requires at least that
         * the current double rep is within bounds.  It may also require that
         * a variable be a whole number, etc. as appropriate.  The exact
         * correction that takes place depends on what is wrong.  If it is out
         * of bounds, it is corrected to a random value.  Otherwise, it is
         * corrected to the nearest valid value.
         *
         * \param group The group whose Designs are to be corrected.
         * \return The number of designs for which an invalid value could not
         *         be corrected.  Such designs will be marked ill-conditioned.
         */
        std::size_t
        ValidateVariableValues(
            JEGA::Utilities::DesignGroup& group
            ) const;

        std::size_t
        LogIllconditionedDesigns(
            const JEGA::Utilities::DesignGroup& from
            ) const;

        double
        GetElapsedTime(
            ) const;

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Returns the default name for this algorithm.
        /**
         * The default name is GetAlgorithmTypeName() + GetInstanceNumber().
         *
         * \return The default name for this algorithm.
         */
        std::string
        GetDefaultName(
            ) const;

        /**
         * \brief This method requests that \a op retrieve its parameter
         *        values from \a pdb.
         *
         * If \a op is unable to do so, this method logs a fatal error.
         *
         * \param op The operator to have retrieve it's parameters.
         * \param pdb The parameter database from which \a op may retrieve it's
         *               parameters.
         */
        void
        ExtractOperatorParameters(
            GeneticAlgorithmOperator& op,
            const JEGA::Utilities::ParameterDatabase& pdb
            );

        /// Returns the mutator being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current mutator.
         */
        inline
        GeneticAlgorithmMutator&
        GetMutator(
            );

        /// Returns the converger being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current converger.
         */
        inline
        GeneticAlgorithmConverger&
        GetConverger(
            );

        /// Returns the crosser being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current crosser.
         */
        inline
        GeneticAlgorithmCrosser&
        GetCrosser(
            );

        /**
         * \brief Returns the fitness assessor being used by this algorithm
         *        (non-const).
         * \return A mutable reference to the current fitness assessor.
         */
        inline
        GeneticAlgorithmFitnessAssessor&
        GetFitnessAssessor(
            );

        /// Returns the initializer being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current initializer.
         */
        inline
        GeneticAlgorithmInitializer&
        GetInitializer(
            );

        /// Returns the selector being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current selector.
         */
        inline
        GeneticAlgorithmSelector&
        GetSelector(
            );

        /**
         * \brief Returns the post processor being used by this algorithm
         *        (non-const).
         *
         * \return A mutable reference to the current post processor.
         */
        inline
        GeneticAlgorithmPostProcessor&
        GetPostProcessor(
            );

        /// Returns the evaluator being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current evaluator.
         */
        inline
        GeneticAlgorithmEvaluator&
        GetEvaluator(
            );

        /**
            * \brief Returns the niche pressure applicator being used by this
         *        algorithm (non-const).
         *
         * \return A mutable reference to the current evaluator.
         */
        inline
        GeneticAlgorithmNichePressureApplicator&
        GetNichePressureApplicator(
            );

        /// Returns the main loop being used by this algorithm (non-const).
        /**
         * \return A mutable reference to the current main loop.
         */
        inline
        GeneticAlgorithmMainLoop&
        GetMainLoop(
            );

        /**
         * \brief Return the registry of all allowed groups for use with this
         *        algorithm.
         *
         * This will resolve to a call to the pure virtual overload.
         *
         * \return The registry of all operator groups that can be used with
         *         this algorithm.
         */
        const GeneticAlgorithmOperatorGroupRegistry&
        GetOperatorGroupRegistry(
            ) const;

        /// Streams the current population out to a data file.
        /**
         * The file name will be population_<GEN#>.dat where <GEN#> is replaced
         * by the current generation number.
         *
         * \return True if the file is successfully written and false
         *         otherwise.
         */
        bool
        WritePopulationToFile(
            ) const;

        /// Streams the current population out to a data file.
        /**
         * \param fname The name of the file to write the population to.
         * \return True if the file is successfully written and false
         *         otherwise.
         */
        bool
        WritePopulationToFile(
            const std::string& fname
            ) const;

        /// Streams the supplied group out to a data file.
        /**
         * \param group The group whose designs are to be streamed out.
         * \param fname The name of the file to write the population to.
         * \return True if the file is successfully written and false
         *         otherwise.
         */
        bool
        WriteGroupToFile(
            const JEGA::Utilities::DesignDVSortSet& group,
            const std::string& fname
            ) const;

        /// Streams the supplied group out to a data file.
        /**
         * \param group The group whose designs are to be streamed out.
         * \param fname The name of the file to write the population to.
         * \return True if the file is successfully written and false
         *         otherwise.
         */
        bool
        WriteGroupToFile(
            const JEGA::Utilities::DesignOFSortSet& group,
            const std::string& fname
            ) const;

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /**
         * \brief Retrieves specific parameters for this algorithm.
         *
         * This method should be used to extract needed information for this
         * algorithm.  It should do so using the ParameterExtractor class
         * methods. The return value should be true if the extraction
         * completes successfully and false otherwise.
         *
         * \param db The database of parameter values from which to do
         *           retrieval.
         * \return true if polling completed successfully and false otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

        /// Initializes this algorithm.
        /**
         * This method should be called once before the algorithm begins doing
         * generations and not again.  It is responsible primarily for getting
         * the population initialized and also does some other misc. stuff.
         *
         * \return True if initialization completes correctly and the algorithm
         *         can progress and false otherwise.
         */
        virtual
        bool
        AlgorithmInitialize(
            );

        /// Runs the main loop once.
        /**
         * This method should be called to cause execution of generations.
         * It will return true if it wishes to be called again and false
         * otherwise.  Subsequent calls after a false return will have no
         * effect.
         *
         * \return True if the algorithm should proceed with more processing
         *         and false if it should not.
         */
        virtual
        bool
        AlgorithmProcess(
            );

        /// Finalizes the algorithm.
        /**
         * This method should be called once after the algorithm has completed
         * all the generations it will and not before or again.  It should be
         * used to collect, post-process, and write data to data files if
         * desired.
         *
         * \return True if finalization completes correctly and false
         *         otherwise.
         */
        virtual
        bool
        AlgorithmFinalize(
            );

        /// Gets one best design in the current population.
        /**
         * This is a very subjective matter especially for a MOGA.  This method
         * is only here to support the needs of DAKOTA and should be avoided
         * at all costs.
         *
         * \return The single best Design currently in the population.
         */
        virtual
        const JEGA::Utilities::Design*
        GetBestDesign(
            );

        /// Override to return the current set of solutions of this algorithm.
        /**
         * This should return whatever this algorithm would claim to be the
         * optimal solutions if it had to stop on the spot.  This can be
         * an expensive call and is meant primarly for use after the call
         * to AlgorithmFinalize at which time this should return the final
         * solution found by the algorithm.
         *
         * This is different from the GetBestDesign method in that it may
         * return multiple Designs.
         *
         * \return The set of solutions currently found by this algorithm
         */
        virtual
        JEGA::Utilities::DesignOFSortSet
        GetCurrentSolution(
            ) const = 0;

        /// Uses a GeneticAlgorithmCrosser to perform crossover.
        /**
         * By default, this method empties the children group and then calls
         * the crosser with the population to use and the set of children to
         * fill up.
         */
        virtual
        void
        DoCrossover(
            );

        /**
         * \brief Uses a GeneticAlgorithmFitnessAssessor to perform fitness
         *        assessment.
         *
         * By default, this method collects the population and the children
         * together into a DesignGroupVector and passes them into a fitness
         * assessor.  The result of the fitness assessment is returned directly
         * from this method.  Note that after the FitnessRecord returned from
         * this method is no longer useful, it should be destroyed.
         *
         * \return The collection of all Designs mapped to their fitness
         *         values.
         */
        virtual
        const FitnessRecord*
        DoFitnessAssessment(
            );

        /// Uses a GeneticAlgorithmMutator to perform mutation.
        /**
         * By default, this method does nothing but call the mutator with
         * the set of children.
         */
        virtual
        void
        DoMutation(
            );

        /**
         * \brief Uses a GeneticAlgorithmNichePressureApplicator to perform any
         *        needed pre-selection operations.
         *
         * Note that the selection operators are not just the
         * GeneticAlgorithmSelector but are each of the fitness assessor,
         * selector, and niche pressure applicator.  This call is issued just
         * prior tofitness assessment.
         */
        virtual
        void
        DoPreSelection(
            );

        virtual
        void
        AbsorbEvaluatorInjections(
            bool allowDuplicates
            );

        /// Uses a GeneticAlgorithmSelector to perform selection.
        /**
         * By default, this method does the following:
         * -Gathers the children and population groups into a design group
         *  vector.
         * -Creates a new DesignGroup into which selections will be placed.
         * -Calls the selector with the group vector, the new group for
         *  selected design placement, the population size as the requested
         *  number of selections, and the record of fitnesses passed into
         *  this method.
         * -Flushes the remains of the population and children groups.
         * -Assigns the population to the contents of the into group.
         *
         * \param fitnesses The set of fitnesses according to which the
         *                  selector should choose designs.
         */
        virtual
        void
        DoSelection(
            const FitnessRecord& fitnesses
            );

        /**
         * \brief Uses the a GeneticAlgorithmInitializer to create the initial
         *        population.
         *
         * By default, this method does nothing more than call the initializer
         * with the current (probably empty) population as the argument.
         */
        virtual
        void
        InitializePopulation(
            );

        /// Uses the a GeneticAlgorithmEvaluator to do Design evaluation.
        /**
         * By default, prior to sending the group in for evaluation, this
         * method checks in the DesignTarget discards deque to see if any
         * evaluations can be avoided.  If any duplicates are found, they are
         * tagged as clones.  It is then up to the programmer of the evaluator
         * to use that information if desired.
         *
         * Prior to returning, this method synchronizes the deques of the
         * passed in group.
         *
         * \param group The group of Designs to be evaluated.
         * \return Direct pass along of return value of
         *         GeneticAlgorithmEvaluator::Evaluate(DesignGroup&).
         */
        virtual
        bool
        DoEvaluation(
            JEGA::Utilities::DesignGroup& group
            );

        /**
         * \brief Uses the a GeneticAlgorithmNichePressureApplicator to apply
         *        niche pressure to the supplied group.
         *
         * By default, this method does the following:
         * -Creates a new DesignGroup into which survivors will be placed.
         * -Calls the applicator with the group to be niched and the new group
         *  for surviving design placement.
         * -Flushes the remains of the population and children groups.
         * -Assigns the population to the contents of the into group.
         *
         * \param group The group of Designs to be niched.
         * \param fitnesses The fitnesses assigned to the designs of \a group
         *                  by a fitness assessor prior to this call.
         */
        virtual
        void
        ApplyNichePressure(
            JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
            );

        /// Uses the a GeneticAlgorithmConverger to check for convergence.
        /**
         * By default, this method does nothing more than call the current
         * converger with the population and the supplied fitness record.
         *
         * The supplied fitness record must contain fitness information
         * associated with all members of the population.
         *
         * \param fitnesses The record of fitnesses for all members of the
         *                  population.
         * \return The return of GeneticAlgorithmConverger::CheckConvergence
         */
        virtual
        bool
        TestForConvergence(
            const FitnessRecord& fitnesses
            );

        /**
         * \brief Uses the a GeneticAlgorithmPostProcessor to perform
         *        post-processing of the final population.
         */
        virtual
        void
        DoPostProcessing(
            );

        /**
         * \brief Override to return the registry of all allowed groups for use
         *        with this algorithm.
         *
         * These groups define what operators can be used with this algorithm
         * and in what configurations (i.e. what operators can be used with
         * what other operators).
         *
         * \return The registry of all operator groups that can be used with
         *         this algorithm.
         */
        virtual
        GeneticAlgorithmOperatorGroupRegistry&
        GetOperatorGroupRegistry(
            ) = 0;

        /**
         * \brief Override to look through the discarded designs and retrieve
         *        any that are optimal.
         *
         * Retrieved designs should be placed back in the population.
         */
        virtual
        void
        ReclaimOptimal(
            ) = 0;

        /**
         * \brief Override to look through the current designs and remove
         *        any that are not optimal.
         *
         * Removeed designs should be placed into the targets discards.
         */
        virtual
        void
        FlushNonOptimal(
            ) = 0;

        /**
         * \brief Override to return the name of the most derived algorithm
         *        type that this GA actually is.
         *
         * Examples would be "moga" or "soga".
         *
         * \return The name of the type of this algorithm.
         */
        virtual
        std::string
        GetAlgorithmTypeName(
            ) const = 0;

    protected:

    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /// Attempts to match a group to \a set.
        /**
         * This method checks with each operator group registered with this
         * algorithm to see if any contain the entire set of operators
         * supplied.
         *
         * If it succeeds, it returns a const pointer to the found group.  If
         * not, it returns null.
         *
         * This does not set the operator group known to this algorithm.
         *
         * \param set The new operator set to find a matching group for.
         * \return A pointer to the newly matched group or Null if no groups
         *         matched.
         */
        const GeneticAlgorithmOperatorGroup*
        MatchGroup(
            const GeneticAlgorithmOperatorSet& set
            ) const;

        /**
         * \brief Used by all of the mutator methods for the operators to
         *        carry out the set operation.
         *
         * This method is called by all the Set* (ex. SetCrosser) methods
         * in order to actually carry out the operation.  It checks to see
         * if the current group allows the supplied operator and if not, it
         * tries to find one that does.  The return value indicates the
         * success of the operation.
         *
         * \param op The new operator to attempt to start using.
         * \param getFunc The function that is used to get the current operator
         *                of type Op_T from the current operator set.
         * \param setFunc The function that is used to set the current operator
         *                of type Op_T in the current operator set.
         * \param groupHasOp True if the current group has the requested
         *                   operator and false otherwise.
         * \param opType A string representation of the type of the operator.
         *               This would typically be something like "converger".
         * \return true if the operator is set and false otherwise.
         */
        template<typename Op_T>
        bool
        SetOperator(
            Op_T* op,
            Op_T& (GeneticAlgorithmOperatorSet::*getFunc)(),
            void (GeneticAlgorithmOperatorSet::*setFunc)(Op_T* op),
            bool groupHasOp,
            const std::string& opType
            );

        const GeneticAlgorithmOperatorGroupRegistry&
        GetOperatorGroupRegistry_FWD(
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a GeneticAlgorithm which will use \a target.
        /**
         * \param target The design target to be used by this algorithm.
         * \param log The log into which this and all its operators will log.
         */
        GeneticAlgorithm(
            JEGA::Utilities::DesignTarget& target,
            JEGA::Logging::Logger& log
            );

        /// Destructs a GeneticAlgorithm.
        virtual
        ~GeneticAlgorithm(
            );

}; // class GeneticAlgorithm


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/GeneticAlgorithm.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHM_HPP
