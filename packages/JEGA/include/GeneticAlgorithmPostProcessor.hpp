/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmPostProcessor

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Aug 17 15:26:36 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmPostProcessor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMPOSTPROCESSOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMPOSTPROCESSOR_HPP







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
        class DesignGroup;
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
class GeneticAlgorithmPostProcessor;




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

/// The base class for all GeneticAlgorithm post processors.
/**
 * The post processor is responsible for taking the results of the algorithm
 * execution and processing them in whatever way is desired.  For example, it
 * may be desired to thin out a Pareto frontier or strip out all but 1 of the
 * equally optimal solutions found by a single objective algorithm, etc.
 *
 * This base class requires of it's derivatives implementation of the
 * PostProcess method.  It requires no configuration input beyond that required
 * by its base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmPostProcessor :
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
         * \brief This is the method in which derived post processors do
         *        whatever they do.
         *
         * This will be called by the GA once at the end of execution.
         * Required information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * \param group The group of designs to perform post processing on.
         */
        virtual
        void
        PostProcess(
            JEGA::Utilities::DesignGroup& group
            ) = 0;


        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
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
         * \return The string "Post Processor".
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

        /// Constructs a GeneticAlgorithmPostProcessor for use by \a algorithm.
        /**
         * \param algorithm The GA for which this post processor is being
         *                  constructed.
         */
        GeneticAlgorithmPostProcessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmPostProcessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmPostProcessor(
            const GeneticAlgorithmPostProcessor& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmPostProcessor for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this post processor is being
         *                  constructed.
         */
        GeneticAlgorithmPostProcessor(
            const GeneticAlgorithmPostProcessor& copy,
            GeneticAlgorithm& algorithm
            );



}; // class GeneticAlgorithmPostProcessor


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
#include "./inline/GeneticAlgorithmPostProcessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMPOSTPROCESSOR_HPP
