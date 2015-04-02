/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of the log4j based logging level class.

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
 * \brief Contains the definition of the log4j based logging level class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_LOG4J_LEVELS_HPP
#define EDDY_LOGGING_LOG4J_LEVELS_HPP


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
 * \brief Logging levels modeled after the org.apache.log4j.Level class.
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
class EDDY_SL_IEDECL log4j_levels
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
         * \brief The All has the lowest possible rank and is intended to turn
         *        on all logging.
         */
        static const level All;

        /**
         * \brief The Debug Level designates fine-grained informational events
         *        that are most useful to debug an application
         */
        static const level Debug;

        /**
         * \brief The Info level designates informational messages that
         *        highlight the progress of the application at coarse-grained
         *        level.
         */
        static const level Info;

        /**
         * \brief The Warn level designates potentially harmful situations.
         */
        static const level Warn;

        /**
         * \brief The Error level designates error events that might still
         *        allow the application to continue running.
         */
        static const level Error;

        /**
         * \brief The Fatal level designates a very severe error events that
         *        will presumably lead the application to abort.
         */
        static const level Fatal;

        /**
         * \brief The Off has the highest possible rank and is intended to turn
         *        off logging.
         */
        static const level Off;

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
            return (l==Debug) ? static_cast<const char_type*>("Debug") :
                   (l==Info ) ? static_cast<const char_type*>("Info")  :
                   (l==Warn ) ? static_cast<const char_type*>("Warn")  :
                   (l==Error) ? static_cast<const char_type*>("Error") :
                   (l==Fatal) ? static_cast<const char_type*>("Fatal") :
                   (l==All  ) ? static_cast<const char_type*>("All")   :
                   (l==Off  ) ? static_cast<const char_type*>("Off")   :
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
            return lt == "Debug" ? Debug :
                   lt == "Info"  ? Info  :
                   lt == "Warn"  ? Warn  :
                   lt == "Error" ? Error :
                   lt == "Fatal" ? Fatal :
                   lt == "All"   ? All   :
                   lt == "Off"   ? Off   :
                   Default;

        } // get_level_of(string_type)

}; // class log4j_levels

// Initialize the inclass static members
template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::All = 0;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Debug = 10;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Info = 20;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Warn = 30;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Error = 40;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Fatal = 50;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Default = 30;

template <typename CharT, typename Traits>
const typename log4j_levels<CharT, Traits>::level
    log4j_levels<CharT, Traits>::Off = UCHAR_MAX;

// Cannot use numeric_limits because of problem with VisualAge C++ Pro. V6.
//  std::numeric_limits<typename log4j_levels<CharT, Traits>::level>::max();

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
#endif // EDDY_LOGGING_LOG4J_LEVELS_HPP
