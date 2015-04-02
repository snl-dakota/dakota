/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmOperatorGroup.

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

        Tue May 27 15:29:52 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmOperatorGroup class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMOPERATORGROUP_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMOPERATORGROUP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>
#include <ostream>
#include <GeneticAlgorithmOperator.hpp>

#include <utilities/include/keyed_registry.hpp>







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
class GeneticAlgorithm;
class GeneticAlgorithmCrosser;
class GeneticAlgorithmMutator;
class GeneticAlgorithmSelector;
class GeneticAlgorithmMainLoop;
class GeneticAlgorithmConverger;
class GeneticAlgorithmEvaluator;
class GeneticAlgorithmInitializer;
class GeneticAlgorithmOperatorSet;
class GeneticAlgorithmOperatorGroup;
class GeneticAlgorithmPostProcessor;
class GeneticAlgorithmFitnessAssessor;
class GeneticAlgorithmNichePressureApplicator;


/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/

/// A registry used to store GeneticAlgorithmOperators.
typedef
eddy::utilities::keyed_registry<
    std::string,
    GeneticAlgorithmOperatorCreateFunc
    >
GeneticAlgorithmOperatorRegistry;

/// Signature of the methods required of each operator group for creation.
typedef
const GeneticAlgorithmOperatorGroup&
(*GeneticAlgorithmOperatorGroupInstanceFunc)(
    void
    );



/*
================================================================================
Class Definition
================================================================================
*/

/// A class for grouping of GeneticAlgorithmOperators
/**
 * This is the base class for groups of compatible GeneticAlgorithm operators.
 * Each group has registries associated with each type of operator.  Only
 * operators that can be used in conjuction with one another should be put into
 * the same group. For example, if you wish to devise a custom crossover scheme
 * that is intimately tied to a custom mutation scheme such that other mutation
 * schemes will cause incorrect behavior, then the two custom schemes should
 * appear as the only operators in their respective group registries (crosser
 * and mutator in this case).
 *
 * In addition to implementing the pure functions in this class, the following
 * requirements hold:
 *
 * All derived classes must implement each of the methods shown below.
 * They should be cut and pasted from below as is without modification
 * into the section entitled "Public Methods".
 * \code

    static
    GeneticAlgorithmOperatorRegistry&
    MutatorRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    ConvergerRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    CrosserRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    NichePressureApplicatorRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    FitnessAssessorRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    SelectorRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    PostProcessorRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    InitializerRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    EvaluatorRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }

    static
    GeneticAlgorithmOperatorRegistry&
    MainLoopRegistry()
    {
        static GeneticAlgorithmOperatorRegistry registry;
        return registry;
    }
    \endcode
 * These static methods should be called and returned from the virtual "Get"
 * methods associated with them.  The "Get" methods need not (and should not)
 * do anything else.
 *
 * All derived types must provide a static method that returns a unique name
 * for the operator group.  The method would look something like this:
 * \code

    static
    const std::string&
    Name()
    {
        static const std::string ret("Operator Group Name Here");
        return ret;
    };

   \endcode
 * This static method should be called and returned from the GetName method.
 * The GetName method need not (and should not) do anything else.
 *
 * Finally, registration of the desired operators and absorption of the desired
 * operator groups should take place in the constructor.  This can be done by
 * writing a static method in each derived class that will perform the
 * necessary operations.  It is recommended that you use a static boolean
 * within the class to prevent re-acquisition (which will not hurt anything, it
 * will just be wasted effort).
 */
class JEGA_SL_IEDECL GeneticAlgorithmOperatorGroup
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








    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Searches for the supplied \a op in the Mutator Registry.
         *
         * \param op The mutator to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasMutator(
            const GeneticAlgorithmMutator& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the Converger Registry.
         *
         * \param op The converger to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasConverger(
            const GeneticAlgorithmConverger& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the Crosser Registry.
         *
         * \param op The crosser to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasCrosser(
            const GeneticAlgorithmCrosser& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the FitnessAssessor
         *        Registry.
         *
         * \param op The fitness assessor to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasFitnessAssessor(
            const GeneticAlgorithmFitnessAssessor& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the
         *        NichePressureApplicator Registry.
         *
         * \param op The nicher to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasNichePressureApplicator(
            const GeneticAlgorithmNichePressureApplicator& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the Selector Registry.
         *
         * \param op The selector to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasSelector(
            const GeneticAlgorithmSelector& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the Post Processor
         *        Registry.
         *
         * \param op The post processor to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasPostProcessor(
            const GeneticAlgorithmPostProcessor& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the Initializer Registry.
         *
         * \param op The initializer to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasInitializer(
            const GeneticAlgorithmInitializer& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the Evaluator Registry.
         *
         * \param op The evaluator to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasEvaluator(
            const GeneticAlgorithmEvaluator& op
            ) const;

        /**
         * \brief Searches for the supplied \a op in the MainLoop Registry.
         *
         * \param op The main loop to test for inclusion in this group.
         * \return True of \a op is found and false otherwise.
         */
        bool
        HasMainLoop(
            const GeneticAlgorithmMainLoop& op
            ) const;


        /**
         * \brief Returns a newly created GeneticAlgorithmMutator or null if
         *        impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Mutator registry.
         */
        GeneticAlgorithmMutator*
        GetMutator(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmConverger or null if
         *        impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Converger registry.
         */
        GeneticAlgorithmConverger*
        GetConverger(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmCrosser or null if
         *        impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Crosser registry.
         */
        GeneticAlgorithmCrosser*
        GetCrosser(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created
         *        GeneticAlgorithmNichePressureApplicator or null if
         *        impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the NichePressureApplicator registry.
         */
        GeneticAlgorithmNichePressureApplicator*
        GetNichePressureApplicator(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmFitnessAssessor or
         *        null if impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the FitnessAssessor registry.
         */
        GeneticAlgorithmFitnessAssessor*
        GetFitnessAssessor(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmSelector or null if
         *        impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Selector registry.
         */
        GeneticAlgorithmSelector*
        GetSelector(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmPostProcessor or null
         *        if impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Post Processor registry.
         */
        GeneticAlgorithmPostProcessor*
        GetPostProcessor(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmInitializer or null
         *        if impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Initializer registry.
         */
        GeneticAlgorithmInitializer*
        GetInitializer(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmEvaluator or null
         *        if impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the Evaluator registry.
         */
        GeneticAlgorithmEvaluator*
        GetEvaluator(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /**
         * \brief Returns a newly created GeneticAlgorithmMainLoop or null
         *        if impossible.
         *
         * \param name The unique name of the operator to create.
         * \param algorithm The GA for which the operator is being created.
         * \return The newly created operator or null if an operator by \a name
         *         could not be found in the MainLoop registry.
         */
        GeneticAlgorithmMainLoop*
        GetMainLoop(
            const std::string& name,
            GeneticAlgorithm& algorithm
            ) const;

        /// Attempts to match operators of \a set to this group.
        /**
         * This method indicates whether or not the operators in the passed
         * \a set are all part of this group.  For this to be true, each
         * operator in set must be registered in the appropriate registry
         * of this group.
         *
         * \param set The set of existing operators to test for complete
         *            inclusion within this group.
         * \return true if this group has every operator in \a set and false
         *         otherwise.
         */
        bool
        ContainsSet(
            const GeneticAlgorithmOperatorSet& set
            ) const;

        /// Directs PrintOperators to a string and returns it.
        /**
         * \return The result of storing the text created by a call to
         *         PrintOperators in a string.
         */
        std::string
        GetOperatorReadout(
            ) const;

        /**
         * \brief Prints a listing of all operators registered in this group to
         *        \a stream.
         *
         * \param stream The output stream into which to write a listing of all
         *               the operators contained in this group.
         */
        void
        PrintOperators(
            std::ostream& stream
            ) const;

        /// Prints a listing of all operators in \a reg to \a stream.
        /**
         * \param reg The registry from which to retrieve operator names to
         *            write into \a stream.
         * \param stream The output stream into which to write the names of
         *               all operators contained in \a reg.
         */
        static
        void
        PrintOperators(
            const GeneticAlgorithmOperatorRegistry& reg,
            std::ostream& stream
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:


        /// Merges operators of \a FROM into \a INTO.
        /**
         * This method will assimilate all operators in the \a FROM group
         * type into the \a INTO group type.  Duplications are not allowed.
         * Both types \a FROM and \a INTO must implement the static *Registry()
         * methods as suggested in the notes for this class
         * (ex: MutatorRegistry()) or this won't compile.
         */
        template <typename FROM, typename INTO>
        static
        void
        AbsorbOperators(
            );

        /// Returns a new instance of the operator named \a name.
        /**
         * This method retrieves the operator named \a name from the group
         * \a from for use by \a algorithm.  If the operator does not exist,
         * the return is Null.
         *
         * \param name The name of the operator type to be created.
         * \param from The registry containing the operator type name mapped
         *             to a creation function.
         * \param algorithm The GA for which this new operator is being
         *                  created.
         * \return The new operator or null if \a name was not found in \a reg
         *         or any other error occurred.
         */
        static
        GeneticAlgorithmOperator*
        GetOperator(
            const std::string& name,
            const GeneticAlgorithmOperatorRegistry& from,
            GeneticAlgorithm& algorithm
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Retrieves the mutator registry.
        /**
         * This method returns the operator registry containing all allowable
         * GeneticAlgorithmMutators for this group.  It is pure and must be
         * implemented by any instantiable derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmMutators for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetMutatorRegistry(
            ) const = 0;

        /// Retrieves the converger registry.
        /**
         * This method is pure and must be implemented by any instantiable
         * derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmConvergers for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetConvergerRegistry(
            ) const = 0;

        /// Retrieves the niche pressure applicator registry.
        /**
         * This method is pure and must be implemented by any instantiable
         * derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmNichePressureApplicators for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetNichePressureApplicatorRegistry(
            ) const = 0;

        /// Retrieves the crosser registry.
        /**
         * This method is pure and must be implemented by any instantiable
         * derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmCrossers for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetCrosserRegistry(
            ) const = 0;

        /// Retrieves the fitness assessor registry.
        /**
         * This method is pure and must be implemented by any instantiable
         * derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmFitnessAssessors for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetFitnessAssessorRegistry(
            ) const = 0;

        /// Retrieves the selector registry.
        /**
         * This method is pure and must be implemented by any instantiable
         * derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmSelectors for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetSelectorRegistry(
            ) const = 0;

        /// Retrieves the post processor registry.
        /**
         * This method is pure and must be implemented by any instantiable
         * derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmPostProcessors for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetPostProcessorRegistry(
            ) const = 0;

        /// Retrieves the initializer registry.
        /**
         * This method returns the operator registry containing all allowable
         * GeneticAlgorithmInitializers for this group.  It is pure and must be
         * implemented by any instantiable derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmInitializers for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetInitializerRegistry(
            ) const = 0;

        /// Retrieves the evaluator registry.
        /**
         * This method returns the operator registry containing all allowable
         * GeneticAlgorithmEvaluators for this group.  It is pure and must be
         * implemented by any instantiable derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmEvaluators for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetEvaluatorRegistry(
            ) const = 0;

        /// Retrieves the main loop registry.
        /**
         * This method returns the operator registry containing all allowable
         * GeneticAlgorithmMainLoops for this group.  It is pure and must be
         * implemented by any instantiable derivatives.
         *
         * \return The operator registry containing all allowable
         *         GeneticAlgorithmMainLoops for this group.
         */
        virtual
        GeneticAlgorithmOperatorRegistry&
        GetMainLoopRegistry(
            ) const = 0;

        /// Retrieves the name of this operator group.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const = 0;


    protected:


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /// Mixes operators of \a from into \a into.
        /**
         * This method takes all operators in \a from and registers them in
         * \a into (if they are not there already).  The action is potentially
         * dangerous because no check is made to be certain that we are not
         * mixing operator types and so this method is private.
         *
         * \param from The operator registry from which to copy entries into
         *             \a into.
         * \param into The operator registry into which to copy entries from
         *             \a from.
         */
        static
        void
        Absorb(
            const GeneticAlgorithmOperatorRegistry& from,
            GeneticAlgorithmOperatorRegistry& into
            );

        /**
         * \brief Prints the operators of \a reg to \a stream each prefixed by
         *        \a prefix.
         *
         * \param reg The registry from which to get the names of operators for
         *            printing.
         * \param prefix The text to append before each name before it is
         *               printed.
         * \param stream The stream into which to insert what is written.
         */
        static
        void
        PrintOps(
            const GeneticAlgorithmOperatorRegistry& reg,
            const std::string& prefix,
            std::ostream& stream
            );

        /**
         * \brief Prints a message that the supplied operator \a op is not a
         *        member of this group.
         *
         * The message is output using the logging facility and is written at
         * the verbose level.  If logging is turned off, no message is written.
         *
         * \param opType The type of the operator sought.  (ex. Crosser).
         * \param op The operator that was not matched in this group.
         */
        void
        OutputMatchFailure(
            const std::string& opType,
            const GeneticAlgorithmOperator& op
            ) const;

        /**
         * \brief Prints a message that the supplied operator \a op is was
         *        found to be a member of this group.
         *
         * The message is output using the logging facility and is written at
         * the verbose level.  If logging is turned off, no message is written.
         *
         * \param opType The type of the operator sought.  (ex. Crosser).
         * \param op The operator that was matched in this group.
         */
        void
        OutputMatchSuccess(
            const std::string& opType,
            const GeneticAlgorithmOperator& op
            ) const;


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Destructs a GeneticAlgorithmOperatorGroup
        virtual
        ~GeneticAlgorithmOperatorGroup(
            );



}; // class GeneticAlgorithmOperatorGroup


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
#include "./inline/GeneticAlgorithmOperatorGroup.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMOPERATORGROUP_HPP
