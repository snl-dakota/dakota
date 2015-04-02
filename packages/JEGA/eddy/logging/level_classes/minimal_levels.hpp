/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of the minumum logging level class.

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

        Mon Feb 07 15:40:08 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the minumum logging level class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_MINIMAL_HPP
#define EDDY_LOGGING_MINIMAL_HPP


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
#include "../include/config.hpp"
#include "../include/default_types.hpp"




/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace logging {





/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A policy with the minimal allowable set of levels.
 *
 * There is a requirement that all level policies have a Default member and
 * a to_ostreamable method.  That is all that is present in this class with the
 * exception of typedefs.
 *
 * In general, other level types including those derived from this will
 * implement their own version of a Default instance.
 *
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup level_class_types
 */
template <
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS
          >
class EDDY_SL_IEDECL minimal_levels
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the levels for this class.
        typedef
        unsigned char
        level;

        /// A synonym for the character type of this file log.
        typedef
        CharT
        char_type;

        /// A synonym for the character traits type.
        typedef
        Traits
        traits_type;

    /*
    ============================================================================
    Class Constants
    ============================================================================
    */
    public:

        /// This is the default level at which to log.
        static const level Default;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief A required method that will return an ostreamable
         *        representation of a level.
         *
         * This version returns a const char_type* representation of the
         * supplied level.
         *
         * The only potential restriction on the return type other than that
         * it be ostreamable may be imposed by an entry into which the text
         * is being written.  Some entries such as the text_entry's and
         * format_entry's support a limited version of stream insertion.
         *
         * \param l The level of which to return an ostream insertible
                    representation.
         * \return A string literal name for the supplied level.
         */
        static
        const char_type*
        to_ostreamable(
            const level& l
            )
        {
            return (l==Default) ? static_cast<const char_type*>("Default")  :
                                  static_cast<const char_type*>("UNKNOWN")
                                  ;
        } // to_ostreamable

        /**
         * \brief A method that will return the level associated with the
         *        supplied text.
         *
         * This will return the value Default no matter what..
         *
         * \param lt The text representation of the level of interest.
         * \return The Default level.
         */
        static
        const level&
        get_level_of(
            const string_type& lt
            )
        {
            return Default;

        } // get_level_of(string_type)

}; // class minimal_levels

// Initialize the inclass static members
template <typename CharT, typename Traits>
const typename minimal_levels<CharT, Traits>::level
    minimal_levels<CharT, Traits>::Default = 50;




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace logging
} // namespace eddy



/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif // EDDY_LOGGING_ON


/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_MINIMAL_HPP
