/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of the volume_levels logging level class.

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

        Fri Jan 28 15:40:08 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the volume_levels logging level class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_VOLUME_LEVELS_HPP
#define EDDY_LOGGING_VOLUME_LEVELS_HPP


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
 * \brief Logging levels using volume-like terminology.
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
class EDDY_SL_IEDECL volume_levels
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

        /**
         * \brief The Verbose level is meant to cause output of all entries.
         */
        static const level Verbose;

        /**
         * \brief The Loud Level is meant to cause output of all but the most
         *        insignificant of entries.
         */
        static const level Loud;

        /**
         * \brief The Normal level should be used to output entries that are
         *        of general use and should normally be output.
         */
        static const level Normal;

        /**
         * \brief The Quiet level outputs only the most serious entries.
         */
        static const level Quiet;

        /**
         * \brief The Fatal level designates a very severe error that
         *        will presumably lead the application to abort.
         */
        static const level Fatal;

        /**
         * \brief The Silent level is meant to turn logging off.
         *
         * You can still log things at this level and they will show up
         * but that is not its intent.
         */
        static const level Silent;

        /// This is the default level at which to log (= Normal).
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
            return (l==Verbose) ? static_cast<const char_type*>("Verbose"   ) :
                   (l==Loud   ) ? static_cast<const char_type*>("Loud"      ) :
                   (l==Normal ) ? static_cast<const char_type*>("Normal"    ) :
                   (l==Quiet  ) ? static_cast<const char_type*>("Quiet"     ) :
                   (l==Fatal  ) ? static_cast<const char_type*>("Fatal"     ) :
                   (l==Silent ) ? static_cast<const char_type*>("Silent"    ) :
                                  static_cast<const char_type*>("UNKNOWN"   )
                                  ;
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
            return lt == "Verbose" ? Verbose :
                   lt == "Loud"    ? Loud    :
                   lt == "Normal"  ? Normal  :
                   lt == "Quiet"   ? Quiet   :
                   lt == "Fatal"   ? Fatal   :
                   lt == "Silent"  ? Silent  :
                   Default;

        } // get_level_of(string_type)

}; // class volume_levels

// Initialize the inclass static members.
template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Verbose = 0;

template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Loud = 10;

template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Normal = 20;

template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Quiet = 30;

template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Fatal = 40;

template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Silent = UCHAR_MAX;

template <typename CharT, typename Traits>
const typename volume_levels<CharT, Traits>::level
    volume_levels<CharT, Traits>::Default = UCHAR_MAX;

// Cannot use numeric_limits because of problem with VisualAge C++ Pro. V6.
//  std::numeric_limits<typename volume_levels<CharT, Traits>::level>::max();

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
#endif // EDDY_LOGGING_VOLUME_LEVELS_HPP
