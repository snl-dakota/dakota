/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class OffsetMutatorBase.

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

        Thu Jul 10 07:56:01 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the OffsetMutatorBase class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_OFFSETMUTATORBASE_HPP
#define JEGA_ALGORITHMS_OFFSETMUTATORBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmMutator.hpp>






/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class DesignVariableInfo;
    }
}


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
class OffsetMutatorBase;






/*
================================================================================
Class Definition
================================================================================
*/

/// Base class for offset mutators.
/**
 * This is a base class for any mutator that wishes to modify design variables
 * by adding random ammounts to them.  This base class takes care of most of
 * the work.  All that is required of derived classes is that they provide
 * offset amounts through an overridden method call.
 *
 * The offset range is extracted from the parameter database using the name
 * "method.mutation_scale".  It is extracted as a double value.  If it is not
 * supplied in the parameter database, the default value as defined by
 * DEFAULT_OFFSET_RANGE will be used.  This is required in addition to any
 * requirements of the base class.
 */
class OffsetMutatorBase :
    public GeneticAlgorithmMutator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default value for the offset range.
        static const double DEFAULT_OFFSET_RANGE;

    private:

        /// The portion of the range used for offsetting.
        /**
         * This quantity is a fraction in the range [0, 1] and is meant to
         * help control the amount of variation that takes place when a
         * variable is mutated.  The exact interpretation is implementation
         * dependent. See the derived operators for details on its
         * interpretation.
         */
        double _offsetRange;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the current offset range to \a val.
        /**
         * \param val The new offset range for this offset mutator.
         */
        void
        SetOffsetRange(
            double val
            );



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the current value of the offset range.
        /**
         * \return The current offset range of this offset mutator.
         */
        inline
        double
        GetOffsetRange(
            ) const;




    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:





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

        /**
         * \brief Override this method to provide the offset amount for a
         *        particular variable.
         *
         * \param varInfo The design variable info of the design for which an
         *                offset amount is sought.
         * \return The amount by which the design variable value should be
         *         altered.
         */
        virtual
        double
        GetOffsetAmount(
            const JEGA::Utilities::DesignVariableInfo& varInfo
            ) const = 0;

        /// Performs random offset mutation.
        /**
         * This mutator only mutates members of the population but places the
         * resulting mutated designs into the children.
         *
         * \param pop The population of designs to which to apply mutation.
         * \param cldrn The group of child designs to which to apply mutation.
         */
        virtual
        void
        Mutate(
            JEGA::Utilities::DesignGroup& pop,
            JEGA::Utilities::DesignGroup& cldrn
            );

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


        /// Constructs an OffsetMutatorBase for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is
         *                  being constructed.
         */
        OffsetMutatorBase(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an OffsetMutatorBase.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        OffsetMutatorBase(
            const OffsetMutatorBase& copy
            );

        /**
         * \brief Copy constructs an OffsetMutatorBase for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is
         *                  being constructed.
         */
        OffsetMutatorBase(
            const OffsetMutatorBase& copy,
            GeneticAlgorithm& algorithm
            );

}; // class OffsetMutatorBase


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
#include "./inline/OffsetMutatorBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_OFFSETMUTATORBASE_HPP
