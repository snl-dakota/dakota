/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class SimpleFunctorEvaluator.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Mon Jul 17 14:57:43 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the SimpleFunctorEvaluator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_SIMPLEFUNCTOREVALUATOR_HPP
#define JEGA_ALGORITHMS_SIMPLEFUNCTOREVALUATOR_HPP







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
class SimpleFunctorEvaluator;







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
 * \brief An evaluator that makes a call to an implementation of the nested
 *        Functor class.
 *
 * The functor must be set prior to using this evaluator or a fatal log message
 * will be issued.  This class does not assume ownership of the Functor nor
 * does JEGA in general.  It is up to front end implementors to be sure that
 * the memory for the Functor is properly managed.
 *
 * See the documentation of the nested Functor class for information on the
 * needs of Functor implementors.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class SimpleFunctorEvaluator :
    public GeneticAlgorithmEvaluator
{

    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    public:

        /**
         * \brief A callback functor interface for classes that will perform
         *        actual evaluations when this evalutator is called.
         *
         * Implementors must override the Evaluate method to perform the actual
         * evaluations of the objectives and constraints based on the design
         * variables.
         *
         * See the documentation for the Evaluate method for more information.
         */
        class Functor
        {
            /*
            ====================================================================
            Subclass Overridable Methods
            ====================================================================
            */
            public:

                /**
                 * \brief Override this method to perform evaluations for a
                 *        given set of design variables.
                 *
                 * Each of the passed in vectors is pre-sized.  The design
                 * variable vector (\a X) cannot be modified but the objective
                 * function (\a F) and constraint (\a G) vectors can be.  Do
                 * not change the size of these vectors.  The only thing you
                 * should do is assign values to them with one of the two
                 * methods shown below:
                 * \code
                    F[0] = 3.4;
                    F.at(0) = 3.4;
                   \endcode
                 *
                 * Using methods like push_back and insert will cause problems.
                 * JEGA will inspect only the first "nof" objective vector
                 * elements and "ncn" elements of the constraint vector.
                 *
                 * NOTE: This method is strictly for the evaluation of
                 * responses. Do not attempt to compute some sort of fitness
                 * value unless you are certain of what you are doing and have
                 * set the rest of the problem up such that that makes sense.
                 * The intention of JEGA is to allow the user to simply supply
                 * raw responses and perform any necessary fitness assessment
                 * internally.
                 *
                 * \param X The vector of design variables.  This vector has
                 *          as many elements as there are design variables in
                 *          the problem.
                 * \param F The vector of objective functions.  This vector
                 *          typically has nonsense for values.  Install
                 *          sensible values as a result of the evaluations.
                 * \param G The vector of constraint functions.  This vector
                 *          typically has nonsense for values.  Install
                 *          sensible values as a result of the evaluations.
                 */
                virtual
                bool
                Evaluate(
                    const JEGA::DoubleVector& X,
                    JEGA::DoubleVector& F,
                    JEGA::DoubleVector& G
                    ) = 0;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Destructs a Functor.
                virtual
                ~Functor(
                    ) {}

        }; // class SimpleFunctorEvaluator::Functor


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

        /// The pointer to the functor used to perform response evaluation.
        Functor* _theFunctor;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /**
         * \brief Allows mutation of the pointer to the evaluation functor.
         *
         * \param theFunc The new evaluation functor for this evaluator to use.
         */
        void
        SetEvaluationFunctor(
            Functor* theFunc
            );

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

        /// Returns the proper name of this operator.
        /**
         * \return The string "simple_functor".
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
         * \return A new, default instance of a SimpleFunctorEvaluator.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            GeneticAlgorithm& algorithm
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

        /**
         * \brief Overridden to call the known functor for evaluations.
         *
         * It may be called either by the genetic algorithm or by the
         * overloaded version of this method taking a DesignGroup argument.
         * This implementation calls back to this base class version after
         * having installed all responses into the Design as per requirements.
         *
         * Note that evaluation in this algorithm has nothing to do with
         * fitness assessment.  This method only installs raw constraint and
         * objective function values.  Fitness assessment based on these values
         * is the job of a GeneticAlgorithmFitnessAssessor.
         *
         * \param des The Design object to evaluate.
         * \return True if \a des winds up with proper responses and everything
         *         is ok and false otherwise.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::Design& evalJob
            );

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
            GeneticAlgorithm& algorithm
            ) const;


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
         * \brief Constructs a SimpleFunctorEvaluator for use by \a algorithm
         *        that uses \a theFunc to get evaluations done.
         *
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         * \param theFunc The Funtor that will be used to acutally perform
         *                evaluations.
         */
        SimpleFunctorEvaluator(
            GeneticAlgorithm& algorithm,
            Functor* theFunc
            );

        /// Constructs a SimpleFunctorEvaluator for use by \a algorithm.
        /**
         * If you use this constructor, it will be necessary for you to supply
         * an evaluation functor via the SetEvaluationFunctor method prior to
         * use.
         *
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        SimpleFunctorEvaluator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an SimpleFunctorEvaluator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        SimpleFunctorEvaluator(
            const SimpleFunctorEvaluator& copy
            );

        /// Copy constructs an SimpleFunctorEvaluator for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        SimpleFunctorEvaluator(
            const SimpleFunctorEvaluator& copy,
            GeneticAlgorithm& algorithm
            );


}; // class SimpleFunctorEvaluator



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
#include "inline/SimpleFunctorEvaluator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_SIMPLEFUNCTOREVALUATOR_HPP
