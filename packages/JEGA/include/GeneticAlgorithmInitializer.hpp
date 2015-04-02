/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmInitializer

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

        Tue May 13 14:20:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmInitializer class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMINITIALIZER_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMINITIALIZER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmOperator.hpp>
#include <utilities/include/int_types.hpp>




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
class GeneticAlgorithmInitializer;




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

/// The base class for all GeneticAlgorithm initializers.
/**
 * The initializer is responsible for creating the initial population. It may
 * do so in any way that it wishes.  To create Designs only requires that
 * design variable values be specified.  This operator is NOT responsible for
 * evaluating the generated Designs.  That will be done in a different step.
 *
 * This base class provides the value for the desired number of designs in the
 * initial population and requires any derived classes to implement the
 * Initialize method.
 *
 * The desired initial size is extracted from the parameter database using the
 * name "method.population_size".  It is extracted as a size type parameter.
 * If it is not supplied in the parameter database, the default value as
 * defined by DEFAULT_INIT_SIZE will be used.  This is required in addition to
 * any requirements of the base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmInitializer :
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
    public:

        /// The default desired initial population size.
        static const std::size_t DEFAULT_INIT_SIZE;

    private:

        /**
         * \brief The number of designs to be inserted into any group by this
         *        initializer.
         *
         * The exact interpretation of this is implementation dependent.  Some
         * initializers may interpret this as the number to be added to
         * whatever already exists, some may consider it to be the number
         * that should ultimately be in the group, and some may use it in some
         * completely different way.
         */
        std::size_t _size;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the number of designs to be created during initialization.
        /**
         * This method logs a verbose level entry informing of the new size.
         *
         * \param size The size to which this initializer should initialize
         *             a group of designs.
         */
        void
        SetSize(
            std::size_t size
            );




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the number of designs to be created during initialization.
        /**
         * \return The number of Designs that will be created/read/whatever by
         *         this initializer.
         */
        inline
        std::size_t
        GetSize(
            ) const;




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
         * \brief This is the method in which derived initializers should do
         *        whatever they do.  It is called by the genetic algorithm.
         *
         * Required information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * This method should go about creating new Designs
         * (using GeneticAlgorithm::GetNewDesign()) and providing them with
         * appropriate design variable values (no evaluation should occur).
         * The new Designs should be place into \a into.
         *
         * \param into The group into which new Designs are to be placed by
         *             this initializer.
         */
        virtual
        void
        Initialize(
            JEGA::Utilities::DesignGroup& into
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the desired initial population size.
         * Derived classes may choose to use this information or not.
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
         * \return The string "Initializer".
         */
        virtual
        std::string
        GetType(
            ) const;

        virtual
        bool
        CanProduceInvalidVariableValues(
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

        /// Constructs a GeneticAlgorithmInitializer for use by \a algorithm.
        /**
         * \param algorithm The GA for which this initializer is being
         *                  constructed.
         */
        GeneticAlgorithmInitializer(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmInitializer.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmInitializer(
            const GeneticAlgorithmInitializer& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmInitializer for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this initializer is being
         *                  constructed.
         */
        GeneticAlgorithmInitializer(
            const GeneticAlgorithmInitializer& copy,
            GeneticAlgorithm& algorithm
            );

}; // class GeneticAlgorithmInitializer


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
#include "./inline/GeneticAlgorithmInitializer.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMINITIALIZER_HPP
