/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmFitnessAssessor

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

        Tue May 20 13:11:41 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmFitnessAssessor class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMFITNESSASSESSOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMFITNESSASSESSOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmOperator.hpp>





/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
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
class FitnessRecord;
class GeneticAlgorithmFitnessAssessor;




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

/// The base class for all GeneticAlgorithm fitness assessors.
/**
 * The fitness assessor is responsible for comparing all the passed in Designs
 * and assigning a singular fitness value to each.  The Designs should have
 * been previously evaluated and so objective function and constraint
 * information should be available.
 *
 * The fitnesses computed must be stored and returned in a FitnessRecord for
 * use by subsequent operators.  By convention, the fitnesses should be such
 * that larger is better.  Derived assessors needn't adhere to this requirement
 * but if that is the case, an appropriate selector must be used that is
 * aware of the deviation.
 *
 * This base class requires any derived class to implement the AssessFitness
 * method.  It requires no configuration input beyond that required by its base
 * class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmFitnessAssessor :
    public GeneticAlgorithmOperator
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
         * \brief This is the method in which derived fitness assessors should
         *        do whatever they do.
         *
         * It is called by the genetic algorithm periodically.  Required
         * information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * This method should consider all Designs in all groups of \a groups
         * and assess a singular fitness value to each.
         *
         * \param groups The groups of Designs that should have their fitnesses
         *               assessed.
         * \return A record of the fitness values assessed to each of the
         *         designs in the passed in groups.
         */
        virtual
        const FitnessRecord*
        AssessFitness(
            const JEGA::Utilities::DesignGroupVector& groups
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version does nothing.
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

        /// Returns the name of the type of this operator.
        /**
         * \return The string "Fitness Assessor".
         */
        virtual
        std::string
        GetType(
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
         * \brief Constructs a GeneticAlgorithmFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this fitness assessor is
         *                  being constructed.
         */
        GeneticAlgorithmFitnessAssessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmFitnessAssessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmFitnessAssessor(
            const GeneticAlgorithmFitnessAssessor& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmFitnessAssessor for use
         *        by \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is
         *                  being constructed.
         */
        GeneticAlgorithmFitnessAssessor(
            const GeneticAlgorithmFitnessAssessor& copy,
            GeneticAlgorithm& algorithm
            );

}; // class GeneticAlgorithmFitnessAssessor



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
#include "./inline/GeneticAlgorithmFitnessAssessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMFITNESSASSESSOR_HPP
