/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignValueMap.

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
 * \brief Contains the definition of the DesignValueMap class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNVALUEMAP_HPP
#define JEGA_UTILITIES_DESIGNVALUEMAP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#ifdef JEGA_HAVE_BOOST
# include <boost/unordered_map.hpp>
#else
# include <map>
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

template <typename VT>
class DesignValueMap;







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/
/**
 * \brief A convenience typedef for the specialization of a value map for real
 *        values.
 */
typedef
DesignValueMap<double>
DesignDoubleMap;

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
 * \brief A means of mapping designs to values of a generic type.
 *
 * This can be used any time you want to associated a collection of designs
 * with some set of values.  An example of its usage is the WeightedSumMap
 * in which Designs are assocated with a weighted sum value according to some
 * set of weights.
 *
 * In general, VT must be an arithmetic type like double, int, etc.
 */
template <typename VT>
class JEGA_SL_IEDECL DesignValueMap :
    public MAP_BASE<const Design*, VT>
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

        /// A shorthand for the base type of this value map.
        typedef
        MAP_BASE<const Design*, VT>
        base_type;

    protected:


    private:

        /// A shorthand for the type of this value map.
        typedef
        DesignValueMap<value_type>
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

        /// The smallest value recorded in this map.
        /**
         * This is kept up to date whenever a value is added.
         */
        value_type _minValue;

        /// The largest value recorded in this map.
        /**
         * This is kept up to date whenever a value is added.
         */
        value_type _maxValue;

        /// The sum of all values recorded in this map.
        /**
         * This is kept up to date whenever a value is added.
         */
        value_type _totalValue;

        /// Whether or not min, max, and total are to be kept strictly up to
        /// date.
        /**
         * If this is true, then whenever it would be expensive to keep totals
         * current, they are allowed to fall out of date.  They can be updated
         * at any time by calling \a UpdateTotals.
         */
        bool _statsSuspended;


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

        /// Returns the stored max value in this map.
        /**
         * \return The largest value known to this value map.
         */
        inline
        const value_type&
        GetMaxValue(
            ) const;

        /// Returns the stored min value in this map.
        /**
         * \return The smallest Svalue known to this value map.
         */
        inline
        const value_type&
        GetMinValue(
            ) const;


        /// Returns the sum of all values in this map.
        /**
         * \return The sum of all values known to this value map.
         */
        inline
        const value_type&
        GetTotalValue(
            ) const;


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

        /**
         * \brief Returns the value associated with the supplied Design.
         *
         * If "of" is not found in this map, the return is the max
         * according to the numeric_limits class.
         *
         * \param of The Design for which the associated value is sought.
         * \return The value associated with "of" or a maximal value if "of" is
         *         not found.
         */
        inline
        value_type
        GetValue(
            const Design* of
            ) const;

        /**
         * \brief Returns the value associated with the supplied Design.
         *
         * If "of" is not found in this map, the return is the max
         * according to the numeric_limits class.
         *
         * \param of The Design for which the associated value is sought.
         * \return The value associated with "of" or a maximal value if "of" is
         *         not found.
         */
        inline
        value_type
        GetValue(
            const Design& of
            ) const;

        /**
         * \brief Adds an association of "of" to "val" to this map.
         *
         * \param of The Design to be mapped to "val".
         * \param val The value to associate with "of".
         * \return This method will return true if the association is made and
         *         false if not.  It will fail if for example "of" is already
         *         present as a key in this map.
         */
        inline
        bool
        AddValue(
            const Design& of,
            const value_type& val
            );

        /**
         * \brief Adds an association of "of" to "val" to this map.
         *
         * \param of The Design to be mapped to "val".
         * \param val The value to associate with "of".
         * \return This method will return true if the association is made and
         *         false if not.  It will fail if for example "of" is already
         *         present as a key in this map.
         */
        bool
        AddValue(
            const Design* of,
            const value_type& val
            );

        /// Returns the average of all values in this map.
        /**
         * \return The average of all values known to this value map or
         *         numeric_limits::max() if no values exist in the map.
         */
        inline
        value_type
        GetAverageValue(
            ) const;

        /**
         * \brief Removes any association "of" may have in this map.
         *
         * \param of The Design that should no longer be associated with a
         *           value in this map.
         * \return True if "of" is removed and false otherwise.  "of" would not
         *         be removed if it were not in the map to begin with.
         */
        inline
        bool
        RemoveValue(
            const Design& of
            );

        /**
         * \brief Removes any association "of" may have in this map.
         *
         * \param of The Design that should no longer be associated with a
         *           value in this map.
         * \return True if "of" is removed and false otherwise.  "of" would not
         *         be removed if it were not in the map to begin with.
         */
        bool
        RemoveValue(
            const Design* of
            );

        /// Adds "amt" to the value mapped to the Design "of".
        /**
         * If no mapping exists, then one is added with the value amt as the
         * initial value.
         *
         * \param of The design whose value is to be modified or which is to
         *           be added if not already present.
         * \param amt The amount to be added to the value associated with "of"
         *            or the initial value if "of" is not already present.
         * \return The new value associated with "of".
         */
        inline
        const value_type&
        AddToValue(
            const Design& of,
            const value_type& amt
            );

        /// Adds "amt" to the value mapped to the Design "of".
        /**
         * If no mapping exists, then one is added with the value amt as the
         * initial value.
         *
         * \param of The design whose value is to be modified or which is to
         *           be added if not already present.
         * \param amt The amount to be added to the value associated with "of"
         *            or the initial value if "of" is not already present.
         * \return The new value associated with "of".
         */
        const value_type&
        AddToValue(
            const Design* of,
            const value_type& amt
            );

        /**
         * \brief Iterates all entries in this map and updates the max, min,
         *        and total values.
         */
        void
        UpdateTotals(
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




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a DesignValueMap.
        DesignValueMap(
            std::size_t initSize = 0
            );

        /// Copy constructs a DesignValueMap.
        /**
         * \param copy An existing DesignValueMap from which to copy properties
         *             into this.
         */
        DesignValueMap(
            const DesignValueMap& copy
            );

        /// Destructs a DesignValueMap.
        virtual
        ~DesignValueMap(
            );



}; // class DesignValueMap



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
#include "inline/DesignValueMap.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNVALUEMAP_HPP
