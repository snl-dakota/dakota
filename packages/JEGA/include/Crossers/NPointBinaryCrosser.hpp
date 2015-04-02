/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NPointBinaryCrosser.

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

        Thu Jun 12 10:12:55 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NPointBinaryCrosser class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NPOINTBINARYCROSSOVER_HPP
#define JEGA_ALGORITHMS_NPOINTBINARYCROSSOVER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utility>
#include <Crossers/NPointCrosserBase.hpp>








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
class NPointBinaryCrosser;
class BitManipulator;





/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/

/// This type is used to store a variable and bit location pair.
/**
 * The first element is the variable index and the second is the bit location.
 */
typedef
std::pair<std::size_t, eddy::utilities::uint16_t>
VariableBitLocPair;








/*
================================================================================
Class Definition
================================================================================
*/


/// Performs crossover at N points in the binary genome.
/**
 * This crosser performs a bit switching crossover routine at N crossover
 * points in the binary encoded genome of two designs.
 *
 * Consider the following example with N=2:
 * \verbatim
      P1       11111 | 11111111111 | 1111111
      P2       00000 | 00000000000 | 0000000
     ----------------|-------------|-----------
      C1       11111 | 00000000000 | 1111111
      C2       00000 | 11111111111 | 0000000
   \endverbatim
 * Since there are a finite number of valid crossover points in a genome,
 * N is a desired number and may be reduced by necessity.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NPointBinaryCrosser :
    public NPointCrosserBase
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
         * \return The string "multi_point_binary".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This crosser performs n-point binary crossover.  This is a binary
            operation that takes place on the entire genome encoded as a
            inary string.

            The rate is used to determine how many members of the
            passed in group (population) should be given an oportunity to
            participate in a crossover operation.  Each operation
            involves 2 members of the passed in group and creates 2 new
            designs.  So the number of operations is round(rate*size/2)
            where size is the number of designs in the passed in group.

            The binary string is crossed in "Number of Crossover Points"
            locations (assuming enough exist) which must be at least 1.  The
            crossover points are chosen at random.  The operation takes place
            as shown below for the case where 2 crossover points are used:
                P1:   00000 | 00000 | 00000
                P2:   11111 | 11111 | 11111
                      ---------------------
                C1:   00000 | 11111 | 00000
                C2:   11111 | 00000 | 11111
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
         * \return A new, default instance of a NPointBinaryCrosser.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            GeneticAlgorithm& algorithm
            );

        /// Returns the number and location of a split variable.
        /**
         * This method returns the number of the design variable which would
         * be split by the crossover point when considering the entire genome
         * as a binary string.
         *
         * \param ofcpt The crossover point for which to determine the
         *              associated variable.
         * \param maniper The manipulator that can be used to determine how
         *                many bits each variable needs, etc.
         * \return The index of the variable and location of a bit at which to
         *         perform a crossover operation.
         */
        VariableBitLocPair
        GetSplitVariable(
            eddy::utilities::uint32_t ofcpt,
            const BitManipulator& maniper
            ) const;

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

        /// Performs N-point binary crossover.
        /**
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


        /// Constructs an NPointBinaryCrosser for use by \a algorithm.
        /**
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointBinaryCrosser(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NPointBinaryCrosser.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NPointBinaryCrosser(
            const NPointBinaryCrosser& copy
            );

        /// Copy constructs an NPointBinaryCrosser for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointBinaryCrosser(
            const NPointBinaryCrosser& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NPointBinaryCrosser


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
#include "./inline/NPointBinaryCrosser.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NPOINTBINARYCROSSOVER_HPP
