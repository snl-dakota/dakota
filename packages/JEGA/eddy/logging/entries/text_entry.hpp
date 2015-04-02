/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class text_entry.

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

        Wed Feb 09 18:42:36 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the text_entry class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_SIMPLE_ENTRY_HPP
#define EDDY_LOGGING_SIMPLE_ENTRY_HPP





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
#include <string>
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
 * \brief An entry type that keeps a string into which text can be written.
 *
 * This entry type allows appending of text in the form of strings or CharT*
 * to the underlying string via ostream like insertion (operator <<).
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
class EDDY_SL_IEDECL text_entry
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

    private:

        /// A shorthand for the type of this entry class.
        typedef
        text_entry<char_type, traits_type>
        my_type;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The string into which everything is ultimately written.
        string_type _text;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Appends \a what into the string of this entry.
        /**
         * \param what The text item to append to this entries contents.
         * \return This entry after appending what.
         */
        inline
        my_type&
        operator << (
            const string_type& what
            )
        {
            this->_text.append(what);
            return *this;
        }

        /// Appends \a what into the string of this entry.
        /**
         * \param what The text item to append to this entries contents.
         * \return This entry after appending what.
         */
        inline
        my_type&
        operator << (
            const char_type* const what
            )
        {
            this->_text.append(what);
            return *this;
        }

        /// Appends 1 of \a what into the string of this entry.
        /**
         * \param what The character to append to this entries contents.
         * \return This entry after appending what.
         */
        inline
        my_type&
        operator << (
            const char_type& what
            )
        {
            this->_text.append(1, what);
            return *this;
        }

        /// Returns the accumulated text in string.
        /**
         * \return The contents of this ostream entry as a string.
         */
        inline
        const string_type&
        get_message_str(
            ) const
        {
            return this->_text;
        }

        /// An operator to convert this entry into a string.
        /**
         * This allows implicit conversion of this entry to an object of
         * type string_type.
         *
         * \return The contents of this ostream entry converted to a string.
         */
        inline
        operator const string_type&(
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
            this->_text.clear();
            return *this;
        }

        /// Appends the supplied text to the end of this entry.
        /**
         * \param what The text item to append to this entries contents.
         * \return This entry after appending what.
         */
        inline
        my_type&
        append(
            const string_type& what
            )
        {
            this->_text.append(what);
            return *this;
        }

        /// Appends the supplied text to the end of this entry.
        /**
         * \param what The text item to append to this entries contents.
         * \return This entry after appending what.
         */
        inline
        my_type&
        append(
            const char_type* const what
            )
        {
            this->_text.append(what);
            return *this;
        }

        /// Appends the supplied character to the end of this entry.
        /**
         * \param what The character to append to this entries contents.
         * \return This entry after appending what.
         */
        inline
        my_type&
        append(
            const char_type& what
            )
        {
            return this->append(1, what);
        }

        /// Appends from \a ct copies of \a what to the end of this entry.
        /**
         * \param ct The number of copies of \a what to append to this entry.
         * \param what The character to append to \a ct copies of to this
         *             entry.
         * \return This entry after appending \a ct \a what's.
         */
        inline
        my_type&
        append(
            typename string_type::size_type ct,
            const char_type& what
            )
        {
            this->_text.append(ct, what);
            return *this;
        }

        /// Appends from \a ct elements of \a what to the end of this entry.
        /**
         * \param what The text item to append to this entry.
         * \param ct The maximum number of characters from \a what to append.
         * \return This entry after appending \a ct \a what's.
         */
        inline
        my_type&
        append(
            const char_type* const what,
            typename string_type::size_type ct
            )
        {
            this->_text.append(what, ct);
            return *this;
        }

        /// Appends from \a roff to \a ct of \a what to the end of this entry.
        /**
         * \param what The text item to append to this entry.
         * \param roff The offset distance from the beginning of \a what from
         *             which to begin collecting characters.
         * \param ct The maximum number of characters from \a what to append.
         * \return This entry after appending \a ct \a what's.
         */
        inline
        my_type&
        append(
            const string_type& what,
            typename string_type::size_type roff,
            typename string_type::size_type ct
            )
        {
            this->_text.append(what, roff, ct);
            return *this;
        }

        inline
        void
        reserve(
            const typename string_type::size_type& capacity = 0
            )
        {
            this->_text.reserve(capacity);
        }

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:



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

        /// Constructs a text_entry with the supplied leading text.
        /**
         * \param lt The leading text to insert into this entry.
         */
        text_entry(
            const char_type* const lt = ""
            ) :
                _text(lt)
        {
        }

        /// Constructs a text_entry with the supplied leading text.
        /**
         * \param lt The leading text to insert into this entry.
         */
        text_entry(
            const string_type& lt
            ) :
                _text(lt)
        {
        }

        /// Copy constructs a text_entry.
        /**
         * \param copy An existing text_entry from which to copy properties
         *             into this.
         */
        text_entry(
            const my_type& copy
            ) :
                _text(copy._text)
        {
        }



}; // class text_entry



/// Operator for insertion of a text_entry into an output stream.
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
    const text_entry<CharT, Traits>& e
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
#endif // EDDY_LOGGING_SIMPLE_ENTRY_HPP
