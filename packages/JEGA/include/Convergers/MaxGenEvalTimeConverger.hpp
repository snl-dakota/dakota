/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MaxGenEvalTimeConverger.

    NOTES:

        See notes under "Document this File" section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.7.0

    CHANGES:

        Thu Sep 18 13:43:57 2014 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MaxGenEvalTimeConverger class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_MAXGENEVALTIMECONVERGER_HPP
#define JEGA_UTILITIES_MAXGENEVALTIMECONVERGER_HPP

#pragma once





/*
================================================================================
Includes
================================================================================
*/
#include <GeneticAlgorithmConverger.hpp>








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
class MaxGenEvalTimeConverger;







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
/**
 * \brief
 *
 *
 */
class MaxGenEvalTimeConverger :
    public GeneticAlgorithmConverger
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





    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "max_evals_gens_time".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This converger returns true if either the maximum allowable
            number of evaluations, maximum allowable number of
            generations, or maximum wall clock time has been reached or
            exceeded.
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
         * \param algorithm The GA for which the new converger is to be used.
         * \return A new, default instance of a MaxGenEvalTimeConverger.
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

        /// Tests for convergence
        /**
         * This method checks to see if either the maximum number of
         * generations, evaluations, or time has been reached (or exceeded) and
         * returns true if so.
         *
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return true if convergence has been achieved and false otherwise.
         */
        virtual
        bool
        CheckConvergence(
            const JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
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


    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


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
         * \brief Constructs a MaxGenEvalTimeConverger for use by \a
         *         algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         */
        MaxGenEvalTimeConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a MaxGenEvalTimeConverger.
        /**
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         */
        MaxGenEvalTimeConverger(
            const MaxGenEvalTimeConverger& copy
            );

        /**
         * \brief Copy constructs a MaxGenEvalTimeConverger for use by
         *        \a algorithm.
         *
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         * \param algorithm The GA for which the new converger is to be used.
         */
        MaxGenEvalTimeConverger(
            const MaxGenEvalTimeConverger& copy,
            GeneticAlgorithm& algorithm
            );

}; // class MaxGenEvalTimeConverger



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "./inline/MaxGenEvalTimeConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_MAXGENEVALTIMECONVERGER_HPP
