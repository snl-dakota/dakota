/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class ostream_entry.

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

        Mon Feb 07 18:42:36 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ostream_entry class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_OSTREAM_ENTRY_HPP
#define EDDY_LOGGING_OSTREAM_ENTRY_HPP





/*
================================================================================
Conditional Inclusion
================================================================================
*/
#ifdef EDDY_LOGGING_ON



/*
================================================================================
Includes
================================================================================
*/
#include <sstream>
#include "../include/config.hpp"
#include "../include/default_types.hpp"


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
namespace eddy {
    namespace logging {





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
 * \brief An entry that houses an ostringstream for use in entry formatting.
 *
 * This entry uses an ostringstream to do formatting.  It forwards anything
 * passed to it via the templatized operator << on to the ostringstream.
 * Therefore, anything "stream-insertible" can become part of the text of
 * this entry.
 *
 * As a convenience and for a minor efficiency boost, this entry has
 * constructor overloads that accept text.  The text will be used as leading
 * text into the entry.  The write position of the stream is moved beyond
 * the added text so that any further writes into the stream appear at the
 * end.
 *
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup entry_types
 */
template <
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS
         >
class EDDY_SL_IEDECL ostream_entry
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A synonym for the character type of this entry.
        typedef
        CharT
        char_type;

        /// A synonym for the character traits type.
        typedef
        Traits
        traits_type;

        /// A shorthand for the type of a string used by this class.
        typedef
        std::basic_string<char_type, traits_type>
        string_type;

        /**
         * \brief A shorthand for the type of an output string stream used by
         * this class.
         */
        typedef
        std::basic_ostringstream<
                                 char_type,
                                 traits_type,
                                 std::allocator<char_type>
                                >
        ostringstream_type;

        /**
         * \brief A shorthand for the type of an output stream used by
         * this class.
         */
        typedef
        std::basic_ostream<char_type, traits_type>
        ostream_type;

    private:

        /// A shorthand for the type of this entry class.
        typedef
        ostream_entry<char_type, traits_type>
        my_type;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The string stream into which everything is ultimately written.
        ostringstream_type _ostream;




    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Writes \a what into the string stream of this entry.
        /**
         * \param what The ostreamable item to append to this entries contents.
         * \return This entry after appending what.
         */
        template <typename T>
        inline
        my_type&
        operator << (
            const T& what
            )
        {
            return this->append(what);
        }

        /// Returns the accumulated text in the ostringstream.
        /**
         * \return The contents of this ostream entry converted to a string.
         */
        inline
        string_type
        get_message_str(
            ) const
        {
            return this->_ostream.str();
        }

        /// An operator to convert this entry into a string.
        /**
         * This allows implicit conversion of this entry to an object of
         * type string_type.
         *
         * \return The contents of this ostream entry converted to a string.
         */
        inline
        operator string_type(
            ) const
        {
            return this->get_message_str();
        }

        /// Clears out the accumulated text in the ostringstream.
        /**
         * After this, this entry will have no contents.
         *
         * \return This entry after the clearing is done.
         */
        inline
        my_type&
        clear(
            )
        {
            this->_ostream.str(string_type());
            return *this;
        }

        /// Appends \a what to the end of this entry.
        /**
         * \param what The ostreamable item to append to this entries contents.
         * \return This entry after appending what.
         */
        template <typename T>
        inline
        my_type&
        append(
            const T& what
            )
        {
            (this->_ostream << what).flush();
            return *this;
        }

        /// Allows immutable access to the ostream used by this class.
        /**
         * \return The output stream into which all items get appended when
         *         using this entry.
         */
        inline
        const ostream_type&
        get_ostream(
            ) const
        {
            return this->_ostream;
        }

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Allows mutable subclass access to the ostream used by this class.
        /**
         * \return The output stream into which all items get appended when
         *         using this entry.
         */
        inline
        ostream_type&
        get_ostream(
            )
        {
            return this->_ostream;
        }

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

        /// Default constructs a ostream_entry.
        ostream_entry(
            ) :
                _ostream()
        {
        }

        /// Constructs a ostream_entry with the supplied leading text.
        /**
         * \param lt The leading text to insert into this entry.
         */
        ostream_entry(
            const string_type& lt
            ) :
                _ostream(lt)
        {
            // move the write position to the end of the loaded buffer.
            // Otherwise, the supplied text will be overwritten.
            _ostream.seekp(0, std::ios_base::end);
        }

        /// Copy constructs a ostream_entry.
        /**
         * \param copy An existing ostream_entry from which to copy properties
         *             into this.
         */
        ostream_entry(
            const my_type& copy
            ) :
                _ostream(copy._ostream.str())
        {
            // move the write position to the end of the loaded buffer.
            // Otherwise, the supplied text will be overwritten.
            this->_ostream.seekp(0, std::ios_base::end);
        }

}; // class ostream_entry


/// Operator for insertion of an ostream_entry into an output stream.
/**
 * Simply inserts the message string into the stream.
 *
 * \param stream The stream into which to write the accumulated text
 *               of entry \a e.
 * \param e The entry whose text is to be inserted into \a stream.
 * \return \a stream after \a e has been inserted.
 */
template<typename CharT, typename Traits>
inline
std::basic_ostream<CharT, Traits>&
operator << (
             std::basic_ostream<CharT, Traits>& stream,
             const ostream_entry<CharT, Traits>& e
             )
{
    return stream << e.get_message_str();
}


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace logging
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_OSTREAM_ENTRY_HPP
