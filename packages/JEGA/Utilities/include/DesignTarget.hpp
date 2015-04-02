/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignTarget.

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

        Tue Dec 20 08:11:48 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignTarget class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNTARGET_HPP
#define JEGA_UTILITIES_DESIGNTARGET_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <vector>







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
    namespace Utilities {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class Design;
class DesignTarget;
class RegionOfSpace;
class ConstraintInfo;
class DesignDVSortSet;
class DesignVariableInfo;
class ObjectiveFunctionInfo;







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/

/**
 * \brief A container for all the information objects describing a set of
 *        Design varaibles.
 */
typedef
std::vector<DesignVariableInfo*>
DesignVariableInfoVector;

/**
 * \brief A container for all the information objects describing a set of
 *        Constraints.
 */
typedef
std::vector<ConstraintInfo*>
ConstraintInfoVector;

/**
 * \brief A container for all the information objects describing a set of
 *        objective functions.
 */
typedef
std::vector<ObjectiveFunctionInfo*>
ObjectiveFunctionInfoVector;







/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A basic implementation of the DesignTarget interface.
 *
 * This implementation is thread safe if JEGA_THREADSAFE is defined.
 */
class JEGA_SL_IEDECL DesignTarget
{
    /*
    ============================================================================
    Nested Utility Class Forward Declares
    ============================================================================
    */
    private:

        /// A class housing all mutexes used by the DesignTarget.
        /**
         * This gets removed if JEGA_THREADSAFE is not defined.
         */
        JEGA_IF_THREADSAFE(class Mutexes;)

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
    Class Data Declarations
    ============================================================================
    */
    public:

        static const std::size_t DEFAULT_MAX_GUFF_SIZE;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// Collection of discarded Design's
        DesignDVSortSet* _discards;

        /// The information objects describing the Design variables.
        DesignVariableInfoVector _dvInfos;

        /// The information objects describing the objective functions.
        ObjectiveFunctionInfoVector _ofInfos;

        /// The information objects describing the constraints.
        ConstraintInfoVector _cnInfos;

        std::vector<Design*> _guff;

        std::size_t _maxGuffSize;

        bool _trackDiscards;

        /// An instance of a set of mutexes used if threadsafe is on.
        JEGA_IF_THREADSAFE(Mutexes* _mutexes;)

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        void
        SetTrackDiscards(
            bool track
            );

        void
        SetMaxGuffSize(
            std::size_t mgs
            );



    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        inline
        bool
        GetTrackDiscards(
            ) const;

        inline
        std::size_t
        GetMaxGuffSize(
            ) const;


        /// Gets the vector of DesignVariableInfo's for this problem.
        /**
         * \return The vector of all design variable information objects.
         */
        inline
        const DesignVariableInfoVector&
        GetDesignVariableInfos(
            ) const;

        /// Gets the vector of ObjectiveFunctionInfo's for this problem.
        /**
         * \return The vector of all objective function information objects.
         */
        inline
        const ObjectiveFunctionInfoVector&
        GetObjectiveFunctionInfos(
            ) const;

        /// Gets the vector of DesignVariableInfo's for this problem.
        /**
         * \return The vector of all constraint information objects.
         */
        inline
        const ConstraintInfoVector&
        GetConstraintInfos(
            ) const;

    protected:


    private:


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

        /// Gets the discarded Design's stored in this (const).
        /**
         * This method uses mutex protection to protect this set from
         * multi-thread corruption.  A lock is placed on the associated mutex.
         * Therefore, in order to release the lock, a call to CheckinDiscards
         * must be paired with each call to this method.
         *
         * \return The set of Designs passed to this Target by way of the
         *         TakeDesign* methods.
         */
        const DesignDVSortSet&
        CheckoutDiscards(
            ) const;

        /**
         * \brief Ends protection of the discards installed at a call to
         *        CheckoutDiscards.
         *
         * The collection of discarded designs is mutex protected.  The only
         * way for an external entity to access that collection is by using
         * the CheckoutDiscards method.  That method locks a mutex.
         * In order to release the lock, a call to this method must be paired
         * with each call to CheckoutDiscards.  If thread safety is not
         * compiled in, this method does nothing.
         */
        void
        CheckinDiscards(
            ) const;

        /// Returns a Design* properly allocated for this target
        /**
         * \return A new Design object properly allocated for this target.
         */
        virtual
        Design*
        GetNewDesign(
            ) const;

        /**
         * \brief Returns a Design* properly allocated for this target
         * which is a duplicate of \a copy.
         *
         * \param copy An existing Design to duplicate.
         * \return A new Design copy constructed from \a copy.
         */
        virtual
        Design*
        GetNewDesign(
            const Design& copy
            ) const;

        /// Returns the number of Design Variables for this problem
        /**
         * \return The number of design variables in the problem represented by
         *         this DesignTarget.
         */
        inline
        std::size_t
        GetNDV(
            ) const;

        /// Returns the number of Objective Functions for this problem
        /**
         * \return The number of objective functions in the problem represented
         *         by this DesignTarget.
         */
        inline
        std::size_t
        GetNOF(
            ) const;

        /// Returns the number of Constraints for this problem
        /**
         * \return The number of constraints in the problem represented by
         *         this DesignTarget.
         */
        inline
        std::size_t
        GetNCN(
            ) const;

        /// Does whatever is necessary to determine the feasibility of \a des.
        /**
         * A design is considered feasible if all constraints (including side
         * constraints) are satisfied.  This generally requires checking for
         * bound and regular constraint violations and tagging the Design
         * object with appropriate attributes.
         *
         * \param des The Design of which to test the feasibility.
         * \return true if \a des is feasible and false otherwise.
         */
        bool
        CheckFeasibility(
            Design& des
            ) const;

        /// Returns true if all variable values in \a des are in bounds.
        /**
         * \param des The Design of which to test compilance with the side
         *            constraints.
         * \return true if \a des satisfies all side constraints and false
         *         otherwise.
         */
        bool
        CheckSideConstraints(
            Design& des
            ) const;

        /// Returns true if all non-side constraints are satisfied by des".
        /**
         * \param des The Design of which to test compilance with the
         *            functional constraints of the problem.
         * \return true if \a des satisfies all non-side constraints and false
         *         otherwise.
         */
        bool
        CheckNonSideConstraints(
            Design& des
            ) const;

        /// This method should be used to discard a no-longer-wanted Design.
        /**
         * The target assumes that if you call this method, you are completely
         * done with the Design.  It may choose to destroy it and so if you try
         * to use it again, crash.
         *
         * \param des The no longer wanted Design.
         */
        void
        TakeDesign(
            Design* des
            );

        /// This method should be used to discard no-longer-wanted Designs.
        /**
         * The target assumes that if you call this method, you are completely
         * done with these Designs.  It may choose to destroy them and so if
         * you try to use them again, crash.
         *
         * DesCont This type must adhere to STL conventions for a simple
         * forward iteratable container including support for const_iterators,
         * begin(), and end().
         *
         * \param cont The container of no longer wanted Designs.
         */
        template <typename DesCont>
        inline
        void
        TakeDesigns(
            const DesCont& cont
            );

        /// This method should be used to re-claim what was an unwanted Design.
        /**
         * Since the target stores discards under certain circumstances, it is
         * possible to get them back.  It is only safe to call this for designs
         * that you have discovered by iterating the discards group after a
         * checkout and prior to a checkin.
         *
         * \param des The now again wanted Design.
         * \return True if the design was in fact a stored discard and no longer
         *         is and false otherwise.
         */
        bool
        ReclaimDesign(
            const Design& des
            );

        /**
         * \brief Records all constraint violations of \a des with
         *        the corresponding info objects.
         *
         * \param des The design whose constraint violations are to be
         *               recorded.
         */
        void
        RecordAllConstraintViolations(
            const Design& des
            ) const;

        /// Inserts "info" into the list of DesignVariableInfo's.
        /**
         * \param info The new design variable information object to add to
         *             this target.
         *
         * \return True if \a info is successfully added and false othewise.
         *      The only current fail conditions are:
         *          - a null "info".
         *          - an "info" built for a different target.
         */
        bool
        AddDesignVariableInfo(
            DesignVariableInfo& info
            );

        /// Inserts "info" into the list of ConstraintInfo's.
        /**
         * \param info The new constraint information object to add to
         *             this target.
         *
         * \return True if \a info is successfully added and false othewise.
         *      The only current fail conditions are:
         *          - a null "info".
         *          - an "info" built for a different target.
         */
        bool
        AddConstraintInfo(
            ConstraintInfo& info
            );

        /// Inserts "info" into the list of ObjectiveFunctionInfo's.
        /**
         * \param info The new objective information object to add to this
         *             target.
         *
         * \return True if \a info is successfully added and false othewise.
         *      The only current fail conditions are:
         *          - a null "info".
         *          - an "info" built for a different target.
         */
        bool
        AddObjectiveFunctionInfo(
            ObjectiveFunctionInfo& info
            );

        /**
         * \brief Returns the complete design space represented as a
         *        RegionOfSpace object.
         *
         * The limits put into the region of space are based on the variable
         * representations, not the values.
         *
         * \return The complete design space represented as a RegionOfSpace
         *         object.
         */
        RegionOfSpace
        GetDesignSpace(
            ) const;


    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        void
        FlushTheGuff(
            );




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        /// Constructs a DesignTarget object.
        DesignTarget(
            );

        /// Destructs a DesignTarget object.
        virtual
        ~DesignTarget(
            );



}; // class DesignTarget



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/DesignTarget.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNTARGET_HPP
