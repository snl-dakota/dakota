/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class BasicParameterDatabaseImpl.

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

        Fri Jan 06 07:35:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the BasicParameterDatabaseImpl class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_BASICPARAMETERDATABASEIMPL_HPP
#define JEGA_UTILITIES_BASICPARAMETERDATABASEIMPL_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cstddef>
#include <typeinfo>
#include <exception>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/JEGATypes.hpp>
#include <../Utilities/include/ParameterDatabase.hpp>

#ifdef JEGA_THREADSAFE
#   include <threads/include/ts_map.hpp>
#else
#   include <map>
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
class BasicParameterDatabaseImpl;







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/
#ifdef JEGA_THREADSAFE
#   define MY_MAP_T eddy::threads::ts_map
#else
#   define MY_MAP_T std::map
#endif








/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A basic implementation of the JEGA::Utilities::ParameterDatabase
 *        class.
 *
 * This implementation stores all parameters in maps of strings to data types.
 * the data can be loaded using the various Add<type>Param methods and
 * retrieved using the Get<type> methods.
 *
 * The maps used are single-keyed maps as opposed to multi-keyed maps.
 * Therefore the keys must be unique.  An attempt to add a parameter with a
 * key that already exists in the same type map will cause replacement of the
 * existing value.
 *
 * The Get methods throw errors when attempts are made to retrieve parameters
 * that are not in the map.  Use the JEGA::Utilities::ParameterExtractor for
 * methods that automatically handle these errors and return a boolean.
 *
 * Here is an example of the useage of this class.
 * \code
        BasicParameterDatabaseImpl pdb;
        pdb.AddIntegralParam("myInt", 123);
        pdb.AddDoubleParam("myReal", 456.789);

        try {
            int myInt = pdb.GetIntegral("myInt");
            double myReal = pdb.GetDouble("myReal");

            // pdb.GetIntegral("noExist"); // throws no_such_parameter_error
        }
        catch (const no_such_parameter_error& e) {
            // handle e
        }
   \endcode
 */
class JEGA_SL_IEDECL BasicParameterDatabaseImpl :
    public JEGA::Utilities::ParameterDatabase
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

    protected:

        /// The type of the map of strings to integer parameters.
        typedef
        MY_MAP_T<std::string, int>
        IntParamMap;

        /// The type of the map of strings to short integer parameters.
        typedef
        MY_MAP_T<std::string, short>
        ShortParamMap;

        /// The type of the map of strings to double precision parameters.
        typedef
        MY_MAP_T<std::string, double>
        DoubleParamMap;

        /// The type of the map of strings to size_t parameters.
        typedef
        MY_MAP_T<std::string, std::size_t>
        SizeTParamMap;

        /// The type of the map of strings to boolean parameters.
        typedef
        MY_MAP_T<std::string, bool>
        BoolParamMap;

        /// The type of the map of strings to string parameters.
        typedef
        MY_MAP_T<std::string, std::string>
        StringParamMap;

        /// The type of the map of strings to vector of double parameters.
        typedef
        MY_MAP_T<std::string, JEGA::DoubleVector>
        DoubleVectorParamMap;

        /// The type of the map of strings to vector of int parameters.
        typedef
        MY_MAP_T<std::string, JEGA::IntVector>
        IntVectorParamMap;

        /// The type of the map of strings to matrix of double parameters.
        typedef
        MY_MAP_T<std::string, JEGA::DoubleMatrix>
        DoubleMatrixParamMap;

        /// The type of the map of strings to vector of string parameters.
        typedef
        MY_MAP_T<std::string, JEGA::StringVector>
        StringVectorParamMap;

    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The map of integer parameters
        IntParamMap _intParams;

        /// The map of integer parameters
        ShortParamMap _shortParams;

        /// The map of double parameters
        DoubleParamMap _doubleParams;

        /// The map of size_t parameters
        SizeTParamMap _sizeTParams;

        /// The map of boolean parameters
        BoolParamMap _boolParams;

        /// The map of string parameters
        StringParamMap _stringParams;

        /// The map of vector of double parameters
        DoubleVectorParamMap _doubleVectorParams;

        /// The map of vector of int parameters
        IntVectorParamMap _intVectorParams;

        /// The map of matrix of double parameters
        DoubleMatrixParamMap _doubleMatrixParams;

        /// The map of vector of string parameters
        StringVectorParamMap _stringVectorParams;

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

        /// Tests the integer param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the integer param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasIntegralParam(
            const std::string& tag
            ) const;

        /// Tests the short param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the short param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasShortParam(
            const std::string& tag
            ) const;

        /// Tests the double param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the double param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasDoubleParam(
            const std::string& tag
            ) const;

        /// Tests the size_t param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the size_t param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasSizeTypeParam(
            const std::string& tag
            ) const;

        /// Tests the boolean param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the boolean param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasBooleanParam(
            const std::string& tag
            ) const;

        /// Tests the string param map for a value mapped to the supplied tag.
        /**
         * \param tag The key to search for in the string param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasStringParam(
            const std::string& tag
            ) const;

        /**
         * \brief Tests the vector of doubles param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the vector of doubles param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasDoubleVectorParam(
            const std::string& tag
            ) const;

        /**
         * \brief Tests the vector of ints param map for a value mapped to the
         *        supplied tag.
         *
         * \param tag The key to search for in the vector of ints param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasIntVectorParam(
            const std::string& tag
            ) const;

        /**
         * \brief Tests the matrix of doubles param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the matrix of doubles param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasDoubleMatrixParam(
            const std::string& tag
            ) const;

        /**
         * \brief Tests the vector of strings param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the vector of strings param map.
         * \return true if \a tag is found as a key and false otherwise.
         */
        bool
        HasStringVectorParam(
            const std::string& tag
            ) const;

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:


        /**
         * \brief Retrieves the value associated with the supplied tag.
         *
         * \param tag The key by which the requested value is to be retrieved.
         * \param from The map from which the value is to be retrieved.
         * \return The value retrieved from the map or a thrown error if not
         *         found.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 value in \a from.
         */
        template <typename T>
        const T&
        GetParamValue(
            const std::string& tag,
            const MY_MAP_T<std::string, T>& from
            ) const;

        /**
         * \brief Adds the supplied value to the supplied map using the
         *        supplied tag as a key.
         *
         * \param tag The key by which the requested value is to be stored.
         * \param value The value that is to be mapped to \a tag.
         * \param theMap The map into which the key-value pair is to be
         *               inserted.
         * \return True if the insertion succeeds and false otherwise.
         *         Insertion always succeeds but may be a replacment.
         */
        template <typename T>
        bool
        AddParamValue(
            const std::string& tag,
            const T& value,
            MY_MAP_T<std::string, T>& theMap
            ) const;

        /**
         * \brief Tests the supplied param map for a value mapped to
         *        the supplied tag.
         *
         * \param tag The key to search for in the supplied param map.
         * \param in The map in which to search for a value keyed by \a tag.
         * \return true if \a tag is found as a key and false otherwise.
         */
        template <typename T>
        bool
        HasParam(
            const std::string& tag,
            const MY_MAP_T<std::string, T>& in
            ) const;

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /**
         * \brief Prints the contents of the entire database into a string and
         *        returns it.
         *
         * Each non-empty map of values is written to the string with each
         * mapping on it's own line in the form key = value.  In the case
         * of vector or matrix valued parameters, the output for each key
         * may take multiple lines.  Finally, each map is preceeded by
         * a description of its contents such as "Integer Parameters:".
         *
         * \return A string into which the contents of this database have been
         *         written.
         */
        virtual
        std::string
        Dump(
            ) const;

        /**
         * \brief Prints the contents of the entire database into the
         *        supplied output stream.
         *
         * Each non-empty map of values is written to the stream with each
         * mapping on it's own line in the form key = value.  In the case
         * of vector or matrix valued parameters, the output for each key
         * may take multiple lines.  Finally, each map is preceeded by
         * a description of its contents such as "Integer Parameters:".
         *
         * \param stream The stream into which to write the contents of
         *               this database.
         */
        virtual
        void
        Dump(
            std::ostream& stream
            ) const;

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
            const std::string& tag
            ) const;

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
            const std::string& tag
            ) const;

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
            const std::string& tag
            ) const;

        /// Supplies the requested parameter as a size_t from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The size_t value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 size_t value.
         */
        virtual
        std::size_t
        GetSizeType(
            const std::string& tag
            ) const;

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
            const std::string& tag
            ) const;

        /// Supplies the requested parameter as a string from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The std::string value associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 string value.
         */
        virtual
        std::string
        GetString(
            const std::string& tag
            ) const;

        /// Supplies the requested parameter as a DoubleVector from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of doubles associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleVector value.
         */
        virtual
        JEGA::DoubleVector
        GetDoubleVector(
            const std::string& tag
            ) const;

        /// Supplies the requested parameter as an IntVector from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of ints associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with an
         *                                 IntVector value.
         */
        virtual
        JEGA::IntVector
        GetIntVector(
            const std::string& tag
            ) const;

        /// Supplies the requested parameter as a DoubleMatrix from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The matrix of doubles associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 DoubleMatrix value.
         */
        virtual
        JEGA::DoubleMatrix
        GetDoubleMatrix(
            const std::string& tag
            ) const;

        /// Supplies the requested parameter as a StringVector from this DB.
        /**
         * \param tag The key by which the requested value is to be retrieved.
         * \return The vector of std::strings associated with the supplied tag.
         * \throws no_such_parameter_error If tag is not associated with a
         *                                 StringVector value.
         */
        virtual
        JEGA::StringVector
        GetStringVector(
            const std::string& tag
            ) const;

        /// Maps the supplied integer value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied short integer value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied double value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied size_t value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied boolean value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied string value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied vector of doubles value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied vector of ints value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied matrix of doubles value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

        /// Maps the supplied vector of strings value to the supplied tag.
        /**
         * The mapping will always succeed.  If the \a tag is already the key
         * for some other value, it will be replaced with the new value.
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
            );

    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Method called prior to dumping the contents of \a theMap to
         *        \a stream
         *
         * The value type of the Map_T type does not matter for this method.
         *
         * \param theMap The map of parameters that is going to be written.
         * \param desc The brief, 1-line description of the maps contents.
         * \param stream The stream into which the map will be dumped.
         */
        template <typename Map_T>
        static
        void
        PreDump(
            const Map_T& theMap,
            const std::string& desc,
            std::ostream& stream
            );

        /**
         * \brief Method called after dumping of the contents of \a theMap to
         *        \a stream
         *
         * The value type of the Map_T type does not matter for this method.
         *
         * \param theMap The map of parameters that was written.
         * \param stream The stream into which the map will be dumped.
         */
        template <typename Map_T>
        static
        void
        PostDump(
            const Map_T& theMap,
            std::ostream& stream
            );

        /// Writes the contents of \a theMap to the \a stream.
        /**
         * The value type of the Map_T type must be singularly ostreamable.
         * The call progression is:
         * \code
                PreDump(theMap, desc, stream);
                <write contents>
                PostDump(theMap, stream);
           \endcode
         *
         * \param theMap The map of parameters that is to be written.
         * \param desc The brief, 1-line description of the maps contents.
         * \param stream The stream into which the map it to be dumped.
         */
        template <typename Map_T>
        static
        void
        DumpValueMap(
            const Map_T& theMap,
            const std::string& desc,
            std::ostream& stream
            );

        /// Writes the contents of \a theMap to the \a stream.
        /**
         * The value type of the Map_T type must be a first class STL style
         * container such as std::vector or std::list.  The call progression
         * is:
         * \code
                PreDump(theMap, desc, stream);
                <write contents>
                PostDump(theMap, stream);
           \endcode
         *
         * \param theMap The map of parameters that is to be written.
         * \param desc The brief, 1-line description of the maps contents.
         * \param stream The stream into which the map it to be dumped.
         */
        template <typename CMap>
        static
        void
        DumpContainerMap(
            const CMap& theMap,
            const std::string& desc,
            std::ostream& stream
            );

        /// Writes the contents of \a theMap to the \a stream.
        /**
         * The value_type of the MMap type must be a first class STL style
         * container of first class STL style containers such as
         * std::vector<std::vector<..> > or std::list<std::list<..> > or
         * any combination.  The call progression is:
         * \code
                PreDump(theMap, desc, stream);
                <write contents>
                PostDump(theMap, stream);
           \endcode
         *
         * \param theMap The map of parameters that is to be written.
         * \param desc The brief, 1-line description of the maps contents.
         * \param stream The stream into which the map it to be dumped.
         */
        template <typename MMap, typename MType>
        static
        void
        DumpMatrixMap(
            const MMap& theMap,
            const std::string& desc,
            std::ostream& stream
            );

        /**
         * \brief Convenience method to write the contents of a container to
         *        \a stream
         *
         * The CType must be a first class STL style container such as
         * std::vector or std::list.  The output is a comma separated list
         * surrounded by square brackets (i.e. [v1, v2, v3, ...]).
         *
         * \param theCont The container of parameter values that is to be
         *                written.
         * \param stream The stream into which the container it to be dumped.
         */
        template <typename CType>
        static
        void
        DumpContainer(
            const CType& theCont,
            std::ostream& stream
            );


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a BasicParameterDatabaseImpl
        BasicParameterDatabaseImpl(
            );

        /// Destructs a BasicParameterDatabaseImpl
        ~BasicParameterDatabaseImpl(
            );


}; // class BasicParameterDatabaseImpl



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
#include "inline/BasicParameterDatabaseImpl.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_BASICPARAMETERDATABASEIMPL_HPP
