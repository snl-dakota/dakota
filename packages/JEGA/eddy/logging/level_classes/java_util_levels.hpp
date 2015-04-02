/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of the java util based logging level class.

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
 * \brief Contains the definition of the java util based logging level class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_JAVA_UTIL_LEVELS_HPP
#define EDDY_LOGGING_JAVA_UTIL_LEVELS_HPP


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
 * \brief Logging levels modeled after the java.util.logging.Levels class.
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
class EDDY_SL_IEDECL java_util_levels
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
         * \brief A message level indicating a highly detailed tracing message.
         */
        static const level All;

        /**
         * \brief A message level indicating a highly detailed tracing message.
         */
        static const level Finest;

        /**
         * \brief A message level indicating a fairly detailed tracing message.
         */
        static const level Finer;

        /**
         * \brief A message level providing tracing information.
         */
        static const level Fine;

        /**
         * \brief A message level for static configuration messages.
         */
        static const level Config;

        /**
         * \brief A message level for informational messages.
         */
        static const level Info;

        /**
         * \brief A a message level indicating a potential problem.
         */
        static const level Warning;

        /**
         * \brief A message level indicating a serious failure.
         */
        static const level Severe;

        /**
         * \brief A special level that can be used to turn off logging.
         */
        static const level Off;

        /// This is the default level at which to log.
        static const level Default;;

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
            return (l==Finest ) ? static_cast<const char_type*>("Finest" ) :
                   (l==Finer  ) ? static_cast<const char_type*>("Finer"  ) :
                   (l==Fine   ) ? static_cast<const char_type*>("Fine"   ) :
                   (l==Config ) ? static_cast<const char_type*>("Config" ) :
                   (l==Info   ) ? static_cast<const char_type*>("Info"   ) :
                   (l==Warning) ? static_cast<const char_type*>("Warning") :
                   (l==Severe ) ? static_cast<const char_type*>("Severe" ) :
                   (l==All    ) ? static_cast<const char_type*>("All"    ) :
                   (l==Off    ) ? static_cast<const char_type*>("Off"    ) :
                                  static_cast<const char_type*>("UNKNOWN")
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
            return lt == "Finest"  ? Finest  :
                   lt == "Finer"   ? Finer   :
                   lt == "Fine"    ? Fine    :
                   lt == "Config"  ? Config  :
                   lt == "Info"    ? Info    :
                   lt == "Warning" ? Warning :
                   lt == "Severe"  ? Severe  :
                   lt == "All"     ? All     :
                   lt == "Off"     ? Off     :
                   Default;

        } // get_level_of(string_type)

}; // class java_util_levels

// Initialize the inclass static members
template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::All = 0;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Finest = 10;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Finer = 20;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Fine = 30;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Config = 40;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Info = 50;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Warning = 60;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Severe = 70;

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Off = UCHAR_MAX;

// Cannot use numeric_limits because of problem with VisualAge C++ Pro. V6.
//        std::numeric_limits<
//            typename java_util_levels<CharT, Traits>::level
//            >::max();

template <typename CharT, typename Traits>
const typename java_util_levels<CharT, Traits>::level
    java_util_levels<CharT, Traits>::Default = 40;
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
#endif // EDDY_LOGGING_JAVA_UTIL_LEVELS_HPP
