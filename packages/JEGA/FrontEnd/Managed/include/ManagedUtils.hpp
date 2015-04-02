/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class ManagedUtils.

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

        Wed Feb 08 16:35:10 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ManagedUtils class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MANAGEDUTILS_HPP
#define JEGA_FRONTEND_MANAGED_MANAGEDUTILS_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>

#pragma unmanaged
#include <../Utilities/include/JEGATypes.hpp>






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
#pragma managed
#using <mscorlib.dll>








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
 * \brief A class with convenience functions for use by the Managed wrapper
 *        classes for JEGA.
 *
 * The methods in this class are all static and needn't have anything in common
 * except that they have no other home.
 */
MANAGED_CLASS(public, ManagedUtils)
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

        /// Converts a const char* into a System::String.
        /**
         * \param cstr The c-style string to convert into a System::String.
         * \return The Microsoft System::String equivolent of \a cstr.
         */
        static
        System::String MOH
        ToSysString(
            const char* cstr
            );

        /// Converts a std::string into a System::String.
        /**
         * \param stdStr The STL style string to convert into a System::String.
         * \return The Microsoft System::String equivolent of \a stdStr.
         */
        static
        System::String MOH
        ToSysString(
            const std::string& stdStr
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


    private:

        /// Default constructs a ManagedUtils object.
        /**
         * All methods of this class are static and so this class should not
         * be instantiated.  Therefore, this constructor is private.
         */
        ManagedUtils()
        {};


}; // class ManagedUtils


/// Converts the supplied list of doubles to a std::vector of doubles.
/**
 * The items from \a ar will be copy constructed into the returned
 * vector.
 *
 * \param ar The list from which to create a std::vector.
 * \return A std::vector containing duplicates of the items in \a ar.
 */
JEGA::DoubleVector
ToStdDoubleVector(
    JEGA::FrontEnd::Managed::DoubleVector MOH ar
    );

JEGA::FrontEnd::Managed::DoubleVector MOH
ToSysDoubleVector(
    const JEGA::DoubleVector& ar
    );

/// Converts the supplied list of doubles to a std::vector of doubles.
/**
 * The items from \a ar will be copy constructed into the returned
 * vector.
 *
 * \param ar The list from which to create a std::vector.
 * \return A std::vector containing duplicates of the items in \a ar.
 */
JEGA::IntVector
ToStdIntVector(
    JEGA::FrontEnd::Managed::IntVector MOH ar
    );

JEGA::StringVector
ToStdStringVector(
    JEGA::FrontEnd::Managed::StringVector MOH ar
    );

JEGA::FrontEnd::Managed::IntVector MOH
ToSysIntVector(
    const JEGA::IntVector& ar
    );

JEGA::FrontEnd::Managed::StringVector MOH
ToSysStringVector(
    const JEGA::StringVector& ar
    );

/// Converts a System::String into a std::string.
/**
 * \param sysStr The microsoft System::String to convert into a std::
 *               string.
 * \return The STL std::string equivolent of \a sysStr.
 */
std::string
ToStdStr(
    System::String MOH sysStr
    );


JEGA::DoubleMatrix
ToStdDoubleMatrix(
    JEGA::FrontEnd::Managed::DoubleMatrix MOH ar
    );

JEGA::FrontEnd::Managed::DoubleMatrix MOH
ToSysDoubleMatrix(
    const JEGA::DoubleMatrix& ar
    );



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
#endif // JEGA_FRONTEND_MANAGED_MANAGEDUTILS_HPP
