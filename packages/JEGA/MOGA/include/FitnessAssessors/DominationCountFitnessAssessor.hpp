/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DominationCountFitnessAssessor.

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

        Thu Dec 22 09:49:30 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DominationCountFitnessAssessor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_DOMINATIONCOUNTFITNESSASSESSOR_HPP
#define JEGA_ALGORITHMS_DOMINATIONCOUNTFITNESSASSESSOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmFitnessAssessor.hpp>









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
class DominationCountFitnessAssessor;







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
 * \brief Assigns fitness to be the negative of the number of designs that
 *        dominate the current design.
 *
 * So if the design being assessed is dominated by 25 other designs being
 * considered, then its fitness is -25.  This is in keeping with the convention
 * that higher fitness is better.
 *
 * This fitness assessor computes fitness using a scheme based on the concepts
 * proposed in:
 *
 * Fonesca, C.M. and Fleming, P.J. (1993) "Genetic Algorithms for
 * Multiobjective Optimization: Formulation, Discussion and Generalization".
 * In Forrest, S., editor, Proceedings of the Fifth International Conference
 * on Genetic Algorithms, pages 416-423, San Mateo, California.  Universit
 * of Illinois at Ubana-Champaigne, Morgan Kaufmann Publishers.
 *
 * As presented in:
 *
 * Coello Coello, C.A., Van Veldhuizen, D.A., and Lamont, G.B.,
 * "Evolutionary Algorithms for Solving Multi-Objective Problems",
 * Kluwer Academic/Plenum Publishers, New York, NY., 2002.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class DominationCountFitnessAssessor :
    public GeneticAlgorithmFitnessAssessor
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
         * \return The string "domination_count".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            "This fitness assessor assigns the negative of the number of "
            "designs in the passed in groups that dominate the given design "
            "as the fitness."
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
         * \param algorithm The GA for which the new fitness assessor is to be
         *        used.
         * \return A new, default instance of a DominationCountFitnessAssessor.
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


        /// Assigns fitnesses to each design in \a groups.
        /**
         * This is the method in which this fitness assessor does what it does.
         * It computes the domination counts and assigns fitnesses to be the
         * negatives of those counts.  The results are returned in a fitness
         * record.
         *
         * \param groups The groups of all Designs that should be assessed for
         *               fitness together.
         * \return A record of all fitnesses computed by this assessor mapped
         *         to the Design to which that value is associated.
         */
        virtual
        const FitnessRecord*
        AssessFitness(
            const JEGA::Utilities::DesignGroupVector& groups
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
         * \brief Constructs an DominationCountFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        DominationCountFitnessAssessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an DominationCountFitnessAssessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        DominationCountFitnessAssessor(
            const DominationCountFitnessAssessor& copy
            );

        /**
         * \brief Copy constructs an DominationCountFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        DominationCountFitnessAssessor(
            const DominationCountFitnessAssessor& copy,
            GeneticAlgorithm& algorithm
            );





}; // class DominationCountFitnessAssessor



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
#include "inline/DominationCountFitnessAssessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_DOMINATIONCOUNTFITNESSASSESSOR_HPP
