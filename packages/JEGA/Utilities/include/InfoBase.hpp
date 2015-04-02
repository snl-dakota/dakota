/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class InfoBase.

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

        Mon Jun 09 16:33:08 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the InfoBase class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_INFOBASE_HPP
#define JEGA_UTILITIES_INFOBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>
#include <utilities/include/int_types.hpp>










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
class InfoBase;
class DesignTarget;






/*
================================================================================
Class Definition
================================================================================
*/

/// Base class for information objects such as DesignVariableInfo
/**
 * This class provides some information and capability common to
 * all information objects.
 */
class JEGA_SL_IEDECL InfoBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The DesignTarget known by this info object.
        DesignTarget& _target;

        /// The textual representation of the name of this info object.
        std::string _label;

        /// The number of this info object in the order of infos.
        std::size_t _number;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the "_label" member data.
        /**
         * \param text The new label for this info object.
         */
        inline
        void
        SetLabel(
            const std::string& text
            );

        /// Sets the "_number" member data.
        /**
         * This should generally be used only upon creation or re-ordering
         * of the info objects.
         *
         * \param number The new number for this info object.
         */
        inline
        void
        SetNumber(
            std::size_t number
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the DesignTarget known by this object (non-const)
        /**
         * \return The DesignTarget being used by this info object.
         */
        inline
        DesignTarget&
        GetDesignTarget(
            );

        /// Returns the DesignTarget known by this object (const)
        /**
         * \return The DesignTarget being used by this info object.
         */
        inline
        const DesignTarget&
        GetDesignTarget(
            ) const;

        /// Gets the "_label" member data.
        /**
         * \return The label of this information object.
         */
        inline
        const std::string&
        GetLabel(
            ) const;

        /// Gets the "_number" member data.
        /**
         * \return The number of this information object in the collection of
         *         all like information objects.
         */
        inline
        std::size_t
        GetNumber(
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

        /// Constructs an InfoBase object knowing "target".
        /**
         * \param target The DesignTarget for this info object to use.
         */
        InfoBase(
            DesignTarget& target
            );

        /// Copy constructs an InfoBase object.
        /**
         * \param copy The InfoBase object from which to copy properties into
         *             this.
         */
        explicit
        InfoBase(
            const InfoBase& copy
            );

        /// Copy constructs an InfoBase object for use in the supplied target.
        /**
         * \param copy The InfoBase object from which to copy properties into
         *             this.
         * \param target The DesignTarget for this info object to use.
         */
        explicit
        InfoBase(
            const InfoBase& copy,
            DesignTarget& target
            );

        /// Destructs an InfoBase object.
        virtual
        ~InfoBase(
            );



}; // class InfoBase


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
#include "./inline/InfoBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_INFOBASE_HPP
