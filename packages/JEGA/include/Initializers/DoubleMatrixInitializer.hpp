/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DoubleMatrixInitializer.

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

        Wed Jul 12 12:47:50 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DoubleMatrixInitializer class.
 */






/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_DOUBLEMATRIXINITIALIZER_HPP
#define JEGA_ALGORITHMS_DOUBLEMATRIXINITIALIZER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <vector>
#include <GeneticAlgorithmInitializer.hpp>
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
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
    }
}


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
class DoubleMatrixInitializer;









/*
================================================================================
Class Definition
================================================================================
*/

/// Performs initialization by reading Design configurations from a matrix.
/**
 * This initializer attempts to read from a matrix in which the data is
 * organized as rows in which the entries are design variable values followed
 * by objective function values followed by constraint values in order.  Each
 * row constitutes a Design.  The objective function and constraint entries are
 * optional but if all are present, the Design will be considered evaluated
 * and evaluators may choose not to re-evaluate them.
 *
 * Setting the size for this initializer has the effect of requiring a minimum
 * number of Designs to create.  If this minimum number has not been created
 * once the matrix is read, the rest are created using the
 * RandomUniqueInitializer.
 *
 * The matrix of values is extracted from the parameter database using the name
 * "method.jega.design_matrix".  It is extracted as a double matrix parameter.
 * If it is not supplied in the parameter database then an warning will be
 * issued.  If no matrix is supplied by the time Initialize is called, then
 * all initialization will be done by the random unique initializer.  This is
 * required in addition to any requirements of the base class.
 */
class DoubleMatrixInitializer :
    public GeneticAlgorithmInitializer
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The matrix that contains the initial designs.
        JEGA::DoubleMatrix _designs;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /**
         * \brief Sets the current matrix from which designs will be read.
         *
         * This method will log a verbose level entry stating that the matrix
         * has been changed and indicating the size of the new matrix.
         *
         * \param designs The new matrix of design values to be used by this
         *                initializer.
         */
        void
        SetDesignMatrix(
            const JEGA::DoubleMatrix& designs
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /**
         * \brief Allows immutable access to the current matrix from which to
         *        read Designs.
         *
         * \return The current set of filenames from which to load initial
         *         designs.
         */
        inline
        const JEGA::DoubleMatrix&
        GetFileNames(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /// Returns the proper name of this operator.
        /**
         * \return The string "double_matrix".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This initializer creates JEGA Design class structures from the
            numerical values contained in a double matrix.  The values must be
            organized such that the first \"NDV\" values are the design
            variables, the next \"NOF\" values are the objective functions and
            the last \"NCN\" values are the constraints.  The objectives and
            constraints are not required but if ALL are supplied, they will be
            recorded and the resulting Design will be labeled evaluated and
            evaluators may then choose not to re-evaluate them.  Note that the
            double matrix is a vector of vectors and thus not all entries need
            to have the same length.  So it is possible to have some evaluated
            and some non-evaluated designs in the same matrix.
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
         * \param algorithm The GA for which the new initializer is to be used.
         * \return A new, default instance of a DoubleMatrixInitializer.
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

        /**
         * \brief Extracts the necessary values to create a new Design from
         *        the values in the supplied vector.
         *
         * \param into The Design class object into which to install the
         *             values read from \a from.
         * \param from The vector from which to extract values to be used
         *             as design variable values and possibly objective
         *             function and constraint values for use in \a into.
         * \return true if there were enough values in \a from to fill the
         *         design variable set and false otherwise.
         */
        static
        bool
        ReadDesignValues(
            JEGA::Utilities::Design& into,
            const DoubleVector& from
            );


    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Performs flat file read initialization.
        /**
         * This is the method called when it is time to read the files.
         * This method reads each file and places any Design successfully
         * created into \a into.
         *
         * \param into The group into which to insert any successfully created
         *             designs read from the files.
         */
        virtual
        void
        Initialize(
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


        /// Constructs an DoubleMatrixInitializer for use by \a algorithm.
        /**
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        DoubleMatrixInitializer(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an DoubleMatrixInitializer.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        DoubleMatrixInitializer(
            const DoubleMatrixInitializer& copy
            );

        /// Copy constructs an DoubleMatrixInitializer for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        DoubleMatrixInitializer(
            const DoubleMatrixInitializer& copy,
            GeneticAlgorithm& algorithm
            );

}; // class DoubleMatrixInitializer


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
#include "./inline/DoubleMatrixInitializer.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_DOUBLEMATRIXINITIALIZER_HPP
