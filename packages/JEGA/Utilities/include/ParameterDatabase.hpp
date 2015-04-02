/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ParameterDatabase.

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

        Wed Dec 07 13:43:40 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ParameterDatabase class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_PARAMETERDATABASE_HPP
#define JEGA_UTILITIES_PARAMETERDATABASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <iosfwd>
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


/*
================================================================================
In-Namespace File Scope Utility Classes
================================================================================
*/
class JEGA_SL_IEDECL no_such_parameter_error :
    public std::exception
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The text to be returned by "what()".
        std::string _message;

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Returns the message of this exception.
        virtual
        const char*
        what(
            ) const throw()
        {
            return this->_message.c_str();
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a no_such_parameter_error.
        /**
         * This causes creation of the message built using the supplied tag.
         */
        no_such_parameter_error(
            const std::string& tag
            ) :
                _message("Parameter not found for tag: " + tag)
        {
        }

        /// Destructs a no_such_parameter_error.
        ~no_such_parameter_error(
            ) throw()
        {
        }

}; // no_such_parameter_error






/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief An interface for any class that can serve as a database of
 *        algorithm configuration parameters.
 *
 * This interface requires of all its derivatives implementation of a number
 * of methods that can be used to retrieve parameters of various types
 * according to a string key.
 */
class JEGA_SL_IEDECL ParameterDatabase
{
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





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:




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
         * \brief Override to print the contents of the entire database into a
         *        string and return it.
         *
         * If there are contents that cannot be written to a string then handle
         * them as best you can.  Formatting is completely up to the
         * implementer.
         *
         * \return The entire contents of the database in a string.
         */
        virtual
        std::string
        Dump(
            ) const = 0;

        /**
         * \brief Override to print the contents of the entire database into
         *        the supplied output stream.
         *
         * If there are contents that cannot be inserted into a stream then
         * handle them as best you can.  Formatting is completely up to the
         * implementer.
         *
         * \param stream The stream into which to write the contents of this
         *               database.
         */
        virtual
        void
        Dump(
            std::ostream& stream
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as an integer from
         *        this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The int value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 integral value.
         */
        virtual
        int
        GetIntegral(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a short integer
         *        from this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The short value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 integral value.
         */
        virtual
        short
        GetShort(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a double from
         *        this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The double value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 double value.
         */
        virtual
        double
        GetDouble(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a size_t from
         *        this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The size_t value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 size_t value.
         */
        virtual
        std::size_t
        GetSizeType(
            const std::string& tag
            ) const = 0;

        /// Override to supply the requested parameter as a bool from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The bool value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 bool value.
         */
        virtual
        bool
        GetBoolean(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a string from
         *        this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The std::string value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 string value.
         */
        virtual
        std::string
        GetString(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a DoubleVector
         *        from this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of doubles associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleVector value.
         */
        virtual
        JEGA::DoubleVector
        GetDoubleVector(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as an IntVector
         *        from this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of ints associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 IntVector value.
         */
        virtual
        JEGA::IntVector
        GetIntVector(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a DoubleMatrix
         *        from this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The matrix of doubles associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleMatrix value.
         */
        virtual
        JEGA::DoubleMatrix
        GetDoubleMatrix(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to supply the requested parameter as a StringVector
         *        from this DB.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of std::strings associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 StringVector value.
         */
        virtual
        JEGA::StringVector
        GetStringVector(
            const std::string& tag
            ) const = 0;

        /**
         * \brief Override to map the supplied integer value to the supplied
         *           tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other integral value, this should do a replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddIntegralParam(
            const std::string& tag,
            const int& value
            ) = 0;

        /**
         * \brief Override to map the supplied short integer value to the
         *         supplied tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other short value, this should do a replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddShortParam(
            const std::string& tag,
            const short& value
            ) = 0;

        /**
         * \brief Override to map the supplied double value to the supplied
         *        tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other double value, this should do a replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddDoubleParam(
            const std::string& tag,
            const double& value
            ) = 0;

        /**
         * \brief Override to map the supplied size_t value to the supplied
         *        tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other size_t value, this should do a replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddSizeTypeParam(
            const std::string& tag,
            const std::size_t& value
            ) = 0;

        /**
         * \brief Override to map the supplied boolean value to the supplied
         *        tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other boolean value, this should do a replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddBooleanParam(
            const std::string& tag,
            const bool& value
            ) = 0;

        /**
         * \brief Override to map the supplied string value to the supplied
         *        tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other string value, this should do a replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddStringParam(
            const std::string& tag,
            const std::string& value
            ) = 0;

        /**
         * \brief Override to map the supplied vector of doubles value to the
         *        supplied tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other vector of double value, this should do a
         * replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddDoubleVectorParam(
            const std::string& tag,
            const JEGA::DoubleVector& value
            ) = 0;

        /**
         * \brief Override to map the supplied vector of ints value to the
         *        supplied tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other vector of int value, this should do a
         * replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddIntVectorParam(
            const std::string& tag,
            const JEGA::IntVector& value
            ) = 0;

        /**
         * \brief Override to map the supplied matrix of doubles value to the
         *        supplied tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other matrix of double value, this should do a
         * replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddDoubleMatrixParam(
            const std::string& tag,
            const JEGA::DoubleMatrix& value
            ) = 0;

        /**
         * \brief Override to map the supplied vector of strings value to the
         *        supplied tag.
         *
         * Generally, the mapping should always succeed.  If \a tag is already
         * the key for some other vector of string value, this should do a
         * replacement.
         *
         * \param tag The key to which the requested value is to be mapped.
         * \param value The value that is to be mapped by key.
         * \return True of the mapping succeeds and false otherwise.
         */
        virtual
        bool
        AddStringVectorParam(
            const std::string& tag,
            const JEGA::StringVector& value
            ) = 0;

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

        /// Default constructs a ParameterDatabase.
        ParameterDatabase(
            ) {}

        /// Destructs a ParemeterDatabase.
        virtual
        ~ParameterDatabase(
            ) {}


}; // class ParameterDatabase



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
// No inline file for interface class




/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_PARAMETERDATABASE_HPP
