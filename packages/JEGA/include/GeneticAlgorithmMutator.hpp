/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmMutator

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

        Wed May 21 14:40:16 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmMutator class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMMUTATOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMMUTATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <vector>
#include <GeneticAlgorithmOperator.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>







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
class GeneticAlgorithmMutator;





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

/// The base class for all GeneticAlgorithm mutators.
/**
 * A mutator is responsible for producing variation in a set of designs.  This
 * class provides a mutation rate to any derived mutator that wishes to use it.
 * It also requires implementation of the Mutate method by any derived
 * mutators.
 *
 * The mutation rate is extracted from the parameter database using the name
 * "method.mutation_rate".  It is extracted as a double. If it is not supplied
 * in the parameter database, the default value as defined by DEFAULT_RATE will
 * be used. This is required in addition to any requirements of the base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmMutator :
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

        /// The default mutation rate.
        static const double DEFAULT_RATE;

    private:

        /// The mutation rate for this mutator.
        /**
         * Its interpretation is dependent on the specific derived mutator.
         */
        double _rate;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the mutation rate for this mutator.
        /**
         * The interpretation of the rate value is dependent on the actual
         * derived mutator type.
         *
         * This method will correct a negative rate to 0 and will warn of a
         * rate greater than 100% (1.0).  It will then put out a verbose
         * level log entry informing of the new rate.
         *
         * \param rate The new mutation rate for this mutator.
         */
        void
        SetRate(
            double rate
            );


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the mutation rate for this mutator.
        /**
         * \return The value currently stored as the mutation rate for this
         *         mutator.
         */
        inline
        double
        GetRate(
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

        static
        std::vector<JEGA::Utilities::DesignDVSortSet::iterator>
        ChooseDesignsToMutate(
            std::size_t howMany,
            JEGA::Utilities::DesignGroup& from
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// This is the method in which derived mutators do whatever they do.
        /**
         * It is called by the genetic algorithm periodically.
         * Required information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * Designs present in \a pop that are to be mutated must first
         * be duplicated, then changes can be made to the duplicate's
         * design variable values.  The duplicate can then be inserted into.
         * \a cldrn.
         *
         * \param pop The population of designs to which to apply mutation.
         * \param cldrn The group of child designs to which to apply mutation.
         */
        virtual
        void
        Mutate(
            JEGA::Utilities::DesignGroup& pop,
            JEGA::Utilities::DesignGroup& cldrn
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the mutation rate which can be used by
         * derived classes in any way they wish.
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
         * \return The string "Mutator".
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


        /// Constructs a GeneticAlgorithmMutator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is being
         *                  constructed.
         */
        GeneticAlgorithmMutator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmMutator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmMutator(
            const GeneticAlgorithmMutator& copy
            );

        ///Copy constructs a GeneticAlgorithmMutator for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is being
         *                  constructed.
         */
        GeneticAlgorithmMutator(
            const GeneticAlgorithmMutator& copy,
            GeneticAlgorithm& algorithm
            );



}; // class GeneticAlgorithmMutator


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
#include "./inline/GeneticAlgorithmMutator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMMUTATOR_HPP
