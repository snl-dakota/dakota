/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MFitnessRecord.

    NOTES:

        See notes under "Document this File" section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories, Albuquerque NM

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Fri Sep 28 09:07:37 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MFitnessRecord class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTENT_MANAGED_MFITNESSRECORD_HPP
#define JEGA_FRONTENT_MANAGED_MFITNESSRECORD_HPP

#pragma once





/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>






/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Algorithms
    {
        class FitnessRecord;
        class FitnessRecord;
    }
}




/*
================================================================================
Namespace Aliases
================================================================================
*/
#pragma managed
#using <mscorlib.dll>






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
    namespace FrontEnd {
        namespace Managed {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/







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
MANAGED_CLASS(public, MFitnessRecord) :
    public System::IDisposable
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

        JEGA::Algorithms::FitnessRecord* _guts;



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
        bool
        AddFitness(
            MDesign MOH des,
            double fitness
            );

        /// Computes and returns the average fitness value in this record.
        /**
         * If there are no records, the return is -DBL_MAX
         *
         * \return The average fitness of all designs in this record or
         *         -DBL_MAX if no fitnesses are on record.
         */
        double
        GetAverageFitness(
            );

        /// Required override of this interface.
        /**
         * Implementations should return the fitness associated with
         * the supplied design or -DBL_MAX if an error occurs.
         *
         * \param des The Design to retrieve the fitness of.
         * \return The fitness recorded for des or -DBL_MAX if des is not
         *         found.
         */
        double
        GetFitness(
            MDesign MOH des
            );

        /// Required override of this interface.
        /**
         * Implementations should return the maximum fitness value in this
         * record or -DBL_MAX if there are none.
         *
         * \return The largest fitness value recorded in this record or
         *         -DBL_MAX if none.
         */
        double
        GetMaxFitness(
            );

        /// Required override of this interface.
        /**
         * Implementations should return the minimum fitness value in this
         * record or DBL_MAX if there are none.
         *
         * \return The smallest fitness value recorded in this record or
         *         DBL_MAX if none.
         */
        double
        GetMinFitness(
            );

        /// Required override of this interface.
        /**
         * Implementations should return the sum of fitness values in this
         * record or 0 if there are none.
         *
         * \return The sum of all fitness values recorded in this record or
         *         0 if none.
         */
        double
        GetTotalFitness(
            );

        /// Required override of this interface.
        /**
         * Implementations should return the number of fitness entries in this
         * record.
         *
         * \return The number of fitness values recorded in this record.
         */
        eddy::utilities::uint64_t
        GetSize(
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

        virtual
        JEGA::Algorithms::FitnessRecord&
        Manifest(
            );

        virtual
        void
        MANAGED_DISPOSE(
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

        MFitnessRecord(
            );

        MFitnessRecord(
            std::size_t initSize
            );

        /// Destructs an MFitnessRecord.
        /**
         * The body of this method calls the Dispose method to destroy the
         * JEGA core fitness record object.
         */
        ~MFitnessRecord(
            );

}; // class MFitnessRecord



/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTENT_MANAGED_MFITNESSRECORD_HPP
