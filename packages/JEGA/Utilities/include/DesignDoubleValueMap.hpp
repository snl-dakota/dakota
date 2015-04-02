/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignDoubleValueMap.

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

        Tue Dec 20 12:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignDoubleValueMap class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNDOUBLEVALUEMAP_HPP
#define JEGA_UTILITIES_DESIGNDOUBLEVALUEMAP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#ifdef JEGA_HAVE_BOOST
#   include <boost/unordered_map.hpp>
#else
#   include <map>
#endif

#include <utility>
#include <utilities/include/numeric_limits.hpp>







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

template <typename VT>
class DesignDoubleValueMap;






/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/


/**
 * \brief A convenience typedef for the specialization of a double value map
 *        for real values.
 */
typedef
DesignDoubleValueMap<double>
DesignDoubleDoubleMap;


#ifdef JEGA_HAVE_BOOST
#   define MAP_BASE boost::unordered_map
#else
#   define MAP_BASE std::map
#endif



/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A map of a Design to a pair of values.
 *
 * This is intended to be used to map a Design to two numerical quantities.
 * For example, it is used by the MultiObjectiveStatistician to map a Design to
 * the number of other Designs it dominates and the number that dominate it.
 *
 * In general, VT must be an arithmetic type like double, int, etc.
 */
template <typename VT>
class JEGA_SL_IEDECL DesignDoubleValueMap :
    public MAP_BASE<const Design*, std::pair<VT, VT> >
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A synonym for the value type of this map.
        typedef
        VT
        value_type;

        /// The pair that holds the two values.
        typedef
        std::pair<value_type, value_type>
        DoubleValue;

        /// A shorthand for the base type of this value map.
        typedef
        MAP_BASE<const Design*, std::pair<VT, VT> >
        base_type;

        /// A shorthand for the numeric limits class for the value_type.
        typedef
        eddy::utilities::numeric_limits<VT>
        my_limits;


    protected:


    private:

        /// A shorthand for the type of this value map.
        typedef
        DesignDoubleValueMap<value_type>
        my_type;

    /*
    ============================================================================
    Static Constants
    ============================================================================
    */
    public:

        /// The largest value that can be handled by this value map.
        static const value_type MAX_POSSIBLE;

        /**
         * \brief The smallest (not in absolute value) value that can be
         *        handled by this map.
         */
        static const value_type MIN_POSSIBLE;



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

        /**
         * \brief Returns a pair comprised of the largest possible values for
         *        the value_type.
         *
         * \return The pair of maximal values for this value map.
         */
        static
        const DoubleValue&
        DoubleValueMax(
            );

        /**
         * \brief Returns a pair comprised of the smallest possible values for
         *        the value_type.
         *
         * \return The pair of minimal values for this value map.
         */
        static
        const DoubleValue&
        DoubleValueMin(
            );

        /**
         * \brief Returns the two values associated with the passed in Design.
         *
         * If \a of is not found in the map, the return is DoubleValueMax.
         *
         * \param of The Design for which to retrieve the stored values.
         * \return The values object associated with \a of or DoubleValueMax
         *         if \a of is not found.
         */
        inline
        const DoubleValue&
        GetValues(
            const Design* of
            ) const;

        /**
         * \brief Returns the two values associated with the passed in Design.
         *
         * If \a of is not found in the map, the return is DoubleValueMax.
         *
         * \param of The Design for which to retrieve the stored values.
         * \return The values object associated with \a of or DoubleValueMax
         *         if \a of is not found.
         */
        inline
        const DoubleValue&
        GetValues(
            const Design& of
            ) const;

        /**
         * \brief Returns the first value associated with the passed in Design.
         *
         * If \a of is not found in the map, the return is
         * MAX_POSSIBLE.
         *
         * \param of The Design for which to retrieve the stored value.
         * \return The first value associated with \a of or
         *         MAX_POSSIBLE if \a of is not found.
         */
        inline
        const value_type&
        GetFirstValue(
            const Design* of
            ) const;

        /**
         * \brief Returns the first value associated with the passed in Design.
         *
         * If \a of is not found in the map, the return is MAX_POSSIBLE.
         *
         * \param of The Design for which to retrieve the stored value.
         * \return The first value associated with \a of or
         *         MAX_POSSIBLE if \a of is not found.
         */
        inline
        const value_type&
        GetFirstValue(
            const Design& of
            ) const;

        /**
         * \brief Returns the second value associated with the passed in
         *        Design.
         *
         * If \a of is not found in the map, the return is
         * MAX_POSSIBLE.
         *
         * \param of The Design for which to retrieve the stored value.
         * \return The second value associated with \a of or
         *         MAX_POSSIBLE if \a of is not found.
         */
        inline
        const value_type&
        GetSecondValue(
            const Design* of
            ) const;

        /**
         * \brief Returns the second value associated with the passed in
         *        Design.
         *
         * If \a of is not found in the map, the return is
         * MAX_POSSIBLE.
         *
         * \param of The Design for which to retrieve the stored value.
         * \return The second value associated with \a of or
         *         MAX_POSSIBLE if \a of is not found.
         */
        inline
        const value_type&
        GetSecondValue(
            const Design& of
            ) const;

        /**
         * \brief Adds a new value record for \a of to this map.
         *
         * The initial values will be \a val1 and \a val2.
         *
         * \param of The Design for which to create a new map entry.
         * \param val1 The first value value to associate with \a of.
         * \param val2 The second value value to associate with \a of.
         * \return true if a new entry was created and false if \a of was
         *         already mapped.
         */
        inline
        bool
        AddValues(
            const Design& of,
            const value_type& val1,
            const value_type& val2
            );

        /**
         * \brief Adds a new value record for \a of to this map.
         *
         * The initial values will be \a val1 and \a val2.
         *
         * \param of The Design for which to create a new map entry.
         * \param val1 The first value value to associate with \a of.
         * \param val2 The second value value to associate with \a of.
         * \return true if a new entry was created and false if \a of was
         *         already mapped.
         */
        inline
        bool
        AddValues(
            const Design* of,
            const value_type& val1,
            const value_type& val2
            );

        /**
         * \brief Adds a new value record for \a of to this map.
         *
         * The initial values will be those contained in \a dval.
         *
         * \param of The Design for which to create a new map entry.
         * \param dval The pair of values to associate with \a of.
         * \return true if a new entry was created and false if \a of was
         *         already mapped.
         */
        inline
        bool
        AddValues(
            const Design& of,
            const DoubleValue& dval
            );

        /**
         * \brief Adds a new value record for \a of to this map.
         *
         * The initial values will be those contained in \a dval.
         *
         * \param of The Design for which to create a new map entry.
         * \param dval The pair of values to associate with \a of.
         * \return true if a new entry was created and false if \a of was
         *         already mapped.
         */
        inline
        bool
        AddValues(
            const Design* of,
            const DoubleValue& dval
            );

        /**
         * \brief Modifies by summation the values associated with \a of in the
         *        ammounts \a amt1 and \a amt2.
         *
         * \param of The Design whose values are to be modified.
         * \param amt1 The amount by which the first value should be modified.
         * \param amt2 The amount by which the second value should be modified.
         */
        void
        AddToValues(
            const Design& of,
            const value_type& amt1,
            const value_type& amt2
            );

        /**
         * \brief Modifies by summation the values associated with \a of in the
         *        ammounts \a amt1 and \a amt2.
         *
         * \param of The Design whose values are to be modified.
         * \param amt1 The amount by which the first value should be modified.
         * \param amt2 The amount by which the second value should be modified.
         */
        void
        AddToValues(
            const Design* of,
            const value_type& amt1,
            const value_type& amt2
            );

        /**
         * \brief Modifies by summation the first value associated with \a of
         *        in the ammount \a amt.
         *
         * \param of The Design whose first value is to be modified.
         * \param amt The amount by which the first value should be modified.
         */
        void
        AddToFirstValue(
            const Design& of,
            const value_type& amt
            );

        /**
         * \brief Modifies by summation the first value associated with \a of
         *        in the ammount \a amt.
         *
         * \param of The Design whose first value is to be modified.
         * \param amt The amount by which the first value should be modified.
         */
        void
        AddToFirstValue(
            const Design* of,
            const value_type& amt
            );

        /**
         * \brief Modifies by summation the second value associated with \a of
         *        in the ammount \a amt.
         *
         * \param of The Design whose second value is to be modified.
         * \param amt The amount by which the second value should be modified.
         */
        void
        AddToSecondValue(
            const Design& of,
            const value_type& amt
            );

        /**
         * \brief Modifies by summation the second value associated with \a of
         *        in the ammount \a amt.
         *
         * \param of The Design whose second value is to be modified.
         * \param amt The amount by which the second value should be modified.
         */
        void
        AddToSecondValue(
            const Design* of,
            const value_type& amt
            );

        /**
         * \brief Creates a DoubleValue object containing the passed in values.
         *
         * \param val1 The first value for the returned DoubleValue.
         * \param val2 The second value for the returned DoubleValue.
         * \return A new DoubleValue object containing \a val1 as the first
         *         and \a val2 as the second value.
         */
        static inline
        DoubleValue
        MakeDoubleValue(
            const value_type& val1,
            const value_type& val2
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
         * \brief Performs the addition of two values.
         *
         * val is passed in by reference and modified in-place.
         *
         * \param val The value to modify by \a amt.
         * \param amt The amount by which to modify \a val.
         */
        static inline
        void
        _AddToValue(
            value_type& val,
            const value_type& amt
            );


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a DesignDoubleValueMap.
        DesignDoubleValueMap(
            std::size_t initSize = 0
            );

        /// Destructs a DesignDoubleValueMap.
        virtual
        ~DesignDoubleValueMap(
            );



}; // class DesignDoubleValueMap



/*
================================================================================
End Namespace
================================================================================
*/
#undef MAP_BASE

    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/DesignDoubleValueMap.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNDOUBLEVALUEMAP_HPP
