/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class FlatFileInitializer.

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

        Thu Jun 26 12:47:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the FlatFileInitializer class.
 */






/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_FLATFILEINITIALIZER_HPP
#define JEGA_ALGORITHMS_FLATFILEINITIALIZER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <vector>
#include <GeneticAlgorithmInitializer.hpp>
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
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
    }
}


/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {








/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class FlatFileInitializer;









/*
================================================================================
Class Definition
================================================================================
*/

/// Performs initialization by reading Design configurations from a file.
/**
 * This initializer attempts to read from delimited files with the following
 * format:
 * \verbatim
   dv0<delim>dv1...dvN[<delim>of0<delim>of1...ofM<delim>con0<delim>con1...conK]
   \endverbatim
 * The delimeter can be any string.  If it is the empty string, then the
 * automatic delimiter detection algorithm will be employed as described in
 * the documation for the DesignFileReader class.  It will continue to read
 * until the end of the file.  It will discard any configurations for which it
 * was unable to retrieve at least the number of design variables.  The
 * objective and constraint entries are not required but if all are present,
 * they will be recorded and the Design will be tagged as evaluated so that
 * evaluators may choose not to re-evaluate them.
 *
 * Setting the size for this initializer has the effect of requiring a minimum
 * number of Designs to create.  If this minimum number has not been created
 * once the files are all read, the rest are created using the
 * RandomUniqueInitializer.
 *
 * The delimiter and file names are extracted from the parameter database using
 * the names "method.jega.initializer_delimiter" and "method.flat_file"
 * respectively.  The delimiter is extracted as a string.  The file names can
 * come in in either of two ways.  They may be supplied as a vector of strings
 * or they/it may be supplied as a single string.  Regardless of how they are
 * supplied, each string received will be parsed into multiple file names if it
 * is tab-delimited.  If the delimiter is not supplied in the parameter
 * database, the default value as defined by DEFAULT_DELIM will be used.  If no
 * file name is supplied, a warning will be issued.  If no file names have been
 * supplied by the time initialize is called, the random unique initializer
 * will be used for all initialization.  These are required in addition to any
 * requirements of the base class.
 */
class FlatFileInitializer :
    public GeneticAlgorithmInitializer
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default delimiter.
        static const std::string DEFAULT_DELIM;

    private:

        /// String to serve as a field delimiter in the initialization files.
        std::string _delim;

        /// The names of the files in which to find the initial Designs.
        JEGA::StringSet _fileNames;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Adopts the supplied fileNames as those from which to read designs.
        /**
         * The names are not tested in any way for validity.  This method will
         * log a status message if logging is enabled.
         *
         * \param fileNames The new set of files from which to read initial
         *                  designs.
         */
        void
        SetFileNames(
            const JEGA::StringSet& fileNames
            );

        /// Sets the field delimiter to \a delim.
        /**
         * The field delimiter is what is used to parse individual values from
         * the lines in the files.  This method will log a status message if
         * logging is enabled.
         *
         * \param delim The new delimiter for values in the files.
         */
        void
        SetDelimiter(
            const std::string& delim
            );


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /// Allows immutable access to the current list of filenames.
        /**
         * \return The current set of filenames from which to load initial
         *         designs.
         */
        inline
        const JEGA::StringSet&
        GetFileNames(
            ) const;

        /// Returns the current field delimiter.
        /**
         * \return The current delimiter used to separate fields in the files.
         */
        inline
        const std::string&
        GetDelimiter(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /// Returns the proper name of this operator.
        /**
         * \return The string "flat_file".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This initializer attempts to read from a delimited file with the
            following format:
            dv0<delim>dv1...dvN[<delim>of0<delim>of1...ofM<delim>con0<delim>
            con1...conK]
            The delimeter can be any string.  If it is the empty string, then
            the automatic delimiter detection algorithm will be employed as
            described in the documation for the DesignFileReader class.  It
            will continue to read until the end of the file.  It will discard
            any configurations for which it was unable to retrieve at least the
            number of design variables. The objective and constraint entries
            are not required but if all are present, they will be recorded and
            the Design will be tagged as evaluated so that evaluators may
            choose not to re-evaluate them. Setting the size for this
            initializer has the effect of requiring a minimum number of Designs
            to create.  If this minimum number has not been created once the
            files are all read, the rest are created using the random unique
            initializer.
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        static
        const std::string&
        Description(
            );

        /**
         * \brief Returns a new instance of this operator class for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which the new initializer is to be used.
         * \return A new, default instance of a FlatFileInitializer.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            GeneticAlgorithm& algorithm
            );

        /// Adds \a fileName to the list of files to be read.
        /**
         * The file name must be unique in the current set of filenames.
         *
         * \param fileName The name of the file to add to the list.
         * \return True if the fileName is added and false otherwise.  It will
         *         not be added if it is not unique.
         */
        bool
        AddFileName(
            const std::string& fileName
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Breaks up the delimited file names from \a from
        /**
         * \a from is expected to contain one or more file names delimited by
         * the delimiter know to this initializer.
         *
         * \param from The string from which to parse filenames.
         * \return The resulting set of filenames.
         */
        JEGA::StringSet
        ParseFileNames(
            const std::string& from
            );

        /// Breaks up the delimited file names from \a from
        /**
         * \a from is expected to contain one or more strings containing one
         * or more file names delimited by a tab character.
         *
         * \param from The strings from which to parse filenames.
         * \return The resulting set of filenames.
         */
        JEGA::StringSet
        ParseFileNames(
            const JEGA::StringSet& from
            );

        /// Gets the next field of a delimited list.
        /**
         * \param from The string from which to obtain the next field.
         * \param off The location after which to begin parsing the next field.
         * \return The next delimited field found in \a from.
         */
        std::string
        GetNextField(
            const std::string& from,
            const std::string::size_type off
            ) const;

        /**
         * \brief Gets the next field of a delimited list using the supplied
         *        delimiter.
         *
         * \param from The string from which to obtain the next field.
         * \param delim The delimiter to use as the break between values in
         *              the supplied string.
         * \param off The location after which to begin parsing the next field.
         * \return The next delimited field found in \a from.
         */
        static
        std::string
        GetNextField(
            const std::string& from,
            const std::string& delim,
            const std::string::size_type off
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Performs flat file read initialization.
        /**
         * This is the method called when it is time to read the files.
         * This method reads each file and places any Design successfully
         * created into \a into.
         *
         * \param into The group into which to insert any successfully created
         *             designs read from the files.
         */
        virtual
        void
        Initialize(
            JEGA::Utilities::DesignGroup& into
            );

        /// Returns the proper name of this operator.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const;

        /// Returns a full description of what this operator does and how.
        /**
         * \return See Description().
         */
        virtual
        std::string
        GetDescription(
            ) const;

        /**
         * \brief Creates and returns a pointer to an exact duplicate of this
         *        operator.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * \param db The database of parameters from which the configuration
         *           information can be retrieved.
         * \return true if the extraction completed successfully and false
         *         otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );


    protected:


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:


        /// Writes a Log message detailing the current list of file names.
        /**
         * The format of the log entry will be:
         * \verbatim
            flat_file: Initialization file(s) now = file1, file2, ..., fileN
           \endverbatim
         */
        void
        LogFilenames(
            ) const;



    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        /// Constructs an FlatFileInitializer for use by \a algorithm.
        /**
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        FlatFileInitializer(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an FlatFileInitializer.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        FlatFileInitializer(
            const FlatFileInitializer& copy
            );

        /// Copy constructs an FlatFileInitializer for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        FlatFileInitializer(
            const FlatFileInitializer& copy,
            GeneticAlgorithm& algorithm
            );

}; // class FlatFileInitializer


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/FlatFileInitializer.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_FLATFILEINITIALIZER_HPP
