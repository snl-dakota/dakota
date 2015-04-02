/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NPointParameterizedBinaryCrosser.

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

        Thu Jun 12 10:13:08 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NPointParameterizedBinaryCrosser
 *        class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NPOINTPARAMETERIZEDBINARYCROSSOVER_HPP
#define JEGA_ALGORITHMS_NPOINTPARAMETERIZEDBINARYCROSSOVER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Crossers/NPointCrosserBase.hpp>
#include <../Utilities/include/JEGATypes.hpp>









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
class NPointParameterizedBinaryCrosser;







/*
================================================================================
Class Definition
================================================================================
*/


/// Performs crossover at N points in the binary cromosomes (dv by dv).
/**
 * This crosser performs a bit switching crossover routine at N crossover
 * points between the binary encoded chromosomes (not genomes) of two designs.
 *
 * Consider the following example with N=2 for all variables:
 * \verbatim
      P1-dv1       11111 | 11111111111 | 1111111
      P2-dv1       00000 | 00000000000 | 0000000
     --------------------|-------------|--------
      C1-dv1       11111 | 00000000000 | 1111111
      C2-dv1       00000 | 11111111111 | 0000000
   \endverbatim
 * Since there are a finite number of valid crossover points in a chromosome,
 * N is a desired number and may be reduced by necessity for any given
 * variable.
 *
 * In short, this performs a typical crossover routine on each design variable
 * of two designs individually, one at a time.
 *
 * The vector of the number of crossover points for each variable (N) is
 * extracted from the parameter database using the name
 * "method.jega.num_cross_points".  It is a vector of integers.  If it is not
 * supplied in the parameter database, a vector of default values as defined by
 * DEFAULT_NUM_CROSS_PTS in the base class is used (1 for each variable).
 * This is required in addition to any requirements of the base class.
 */
class NPointParameterizedBinaryCrosser :
    public NPointCrosserBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /**
         * \brief The vector of the number of crossover points to use for each
         *        variable.
         *
         * This vector will be NDV elements in size.  The first entry will be
         * the number of crossover points to use when crossing the first
         * variable.  The second will be the number of points to use when
         * crossing the second variable, etc.
         */
        JEGA::IntVector _numCrossPts;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /**
         * \brief Allows mutation of the vector of numbers of crossover points.
         *
         * This does sanity checking and possible correction and logs messages
         * about its actions.
         *
         * \param ncpts The new vector of numbers of crossover points.
         */
        void
        SetNumCrossPoints(
            const JEGA::IntVector& ncpts
            );

        /// Sets the number of crossover points for all variables to \a pct.
        /**
         * This will set all number of crossover points for all variables to
         * the same supplied value.
         *
         * An informative log entry will be issued at the verbose level for
         * each variable.
         *
         * \param ncpts The new number of crossover points for all variables.
         */
        void
        SetNumCrossPoints(
            eddy::utilities::int32_t ncpts
            );

        /// Sets the number of crossover points used for the \a dv'th variable.
        /**
         * This method will log a verbose level message informing of the new
         * value.  It also enforces a minimum value of 1.  No maximum is
         * enforced at this point.  That information is checked at crossover
         * time.
         *
         * \param dv The index of the variable for which the number of
         *           crossover points is being supplied.
         * \param ncpts The new number of crossover points for variable #\a dv.
         */
        void
        SetNumCrossPoints(
            std::size_t dv,
            eddy::utilities::int32_t ncpts
            );


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /**
         * \brief Allows access to the vector of numbers of crossover points.
         *
         * \return The vector of numbers of crossover points to use when
         *         crossing the different variables.
         */
        inline
        JEGA::IntVector
        GetNumCrossPoints(
            ) const;



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "multi_point_parameterized_binary".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
           This crosser performs n-point parameterized binary crossover.
           This is a binary operation that takes place on each
           variable individually.  Each variable has its own desired number of
           crossover locations.

           The rate is used to determine how many members of the
           passed in group (population) should be given an oportunity to
           participate in a crossover operation.  Each operation
           involves 2 members of the passed in group and creates 2 new
           designs.  So the number of operations is round(rate*size/2)
           where size is the number of designs in the passed in group.
           Each variable of the parent designs is crossed independently
           of each other.

           To carry this out, each variable is converted into a binary
           representation using a BitManipulator.  The binary strings are
           then crossed in the appropriate number of locations
           (assuming enough exist) which must be at least 1.  The
           crossover points are chosen at random.  The operation takes place
           as shown below for the case where 1 crossover point is used for each
           variable:

                      dv1     dv2     dv3
               P1:   000|00  00|000  0000|0
               P2:   111|11  11|111  1111|1
                     ------  ------  ------   ....
               C1:   000|11  00|111  0000|1
               C2:   111|00  11|000  1111|0

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
         * \return A new, default instance of a
         *         NPointParameterizedBinaryCrosser.
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

        /// Performs N-point parameterized binary crossover.
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

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the number of crossover points for each design
         * variable.  Derived crossers may choose to use these values in any
         * way they wish.  This crosser ignores the base class value for
         * crossover points.
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
         * \brief Constructs an NPointParameterizedBinaryCrosser for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointParameterizedBinaryCrosser(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NPointParameterizedBinaryCrosser.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NPointParameterizedBinaryCrosser(
            const NPointParameterizedBinaryCrosser& copy
            );

        /**
         * \brief Copy constructs an NPointParameterizedBinaryCrosser for use
         *        by \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointParameterizedBinaryCrosser(
            const NPointParameterizedBinaryCrosser& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NPointParameterizedBinaryCrosser


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
#include "./inline/NPointParameterizedBinaryCrosser.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NPOINTPARAMETERIZEDBINARYCROSSOVER_HPP
