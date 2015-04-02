/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of the dakota_levels logging level class.

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

        Wed Jan 04 15:40:08 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the dakota_levels logging level class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_DAKOTA_LEVELS_HPP
#define EDDY_LOGGING_DAKOTA_LEVELS_HPP


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
// Cannot use numeric_limits because of problem with VisualAge C++ Pro. V6.
// #include <limits>
#include <climits>
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
 * \brief Logging levels using terminology adopted from the Dakota software.
 *
 * See the description of Dakota at http://www.cs.sandia.gov/DAKOTA.
 * In particular, in the top level src directory, view the file
 * dakota.input.txt and notice the input specification for "output".
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
class EDDY_SL_IEDECL dakota_levels
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

        /// A shorthand for the type of a string used by this class.
        typedef
        std::basic_string<char_type, traits_type>
        string_type;

    /*
    ============================================================================
    Class Constants
    ============================================================================
    */
    public:

        /**
         * \brief The debug level is meant to cause output of all entries.
         */
        static const level debug;

        /**
         * \brief The verbose level is meant to cause output of all but the
         *        most insignificant of entries.
         */
        static const level verbose;

        /**
         * \brief The normal level outputs informational entries or above.
         */
        static const level normal;

        /**
         * \brief The quiet level outputs only entries that are warning level
         *        or above.
         */
        static const level quiet;

        /**
         * \brief The silent level is meant to turn logging virtually off.
         *
         * The only log entries that will appear with this level are fatal
         * and if you choose to log entries at silent, those.  That is not
         * the intent of this level however.
         */
        static const level silent;

        /**
         * \brief The fatal level designates a very severe error that
         *        will presumably lead the application to abort.
         */
        static const level fatal;

        /// This is the default level at which to log (= quiet).
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
            return (l==quiet  ) ? static_cast<const char_type*>("quiet")   :
                   (l==verbose) ? static_cast<const char_type*>("verbose") :
                   (l==normal ) ? static_cast<const char_type*>("normal")  :
                   (l==silent ) ? static_cast<const char_type*>("silent")  :
                   (l==fatal  ) ? static_cast<const char_type*>("fatal")   :
                   (l==debug  ) ? static_cast<const char_type*>("debug")   :
                                  static_cast<const char_type*>("UNKNOWN");

        } // to_ostreamable

        /**
         * \brief A method that will return the level associated with the
         *        supplied text.
         *
         * This will return the level associated with the supplied level name.
         * If that name is not correlated to a level, the default level
         * is returned.
         *
         * \param lt The text representation of the level of interest.
         * \return The level associated with the supplied text representation.
         */
        static
        const level&
        get_level_of(
            const string_type& lt
            )
        {
            return lt == "debug" ? debug :
                   lt == "verbose" ? verbose :
                   lt == "normal" ? normal :
                   lt == "quiet" ? quiet :
                   lt == "silent" ? silent :
                   lt == "fatal" ? fatal :
                   Default;

        } // get_level_of(string_type)

}; // class dakota_levels

// Initialize the inclass static members.
template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::debug = 0;

template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::verbose = 1;

template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::normal = 2;

template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::quiet = 3;

template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::silent = 4;

template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::fatal = UCHAR_MAX;

// Cannot use numeric_limits because of problem with VisualAge C++ Pro. V6.
//        std::numeric_limits<
//            typename dakota_levels<CharT, Traits>::level
//            >::max();

template <typename CharT, typename Traits>
const typename dakota_levels<CharT, Traits>::level
    dakota_levels<CharT, Traits>::Default = 2;

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
#endif // EDDY_LOGGING_DAKOTA_LEVELS_HPP
