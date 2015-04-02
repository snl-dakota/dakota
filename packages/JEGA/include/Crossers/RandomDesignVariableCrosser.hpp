/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RandomDesignVariableCrosser.

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

        Wed Jun 11 07:12:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RandomDesignVariableCrosser class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_RANDOMDESIGNVARIABLECROSSER_HPP
#define JEGA_ALGORITHMS_RANDOMDESIGNVARIABLECROSSER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmCrosser.hpp>









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
class RandomDesignVariableCrosser;







/*
================================================================================
Class Definition
================================================================================
*/

/// Performs crossover by random design variable selection.
/**
 * This crosser performs crossover by choosing design variables at random from
 * a prespecified number of parents enough times that the requested number of
 * children are produced.
 *
 * For example, consider the case of 3 parents producing 2 children.  This
 * operator would go through and for each design variable, select one of the
 * parents as the donor for the child.  So it creates a random shuffle of the
 * parent design variable values.
 *
 * The number of parents and number of children per mating are extracted from
 * the parameter database using the names "method.jega.num_parents" and
 * "method.jega.num_offspring" respectively.  They are both extracted as size
 * type parameters. If they are not supplied in the parameter database, the
 * default values as defined by DEFAULT_NOPPM and DEFAULT_NOCPM respectively
 * will be used.  These are required in addition to any requirements of the
 * base class.
 */
class RandomDesignVariableCrosser :
    public GeneticAlgorithmCrosser
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default number of children per mating.
        static const std::size_t DEFAULT_NOCPM;

        /// The default number of parents per mating.
        static const std::size_t DEFAULT_NOPPM;

    private:

        /// The number of children produced by each mating.
        std::size_t _nocpm;

        /// The number of parents participating in each mating.
        std::size_t _noppm;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the number of children produced with each mating to \a to.
        /**
         * \param to The new number of children per mating.
         */
        void
        SetNumChildrenPerMating(
            std::size_t to
            );

        /// Sets the number of parents used for each mating to \a to.
        /**
         * \param to The new number of parents per mating.
         */
        void
        SetNumParentsPerMating(
            std::size_t to
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the current number of children per mating.
        /**
         * \return The current number of children per mating.
         */
        inline
        std::size_t
        GetNumChildrenPerMating(
            ) const;

        /// Returns the current number of parents per mating.
        /**
         * \return The current number of parents per mating.
         */
        inline
        std::size_t
        GetNumParentsPerMating(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "shuffle_random".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This crosser randomly selects a design variable donor
            from amongs the parents and assigns that variable value
            into a child.  In this way, it creates a random shuffling
            the design variables from the parents for each child.  The
            number of mutations is the rate times the size of the group
            passed in rounded to the nearest whole number.
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
         * \param algorithm The GA for which the new crosser is to be used.
         * \return A new, default instance of a RandomDesignVariableCrosser.
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
         * \brief This is the method in which this crosser does what it does.
         *
         * This method begins the process of random design variable crossover
         * on the designs of \a from.
         *
         * \param from The group of Designs on which to perform crossover.
         * \param into The group into which to place newly created Designs.
         */
        virtual
        void
        Crossover(
            const JEGA::Utilities::DesignGroup& from,
            JEGA::Utilities::DesignGroup& into
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

        /// Constructs an RandomDesignVariableCrosser for use by \a algorithm.
        /**
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        RandomDesignVariableCrosser(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an RandomDesignVariableCrosser.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RandomDesignVariableCrosser(
            const RandomDesignVariableCrosser& copy
            );

        /**
         * \brief Copy constructs an RandomDesignVariableCrosser for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        RandomDesignVariableCrosser(
            const RandomDesignVariableCrosser& copy,
            GeneticAlgorithm& algorithm
            );

}; // class RandomDesignVariableCrosser


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
#include "./inline/RandomDesignVariableCrosser.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_RANDOMDESIGNVARIABLECROSSER_HPP
