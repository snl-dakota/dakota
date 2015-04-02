/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmOperator

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

        Tue May 13 13:04:06 2003 - Original Version (JE)
        Wed Dec 07 15:30:05 2005 - Made modifications necessary for replacement
                                   of ProblemDescDB with ParameterDatabase.(JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmOperator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMOPERATOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMOPERATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>







/*
================================================================================
Namespace Aliases
================================================================================
*/





/*
================================================================================
Pre Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class DesignTarget;
        class ParameterDatabase;
    }

    namespace Logging
    {
        class Logger;
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
class GeneticAlgorithm;
class GeneticAlgorithmOperator;





/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/

/// Signature of the methods required of each operator for creation.
typedef
GeneticAlgorithmOperator*
(*GeneticAlgorithmOperatorCreateFunc)(
    GeneticAlgorithm&
    );



/*
================================================================================
Class Definition
================================================================================
*/

/// The base class for all kinds of GeneticAlgorithm operators.
/**
 * This base class provides some useful capabilities needed by all operators.
 * It also requires that derived operators implement a few methods. It requires
 * no configuration input.
 */
class JEGA_SL_IEDECL GeneticAlgorithmOperator
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

        /// The GeneticAlgorithm for which this operator is used.
        GeneticAlgorithm& _algorithm;

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

        /**
         * \brief Allows mutable access to the GeneticAlgorithm know to this
         *        operator.
         *
         * \return The GeneticAlgorithm for which this is an operator.
         */
        inline
        GeneticAlgorithm&
        GetAlgorithm(
            );

        /**
         * \brief Allows immutable access to the GeneticAlgorithm know to this
         *        operator.
         *
         * \return The GeneticAlgorithm for which this is an operator.
         */
        inline
        const GeneticAlgorithm&
        GetAlgorithm(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Used to call PollForParameters.
        /**
         * This method issues a call to PollForParameters at which time derived
         * operators can ask for specific parameters by string.  This is the
         * method that should be used by whomever creates operators.
         *
         * \param db The database of parameters from which operators can
         *           extract their needed configuration information.
         * \return True if polling for parameters succeeds and false otherwise.
         */
        bool
        ExtractParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

        /**
         * \brief Allows immutable access to the DesignTarget of the algorithm
         *        for which this is an operator.
         *
         * \return The DesignTarget for Designs manipulated by this operator.
         */
        const JEGA::Utilities::DesignTarget&
        GetDesignTarget(
            ) const;

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /**
         * \brief Allows mutable access to the DesignTarget of the algorithm
         *        for which this is an operator to subclasses.
         *
         * \return The DesignTarget for Designs manipulated by this operator.
         */
        JEGA::Utilities::DesignTarget&
        GetDesignTarget(
            );

        /**
         * \brief Allows subclass access to the Logger being used by the
         *        algorithm for which this is an operator.
         *
         * The logger should not be used directly except in the JEGA_LOGGING
         * macros.  If logging from a static method, either include the
         * logger as a parameter or use the global log via the _G macros.
         *
         * \return The Logger class object that should be used by this
         *         operator.
         */
        JEGA::Logging::Logger&
        GetLogger(
            ) const;

        /**
         * \brief Returns the individual name given to the instance of an
         *        algorithm that is using this operator.
         *
         * \return The name given to the GA using this operator.
         */
        const std::string&
        GetAlgorithmName(
            ) const;

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Override to return the proper name of this operator.
        /**
         * \return The proper unique name of this operator.
         */
        virtual
        std::string
        GetName(
            ) const = 0;

        /// Override to return the name of the type of this operator.
        /**
         * Examples include Crosser, Mutator, etc.
         *
         * \return A string describing the type of this operator.
         */
        virtual
        std::string
        GetType(
            ) const = 0;

        /**
         * \brief Override to return a full description of what this operator
         *        does and how.
         *
         * \return A full description the operation of this operator.
         */
        virtual
        std::string
        GetDescription(
            ) const = 0;

        /**
         * \brief Override to return an exact duplicate of this operator for
         *        use by \a algorithm.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const = 0;

        /**
         * \brief Called when the operator is no longer needed.
         *
         * This method is called when the algorithm is done with this operator.
         * It can be assumed that the algorithm will receive no further calls
         * to the main method after this.  This typically happens just prior
         * to destruction of this operator.
         *
         * The default implementation is to do nothing.  Override to do any
         * cleanup, etc. required by this operator.
         *
         * \return true if finalization completed successfully and false
         *         otherwise.
         */
        virtual
        bool
        Finalize(
            );

    protected:

        /**
         * \brief Override to retrieve specific parameters for this operator.
         *
         * This method should be used to extract needed information for this
         * operator.  It should do so using the ParameterExtractor class
         * methods. The return value should be true if the extraction
         * completes successfully and false otherwise.
         *
         * It is pure virtual and must be overridden by any instantiable
         * derived class.
         *
         * \param db The database of parameter values from which to do
         *           retrieval.
         * \return true if polling completed successfully and false otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            ) = 0;

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

        /// Constructs a GeneticAlgorithmOperator for use with \a algorithm.
        /**
         * \param algorithm The GA for which this operator is being
         *                  constructed.
         */
        GeneticAlgorithmOperator(
            GeneticAlgorithm& algorithm
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmOperator for use in the
         *        same algorithm as \a copy.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmOperator(
            const GeneticAlgorithmOperator& copy
            );

        /// Copy constructs a GeneticAlgorithmOperator for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this operator is being
         *                  constructed.
         */
        GeneticAlgorithmOperator(
            const GeneticAlgorithmOperator& copy,
            GeneticAlgorithm& algorithm
            );

        /// Destructs a GeneticAlgorithmOperator.
        virtual
        ~GeneticAlgorithmOperator(
            );

}; // class GeneticAlgorithmOperator


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
#include "./inline/GeneticAlgorithmOperator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMOPERATOR_HPP
