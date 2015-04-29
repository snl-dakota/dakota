/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DesignFileReader.

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

        Fri Mar 03 13:23:45 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DesignFileReader class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_DESIGNFILEREADER_HPP
#define JEGA_UTILITIES_DESIGNFILEREADER_HPP







/*
================================================================================
Includes
================================================================================
*/
#include <string>
#include <utility>
#include <../Utilities/include/JEGATypes.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>





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
class DesignTarget;
class DesignFileReader;







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
/// This class reads Design class objects out of a data file.
/**
 * Currently, only flat file formats are supported.  The Designs in a flat file
 * must be organized as follows:
 *
 * \verbatim
   dv0<delim>dv1...dvN[<delim>of0<delim>of1...ofM<delim>con0<delim>con1...conK]
   \endverbatim
 *
 * This class assumes that the design variable values in the file are actually
 * values as opposed to representations (see the DesignVariableInfo class for
 * a detailed description of values vs. representations).
 *
 * The delimeter is usually a single character such as a tab or a space.  The
 * public read methods each have 2 versions.  One that takes a delimiter and
 * one that doesn't.  If a delimiter is supplied, the fields must all be
 * separated by exactly that delimiter on all lines.  If one is not supplied
 * the automatic delimiter detection algorithm is employed on a line-by-line
 * basis.  In that case, the delimiter on any given line can be any string that
 * does not contain any digits or any of the characters .+-dDeE (leading and
 * trailing whitespace is also ignored on those delimiters that are not all
 * whitespace).  The delimiters from line to line can vary but the delimiter
 * used on any given line must be consistent with the exception of additional
 * leading and trailing whitespace.
 *
 * The reader will continue to read until the end of the file.  It will discard
 * any configurations for which it was unable to retrieve at least the
 * number of design variables.  The objective and constraint entries are
 * not required but if ALL are present, they will be recorded and the Design
 * will be tagged as evaluated so that evaluators may choose not to re-evaluate
 * them.
 */
class DesignFileReader
{
    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    public:

        /**
         * \brief A class to store and return the results of reading Designs
         *        out of a file.
         *
         * This class has the ability to store various statistics about what
         * was encountered in a file when attempting to read designs.
         */
        class Result
        {
            /*
            ====================================================================
            Friend Declarations
            ====================================================================
            */
            /**
             * \brief The DesignFileReader is a friend so that it can directly
             *        manipulate the member data.
             */
            friend class DesignFileReader;

            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            private:

                /// The name of the file used to create these results.
                std::string _filename;

                /// The Designs read in.
                DesignDVSortSet _designs;

                /// The number of full Designs read in.
                /**
                 * A full Design means that objectives and constraints were all
                 * found in addition to design variables.  Such a design will
                 * be labeled evaluated.
                 */
                eddy::utilities::uint64_t _fullReads;

                /// The number of partial Designs read in.
                /**
                 * This is the number of designs for which all design variables
                 * were found but not enough objectives and/or constraints.
                 * Such a Design will not be labeled evaluated.
                 */
                eddy::utilities::uint64_t _partReads;

                /// The number of attempts at reading a Design from the file.
                /**
                 * Every line in a file is attempted.  So if you have a header
                 * line for example without numbers or whatever, this class
                 * will attempt to interpret that line as a Design.  That is
                 * not a problem, it will simply fail.  Empty lines do not
                 * count as attempts.  This variable counts all attempts.
                 */
                eddy::utilities::uint64_t _attempts;

                /**
                 * \brief The number of non-empty lines that failed to produce
                 *        a usable design.
                 *
                 * This is for when a line has either non-numeric entries or
                 * not enough entries for to account for all the design
                 * variables.
                 */
                eddy::utilities::uint64_t _failures;

                /// The number of empty lines found and skipped.
                eddy::utilities::uint64_t _emptyLines;

                /**
                 * \brief A flag that indicates that a catestrophic error has
                 *        occured.
                 *
                 * An example of such an error is the file not opening.
                 */
                bool _error;

            /*
            ====================================================================
            Accessors
            ====================================================================
            */
            public:

                /**
                 * \brief Returns the name of the file for the read whose
                 *        results this is.
                 *
                 * \return The file name from which Designs were read.
                 */
                inline
                const std::string&
                GetFilename(
                    ) const;

                /// Returns the Designs that were read in and stored.
                /**
                 * \return The set of read in Designs.
                 */
                inline
                const DesignDVSortSet&
                GetDesigns(
                    ) const;

                /// Returns the number of full Design reads completed.
                /**
                 * \return The number of Designs that were read complete with
                 *         all responses.
                 */
                inline
                eddy::utilities::uint64_t
                GetNumFullReads(
                    ) const;

                /// Returns the number of partial Design reads completed.
                /**
                 * \return The number of Designs for which enough variables
                 *         were found but for which not enough responses were
                 *         found.
                 */
                inline
                eddy::utilities::uint64_t
                GetNumPartialReads(
                    ) const;

                /// Returns the number of attempts at reading a Design.
                /**
                 * \return The number of lines that contained information that
                 *         could not be construed as a Design configuration.
                 */
                inline
                eddy::utilities::uint64_t
                GetTotalAttemptedReads(
                    ) const;

                /// Returns the number of failed Design reads.
                /**
                 * \return The number of lines that contained too little
                 *         information from which to construct a Design.
                 */
                inline
                eddy::utilities::uint64_t
                GetNumFailures(
                    ) const;

                /// Returns the number of empty lines encountered.
                /**
                 * \return The number of blank lines found in the file.
                 */
                inline
                eddy::utilities::uint64_t
                GetNumEmptyLines(
                    ) const;

                /**
                 * \brief Returns whether or not a catastrophic error was
                 *        encountered.
                 *
                 * An example of a catastrophic error would be if the file
                 * could not be opened.
                 *
                 * \return true if a catastrophic error was encountered and
                 *         false otherwise.
                 */
                inline
                bool
                GetErrorFlag(
                    ) const;

                /// Returns the sum of partial and full Design reads.
                /**
                 * \return The total number of Designs created either with or
                 *         without responses.
                 */
                inline
                eddy::utilities::uint64_t
                GetTotalReads(
                    ) const;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /**
                 * \brief Returns a message that can be displayed detailing the
                 *        results of a read.
                 *
                 * \return A string reading:
                 * \verbatim
                    Read File: GetFilename()
                    Catastrophic Error: GetErrorFlag()
                    Total Attempts: GetTotalAttemptedReads()
                    Full Design Reads: GetNumFullReads()
                    Partial Design Reads: GetNumPartialReads()
                    Failed Reads: GetNumFailures()
                    Empty Lines: GetNumEmptyLines()
                   \endverbatim
                 */
                std::string
                GetResultsString(
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Default constructs a read results object.
                Result(
                    );

                /// Copy constructs a read results object.
                /**
                 * \param copy An existing Result object to copy properties
                 *        from.
                 */
                Result(
                    const Result& copy
                    );

                /// Destructs a read results object.
                ~Result(
                    );

        }; // class Result


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

        /// A convenience string that contains all whitespace characters.
        /**
         * The contained characters are 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20 in
         * that order.
         */
        static const std::string ALL_WHITESPACE;

        /**
         * \brief A tag to indicate to the file reader that an automatically
         *        determined delimiter is nothing but whitespace.
         *
         * This actually reads: d__WHITE__d.  The d's are in there to make
         * sure that this is not a legal automatic delimiter.
         */
        static const std::string WHITESPACE_DELIMITER;



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

        /// Reads Designs from the supplied \a file.
        /**
         * \a file is expected to be an ASCII file with Designs in it that
         * conform to the configuration described by \a target.  The format of
         * information is expected to be as described in the documentation of
         * this class type.
         *
         * \param file The file from which to read Designs.
         * \param delim The delimeter used to separate values.
         * \param target The target describing the expected configuration of
         *               Designs and used to create new Design class objects.
         * \return A fully loaded Result class object describing the results of
         *         the file read.
         */
        static
        Result
        ReadFlatFile(
            const std::string& file,
            const std::string& delim,
            const DesignTarget& target
            );

        /// Reads Designs from the supplied \a file.
        /**
         * \a file is expected to be an ASCII file with Designs in it that
         * conform to the configuration described by \a target.  The format of
         * information is expected to be as described in the documentation of
         * this class type.  This version will cause the use of the automatic
         * delimiter detection algorithm.
         *
         * \param file The file from which to read Designs.
         * \param target The target describing the expected configuration of
         *               Designs and used to create new Design class objects.
         * \return A fully loaded Result class object describing the results of
         *         the file read.
         */
        static
        Result
        ReadFlatFile(
            const std::string& file,
            const DesignTarget& target
            );


    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Records constraint violations of \a des with \a target
        /**
         * \param des The design whose constraint violations are to be
         *            recorded.
         * \param target The target into which to record constraint violations.
         */
        static
        void
        RecordViolations(
            const Design& des,
            const DesignTarget& target
            );

        /// Interprets as many delimited doubles from \a from as possible.
        /**
         * Stores the results in \a into.  into.size() will indicate the number
         * of successes after this method call.
         *
         * \param from The unparsed string of textual representations of values
         *             separated by the supplied delimeter.
         * \param delim The delimeter separating values in \a from.
         * \param into The vector into which to store parsed values.  This
         *             vector is initially cleared by this method.
         */
        static
        void
        ParseValues(
            const std::string& from,
            const std::string& delim,
            JEGA::DoubleVector& into
            );

        /// Interprets as many delimited doubles from \a from as possible.
        /**
         * Stores the results in \a into.  into.size() will indicate the number
         * of successes after this method call.  This version will cause the
         * use of the automatic delimiter detection algorithm.
         *
         * \param from The unparsed string of textual representations of values
         *             separated by the supplied delimeter.
         * \param into The vector into which to store parsed values.  This
         *             vector is initially cleared by this method.
         */
        static
        std::string
        ParseValues(
            const std::string& from,
            JEGA::DoubleVector& into
            );

        /// Records values from \a from into \a des.
        /**
         * Interprets the values in \a from as design variable values, followed
         * by objective function values, folowed by constraint values for a
         * Design.
         *
         * \param from The vector of values to copy into \a into.
         * \param into The Design into which to record the supplied values in
         *             the described order.
         */
        static
        void
        RecordValues(
            const JEGA::DoubleVector& from,
            JEGA::Utilities::Design& into
            );

        /// Gets the next field of a delimited list.
        /**
         * \param from The unparsed string of textual representations of values
         *             separated by the supplied delimeter.
         * \param delim The delimeter separating values in \a from.
         * \param off The offset amount at which to start looking for the next
         *            field.
         */
        static
        std::string
        GetNextField(
            const std::string& from,
            const std::string& delim,
            std::string::size_type& off
            );

        /**
         * \brief Attempts to interpret the supplied string as a list of
         *        delimited values and extract and return the delimiter.
         *
         * This works by repeatedly finding the next "candiatate" delimiter
         * and comparing it to the existing candidate delimiter.  If there
         * is dissagreement at any point, the method fails and returns an
         * empty string.
         *
         * \param from The string that is supposed to be a list of delimited
         *             numeric values.
         * \return The delimiter used to separate the values in the string
         *         or an empty string if one cannot be determined.
         */
        static
        std::string
        DetermineDelimeter(
            const std::string& from
#ifdef JEGA_LOGGING_ON
            , const std::string& prev
#endif
            );

        /**
         * \brief A convenience method to convert the supplied string to a
         *        value of type double.
         *
         * \param str The string to attempt to parse as a double.
         * \return A pair in which the first value is boolean and indicates
         *         whether or not the parse succeeded and in which the second
         *         value is a double and is the value that was parsed out of
         *         the string.  Note that the double value is only meaningful
         *         if the boolean is true.
         */
        static
        std::pair<bool, double>
        ToDouble(
            const std::string& str
            );

        /// Finds and returns the next possible delimiter in the input string.
        /**
         * This is carried out by skipping a number and then recording
         * everything between the end of the found number and the beginning of
         * the next number as the candidate delimiter.
         *
         * This method is used repeatedly during the course of reading a line
         * of input.  The results of the calls to this method are compared in
         * an attempt to determine the actual delimiter used on the line.
         *
         * \param in The string in which to try to find a candidate delimiter.
         * \param off The current location in \a in at which to begin looking.
         * \return The identified candidate delimiter or an empty string if one
         *         could not be found.
         */
        static
        std::string
        FindCandidateDelimiter(
            const std::string& in,
            std::string::size_type& off
            );

        /// Takes a potential delimiter and does required formatting.
        /**
         * This will identify an all-whitespace delimiter and replace it with
         * the whitespace delimiter sentinal value and will otherwise strip
         * the leading and trailing whitespace off.
         *
         * \param unformatted The candidate delimiter to format.
         * \return The formatted delimiter.
         */
        static
        std::string
        FormatDelimiter(
            const std::string& unformatted
            );

        /**
         * \brief Tests to see if all characters in the supplied string are
         *        white characters.
         *
         * The ANSI isspace method is what is used to determine what is white
         * and what is not.
         *
         * \param str The string to test to see if it is all white.
         * \return True if \a str contains nothing but whitespace and false
         *         otherwise.
         */
        static
        bool
        IsAllWhite(
            const std::string& str
            );

        /**
         * \brief Returns a string that is a duplicate of the supplied string
         *        with all leading and trailing whitespace removed.
         *
         * \param str The string a version of which with no leading or trailing
         *            whitespace is desired.
         * \return A duplicate of \a str with no leading or trailing
         *         whitespace.
         */
        static
        std::string
        TrimWhitespace(
            const std::string& str
            );



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

        /// A function used to initialize the static variable ALL_WHITESPACE.
        /**
         * This method is intended to be used once by the constructor of the
         * ALL_WHITESPACE variable.  After that, you have the ALL_WHITESPACE
         * variable to use and don't need this method.
         *
         * \return A string whose characters are the whitespace characters.
         */
        static
        std::string
        InitializeWhiteString(
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    private:

        /// This constructor is private and has no implementation.
        /**
         * All methods of this class are static and thus this class should not
         * and cannot be instantiated.
         */
        DesignFileReader(
            );



}; // class DesignFileReader



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
#include "inline/DesignFileReader.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_DESIGNFILEREADER_HPP
