/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullCrosser.

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

        Thu May 29 09:26:06 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullCrosser class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLCROSSER_HPP
#define JEGA_ALGORITHMS_NULLCROSSER_HPP







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
class NullCrosser;







/*
================================================================================
Class Definition
================================================================================
*/


/// The null object of crossers.
/**
 * This crosser does nothing.  It merely serves as a null object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullCrosser :
    public GeneticAlgorithmCrosser
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
         * \return The string "null_crossover".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This crosser does nothing.  It is intended to serve as
            a null object for crossers.  Use it if you do not wish
            to perform any crossover.
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
         * \return A new, default instance of a NullCrosser.
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

        /// This version of Crossover does absolutely nothing.
        /**
         * Nothing except logging a debug level message that it is being used.
         *
         * \param from The group of candidate designs for crossover.
         * \param into The group into which to place the newly created
         *             offspring designs.
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

        /// Constructs an NullCrosser for use by \a algorithm.
        /**
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NullCrosser(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NullCrosser.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NullCrosser(
            const NullCrosser& copy
            );

        /// Copy constructs an NullCrosser for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NullCrosser(
            const NullCrosser& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NullCrosser


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
#include "./inline/NullCrosser.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLCROSSER_HPP
