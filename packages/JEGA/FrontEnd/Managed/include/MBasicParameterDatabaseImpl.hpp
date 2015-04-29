/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MBasicParameterDatabaseImpl.

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

        Wed Feb 08 16:29:50 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MBasicParameterDatabaseImpl class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MBASICPARAMETERDATABASEIMPL_HPP
#define JEGA_FRONTEND_MANAGED_MBASICPARAMETERDATABASEIMPL_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>
#include <MParameterDatabase.hpp>






/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class ParameterDatabase;
        class BasicParameterDatabaseImpl;
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
 * \brief A managed wrapper for the JEGA::Utilities::BasicParameterDatabaseImpl
 *        class.
 *
 * This class uses a JEGA::Utilities::BasicParameterDatabaseImpl internally
 * and exposes the necessary methods in a managed compliant manner for use with
 * all .NET languages.
 */
MANAGED_CLASS(public, MBasicParameterDatabaseImpl) :
    public MParameterDatabase
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

        /**
         * \brief The actual JEGA::Utilities::BasicParameterDatabaseImpl
         *        wrapped by this managed class.
         */
        JEGA::Utilities::BasicParameterDatabaseImpl* _thePDB;




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
         * \brief Supplies the JEGA::Utilities::ParameterDatabase that actually
         *        underlies this managed basic database.
         *
         * \return The JEGA Core project ParameterDatabase that is the guts of
         *         this managed version.
         */
        virtual
        JEGA::Utilities::ParameterDatabase&
        Manifest(
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

        /// Tests the integer param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the integer param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasIntegralParam(
            System::String MOH tag
            );

        /// Tests the short param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the short param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasShortParam(
            System::String MOH tag
            );

        /// Tests the double param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the double param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasDoubleParam(
            System::String MOH tag
            );

        /// Tests the size_t param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the size_t param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasSizeTypeParam(
            System::String MOH tag
            );

        /// Tests the boolean param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the boolean param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasBooleanParam(
            System::String MOH tag
            );

        /// Tests the string param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the string param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasStringParam(
            System::String MOH tag
            );

        /**
         * \brief Tests the vector of doubles param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the vector of doubles param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasDoubleVectorParam(
            System::String MOH tag
            );

        /**
         * \brief Tests the vector of ints param map for a value mapped to the
         *        supplied tag.
         *
         * \param tag The key to search for in the vector of ints param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasIntVectorParam(
            System::String MOH tag
            );

        /**
         * \brief Tests the matrix of doubles param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the matrix of doubles param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasDoubleMatrixParam(
            System::String MOH tag
            );

        /**
         * \brief Tests the vector of strings param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the vector of strings param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        virtual
        bool
        HasStringVectorParam(
            System::String MOH tag
            );

        /// Supplies the requested parameter as an integer from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The int value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 int value.
         */
        virtual
        int
        GetIntegral(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a short from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The short value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 short value.
         */
        virtual
        short
        GetShort(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a double from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The double value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 double value.
         */
        virtual
        double
        GetDouble(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a size_t from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The size_t value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 size_t value.
         */
        virtual
        eddy::utilities::uint64_t
        GetSizeType(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a bool from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The bool value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 bool value.
         */
        virtual
        bool
        GetBoolean(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a string from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The std::string value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 string value.
         */
        virtual
        System::String MOH
        GetString(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a DoubleVector from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of doubles associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleVector value.
         */
        virtual
        JEGA::FrontEnd::Managed::DoubleVector MOH
        GetDoubleVector(
            System::String MOH tag
            );

        /// Supplies the requested parameter as an IntVector from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of ints associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 IntVector value.
         */
        virtual
        JEGA::FrontEnd::Managed::IntVector MOH
        GetIntVector(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a DoubleMatrix from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The matrix of doubles associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleMatrix value.
         */
        virtual
        JEGA::FrontEnd::Managed::DoubleMatrix MOH
        GetDoubleMatrix(
            System::String MOH tag
            );

        /// Supplies the requested parameter as a StringVector from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of std::strings associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 StringVector value.
         */
        virtual
        JEGA::FrontEnd::Managed::StringVector MOH
        GetStringVector(
            System::String MOH tag
            );

        /// Maps the supplied integer value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other integral value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddIntegralParam(
            System::String MOH tag,
            int value
            );

        /// Maps the supplied double value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other double value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddDoubleParam(
            System::String MOH tag,
            double value
            );

        /// Maps the supplied size_t value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other size_t value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddSizeTypeParam(
            System::String MOH tag,
            eddy::utilities::uint64_t value
            );

        /// Maps the supplied boolean value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other boolean value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddBooleanParam(
            System::String MOH tag,
            bool value
            );

        /// Maps the supplied string value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other string value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddStringParam(
            System::String MOH tag,
            System::String MOH value
            );

        /// Maps the supplied vector of doubles value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other vector of doubles value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddDoubleVectorParam(
            System::String MOH tag,
            JEGA::FrontEnd::Managed::DoubleVector MOH value
            );

        /// Maps the supplied vector of ints value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other vector of ints value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddIntVectorParam(
            System::String MOH tag,
            JEGA::FrontEnd::Managed::IntVector MOH value
            );

        /// Maps the supplied matrix of doubles value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other matrix of doubles value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddDoubleMatrixParam(
            System::String MOH tag,
            JEGA::FrontEnd::Managed::DoubleMatrix MOH value
            );

        /// Maps the supplied vector of strings value to the supplied tag.
        /**
         * The mapping will only succeed if \a tag is not already the key for
         * some other vector of strings value.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddStringVectorParam(
            System::String MOH tag,
            JEGA::FrontEnd::Managed::StringVector MOH value
            );

        /// Implementation of the Dispose method for the IDisposable interface.
        /**
         * This method destroys the underlying JEGA basic parameter database.
         * It is safe to call this more than once but it is not safe to use
         * the other methods of this class once this has been called.
         *
         * The destructor should call this method.
         */
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

        /// Default constructs an MBasicParameterDatabaseImpl.
        MBasicParameterDatabaseImpl(
            );

        /// Destructs an MBasicParameterDatabaseImpl.
        /**
         * The body of this method calls the Dispose method to destroy the
         * JEGA core basic parameter database object.
         */
        ~MBasicParameterDatabaseImpl(
            );


}; // class MBasicParameterDatabaseImpl



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
#endif // JEGA_FRONTEND_MANAGED_MBASICPARAMETERDATABASEIMPL_HPP
