/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class FitnessRecord.

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

        Wed Dec 21 15:49:27 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the FitnessRecord class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_FITNESSRECORD_HPP
#define JEGA_ALGORITHMS_FITNESSRECORD_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/DesignValueMap.hpp>







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
class FitnessRecord;







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
 * \brief A simple implementation of the FitnessRecord interface.
 *
 * This fitness record keeps a DesignValueMap for mapping Designs to fitness
 * values.
 */
class FitnessRecord
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        typedef
        JEGA::Utilities::DesignDoubleMap::const_iterator
        const_iterator;

    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The main data structure for this fitness record.
        /**
         * This is used to map Design*'s to fitness values.
         */
        JEGA::Utilities::DesignDoubleMap _data;

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

        inline
        void
        SuspendStatistics(
            );

        inline
        void
        ResumeStatistics(
            bool performUpdate = true
            );

        inline
        bool
        UpdatingStatistics(
            );

        inline
        void
        UpdateTotals(
            );

        /// Adds a fitness value for \a des to this record.
        /**
         * This method updates the max, min, and total fitness after inserting
         * the design/value mapping to the main data structure.
         *
         * \param des The design to which \a fitness is to be mapped.
         * \param fitness The value of the fitness to assign to \a des
         * \return True if the fitness record is added and false otherwise.
         *         Failure to add would result if \a des is already mapped to a
         *         fitness value.
         */
        inline
        bool
        AddFitness(
            const JEGA::Utilities::Design* des,
            double fitness
            );

        /// Computes and returns the average fitness value in this record.
        /**
         * If there are no records, the return is -DBL_MAX
         *
         * \return The average fitness of all designs in this record or
         *         -DBL_MAX if no fitnesses are on record.
         */
        inline
        double
        GetAverageFitness(
            ) const;

        inline
        const_iterator
        begin(
            ) const;

        inline
        const_iterator
        end(
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

        /// Required override of the FitnessRecord base class.
        /**
         * This implemenation uses the DesignValueMap base class for retrieval
         * of the fitnesses assocated with supplied designs.
         *
         * If the fitness of des cannot be determined from this record, then
         * -DBL_MAX is returned.
         *
         * \param des The Design to retrieve the fitness of.
         * \return The fitness of \a des or -DBL_MAX if \a des not found.
         */
        inline
        double
        GetFitness(
            const JEGA::Utilities::Design& des
            ) const;

        /// Required override of the FitnessRecord base class.
        /**
         * This method returns the stored max fitness value.  This is the
         * numerical maximum which would typically be the best fitness.
         *
         * \return The largest fitness value known to this fitness record.
         */
        inline
        double
        GetMaxFitness(
            ) const;

        /// Required override of the FitnessRecord base class.
        /**
         * This method returns the stored min fitness value.  This is the
         * numerical minimum which would typically be the worst fitness.
         *
         * \return The smallest fitness value known to this fitness record.
         */
        inline
        double
        GetMinFitness(
            ) const;


        /// Required override of the FitnessRecord base class.
        /**
         * This method returns the stored sum of fitnesses.
         *
         * \return The sum of all fitnesses known to this fitness record.
         */
        inline
        double
        GetTotalFitness(
            ) const;

        /// Required override of the FitnessRecord base class.
        /**
         * This method returns the number of stored fitness records.
         *
         * \return The average of all fitnesses known to this fitness record.
         */
        inline
        std::size_t
        GetSize(
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

        /// Default constructs a FitnessRecord object.
        FitnessRecord(
            std::size_t initSize = 0
            );

        /// Destructs a FitnessRecord object.
        virtual
        ~FitnessRecord(
            );



}; // class FitnessRecord



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
#include "./inline/FitnessRecord.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_FITNESSRECORD_HPP
