/*
================================================================================
    PROJECT:

        Eddy C++ Configuration Project

    CONTENTS:

        Contains common configuration code.

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

        Thu Feb 02 09:32:21 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains common configuration code.
 *
 * This includes some platform and compiler specific configuration.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_CONFIG_CONFIG_HPP
#define EDDY_CONFIG_CONFIG_HPP







/*
================================================================================
Includes
================================================================================
*/
// This will be here if using autoconf.
#ifdef HAVE_CONFIG_H
#include <eddy_config.h>
#endif

#ifndef EDDY_HAVE_EXCEPTIONS
#define EDDY_NO_EXCEPTIONS
#endif


/*
================================================================================
Macro Definitions
================================================================================
*/



/// A macro that always expands to a closing scope bracket "}".
#define EDDY_CLOSE_SCOPE }

// The following is code specific to the case where structured exception
// handling is not supported.
#ifdef EDDY_NO_EXCEPTIONS

/**
 * \brief A macro to conditionally include code when EDDY_NO_EXCEPTIONS is
 *        defined.
 *
 * When EDDY_NO_EXCEPTIONS is defined, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_NO_EXCEPTIONS(abort();)
   \endcode
 * would become:
 * \code
        abort();
   \endcode
 * if EDDY_NO_EXCEPTIONS were defined and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_NO_EXCEPTIONS(a) a

/**
 * \brief A macro to conditionally include code when EDDY_NO_EXCEPTIONS is NOT
 *        defined.
 *
 * When EDDY_NO_EXCEPTIONS is not defined, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_EXCEPTIONS(std::exception ex;)
   \endcode
 * would become:
 * \code
        std::exception ex;
   \endcode
 * if EDDY_NO_EXCEPTIONS weren't defined and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_EXCEPTIONS(a)

#else

/// Expands to nothing because EDDY_NO_EXCEPTIONS is not defined.
#   define EDDY_IF_NO_EXCEPTIONS(a)


/// Expands to \a a because EDDY_NO_EXCEPTIONS is defined.
#   define EDDY_IF_EXCEPTIONS(a) a

#endif

/**
 * \brief A macros that expands to "try { " if EDDY_NO_EXCEPTIONS is not
 *        defined and just "{" if it is.
 */
#define EDDY_TRY EDDY_IF_EXCEPTIONS(try) {

/**
 * \brief A macros that expands to "} catch(arg) { " if EDDY_NO_EXCEPTIONS is
 *        not defined and "} if(false) {" if it is.
 *
 * \param arg The exception that is to be caught if exceptions are enabled.
 */
#define EDDY_CATCH(arg)                                                     \
    } EDDY_IF_EXCEPTIONS(catch(arg)) EDDY_IF_NO_EXCEPTIONS(if(false)) {

/**
 * \brief A macros that expands to "throw ex;" if EDDY_NO_EXCEPTIONS is
 *        not defined and nothing if it is.
 *
 * \param arg The exception that is to be thrown if exceptions are enabled.
 */
#define EDDY_THROW(ex) EDDY_IF_EXCEPTIONS(throw ex;)

/**
 * \brief A macros that expands to "throw;" if EDDY_NO_EXCEPTIONS is
 *        not defined and nothing if it is.
 */
#define EDDY_RETHROW EDDY_IF_EXCEPTIONS(throw;)



/*
================================================================================
Utility Macros
================================================================================
*/
/// A macro to add things to doxygen commenting.
/**
 * This should never turn into anything with coding significance.  It will
 * always cause the argument to be removed completely.  However, the doxygen
 * preprocessor can be instructed to expand the macro.  The doxygen
 * instruction should be placed in your Doxyfile at the tag for PREDEFINED
 * and should look like this:
 * \verbatim
    "DOXYGEN_ONLY(a)=a"
   \endverbatim
 *
 * An example of a good time to use this is when you would like an unused
 * parameter to have a name in your documentation but do not want to have
 * the warning produced by the compiler.  For example,
 * \code
    void method(double DOXYGEN_ONLY(arg)) {}
 * \endcode
 *
 * \param a The text to appear in doxygen documentation and not in the code.
 */
#define DOXYGEN_ONLY(a)


/*
================================================================================
Feature Macros
================================================================================
*/

/**
 * \brief Use wherever the template keyword must be used as a qualifier for
 *        dependent method calls.
 *
 * So instead of: \code a.template f<T>(); \endcode<BR>
 * Use: \code a.EDDY_TEMPLATE_KEYWORD_QUALIFIER f<T>();
 */
#ifdef EDDY_HAVE_TEMPLATE_KEYWORD_QUALIFIER
#define EDDY_TEMPLATE_KEYWORD_QUALIFIER template
#else
#define EDDY_TEMPLATE_KEYWORD_QUALIFIER
#endif



/*
================================================================================
Shared Library Specifications
================================================================================
*/


// really, only _WIN32 is necessary of these 2.
#if defined(_WIN32) || defined(_WIN64)

/**
 * \brief A preprocessor flag that can be used to conditionally compile or
 *        exclude code for windows.
 */
#define EDDY_WINDOWS

#ifndef NOMINMAX
#   define NOMINMAX
#endif

/**
 * \brief A macro to conditionally include code when on a windows platform
 *
 * When using windows, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_WINDOWS(_getch();)
   \endcode
 * would become:
 * \code
        _getch();
   \endcode
 * if using windows and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_WINDOWS(a) a

/**
 * \brief A macro to conditionally include code when NOT using windows.
 *
 * When windows is not the platform, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_NO_WINDOWS(getch();)
   \endcode
 * would become:
 * \code
        getch();
   \endcode
 * if windows was not the platform and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_NO_WINDOWS(a)

#else // !defined(_WIN32) && !defined(_WIN64)

/// Expands to nothing because windows is not the platform.
#   define EDDY_IF_WINDOWS(a)

/// Expands to \a a because windows is the platform.
#   define EDDY_IF_NO_WINDOWS(a) a

#endif // defined(_WIN32) || defined(_WIN64)


#ifdef EDDY_HAVE_BOOST

/**
 * \brief A macro to conditionally include code when boost is available
 *
 * When using boost, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_BOOST(myfunc();)
   \endcode
 * would become:
 * \code
        myfunc();
   \endcode
 * if using boost and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_BOOST(a) a

/**
 * \brief A macro to conditionally include code when NOT using boost.
 *
 * When boost is not available, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_NO_BOOST(myfunc();)
   \endcode
 * would become:
 * \code
        myfunc();
   \endcode
 * if boost was available and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_NO_BOOST(a)

#else // !defined EDDY_HAVE_BOOST

/// Expands to nothing because windows is not the platform.
#   define EDDY_IF_BOOST(a)

/// Expands to \a a because windows is the platform.
#   define EDDY_IF_NO_BOOST(a) a

#endif // defined EDDY_HAVE_BOOST

/**
 * \brief A macro definition of any precursors necessary to specify that an
 *        entity is to be exported to a shared library.
 *
 * In windows, this becomes __declspec(dllexport).  In anything else,
 * it becomes nothing.
 */
#define EDDY_SL_EXPORT EDDY_IF_WINDOWS(__declspec(dllexport))

/**
 * \brief A macro definition of any precursors necessary to specify that an
 *        entity is to be imported from a shared library.
 *
 * In windows, this becomes __declspec(dllimport).  In anything else,
 * it becomes nothing.
 */
#define EDDY_SL_IMPORT EDDY_IF_WINDOWS(__declspec(dllimport))

#ifdef EDDY_SL

/**
 * \brief A macro to conditionally include code when EDDY_SL is
 *        defined.
 *
 * When EDDY_SL is defined, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_SL(__declspec(dllimport))
   \endcode
 * would become:
 * \code
        __declspec(dllimport)
   \endcode
 * if EDDY_SL were defined and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_SL(a) a

/**
 * \brief A macro to conditionally include code when EDDY_SL is NOT
 *        defined.
 *
 * When EDDY_SL is not defined, the following macro expands to
 * exactly \a a. Otherwise, it expands to nothing.
 *
 * So the following line:
 * \code
        EDDY_IF_NO_SL(int i;)
   \endcode
 * would become:
 * \code
        int i;
   \endcode
 * if EDDY_SL weren't defined and would disappear otherwise.
 *
 * \param a The statement to conditionally create.
 */
#   define EDDY_IF_NO_SL(a)

#else

/// Expands to nothing because EDDY_SL is not defined.
#   define EDDY_IF_SL(a)

/// Expands to \a a because EDDY_SL is defined.
#   define EDDY_IF_NO_SL(a) a

#endif


#if defined(EDDY_SL)

#   if defined(EDDY_EXPORTING)

        /**
         * \brief The shared library tag that resolves to either EDDY_SL_IMPORT
         *        or EDDY_SL_EXPORT depending on whether EDDY_EXPORTING is
         *        defined.
         *
         * If EDDY_EXPORTING is defined, this resolves to EDDY_SL_EXPORT.
         * Otherwise it resolves to EDDY_SL_IMPORT.  See the documentation on
         * each of those for information on what they mean.
         */
#       define EDDY_SL_IEDECL EDDY_SL_EXPORT

#   else // #if !defined(EDDY_EXPORTING)

#       define EDDY_SL_IEDECL EDDY_SL_IMPORT

#   endif

#else // #if !defined(EDDY_SL)

/// Expands to nothing because EDDY_SL is not defined.
#   define EDDY_SL_IEDECL

#endif



/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace config {




/*
================================================================================
End Namespace
================================================================================
*/

    } // namespace config
} // namespace eddy







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
#endif // EDDY_CONFIG_CONFIG_HPP
