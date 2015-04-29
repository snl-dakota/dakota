/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class Design.

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

        Thu May 15 11:04:05 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the Design class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGN_HPP
#define JEGA_UTILITIES_DESIGN_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cstddef>
#include <utilities/include/bit_mask.hpp>
#include <utilities/include/int_types.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>

#ifdef JEGA_THREADSAFE
#include <threads/include/mutex.hpp>
#include <threads/include/mutex_lock.hpp>
#endif

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




/*
================================================================================
Class Definition
================================================================================
*/

/// The class used to represent Design configurations.
/**
 * This class stores information about Design configurations that arise
 * during optimization runs.
 *
 * It represents a single solution including all variable,
 * objective function, and constraint values.
 */
class JEGA_SL_IEDECL Design
{
    friend class DesignTarget;

    /*
    ============================================================================
    Enumerations
    ============================================================================
    */
    public:

        /// The pre-defined attributes that any Design may have.
        /**
         * These attributes reserve the first 4 of the 8 bits of
         * \a _attributes. The remaining bits can be used at your discretion.
         * They are indexes 4, 5, 6, and 7.
         */
        enum Attributes
        {
            /// Indicates that a Design has been evaluated.
            /**
             * The variables and constraints of an evaluated design must be
             * valid and correct unless the Illconditioned attribute is also
             * set.
             */
            Evaluated = 0,

            /**
             * \brief Indicates that all the variables in the design are within
             *        bounds.
             *
             * All side constraints must be satisfied for this flag to be set.
             */
            FeasibleBounds = Evaluated + 1,

            /**
             * \brief Indicates that all non-side constraints are satisfied for
             *        this design.
             *
             * Non-side constraints are those that are defined as constraints
             * within a DesignTarget definition.
             */
            FeasibleConstraints = FeasibleBounds + 1,

            /**
             * \brief A flag for use in the case that a Design cannot be
             *        successfully evaluated.
             */
            Illconditioned = FeasibleConstraints + 1

        }; // enum Attributes

    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /**
         * \brief The mask of bits used to store the boolean style attributes
         *        of a Design class object.
         */
        typedef
        eddy::utilities::bit_mask<8>
        AttributeMask;

        typedef void (*DisposeCallback)(Design*);


    /*
    ============================================================================
    Class Members
    ============================================================================
    */
    private:

        static std::size_t NEXT_ID;

        JEGA_IF_THREADSAFE(static eddy::threads::mutex ID_MUTEX);

        static DisposeCallback DISP_CBK;

    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    public:

        /// A class to check for particular attributes of Designs.
        class AttributePred
        {
            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            private:

                /**
                 * \brief The attributes that a Design must have to pass this
                 *        predicate.
                 */
                const AttributeMask _attributes;

                /**
                 * \brief The flag that indicates whether those that have the
                 *        attributes should get flushed or those that do not.
                 */
                bool _hasOrNot;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// One of the operators that activates this predicate
                /**
                 * \param des The Design to test for the attributes known to
                 *            this predicate.
                 * \return True if \a des has all of \a _attributes and false
                 *         otherwise.
                 */
                inline
                bool
                operator ()(
                    const Design& des
                    ) const;

                /// One of the operators that activates this predicate
                /**
                 * \param des The Design to test for the attributes known to
                 *            this predicate.
                 * \return True if \a des has all of \a _attributes and false
                 *         otherwise.
                 */
                inline
                bool
                operator ()(
                    const Design* des
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Constructs an AttributePred using the supplied parameters.
                /**
                 * \param attrs The attributes that this predicate will test
                 *              for in any Designs it receives.
                 * \param hasOrNot whether those that have get flushed or those
                 *                 that do not get flushed.
                 */
                inline
                AttributePred(
                    const AttributeMask& attrs,
                    bool hasOrNot = true
                    );

                /// Constructs an AttributePred using the supplied parameters.
                /**
                 * \param attrIndex The attribute index that this predicate
                 *                  will test for in any Designs it receives.
                 * \param hasOrNot Whether those that have get flushed or those
                 *                 that do not get flushed.
                 */
                inline
                AttributePred(
                    std::size_t attrIndex,
                    bool hasOrNot = true
                    );

        }; // class AttributePred

#define DESIGN_BOOL_PRED_STRUCT(desMeth) \
        struct desMeth##Pred : std::unary_function<Design, bool> { \
            bool operator()(const Design& des) const { \
                return des.desMeth(); \
            } \
            bool operator()(const Design* des) const { \
                return des->desMeth(); \
            } \
        };

        DESIGN_BOOL_PRED_STRUCT(IsIllconditioned)
        DESIGN_BOOL_PRED_STRUCT(IsFeasible)
        DESIGN_BOOL_PRED_STRUCT(IsCloned)
        DESIGN_BOOL_PRED_STRUCT(IsEvaluated)
        DESIGN_BOOL_PRED_STRUCT(SatisfiesBounds)
        DESIGN_BOOL_PRED_STRUCT(SatisfiesConstraints)

#undef DESIGN_BOOL_PRED_STRUCT

        /**
         * \brief A class to call functions on Designs with the \a BoolFunc
         *        signature.
         *
         * The \a BoolFunc signature is \code bool (Design::*BoolFunc)(void)
         * \endcode.  Examples of such functions are \code Design::IsFeasible,
         * Design::IsEvaluated, \endcode etc.
         */
        template <typename BoolFuncPredT>
        class BoolFuncRetValPred
        {
            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            private:

                /// The method to evaluate on a Design.
                BoolFuncPredT _func;

                /**
                 * \brief The logical value expected of \a _func for a true
                 *        return of operator ().
                 */
                bool _evaluatesTo;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// One of the operators that activates this predicate
                /**
                 * \param des The Design to test to see if the BoolFunc
                 *            \a _func evaluates to \a _evaluatesTo.
                 * \return The result of this->_func(des) == _evaluatesTo.
                 */
                inline
                bool
                operator ()(
                    const Design& des
                    ) const;

                /// One of the operators that activates this predicate
                /**
                 * \param des The Design to test to see if the BoolFuncPredT
                 *            \a _func evaluates to \a _evaluatesTo.
                 * \return The result of this->_func(*des) == _evaluatesTo.
                 */
                inline
                bool
                operator ()(
                    const Design* des
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /**
                 * \brief Constructs a BoolFuncRetValPred using \a func and
                 *        looking for a return of \a evaluatesTo.
                 *
                 * \param func The function to call on each Design that comes
                 *             to this predicate.
                 * \param evaluatesTo The return of \a func that results in a
                 *                    return of true from this predicate.
                 */
                inline
                BoolFuncRetValPred(
                    const BoolFuncPredT& func,
                    bool evaluatesTo
                    );

        }; // class DesignBoolFuncPred

        struct IDHasher :
            public std::unary_function<Design*, std::size_t>
        {
            std::size_t operator()(const Design* des) const {
                return des->GetID();
            }
            std::size_t operator()(const Design& des) const {
                return des.GetID();
            }
        };

        struct IDEquals :
            public std::binary_function<Design*, Design*, std::size_t>
        {
            std::size_t operator()(
                const Design* des1, const Design* des2
                ) const
            {
                return des1->GetID() == des1->GetID();
            }

            std::size_t operator()(
                const Design& des1, const Design& des2
                ) const
            {
                return des1.GetID() == des2.GetID();
            }
        };

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        std::size_t _id;

        /// The design variable values for this Design.
        double* _variables;

        /// The objective function values for this Design.
        double* _objectives;

        /// The constraint function values for this Design.
        double* _constraints;

        /// A bitmask of attributes used to eliminate the need for many bools.
        AttributeMask _attributes;

        /// The DesignTarget for which this Design was created.
        DesignTarget& _target;

        /// The next Design in this ones clone list.
        Design* _nClone;

        /// The preceeding Design in this ones clone list.
        Design* _pClone;

        void* _tag;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the indicated objective function value.
        /**
         * It is up to you to make sure that "num" is in bounds
         * (less than nof).
         *
         * \param num The index of the objective to set.
         * \param val The new value for the objective.
         */
        inline
        void
        SetObjective(
            std::size_t num,
            double val
            );

        /// Sets the indicated constraint function value.
        /**
         * It is up to you to make sure that "num" is in bounds
         * (less than ncn).
         *
         * \param num The index of the constraint to set.
         * \param val The new value for the constraint.
         */
        inline
        void
        SetConstraint(
            std::size_t num,
            double val
            );

        /// Sets the entire set of attributes for this Design
        /**
         * This overrides any existing attributes.
         *
         * \param attr The new attributes for this Design.
         */
        inline
        void
        SetRawAttributes(
            const AttributeMask& attr
            );

        inline
        void
        SetTag(
            void* tag
            );

        inline static
        void
        SetDisposeCallback(
            DisposeCallback destCbk
            );

    protected:

        /// Sets the identity of the "next" Design in this ones clone list.
        /**
         * This does not check to see that clone isn't already in the list
         * or anything like that.  It doesn't even set the previous clone of
         * \a clone to this design.  It just sets the next clone.
         *
         * \param clone A Design that has the same variable values as this.
         */
        inline
        void
        SetNextClone(
            Design* clone
            );

        /// Sets the identity of the "previous" Design in this ones clone list.
        /**
         * This does not check to see that clone isn't already in the list
         * or anything like that.  It doesn't even set the next clone of
         * \a clone to this design.  It just sets the previous clone.
         *
         * \param clone A Design that has the same variable values as this.
         */
        inline
        void
        SetPreviousClone(
            Design* clone
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        inline
        std::size_t
        GetID(
            ) const;

        /// Retrieves the requested objective function value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than nof).
         *
         * \param num The index of the objective whose value is sought.
         * \return The objective value stored in the \a num'th location.
         */
        inline
        double
        GetObjective(
            std::size_t num
            ) const;

        /// Retrieves the requested constraint function value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ncn).
         *
         * \param num The index of the constraint whose value is sought.
         * \return The constraint value stored in the \a num'th location.
         */
        inline
        double
        GetConstraint(
            std::size_t num
            ) const;

        /// Returns the entire set of attributes for this Design
        /**
         * \return The complete mask of attributes for this Design.
         */
        inline
        const AttributeMask&
        GetRawAttributes(
            ) const;

        /// Gets the Design target being used by this Design
        /**
         * \return The DesignTarget for which this Design was created.
         */
        inline
        DesignTarget&
        GetDesignTarget(
            ) const;

        /// Retrieves the next known clone Design
        /**
         * \return The pointer to the next clone in this Designs list of
         *         clones.  May be null.
         */
        inline
        Design*
        GetNextClone(
            ) const;

        /// Retrieves the next known clone Design
        /**
         * \return The pointer to the previous clone in this Designs list of
         *         clones.  May be null.
         */
        inline
        Design*
        GetPreviousClone(
            ) const;

        inline
        void*
        GetTag(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Retrieves the requested design variable value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ndv).
         *
         * \param num The index of the variable whose value is sought.
         * \return The design variable value stored in the \a num'th location.
         */
        inline
        double
        GetVariableValue(
            std::size_t num
            ) const;

        /// Retrieves the requested design variable representation.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ndv).
         *
         * \param num The index of the variable whose representation is sought.
         * \return The double representation stored in the \a num'th location.
         */
        inline
        double
        GetVariableRep(
            std::size_t num
            ) const;

        /// Sets the indicated design variable value.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ndv).
         *
         * \param num The index of the variable to set.
         * \param val The new value for the variable.
         */
        inline
        void
        SetVariableValue(
            std::size_t num,
            double val
            );

        /// Sets the indicated design variable representation.
        /**
         * It is up to you to make sure that \a num is in bounds
         * (less than ndv).
         *
         * \param num The index of the variable to set.
         * \param rep The new representation for the variable.
         */
        inline
        void
        SetVariableRep(
            std::size_t num,
            double rep
            );

        /// Check to see if the indicated attribute is set for this Design.
        /**
         * This method is public because it is useful in checking the "extra"
         * attribute bits which are #'s 4-7 which you can use freely.
         *
         * \param attribute_index The attribute index for which to test this
         *                        design.
         * \return true if this Design has the attribute indicated by
         *         \a attribute_index and false otherwise.  The Design is
         *         considered to have the attribute if the bit at the supplied
         *         location is set to 1 and false otherwise.
         */
        inline
        bool
        HasAttribute(
            std::size_t attribute_index
            ) const;

        /// Check to see if the indicated attributes are set for this Design.
        /**
         * This method is public because it is useful in checking the "extra"
         * attribute bits which are #'s 4-7 which you can use freely.
         *
         * \param attributes The attributes for which to test this design.
         * \return True if this Design has all the attributes indicated by
         *         \a attributes and false otherwise.
         */
        inline
        bool
        HasAttributes(
            const AttributeMask& attributes
            ) const;

        /// Returns true if this Design has been evaluated.
        /**
         * \return true if this Design has the Evaluated attribute bit set and
         *         false otherwise.
         */
        inline
        bool
        IsEvaluated(
            ) const;

        /// Returns true if no constraints are violated for this Design.
        /**
         * This takes into account both bound and non-bound constraints.
         *
         * \return true if this Design has the FeasibleBounds and
         *         FeasibleConstraints attribute bits set and false otherwise.
         */
        inline
        bool
        IsFeasible(
            ) const;

        /// Returns true if no bound constraints are violated for this Design.
        /**
         * This takes into account only bound constraints.  This method may
         * return true even if IsFeasible returns false.
         *
         * \return true if this Design has the FeasibleBounds attribute bit set
         *         and false otherwise.
         */
        inline
        bool
        SatisfiesBounds(
            ) const;

        /**
         * \brief Returns true if no non-bound constraints are violated for
         *        this Design.
         *
         * This takes into account only non-bound constraints.  This method may
         * return true even if IsFeasible returns false.
         *
         * \return true if this Design has the FeasibleConstraints attribute
         *         bit set and false otherwise.
         */
        inline
        bool
        SatisfiesConstraints(
            ) const;

        /// Returns true if evaluation failed for this Design.
        /**
         * \return true if this Design has the Illconditioned attribute
         *         bit set and false otherwise.
         */
        inline
        bool
        IsIllconditioned(
            ) const;

        /// Returns true if \a right has the same configuration as this.
        /**
         * To have the same configuration implies only that all the
         * Design Variable values compare equal.  Nothing else.
         *
         * \param right The Design to test for logical equivalence to this.
         * \return true if \a right has the same variable values and false
         *         otherwise.
         */
        bool
        operator == (
            const Design& right
            );

        /// Returns the number of design variables.
        /**
         * This information is retrieved from the known DesignTarget.
         *
         * \return The number of design variables for the current problem.
         */
        inline
        std::size_t
        GetNDV(
            ) const;

        /// Returns the number of objective functions.
        /**
         * This information is retrieved from the known DesignTarget.
         *
         * \return The number of objective functions for the current problem.
         */
        inline
        std::size_t
        GetNOF(
            ) const;

        /// Returns the number of constraints.
        /**
         * This information is retrieved from the known DesignTarget.
         *
         * \return The number of constraints for the current problem.
         */
        inline
        std::size_t
        GetNCN(
            ) const;

        /// Returns true if this Design knows of a duplicate of itself.
        /**
         * \return True if either of the previous or next clone is non-null.
         */
        inline
        bool
        IsCloned(
            ) const;

        /**
         * \brief Combines des1's and des2's clone lists if they are not
         *        already the same.
         *
         * Return is the number of newly found clones (0, 1, or 2).  That is,
         * for each of des1 and des2 that was not previously known to be a
         * clone of any other Design, the return value is incremented.
         *
         * \param des1 The first Design to be tagged as a clone.
         * \param des2 The second Design to be tagged as a clone.
         * \return The number of newly found clone Designs.
         */
        static
        eddy::utilities::uint8_t
        TagAsClones(
            Design& des1,
            Design& des2
            );

        /// Removes this Design from the clone list it is currently in.
        void
        RemoveAsClone(
            );

        /// Checks to see if "des" is a member of this' clone list.
        /**
         * \param des The Design to look for in this Design's entire list of
         *            clones.
         * \return true if yes, false if no.
         */
        bool
        HasInCloneList(
            const Design& des
            ) const;

        /// Counts the number of Designs known to be a clone of this Design.
        /**
         * \return The number of Designs that can be found by following
         *         the paths of the previous and next clones.
         */
        eddy::utilities::uint64_t
        CountClones(
            ) const;

        /**
         * \brief Set the indicated attribute for this Design to on or off
         *        (true or false).
         *
         * To be on is to have a value of 1 in the corresponding bit location
         * and to be off is to have a 0.
         *
         * This method is public because it is useful in setting the "extra"
         * attribute bits #'s 4-7 which you can use freely.
         *
         * \param attribute_index The index of the attribute to turn either on
         *        or off.
         * \param on true to turn the attribute on and false to turn it off.
         */
        inline
        void
        ModifyAttribute(
            std::size_t attribute_index,
            bool on
            );

        /**
         * \brief Set the indicated attributes for this Design to on or off
         *        (true or false).
         *
         * To be on is to have a value of 1 in the corresponding bit location
         * and to be off is to have a 0.
         *
         * This method is public because it is useful in setting the "extra"
         * attribute bits #'s 4-7 which you can use freely.
         *
         * \param attributes The attributes to turn either on or off.
         * \param on true to turn the attributes on and false to turn them off.
         */
        inline
        void
        ModifyAttributes(
            const AttributeMask& attributes,
            bool on
            );

        /// Marks this Design as having been evaluated.
        /**
         * \param lval True to set the Evaluated bit and false to unset it.
         */
        inline
        void
        SetEvaluated(
            bool lval
            );

        /// Marks this Design as satisfying or not all design variable bounds.
        /**
         * This is only useful to indicate that a Design does or does not
         * satisfy all bound constraints.  This says nothing about non-bound
         * constraints.
         *
         * \param lval True to set the FeasibleBounds bit and false to unset
         *             it.
         */
        inline
        void
        SetSatisfiesBounds(
            bool lval
            );

        /// Marks this Design as satisfying or not all non-bound constraints.
        /**
         * This is only useful to indicate that a Design does or does not
         * satisfy all non-bound constraints.  This says nothing about
         * bound constraints.
         *
         * \param lval True to set the FeasibleConstraints bit and false to
         *             unset it.
         */
        inline
        void
        SetSatisfiesConstraints(
            bool lval
            );

        /// Marks this Design as being illconditioned.
        /**
         * \param lval True to set the Illconditioned bit and false to
         *             unset it.
         */
        inline
        void
        SetIllconditioned(
            bool lval
            );

        /// Assigns this Design to be identical to "right".
        /**
         * This includes adoption of the variable, objective, and constraint
         * values as well as the attributes.  "this" and "right" are then
         * tagged as clones of one-another.
         *
         * \param right The Design to assign properties from into this.
         * \return This after properties have been adopted from "right".
         */
        const Design&
        operator = (
            const Design& right
            );

        /// Sets the attributes back to the default values.
        inline
        void
        ResetAttributes(
            );

        void
        CopyResponses(
            const Design& from
            );

        void
        Dispose(
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


    protected:


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Allocates memory for design variables, objective fucntions,
         *        and constraints.
         */
        void
        DoAllocation(
            );

        inline
        std::size_t
        ResetID(
            );

        inline static
        std::size_t
        NextID(
            );


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs and properly allocates a Design for use on "target"
        /**
         * \param target The DesignTarget for which this Design is being
         *               created.
         */
        Design(
            DesignTarget& target
            );

        /// Copy constructs a Design.
        /**
         * \param copy The existing Design from which to copy properties into
         *             this.
         */
        Design(
            const Design& copy
            );

        /// Destructs a Design.
        /**
         * This causes release of all dynamically obtained resources.
         */
        virtual
        ~Design(
            );

}; // class Design


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/Design.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGN_HPP
