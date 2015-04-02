/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullNichePressureApplicator.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Jan 05 14:48:43 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullNichePressureApplicator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLNICHEPRESSUREAPPLICATOR_HPP
#define JEGA_ALGORITHMS_NULLNICHEPRESSUREAPPLICATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmNichePressureApplicator.hpp>








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
class NullNichePressureApplicator;







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
 * \brief The null object of niche pressure applicators.
 *
 * This niche pressure applicator does nothing.  It merely serves as a null
 * object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullNichePressureApplicator :
    public GeneticAlgorithmNichePressureApplicator
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
         * \return The string "null_niching".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This niche pressure applicator does nothing.  It is intended to
            serve as a null object for niche pressure applicators.  Use it if
            you do not wish to apply any niche pressure.
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
         * \param algorithm The GA for which the new niche pressure applicator
         *        is to be used.
         * \return A new, default instance of a
         *         NullNichePressureApplicator.
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

        /// The method in which derived nichers do whatever they do.
        /**
         * \param population The group to which niche pressure should be
         *        applied.
         * \param fitnesses The fitness values associated with each member of
         *                  \a population in case they might be useful.
         */
        virtual
        void
        ApplyNichePressure(
            JEGA::Utilities::DesignGroup& population,
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


        /// Constructs an NullNichePressureApplicator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        NullNichePressureApplicator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NullNichePressureApplicator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NullNichePressureApplicator(
            const NullNichePressureApplicator& copy
            );

        /**
         * \brief Copy constructs an NullNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        NullNichePressureApplicator(
            const NullNichePressureApplicator& copy,
            GeneticAlgorithm& algorithm
            );


}; // class NullNichePressureApplicator



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
#include "inline/NullNichePressureApplicator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLNICHEPRESSUREAPPLICATOR_HPP
