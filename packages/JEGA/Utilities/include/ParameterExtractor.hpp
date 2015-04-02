/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ParameterExtractor.

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

        Tue Feb 28 09:53:33 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ParameterExtractor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_PARAMETEREXTRACTOR_HPP
#define JEGA_UTILITIES_PARAMETEREXTRACTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

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
class ParameterDatabase;
class ParameterExtractor;







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
 * \brief A class with functions useful in retrieving data from a
 *        ParameterDatabase.
 *
 * This class has a bunch of static methods that retrieve data of the requested
 * kind and store it in a specified location.  The methods handle catching of
 * exceptions and logging them to the global logger.
 */
class ParameterExtractor
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

        /// Retrieves an integer from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 integral value.
         */
        static
        int
        GetIntegralFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves an integer from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetIntegralFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            int& into
            );

        /// Retrieves a double from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 double value.
         */
        static
        double
        GetDoubleFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a double from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetDoubleFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            double& into
            );

        /// Retrieves a size_t from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 size_t value.
         */
        static
        std::size_t
        GetSizeTypeFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a size_t from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetSizeTypeFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            std::size_t& into
            );

        /// Retrieves a bool from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 bool value.
         */
        static
        bool
        GetBooleanFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a bool from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetBooleanFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            bool& into
            );

        /// Retrieves a string from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 string value.
         */
        static
        std::string
        GetStringFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a string from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetStringFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            std::string& into
            );

        /// Retrieves a DoubleVector from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleVector value.
         */
        static
        JEGA::DoubleVector
        GetDoubleVectorFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a DoubleVector from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetDoubleVectorFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            JEGA::DoubleVector& into
            );

        /// Retrieves a IntVector from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 IntVector value.
         */
        static
        JEGA::IntVector
        GetIntVectorFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a IntVector from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetIntVectorFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            JEGA::IntVector& into
            );

        /// Retrieves a DoubleMatrix from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleMatrix value.
         */
        static
        JEGA::DoubleMatrix
        GetDoubleMatrixFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a DoubleMatrix from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetDoubleMatrixFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            JEGA::DoubleMatrix& into
            );

        /// Retrieves a StringVector from the ParameterDatabase.
        /**
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 StringVector value.
         */
        static
        JEGA::StringVector
        GetStringVectorFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag
            );

        /// Retrieves a StringVector from the ParameterDatabase.
        /**
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        static
        bool
        GetStringVectorFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            JEGA::StringVector& into
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

        /// Retrieves a parameter of type R from the ParameterDatabase.
        /**
         * This method is used by all versions of the Get*FromDB methods that
         * take only a tag and database.
         *
         * If retrieval is not successful, a log entry is issued and an
         * exception of type no_such_parameter_error is thrown.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param func The ParameterDatabase function to use to return a
         *             parameter of type R given the tag.
         * \return The value retrieved.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 type R value.
         */
        template<typename R>
        static
        R
        _GetParamFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            R (JEGA::Utilities::ParameterDatabase::*func)(
                const std::string&
                ) const
            );

        /// Retrieves a parameter of type R from the ParameterDatabase.
        /**
         * This method is used by all versions of the Get*FromDB methods that
         * take reference arguments.
         *
         * If retrieval is not successful, then \a into is not modified in any
         * way.
         *
         * \param db The database from which to retrieve the parameter
         *           assiciated with \a tag.
         * \param tag The name of the parameter to be retrieved.
         * \param into The instance into which to copy the retrieved value if
         *             successfully retrieved.
         * \param func The ParameterDatabase function to use to return a
         *             parameter of type R given the tag.
         * \return true if the retrieval was successful and \a into modified
         *         and false otherwise.
         */
        template<typename R>
        static
        bool
        _GetParamFromDB(
            const JEGA::Utilities::ParameterDatabase& db,
            const std::string& tag,
            R& into,
            R (JEGA::Utilities::ParameterDatabase::*func)(
                const std::string&
                ) const
            );





    /*
    ============================================================================
    Structors
    ============================================================================
    */
    private:

        /// This constructor is private and has no implementation.
        /**
         * This class has nothing but static methods and thus should not and
         * cannot be instantiated.
         */
        ParameterExtractor(
            );




}; // class ParameterExtractor



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
#include "inline/ParameterExtractor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_PARAMETEREXTRACTOR_HPP
