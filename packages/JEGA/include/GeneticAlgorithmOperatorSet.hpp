/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmOperatorSet.

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

        Wed May 28 16:03:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmOperatorSet class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMOPERATORSET_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMOPERATORSET_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

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
namespace JEGA {
    namespace Algorithms {








/*
================================================================================
Begin Namespace
================================================================================
*/








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
class GeneticAlgorithmEvaluator;
class GeneticAlgorithmConverger;
class GeneticAlgorithmInitializer;
class GeneticAlgorithmOperatorSet;
class GeneticAlgorithmPostProcessor;
class GeneticAlgorithmFitnessAssessor;
class GeneticAlgorithmNichePressureApplicator;





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

/// A holder for a complete set of GeneticAlgorithmOperators.
/**
 * They include (in no particular order) a GeneticAlgorithm:
 *
 * Mutator
 * Crosser
 * Converger
 * Initializer
 * Selector
 * MainLoop
 * Evaluator
 * PostProcessor
 * FitnessAssessor
 * NichePressureApplicator
 *
 * Each instance of an operator set maintains a collection of default operators
 * that cannot be changed.  All the defaults are instances of the Null version
 * of the associated operator.  The set is considered to contain the default
 * operator if and only if the currently held operator is the exact same
 * instance as the default operator.  Being the same type means nothing.
 *
 * This object does not "own" the operators within it although it does offer
 * method that can be used to destroy them.  Do not directly destroy an
 * operator obtained from an operator set as it may be a default operator
 * and thus should not be deleted external to this class.  Instead use
 * the methods provided to destroy the operators which will account for the
 * possiblility of having default operators.
 *
 * If you do delete an operator external to a set containing it, be sure to set
 * the operator appropriately either to another new operator or to null in
 * which case the default version will be used.
 */
class JEGA_SL_IEDECL GeneticAlgorithmOperatorSet
{
    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    private:

        /**
         * \brief A class encapsulating the default operators used all of which
         *        are Null.
         */
        class DefaultOperators;


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

        /// The set of default operators used by this set.
        DefaultOperators* _defaults;

        /// A pointer to the operator that performs mutation for this algorithm
        GeneticAlgorithmMutator* _mutator;

        /**
         * \brief A pointer to the operator that performs convergence testing
         *        for this algorithm.
         */
        GeneticAlgorithmConverger* _converger;

        /**
         * \brief A pointer to the operator that performs crossover for this
         *        algorithm
         */
        GeneticAlgorithmCrosser* _crosser;

        /**
         * \brief A pointer to the operator that performs fitness assessment
         *        for this algorithm
         */
        GeneticAlgorithmFitnessAssessor* _ftnsAssessor;

        /**
         * \brief A pointer to the operator that performs initialization for
         *        this algorithm
         */
        GeneticAlgorithmInitializer* _initializer;

        /**
         * \brief A pointer to the operator that performs selection for this
         *        algorithm
         */
        GeneticAlgorithmSelector* _selector;

        /**
         * \brief A pointer to the operator that performs post processing for
         *        this algorithm
         */
        GeneticAlgorithmPostProcessor* _postProcessor;

        /**
         * \brief A pointer to the operator that performs evaluation for this
         *        algorithm
         */
        GeneticAlgorithmEvaluator* _evaluator;

        /**
         * \brief A pointer to the operator that applies niche pressure for
         *        this algorithm
         */
        GeneticAlgorithmNichePressureApplicator* _nicher;

        /**
         * \brief A pointer to the operator that performs the main loop for
         *        this algorithm
         */
        GeneticAlgorithmMainLoop* _mainLoop;

        /**
         * \brief The GeneticAlgorithm for which this is the operator set.
         *
         * All the operators in this set should have been created for the
         * same algorithm as this one.
         */
        GeneticAlgorithm& _algorithm;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the \a _mutator pointer to \a op.
        /**
         * \a op may be null in which case the default mutator is used.
         *
         * \param op The new mutator to use or null for default.
         */
        void
        SetMutator(
            GeneticAlgorithmMutator* op
            );

        /// Sets the \a _crosser pointer to \a op.
        /**
         * \a op may be null in which case the default crosser is used.
         *
         * \param op The new crosser to use or null for default.
         */
        void
        SetCrosser(
            GeneticAlgorithmCrosser* op
            );

        /// Sets the \a _converger pointer to \a op.
        /**
         * \a op may be null in which case the default converger is used.
         *
         * \param op The new converger to use or null for default.
         */
        void
        SetConverger(
            GeneticAlgorithmConverger* op
            );

        /// Sets the \a _evaluator pointer to \a op.
        /**
         * \a op may be null in which case the default evaluator is used.
         *
         * \param op The new evaluator to use or null for default.
         */
        void
        SetEvaluator(
            GeneticAlgorithmEvaluator* op
            );

        /// Sets the \a _nicher pointer to \a op.
        /**
         * \a op may be null in which case the default niche pressure
         * applicator is used.
         *
         * \param op The new niche pressure applicator to use or null for
         *           default.
         */
        void
        SetNichePressureApplicator(
            GeneticAlgorithmNichePressureApplicator* op
            );

        /// Sets the \a _ftnsAssessor pointer to \a op.
        /**
         * \a op may be null in which case the default fitness assessor is
         * used.
         *
         * \param op The new fitness assessor to use or null for default.
         */
        void
        SetFitnessAssessor(
            GeneticAlgorithmFitnessAssessor* op
            );

        /// Sets the \a _mainLoop pointer to \a op.
        /**
         * \a op may be null in which case the default main loop is used.
         *
         * \param op The new main loop to use or null for default.
         */
        void
        SetMainLoop(
            GeneticAlgorithmMainLoop* op
            );

        /// Sets the \a _initializer pointer to \a op.
        /**
         * \a op may be null in which case the default initializer is used.
         *
         * \param op The new initializer to use or null for default.
         */
        void
        SetInitializer(
            GeneticAlgorithmInitializer* op
            );

        /// Sets the \a _selector pointer to \a op.
        /**
         * \a op may be null in which case the default selector is used.
         *
         * \param op The new selector to use or null for default.
         */
        void
        SetSelector(
            GeneticAlgorithmSelector* op
            );

        /// Sets the \a _postProcessor pointer to \a op.
        /**
         * \a op may be null in which case the default post processor is used.
         *
         * \param op The new post processor to use or null for default.
         */
        void
        SetPostProcessor(
            GeneticAlgorithmPostProcessor* op
            );



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Allows mutable access to the algorithm known by this set.
        /**
         * \return The GA for which this is or will be the operator set.
         */
        inline
        GeneticAlgorithm&
        GetAlgorithm(
            );

        /// Allows immutable access to the algorithm known by this set.
        /**
         * \return The GA for which this is or will be the operator set.
         */
        inline
        const GeneticAlgorithm&
        GetAlgorithm(
            ) const;

        /// Gets the current mutator (non-const).
        /**
         * This method allows mutable access to the mutator known by this
         * operator set.  It is always safe to call this method, there is
         * always a mutator associated but may be an instance of the
         * NullMutator (default).
         *
         * \return A reference to the mutator known to this operator set.
         */
        GeneticAlgorithmMutator&
        GetMutator(
            );

        /// Gets the current crosser (non-const).
        /**
         * This method allows mutable access to the crosser known by this
         * operator set.  It is always safe to call this method, there is
         * always a crosser associated but may be an instance of the
         * NullCrosser (default).
         *
         * \return A reference to the crosser known to this operator set.
         */
        GeneticAlgorithmCrosser&
        GetCrosser(
            );

        /// Gets the current converger (non-const).
        /**
         * This method allows mutable access to the converger known by this
         * operator set.  It is always safe to call this method, there is
         * always a converger associated but may be an instance of the
         * NullConverger (default).
         *
         * \return A reference to the converger known to this operator set.
         */
        GeneticAlgorithmConverger&
        GetConverger(
            );

        /// Gets the current evaluator (non-const).
        /**
         * This method allows mutable access to the evaluator known by this
         * operator set.  It is always safe to call this method, there is
         * always an evaluator associated but may be an instance of the
         * NullEvaluator (default).
         *
         * \return A reference to the evaluator known to this operator set.
         */
        GeneticAlgorithmEvaluator&
        GetEvaluator(
            );

        /// Gets the current niche pressure applicator (non-const).
        /**
         * This method allows mutable access to the niche pressure applicator
         * known by this operator set.  It is always safe to call this method,
         * there is always a niche pressure applicator associated but may be an
         * instance of the NullNichePressureApplicator (default).
         *
         * \return A reference to the niche pressure applicator known to this
         *         operator set.
         */
        GeneticAlgorithmNichePressureApplicator&
        GetNichePressureApplicator(
            );

        /// Gets the current fitness assessor (non-const).
        /**
         * This method allows mutable access to the fitness assessor known by
         * this operator set.  It is always safe to call this method, there is
         * always a fitness assessor associated but may be an instance of the
         * NullFitnessAssessor (default).
         *
         * \return A reference to the fitness assessor known to this operator
         *         set.
         */
        GeneticAlgorithmFitnessAssessor&
        GetFitnessAssessor(
            );

        /// Gets the current main loop (non-const).
        /**
         * This method allows mutable access to the main loop known by this
         * operator set.  It is always safe to call this method, there is
         * always a main loop associated but may be an instance of the
         * NullMainLoop (default).
         *
         * \return A reference to the main loop known to this operator set.
         */
        GeneticAlgorithmMainLoop&
        GetMainLoop(
            );

        /// Gets the current initializer (non-const).
        /**
         * This method allows mutable access to the initializer known by this
         * operator set.  It is always safe to call this method, there is
         * always an initializer associated but may be an instance of the
         * NullInitializer (default).
         *
         * \return A reference to the initializer known to this operator set.
         */
        GeneticAlgorithmInitializer&
        GetInitializer(
            );

        /// Gets the current selector (non-const).
        /**
         * This method allows mutable access to the selector known by this
         * operator set.  It is always safe to call this method, there is
         * always a selector associated but may be an instance of the
         * NullSelector (default).
         *
         * \return A reference to the selector known to this operator set.
         */
        GeneticAlgorithmSelector&
        GetSelector(
            );

        /// Gets the current post processor (non-const).
        /**
         * This method allows mutable access to the post processor known by
         * this operator set.  It is always safe to call this method, there is
         * always a post processor associated but may be an instance of the
         * NullPostProcessor (default).
         *
         * \return A reference to the post processor known to this operator
         *         set.
         */
        GeneticAlgorithmPostProcessor&
        GetPostProcessor(
            );

        /// Gets the current mutator (const).
        /**
         * This method allows immutable access to the mutator known by this
         * operator set.  It is always safe to call this method, there is
         * always a mutator associated but may be an instance of the
         * NullMutator (default).
         *
         * \return A reference to the mutator known to this operator set.
         */
        const GeneticAlgorithmMutator&
        GetMutator(
            ) const;

        /// Gets the current crosser (const).
        /**
         * This method allows immutable access to the crosser known by this
         * operator set.  It is always safe to call this method, there is
         * always a crosser associated but may be an instance of the
         * NullCrosser (default).
         *
         * \return A reference to the crosser known to this operator set.
         */
        const GeneticAlgorithmCrosser&
        GetCrosser(
            ) const;

        /// Gets the current converger (const).
        /**
         * This method allows immutable access to the converger known by this
         * operator set.  It is always safe to call this method, there is
         * always a converger associated but may be an instance of the
         * NullConverger (default).
         *
         * \return A reference to the converger known to this operator set.
         */
        const GeneticAlgorithmConverger&
        GetConverger(
            ) const;

        /// Gets the current evaluator (const).
        /**
         * This method allows immutable access to the evaluator known by this
         * operator set.  It is always safe to call this method, there is
         * always an evaluator associated but may be an instance of the
         * NullEvaluator (default).
         *
         * \return A reference to the evaluator known to this operator set.
         */
        const GeneticAlgorithmEvaluator&
        GetEvaluator(
            ) const;

        /// Gets the current niche pressure applicator (const).
        /**
         * This method allows immutable access to the niche pressure applicator
         * known by this operator set.  It is always safe to call this method,
         * there is always a niche pressure applicator associated but may be an
         * instance of the NullNichePressureApplicator (default).
         *
         * \return A reference to the niche pressure applicator known to this
         *         operator set.
         */
        const GeneticAlgorithmNichePressureApplicator&
        GetNichePressureApplicator(
            ) const;

        /// Gets the current fitness assessor (const).
        /**
         * This method allows immutable access to the fitness assessor known by
         * this operator set.  It is always safe to call this method, there is
         * always a fitness assessor associated but may be an instance of the
         * NullFitnessAssessor (default).
         *
         * \return A reference to the fitness assessor known to this operator
         *         set.
         */
        const GeneticAlgorithmFitnessAssessor&
        GetFitnessAssessor(
            ) const;

        /// Gets the current main loop (const).
        /**
         * This method allows immutable access to the main loop known by this
         * operator set.  It is always safe to call this method, there is
         * always a main loop associated but may be an instance of the
         * NullMainLoop (default).
         *
         * \return A reference to the main loop known to this operator set.
         */
        const GeneticAlgorithmMainLoop&
        GetMainLoop(
            ) const;

        /// Gets the current initializer (const).
        /**
         * This method allows immutable access to the initializer known by this
         * operator set.  It is always safe to call this method, there is
         * always an initializer associated but may be an instance of the
         * NullInitializer (default).
         *
         * \return A reference to the initializer known to this operator set.
         */
        const GeneticAlgorithmInitializer&
        GetInitializer(
            ) const;

        /// Gets the current selector (const).
        /**
         * This method allows immutable access to the selector known by this
         * operator set.  It is always safe to call this method, there is
         * always a selector associated but may be an instance of the
         * NullSelector (default).
         *
         * \return A reference to the selector known to this operator set.
         */
        const GeneticAlgorithmSelector&
        GetSelector(
            ) const;

        /// Gets the current post processor (const).
        /**
         * This method allows immutable access to the post processor known by
         * this operator set.  It is always safe to call this method, there is
         * always a post processor associated but may be an instance of the
         * NullPostProcessor (default).
         *
         * \return A reference to the post processor known to this operator
         *         set.
         */
        const GeneticAlgorithmPostProcessor&
        GetPostProcessor(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Assignement operator.
        /**
         * Assignment is carried out by direct pointer duplication of the
         * operators in \a rhs to \a this.  The only exception is the case
         * where a default operator is used.  In that case, the default
         * operator of \a this is used instead of the default operator of
         * \a rhs.
         *
         * Note that no operator destruction occurs so if the operators held
         * in \a this prior to this call are not accounted for, there will be
         * a memory leak.
         *
         * \param rhs The operator set from which to get the new operators of
         *            this.
         * \return This after assignment.
         */
        const GeneticAlgorithmOperatorSet&
        operator =(
            const GeneticAlgorithmOperatorSet& rhs
            );

        /// Resets all operators to the default operators.
        /**
         * Note that no operator destruction occurs so if the operators held
         * in \a this prior to this call are not accounted for, there will be
         * a memory leak.
         */
        void
        Clear(
            );

        /// Creates a duplicate of this set containing cloned operators.
        /**
         * The returned set contains clones of the operators in this set
         * constructed for use with the supplied algorithm.  The only exception
         * is of any default operators.  They are not cloned but are
         * instead left as the existing default operators in the new group.
         *
         * \param algorithm The GA for which the new group is being
         *                  created.
         * \return A new operator set that is an exact duplicate of this.
         */
        GeneticAlgorithmOperatorSet
        Clone(
            GeneticAlgorithm& algorithm
            ) const;

        /// Returns true if this operator set is using the default mutator.
        /**
         * \return true if the known mutator is the default mutator.
         */
        bool
        HasDefaultMutator(
            ) const;

        /// Returns true if this operator set is using the default converger.
        /**
         * \return true if the known converger is the default converger.
         */
        bool
        HasDefaultConverger(
            ) const;

        /// Returns true if this operator set is using the default crosser.
        /**
         * \return true if the known crosser is the default crosser.
         */
        bool
        HasDefaultCrosser(
            ) const;

        /// Returns true if this operator set is using the default initializer.
        /**
         * \return true if the known initializer is the default initializer.
         */
        bool
        HasDefaultInitializer(
            ) const;

        /// Returns true if this operator set is using the default evaluator.
        /**
         * \return true if the known evaluator is the default evaluator.
         */
        bool
        HasDefaultEvaluator(
            ) const;

        /**
         * \brief Returns true if this operator set is using the default niche
         *        pressure applicator.
         *
         * \return true if the known niche pressure applicator is the default
         *         niche pressure applicator.
         */
        bool
        HasDefaultNichePressureApplicator(
            ) const;

        /**
         * \brief Returns true if this operator set is using the default
         *        fitness assessor.
         *
         * \return true if the known mutator is the default mutator.
         */
        bool
        HasDefaultFitnessAssessor(
            ) const;

        /// Returns true if this operator set is using the default main loop.
        /**
         * \return true if the known main loop is the default main loop.
         */
        bool
        HasDefaultMainLoop(
            ) const;

        /// Returns true if this operator set is using the default selector.
        /**
         * \return true if the known selector is the default selector.
         */
        bool
        HasDefaultSelector(
            ) const;

        /**
         * \brief Returns true if this operator set is using the default post
         *        processor.
         *
         * \return true if the known post processor is the default post
         *         processor.
         */
        bool
        HasDefaultPostProcessor(
            ) const;

        /// Calls the corresponding Destroy* for each operator type.
        void
        DestroyOperators(
            );

        /**
         * \brief Deletes the currently known mutator and adopts the default
         *        as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyMutator(
            );

        /**
         * \brief Deletes the currently known converger and adopts the default
         *        as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyConverger(
            );

        /**
         * \brief Deletes the currently known crosser and adopts the default
         *        as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyCrosser(
            );

        /**
         * \brief Deletes the currently known initializer and adopts the
         *        default as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyInitializer(
            );

        /**
         * \brief Deletes the currently known niche pressure applicator and
         *        adopts the default as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyNichePressureApplicator(
            );

        /**
         * \brief Deletes the currently known evaluator and adopts the default
         *        as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyEvaluator(
            );

        /**
         * \brief Deletes the currently known fitness assessor and adopts the
         *        default as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyFitnessAssessor(
            );

        /**
         * \brief Deletes the currently known main loop and adopts the default
         *        as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyMainLoop(
            );

        /**
         * \brief Deletes the currently known selector and adopts the default
         *        as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroySelector(
            );

        /**
         * \brief Deletes the currently known post processor and adopts the
         *        default as the current.
         *
         * If the current is the default, nothing happens.
         */
        void
        DestroyPostProcessor(
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

        /// Constructs a GeneticAlgorithmOperatorSet for use by \a algorithm.
        /**
         * The default operators will be installed by this constructor.
         *
         * \param algorithm The GA for which this set is being created.
         */
        GeneticAlgorithmOperatorSet(
            GeneticAlgorithm& algorithm
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmOperatorSet for use by the
         *        same algorithm.
         *
         * After copy construction, this set will contain exactly the same
         * instances of the operators as \a copy and will know the same
         * algorithm.  The one exception will be that if any of the operators
         * known by \a copy are it's default operators, then this will use it's
         * own defaults, not \a copy's.
         *
         * \param copy The operator set to duplicate into this (without
         *             cloning).
         */
        GeneticAlgorithmOperatorSet(
            const GeneticAlgorithmOperatorSet& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmOperatorSet for use by a
         *        different algorithm.
         *
         * After copy construction, this set will contain clones of the
         * operators of \a copy created for use with \a algorithm.
         *
         * \param copy The operator set to duplicate into this (with cloning).
         * \param algorithm The GA for which this set is being created.
         */
        GeneticAlgorithmOperatorSet(
            const GeneticAlgorithmOperatorSet& copy,
            GeneticAlgorithm& algorithm
            );

        /// Destructs a GeneticAlgorithmOperatorSet.
        /**
         * This does not cause destruction of any of the operators in the set
         * with the exception of any default operators.
         */
        ~GeneticAlgorithmOperatorSet(
            );

}; // class GeneticAlgorithmOperatorSet


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
#include "./inline/GeneticAlgorithmOperatorSet.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMOPERATORSET_HPP
