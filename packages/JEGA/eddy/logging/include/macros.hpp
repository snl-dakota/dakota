/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of logging project macros.

    NOTES:



    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Feb 03 12:22:09 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains various macros for use with the logging project.
 */



/*
================================================================================
Create a Group for All End-User Macros
================================================================================
*/
/**
 * \defgroup user_macros End User-Macro Interface
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_MACROS_HPP
#define EDDY_LOGGING_MACROS_HPP




/*
================================================================================
Includes
================================================================================
*/
#include "../detail/macros.hpp"
#include "config.hpp"





/*
================================================================================
Conditional Inclusion
================================================================================
*/
#ifdef EDDY_LOGGING_ON



/*
================================================================================
Contitional Includes
================================================================================
*/







/*
================================================================================
Macro Definitions
================================================================================
*/

/// A macro to execute the statement \a a if EDDY_LOGGING_ON is defined.
/**
 * If EDDY_LOGGING_ON is defined, the following macro expands to exactly \a a.
 * Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_LOGGING_IF_ON(my_log_t alog());
   \endcode
 * would become:
 * \code
        my_log_t alog();
   \endcode
 * if EDDY_LOGGING_ON were defined and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_IF_ON(a) a

/// Expands to nothing b/c EDDY_LOGGING_ON is defined.
/**
 * This is the exact opposite of EDDY_LOGGING_IF_ON
 *
 * \param a The statement to conditionally create.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_IF_OFF(a)

/// A macro to execute a method on an object of non-pointer type.
/**
 * This macro is intended to aid in calling various functions on a log_gateway
 * but could be used on anything.  It expands to a call on \a tlg of \a method
 * using params if EDDY_LOGGING_ON is defined.  Otherwise it expands to
 * nothing.
 *
 * If \a method requires a template list of more than one argument, use the
 * EDDY_LOGGING_TPL macro to create the list.  The parameters must be
 * surrounded by parenthesis and therefore require no special treatment
 * (can have a comma separated list).
 *
 * Using this macro will cause the line:
 * \code
        EDDY_LOGGING_ON_LOG(mylog, set_default_level, (level_p::L1));
 * \endcode
 * to become:
 * \code
        mylog.set_default_level(level_p::L1);
   \endcode
 *
 * Note that to execute the log_gateway::log methods, you should use the
 * EDDY_LOGGING_LOG* macros as they can be much more efficient for that
 * purpose.
 *
 * \param tlg The object on which to execute \a method using params.
 * \param method The method to execute on \a tlg using params.
 * \param params The parenthesized, comma-separated list of \a method
 *               arguments.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_ON_LM(tlg, method, params) tlg.method(params)

/// Logs entry \a e using log gateway \a tlg.
/**
 * This version does no level checking or anything else.  It simply calls
 * the log method on \a tlg with \a e as an argument.  Therefore this macro is
 * appropriate for use with the basic log gateway type or with any gateway
 * so long as you don't wish to do any testing.
 *
 * THIS VERSION WILL REQUIRE CREATION OF \a e NO MATTER WHAT!!
 *
 * \param tlg The object on which to execute log using params.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LOG(tlg, e) tlg.log(e);

/// Logs entry \a e using log gateway \a tlg if cond evaluates to true.
/**
 * This version uses EDDY_LOGGING_LOG if cond evaluates to true.  cond
 * is always evaluated so long as EDDY_LOGGING_ON is defined.
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The object on which to execute log using params.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_IFLOG(cond, tlg, e)                                    \
            { if(cond) EDDY_LOGGING_LOG(tlg, e) }

/**
 * \brief Logs entry \a e at level \a ll checked against the default with log
 *        gateway \a tlg
 *
 * This version is intended specifically for use with a level gateway.  It will
 * work for any gateway type that has a log method taking two parameters.  The
 * _D is meant to indicate that the log gateways default level is to be
 * checked against \a ll for logging.
 *
 * This version will not create \a e if \a ll is not sufficient to log at the
 * gateways default level.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_LOG_D(my_lg, my_levels::Debug, "Hi")
    }
   \endcode
 *
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_LOG_D(tlg, ll, e)                                \
            { if(tlg.will_log(tlg.get_default_level(), ll))                 \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll checked against the default with log
 *        gateway \a tlg if cond evaluates to true.
 *
 * This version is the same as EDDY_LOGGING_LEVEL_LOG_D with the additional
 * requirement that \a cond evaluates to true.  The level test is performed
 * prior to the condition test in this version.  Use it if your condition test
 * is or may be expensive.  For an alternate that performs the condition test
 * first see the EDDY_LOGGING_LEVEL_IFLOG_CF_D macro.
 *
 * Because the level test is performed first, the condition may not be evaluated
 * if not needed.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_IFLOG_D(i==10, my_lg, my_levels::Debug, "Hi")
    }
   \endcode
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_IFLOG_D(cond, tlg, ll, e)                        \
            { if(tlg.will_log(tlg.get_default_level(), ll) && (cond))       \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll checked against the default with log
 *        gateway \a tlg if cond evaluates to true.
 *
 * This version is the same as EDDY_LOGGING_LEVEL_LOG_D with the additional
 * requirement that \a cond evaluates to true.  The condition test is performed
 * prior to the level test in this version.  Use it if your condition tests are
 * very cheap.  For an alternate that performs the level test first see
 * the EDDY_LOGGING_LEVEL_IFLOG_D macro.
 *
 * The condition will be evaluated by this version no matter what.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_IFLOG_CF_D(i==10, my_lg, my_levels::Debug, "Hi")
    }
   \endcode
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_IFLOG_CF_D(cond, tlg, ll, e)                     \
            { if((cond) && tlg.will_log(tlg.get_default_level(), ll))       \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll if the level for issuer \a ii allows.
 *
 * This version is intended for use with a level log gateway.  It will test
 * to see if the logging will occur using the supplied logging level \a ll and
 * the level at which the issuer instance \a ii is allowed to log.  If the
 * logging will not occur based on the levels, then the entry \a e is never
 * constructed (assuming it was not constructed prior to usage of this macro).
 * The _II is intended to indicate that this is for use with an issuer
 * instance.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_LOG_II(my_lg, my_levels::Debug, this, "Hi")
    }
   \endcode
 *
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param ii The issuer instance presumably responsible for entry \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_LOG_II(tlg, ll, ii, e)                           \
            { if(tlg.will_log(tlg.get_logging_level(ii), ll))               \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll if the level for issuer \a ii allows
 *        and cond evaluates to true.
 *
 * This version is the same as EDDY_LOGGING_LEVEL_LOG_II with the additional
 * requirement that \a cond evaluates to true.  The level test is performed
 * prior to the condition test in this version.  Use it if your condition test
 * is or may be expensive.  For an alternate that performs the condition test
 * first see the EDDY_LOGGING_LEVEL_IFLOG_CF_II macro.
 *
 * Because the level test is performed first, the condition may not be evaluated
 * if not needed.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_IFLOG_II(i==10, my_lg, my_levels::Debug, this, "Hi")
    }
   \endcode
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The log gateway to use to log \a e at level \a ll.S
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param ii The issuer instance presumably responsible for entry \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_IFLOG_II(cond, tlg, ll, ii, e)                   \
            { if(tlg.will_log(tlg.get_logging_level(ii), ll) && (cond))     \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll if the level for issuer \a ii allows
 *        and cond evaluates to true.
 *
 * This version is the same as EDDY_LOGGING_LEVEL_LOG_II with the additional
 * requirement that \a cond evaluates to true.   The condition test is performed
 * prior to the level test in this version.  Use it if your condition tests are
 * very cheap.  For an alternate that performs the level test first see
 * see the EDDY_LOGGING_LEVEL_IFLOG_II macro.
 *
 * The condition will be evaluated by this version no matter what.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_IFLOG_CF_II(
            i==10, my_lg, my_levels::Debug, this, "Hi"
            )
    }
   \endcode
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The log gateway to use to log \a e at level \a ll.S
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param ii The issuer instance presumably responsible for entry \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_IFLOG_CF_II(cond, tlg, ll, ii, e)                \
            { if((cond) && tlg.will_log(tlg.get_logging_level(ii), ll))     \
                    tlg.simple_log(ll, e); }

/// Logs entry \a e at level \a ll if the level for issuer type \a it allows.
/**
 * This version is intended for use with a level log gateway.  It will test
 * to see if the logging will occur using the supplied logging level \a ll and
 * the level at which the issuer type \a it is allowed to log.  If the logging
 * will not occur based on the levels, then the entry \a e is never constructed
 * (assuming it was not constructed prior to usage of this macro).  The
 * _IT is intended to indicate that this is for use with an issuer type.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_LOG_IT(my_lg, my_levels::Debug, MyClass, "Hi")
    }
   \endcode
 *
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param it The issuer type presumably responsible for entry \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_LOG_IT(tlg, ll, it, e)                           \
            { if(tlg.will_log(tlg.get_logging_level<it>(), ll))             \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll if the level for issuer type \a it
 *        allows.
 *
 * This version is the same as EDDY_LOGGING_LEVEL_LOG_IT with the additional
 * requirement that \a cond evaluates to true.  The level test is performed
 * prior to the condition test in this version.  Use it if your condition test
 * is or may be expensive.  For an alternate that performs the condition test
 * first see the EDDY_LOGGING_LEVEL_IFLOG_CF_IT macro.
 *
 * Because the level test is performed first, the condition may not be evaluated
 * if not needed.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_IFLOG_IT(
            i==10, my_lg, my_levels::Debug, MyClass, "Hi"
            )
    }
   \endcode
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param it The issuer type presumably responsible for entry \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_IFLOG_IT(cond, tlg, ll, it, e)                   \
            { if(tlg.will_log(tlg.get_logging_level<it>(), ll) && (cond))   \
                    tlg.simple_log(ll, e); }

/**
 * \brief Logs entry \a e at level \a ll if the level for issuer type \a it
 *        allows.
 *
 * This version is the same as EDDY_LOGGING_LEVEL_LOG_IT with the additional
 * requirement that \a cond evaluates to true.  The condition test is performed
 * prior to the level test in this version.  Use it if your condition tests are
 * very cheap.  For an alternate that performs the level test first see
 * see the EDDY_LOGGING_LEVEL_IFLOG_IT macro.
 *
 * The condition will be evaluated by this version no matter what.
 *
 * Consider the following example:
 * \code
    MyClass::SomeMethod() {
        EDDY_LOGGING_LEVEL_IFLOG_CF_IT(
            i==10, my_lg, my_levels::Debug, MyClass, "Hi"
            )
    }
   \endcode
 *
 * \param cond The condition based on which to log e on tlg.
 * \param tlg The log gateway to use to log \a e at level \a ll.
 * \param ll The level to supply to the gateway at which to log \a e.
 * \param it The issuer type presumably responsible for entry \a e.
 * \param e The entry to log.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_LEVEL_IFLOG_CF_IT(cond, tlg, ll, it, e)                \
            { if((cond) && tlg.will_log(tlg.get_logging_level<it>(), ll))   \
                    tlg.simple_log(ll, e); }

#else // ifndef EDDY_LOGGING_ON




/// See documentation listed above in EDDY_LOGGING_ON section.
#define DOXYGEN_ONLY(a)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_IF_ON(a)

/// A macro to execute the statement \a a if EDDY_LOGGING_ON is not defined.
/**
 * If EDDY_LOGGING_ON isn't defined, the following macro expands to exactly
 * a.  Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_LOGGING_IF_OFF(my_log_t alog());
   \endcode
 * would become:
 * \code
        my_log_t alog();
   \endcode
 * if EDDY_LOGGING_ON weren't defined and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 *
 * \ingroup user_macros
 */
#define EDDY_LOGGING_IF_OFF(a) a

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_ON_LM(tlg, method, params)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LOG(tlg, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_IFLOG(cond, tlg, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_LOG_D(tlg, ll, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_IFLOG_D(cond, tlg, ll, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_IFLOG_CF_D(cond, tlg, ll, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_LOG_II(tlg, ll, i, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_IFLOG_II(cond, tlg, ll, ii, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_IFLOG_CF_II(cond, tlg, ll, ii, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_LOG_IT(tlg, ll, it, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_IFLOG_IT(cond, tlg, ll, it, e)

/// Expands to nothing b/c EDDY_LOGGING_ON is not defined.
#define EDDY_LOGGING_LEVEL_IFLOG_CF_IT(cond, tlg, ll, it, e)

/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif


/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_MACROS_HPP
