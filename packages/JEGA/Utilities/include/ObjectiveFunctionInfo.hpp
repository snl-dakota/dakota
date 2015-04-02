/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ObjectiveFunctionInfo.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 08 12:19:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ObjectiveFunctionInfo class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_OBJECTIVEFUNCTIONINFO_HPP
#define JEGA_UTILITIES_OBJECTIVEFUNCTIONINFO_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/int_types.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/InfoBase.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>




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
class ObjectiveFunctionInfo;
class ObjectiveFunctionTypeBase;
class ObjectiveFunctionNatureBase;






/*
================================================================================
In Namespace File Scope Typedefs
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

/// This class stores information about objective functions.
/**
 * The information stored includes the type (minimize, maximize, etc) and the
 * nature (linear, non-linear) as well as any inherited information from
 * InfoBase.  The majority of work done on behalf of this class is either
 * done by the type or the nature which must be derivatives of the
 * ObjectiveFunctionTypeBase and ObjectiveFunctionNatureBase respectively.
 */
class JEGA_SL_IEDECL ObjectiveFunctionInfo :
    public InfoBase
{


    /*
    ============================================================================
    Typedefs
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

        /// Represents the type of this objective.
        /**
         * It is something like minimize, maximize, etc.
         */
        ObjectiveFunctionTypeBase* _type;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Tells this object what type objective it represents.
        /**
         * \param type The new type for this objective function.
         */
        void
        SetType(
            ObjectiveFunctionTypeBase* type
            );


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /// Returns the type represented by this object.
        /**
         * \return The type object stored in this info object.
         */
        inline
        ObjectiveFunctionTypeBase&
        GetType(
            ) const;



    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the value stored in the proper location for this objective.
        /**
         * For example, if this is the 3rd ObjectiveFunctionInfo, then
         * this method will return the number stored in the third
         * objective location in \a des.
         *
         * \param des The Design from which to retrieve the proper value.
         * \return The corresponding objective value of the supplied Design.
         */
        inline
        double
        Which(
            const Design& des
            ) const;

        /// Returns the value converted for a minimization process.
        /**
         * This method uses the type and nature to resolve a proper value.
         * The proper value is what should be used when trying to minimize
         * this objective function.
         *
         * \param des The Design from which to retrieve the proper value.
         * \return The corresponding objective value transformed for
         *         minimization of the supplied Design.
         */
        inline
        double
        WhichForMinimization(
            const Design& des
            ) const;

        /**
         * \brief Determines which value is preferred for this objective.
         *
         * The return is modeled after strcmp and is thus -1 if \a val1 is
         * preferred, 1 if \a val2 is preferred, and 0 if neither is preferred.
         *
         * \param val1 The first value to compare.
         * \param val2 The second value to compare.
         * \return -1 if \a val1 is preferred, 1 if \a val2 is preferred, and 0
         *         if neither is preferred.
         */
        int
        PreferComp(
            double val1,
            double val2
            ) const;

        /**
         * \brief Returns a pointer to the preferred Design or Null if neither
         *        is preferred.
         *
         * This is computed by comparing the WhichForMinimization values of
         * each Design.
         *
         * \param des1 The first Design object to compare.
         * \param des2 The second Design object to compare.
         * \return A pointer to the preferred Design or Null if neither.
         */
        inline
        const Design*
        GetPreferredDesign(
            const Design& des1,
            const Design& des2
            ) const;

        /// Returns the amount by which \a des1 is preferred to \a des2.
        /**
         * If \a des1 is not preferred to \a des2, 0.0 is returned.
         *
         * \param des1 The first Design object to compare.
         * \param des2 The second Design object to compare.
         * \return The amount by which \a des1 is preferred to \a des2 or
         *         0 if \a des1 is not preferred..
         */
        inline
        double
        GetPreferredAmount(
            const Design& des1,
            const Design& des2
            ) const;

        /// Returns a string representation of the type of this objective.
        /**
         * Some examples are "Minimize", "Maximize", etc.
         *
         * \return The string representation of the type of this objective.
         */
        inline
        std::string
        GetTypeString(
            ) const;

        /// Returns a string representation of the nature of this objective.
        /**
         * Some examples are "Minimize", "Maximize", etc.
         *
         * \return The string representation of the nature of this objective.
         */
        std::string
        GetNatureString(
            ) const;

        /// Tells this object of the nature of the variable it represents.
        /**
         * This method fails if IsNatureLocked returns true.  This object
         * assumes ownership of \a nature and will destroy it as it sees fit.
         * Do not store \a nature elsewhere.
         *
         * \param nature The new nature for this objective info.
         * \return true on success and false on failure.
         */
        bool
        SetNature(
            ObjectiveFunctionNatureBase* nature
            );

        /// Returns the nature of the variable represented by this object.
        /**
         * \return A reference to the nature object of this objective info.
         */
        ObjectiveFunctionNatureBase&
        GetNature(
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


        /// Constructs an ObjectiveFunctionInfo object knowing \a target.
        /**
         * Default properties are type = real and nature = continuum.
         *
         * \param target The DesignTarget for which this info is being
         *               constructed.
         */
        ObjectiveFunctionInfo(
            DesignTarget& target
            );

        /// Copy constructs an ObjectiveFunctionInfo for use by \a target.
        /**
         * \param copy The info to copy properties from.
         * \param target The DesignTarget for which this info is being
         *               constructed.
         */
        ObjectiveFunctionInfo(
            const ObjectiveFunctionInfo& copy,
            DesignTarget& target
            );

        /// Copy constructs an ObjectiveFunctionInfo object.
        /**
         * \param copy The info to copy properties from including the target.
         */
        ObjectiveFunctionInfo(
            const ObjectiveFunctionInfo& copy
            );

        /// Destructs an ObjectiveFunctionInfo object.
        /**
         * This destructor destoys the type object.
         */
        virtual
        ~ObjectiveFunctionInfo(
            );



}; // class ObjectiveFunctionInfo





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
#include "./inline/ObjectiveFunctionInfo.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_OBJECTIVEFUNCTIONINFO_HPP
