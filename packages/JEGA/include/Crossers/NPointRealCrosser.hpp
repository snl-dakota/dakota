/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NPointRealCrosser.

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

        Thu Jun 12 10:12:43 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NPointRealCrosser class.
 */






/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NPOINTREALCROSSOVER_HPP
#define JEGA_ALGORITHMS_NPOINTREALCROSSOVER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

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
class NPointRealCrosser;







/*
================================================================================
Class Definition
================================================================================
*/

/// Performs crossover at N points in the real encoded genome.
/**
 * This crosser performs a variable switching crossover routine at N crossover
 * points in the real encoded genome of two designs.
 *
 * Consider the following example with N=2:
 * \verbatim
      P1       X1 X2 X2 | X3 X4 | X5 X6 X7
      P2       Y0 Y1 Y2 | Y3 Y4 | Y5 Y6 Y7
     -------------------|-------|----------
      C1       X1 X2 X2 | Y3 Y4 | X5 X6 X7
      C2       Y0 Y1 Y2 | X3 X4 | Y5 Y6 Y7
   \endverbatim
 * Since there are a finite number of valid crossover points in a genome,
 * N is a desired number and may be reduced by necessity.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NPointRealCrosser :
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
         * \return The string "multi_point_real".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This crosser performs n-point real crossover.  This is an
            operation that takes place on the entire genome encoded as a
            set of real numbers.  Each design variable serves as a real
            number in the genome

            The rate is used to determine how many members of the
            passed in group (population) should be given an oportunity to
            participate in a crossover operation.  Each operation
            involves 2 members of the passed in group and creates 2 new
            designs.  So the number of operations is round(rate*size/2)
            where size is the number of designs in the passed in group.

            The genome is crossed in "Number of Crossover Points"
            locations (assuming enough exist) which must be at least 1.  The
            crossover points are chosen at random.  The operation takes place
            as shown below for the case where 2 crossover points are used:
                P1:   DV1 DV2 | DV3 DV4 DV5 | DV6 DV7 DV8
                P2:   dv1 dv2 | dv3 dv4 dv5 | dv6 dv7 dv8
                      -----------------------------------
                C1:   DV1 DV2 | dv3 dv4 dv5 | DV6 DV7 DV8
                C2:   dv1 dv2 | DV3 DV4 DV5 | dv6 dv7 dv8
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
         * \return A new, default instance of a NPointRealCrosser.
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

        /// Performs N-point real crossover.
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

        /// Constructs an NPointRealCrosser for use by \a algorithm.
        /**
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointRealCrosser(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NPointRealCrosser.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NPointRealCrosser(
            const NPointRealCrosser& copy
            );

        /// Copy constructs an NPointRealCrosser for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointRealCrosser(
            const NPointRealCrosser& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NPointRealCrosser


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
#include "./inline/NPointRealCrosser.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NPOINTREALCROSSOVER_HPP
