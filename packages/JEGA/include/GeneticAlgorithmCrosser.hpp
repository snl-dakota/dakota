/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmCrosser

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

        Tue May 20 13:32:42 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmCrosser class.
 */






/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMCROSSER_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMCROSSER_HPP







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
class GeneticAlgorithmCrosser;




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

/// The base class for all GeneticAlgorithm crossers.
/**
 * The crosser is responsible for performing crossover (re-combination) on the
 * Designs passed into it.  It should store the resulting offspring in the
 * passed in group for them.
 *
 * This base class provides a crossover rate to any derivative that needs it
 * and requires of it's derivatives implementation of the Crossover method.
 *
 * The crossover rate is extracted from the parameter database using the name
 * "method.crossover_rate".  It is extracted as a double value. If it is not
 * supplied in the parameter database, the default value as defined by
 * DEFAULT_RATE will be used. This is required in addition to any requirements
 * of the base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmCrosser :
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

        /// The default crossover rate.
        static const double DEFAULT_RATE;

    private:

        ///  The crossover rate for this crosser.
        /**
         * Its interpretation is specific to the specific crosser instantiated.
         */
        double _rate;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the crossover rate for this crosser.
        /**
         * This method checks the supplied rate and provides output to the user
         * of any problems and progress.
         *
         * \param rate The new rate for this crosser to use.
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

        /// Gets the crossover rate for this crosser.
        /**
         * \return The value currently being used as the crossover rate.
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





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /**
         * \brief This is the method in which derived crossers should do
         *        whatever they do.
         *
         * It is called by the genetic algorithm periodically.  Required
         * information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * Designs from \a from should undergo re-combination and the resulting
         * Designs should be placed into \a into.
         *
         * \param from The Designs to involve in crossover operations.
         * \param into The group into which new Designs should be placed.
         */
        virtual
        void
        Crossover(
            const JEGA::Utilities::DesignGroup& from,
            JEGA::Utilities::DesignGroup& into
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class of methods from
         * the GeneticAlgorithmOperator base class.
         *
         * This version retrieves the crossover rate which can be used by
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
         * \return The string "Crosser".
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

        /// Constructs a GeneticAlgorithmCrosser for use by \a algorithm.
        /**
         * \param algorithm The GA for which this crosser is being constructed.
         */
        GeneticAlgorithmCrosser(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmCrosser.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmCrosser(
            const GeneticAlgorithmCrosser& copy
            );

        /// Copy constructs a GeneticAlgorithmCrosser for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being constructed.
         */
        GeneticAlgorithmCrosser(
            const GeneticAlgorithmCrosser& copy,
            GeneticAlgorithm& algorithm
            );

}; // class GeneticAlgorithmCrosser


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
#include "./inline/GeneticAlgorithmCrosser.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMCROSSER_HPP
