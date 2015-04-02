/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of the Cougaar based levels logging level class.

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
 * \brief Contains the definition of the default levels logging level class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_COUGAAR_LEVELS_HPP
#define EDDY_LOGGING_COUGAAR_LEVELS_HPP


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
#include "log4j_levels.hpp"
#include "../include/config.hpp"




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
 * \brief The Cougaar based logging levels.
 *
 * <a href=http://www.cougaar.org>Cougaar</a> (The Cognitive Agent
 * Architecture) uses a logging level scheme based on the log4j scheme.  Two
 * additions are the levels Detail and Shout.
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
class EDDY_SL_IEDECL cougaar_levels :
    public log4j_levels<CharT, Traits>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    protected:

        /// A shorthand for the base level type of this class.
        typedef
        log4j_levels<CharT, Traits>
        base_levels;

    public:

        /// The type of the levels for this class.
        typedef
        typename base_levels::level
        level;

        /// A synonym for the character type of this file log.
        typedef
        typename base_levels::char_type
        char_type;

        /// A synonym for the character traits type.
        typedef
        typename base_levels::traits_type
        traits_type;

    /*
    ============================================================================
    Class Constants
    ============================================================================
    */
    public:

        /**
         * \brief A level for which every little message of any kind will
         *        be reported.
         */
        static const level Detail;

        /**
         * \brief A level one step below Fatal that is generally reserved
         *        for pretty serious problems.
         */
        static const level Shout;

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
            return (l==Detail) ? static_cast<const char_type*>("Detail") :
                   (l==Shout ) ? static_cast<const char_type*>("Shout")  :
                                 base_levels::to_ostreamable(l)
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
            return lt == "Detail"   ? Detail    :
                   lt == "Shout"    ? Shout     :
                   Default;

        } // get_level_of(string_type)


}; // class cougaar_levels

// Initialize the inclass static members
template <typename CharT, typename Traits>
const typename cougaar_levels<CharT, Traits>::level
    cougaar_levels<CharT, Traits>::Detail = 5;

template <typename CharT, typename Traits>
const typename cougaar_levels<CharT, Traits>::level
    cougaar_levels<CharT, Traits>::Shout = 65;

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
#endif // EDDY_LOGGING_COUGAAR_LEVELS_HPP
