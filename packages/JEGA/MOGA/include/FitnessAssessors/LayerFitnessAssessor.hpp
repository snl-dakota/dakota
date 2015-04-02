/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class LayerFitnessAssessor.

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

        Tue Jun 24 14:32:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the LayerFitnessAssessor class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_LAYERFITNESSASSESSOR_HPP
#define JEGA_ALGORITHMS_LAYERFITNESSASSESSOR_HPP







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
class LayerFitnessAssessor;







/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief Considers a Designs layer its fitness (see class
 *        MultiObjectiveStatistician).
 *
 * This fitness assessor computes fitness using a scheme based on the concepts
 * proposed in:
 *
 * Goldberg, D.E., Korb, B., and Deb, K. (1989). "Messy Genetic Algorithms:
 * Motivation, Analysis, and First Results". Complex Systems, 3:493-530
 *
 * As presented in:
 *
 * Coello Coello, C.A., Van Veldhuizen, D.A., and Lamont, G.B.,
 * "Evolutionary Algorithms for Solving Multi-Objective Problems",
 * Kluwer Academic/Plenum Publishers, New York, NY., 2002.
 *
 * In Goldberg's scheme, the idea of rank is akin to a
 * MultiObjectiveStatistician's idea of layer.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class LayerFitnessAssessor :
    public GeneticAlgorithmFitnessAssessor
{
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

        /// Returns the proper name of this operator.
        /**
         * \return The string "layer_rank".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This fitness assessor computes fitness according to the algorithm
            proposed in:
            Goldberg, D.E., Korb, B., and Deb, K. (1989). "Messy Genetic
            Algorithms: Motivation, Analysis, and First Results". Complex
            Systems, 3:493-530

            As presented in:
            Coello Coello, C.A., Van Veldhuizen, D.A., and Lamont, G.B.,
            "Evolutionary Algorithms for Solving Multi-Objective Problems",
            Kluwer Academic/Plenum Publishers, New York, NY., 2002.
            In the above references, the idea of rank is akin to a
            MultiObjectiveStatistician 's idea of layer.
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
         * \return A new, default instance of a LayerFitnessAssessor.
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

        /// This version of AssessFitness does absolutely nothing.
        /**
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


        /// Constructs an LayerFitnessAssessor for use by \a algorithm.
        /**
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        LayerFitnessAssessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an LayerFitnessAssessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        LayerFitnessAssessor(
            const LayerFitnessAssessor& copy
            );

        /// Copy constructs an NullFitnessAssessor for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        LayerFitnessAssessor(
            const LayerFitnessAssessor& copy,
            GeneticAlgorithm& algorithm
            );

}; // class LayerFitnessAssessor


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
#include "./inline/LayerFitnessAssessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_AYERFITNESSASSESSOR_HPP
