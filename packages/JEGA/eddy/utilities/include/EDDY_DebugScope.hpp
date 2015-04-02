/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of classes EDDY_DebugScope and EDDY_Debug.

    NOTES:

        These classes along with the #defined macros constitute a simple
        debugging facility that supports assertions, messages, warnings, etc.
        as well as a simple stack tracing functionality.  These functionalities
        are preprocessed out unless EDDY_OPTION_DEBUG is defined.  This is
        useful to allow expensive assertions and such during debugging and have
        them automatically stripped out for release builds.

        See also the notes under the "Class Definition" sections of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 13:52:29 2003 - Original Version (JE)

================================================================================
*/


/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition classes EDDY_DebugScope and EDDY_Debug.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_EDDY_DEBUGSCOPE_HPP
#define EDDY_UTILITIES_EDDY_DEBUGSCOPE_HPP







/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"








/*
================================================================================
Preprocessor Definitions and Macros
================================================================================
*/
#ifdef EDDY_OPTION_DEBUG

#   include "../../config/include/current_function.hpp"

    /// This macro causes a scope trace with messaging and program abort.
    /**
     * If \a a evaluates to true, this macro will print the message of \a b,
     * print a scope trace, and abort the program.  It will also inform of
     * the file and line number where the problem occurred.
     *
     * \param a The logical test based on which to abort.
     * \param b The message to report along with file and line.
     */
#   define EDDY_DEBUG(a, b)                                                 \
        if(a) {                                                             \
            eddy::utilities::EDDY_Debug::_Report((b), __FILE__, __LINE__);  \
        }

    /// This macro causes a scope trace with messaging (but no abort).
    /**
     * If \a a evaluates to true, this macro will print the message of \a b and
     * print a scope trace.  It will also inform of the file and line
     * number where the problem occurred.
     *
     * \param a The logical test based on which to warn.
     * \param b The message to report along with file and line.
     */
#   define EDDY_WARNING(a, b)                                               \
        if(a) {                                                             \
            eddy::utilities::EDDY_Debug::_Warning((b), __FILE__, __LINE__); \
        }

    /// This macro causes a scope trace and program abort.
    /**
     * If \a a evaluates to true, this macro will print a scope trace
     * and abort the program.  It will also inform of the file and line
     * number where the problem occurred.
     *
     * \param a The logical description of the exceptional condition.
     */
#   define EDDY_ASSERT(a)                                                   \
        if(!(a)) {                                                          \
            eddy::utilities::EDDY_Debug::_Assert(#a, __FILE__, __LINE__);   \
        }

    /// This macro enables catching of signal \a a.
    /**
     * When a signal is caught (such as SIGSEGV, etc.), this method will
     * indicate which signal was caught, print a scope trace, and abort
     * the program.  The allowable signals are:
     *
     * -SIGSEGV - Illegal storage access.
     * -SIGINT  - CTRL+C interrupt.
     * -SIGFPE  - Floating-point error, such as overflow, division by zero,
     *            or invalid operation.
     * -SIGILL  - Illegal instruction.
     * -SIGTERM - Termination request sent to the program.
     * -SIGABRT - Abnormal termination.
     *
     * \param a The signal value to enable catching of by the EDDY_DEBUGSCOPE
     *          project.
     */
#   define EDDY_DEBUGSIGNAL(a)                                              \
        eddy::utilities::EDDY_Debug::_RegisterSignal(a);

    /// This macro pushes \a a onto the scope trace stack.
    /**
     * This macro actually creates an object of type EDDY_DebugScope identified
     * by \a a.  \a a should be a string literal like "someclass::somefunc".
     * In case of a scope trace, the text associated with this object will be
     * printed in turn.  This should appear at the head of each method or
     * function.  When this object goes out of scope (when the method or
     * function exits), its destructor will be called (guaranteed by C++) and
     * it will be removed from the scope trace stack.
     *
     * \param a The text to be associated with the newly created debug scope
     *          object.
     */
#   define EDDY_DEBUGSCOPE(a)                                               \
        eddy::utilities::EDDY_DebugScope otb_debugscope_(a);

    /**
     * \brief This macro pushes the name of the current function onto the scope
     *        trace stack.
     *
     * This macro uses the EDDY_DEBUGSCOPE macro with an argument of
     * EDDY_CURRENT_FUNCTION.
     */
#   define EDDY_FUNC_DEBUGSCOPE                                             \
        EDDY_DEBUGSCOPE(EDDY_CURRENT_FUNCTION)

    /// This macro causes a print of the scope trace.
    /**
     * This causes the printing of each EDDY_DebugScope object text
     * in the order that they were pushed onto the stack.  The text
     * is printed to standard error.
     */
#   define EDDY_TRACESCOPE                                                  \
        eddy::utilities::EDDY_DebugScope::_PrintScopeTrace();

    /// This macro causes a message to be presented on standard error.
    /**
     * If \a a evaluates to true, this macro will print the message of \a b.
     * It will neither print a scope trace nor abort the program.  It will
     * inform of the file and line number where the message originated.
     *
     * \param a The logical test based on which to print a message.
     * \param b The message to print along with file and line.
     */
#   define EDDY_MESSAGE(a, b)                                               \
        if(a) {                                                             \
            eddy::utilities::EDDY_Debug::_Message((b),__FILE__,__LINE__);   \
        }

    /// This macro causes execution of the statement \a a.
    /**
     * This macro doesn't cause abortion or scope tracing.  It simply
     * causes execution of the statement represented by \a a.
     *
     * \param a The statement to execute if debug scope is enabled and not
     *          otherwise.  A semicolon will automatically appended.
     */
#   define EDDY_DEBUGEXEC(a) a;

#else

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_DEBUG(a, b)

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_WARNING(a, b)

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_ASSERT(a)

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_DEBUGSIGNAL(a)

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_DEBUGSCOPE(a)

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_FUNC_DEBUGSCOPE

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_TRACESCOPE

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_MESSAGE(a, b)

/// Expands to nothing because EDDY_OPTION_DEBUG is not defined.
#   define EDDY_DEBUGEXEC(a)

#endif









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
    namespace utilities {






/*
================================================================================
This Code is Only Compiled in DebugMode
================================================================================
*/
#ifdef EDDY_OPTION_DEBUG





/*
================================================================================
Forward Declares
================================================================================
*/
class EDDY_Debug;
class EDDY_DebugScope;










/*
================================================================================
Class Definition for EDDY_DebugScope
================================================================================
*/

/// Class used to manage the rudimentary scope tracing functionality.
/**
 * This class only exists if EDDY_OPTION_DEBUG is defined.
 * This class stores a list of strings which should describe methods or
 * functions.  When one of these is created, the string passed in is
 * pushed onto the list and when it is destroyed, the string is popped
 * off the list.  In case of a scope trace, all the strings currently
 * on the stack are printed out in reverse order.
 */
class EDDY_SL_IEDECL EDDY_DebugScope
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /**
         * \brief Pointer to the text spit out for this object
         *        in the event of a scope trace.
         */
        const char* const _data;

        /**
         * \brief Pointer to the next of this type to show up
         *        in the event of a scope trace.
         */
        EDDY_DebugScope* _next;

        /**
         * \brief Pointer to the very first of this type to show up
         *        in the event of a scope trace.
         */
        static EDDY_DebugScope* _top;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// This method is in the expansion of the EDDY_TRACESCOPE macro.
        /**
         * It causes a listing of entered but not yet exited method and
         * functions having an EDDY_DEBUGSCOPE statement.  The listing is
         * printed to the standard output.
         */
        static
        void
        _PrintScopeTrace(
            ) throw();

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:





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

        /// Constructs an EDDY_DebugScope object with _data = name
        /**
         * \param name The name to be associated with this stack entry.
         */
        EDDY_DebugScope(
            const char* name
            ) throw();

        /// Destructs an EDDY_DebugScope object
        /**
         * This has the effect of removing it from the list of scope traced
         * objects.
         */
        ~EDDY_DebugScope(
            ) throw();

}; // class EDDY_DebugScope





/*
================================================================================
Class Definition for EDDY_Debug
================================================================================
*/

/// Class used to manage errors, warnings, etc. and show them to the user.
/**
 * Like the EDDY_DebugScope class, this class only exists if EDDY_OPTION_DEBUG
 * is defined.  This class handles errors, warning, messages, etc. issued
 * by the user in debug mode.  It is part of this rudimentary debugging
 * facility that supports assertions and scope tracing.
 */
class EDDY_SL_IEDECL EDDY_Debug
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief This method is in the expansion of the
         *        EDDY_DEBUG(cond, message) macro.
         *
         * It causes the supplied failure message \a message to appear.
         * This method causes a scope trace print and program abort.
         *
         * \param message The message to display.
         * \param fileName The name of the file in which the problem occurred.
         * \param line The line number in the file on which the problem
         *             occurred.
         */
        static
        void
        _Report(
            const char* message,
            const char* fileName,
            long line
            ) throw();


        /**
         * \brief This method is in the expansion of the
         *        EDDY_WARNING(cond, message) macro.
         *
         * It causes the supplied warning message \a message to appear.
         * This method causes a scope trace print (without program abort).
         *
         * \param message The message to display.
         * \param fileName The name of the file in which the problem occurred.
         * \param line The line number in the file on which the problem
         *             occurred.
         */
        static
        void
        _Warning(
            const char* message,
            const char* fileName,
            long line
            ) throw();

        /**
         * \brief This method is in the expansion of the
         *        EDDY_MESSAGE(cond, message) macro.
         *
         * It causes the supplied message \a message to appear.
         * This method does not cause a scope trace or program abort.
         *
         * \param message The message to display.
         * \param fileName The name of the file from which the message was
         *                 sent.
         * \param line The line number in the file from which the message was
         *             sent.
         */
        static
        void
        _Message(
            const char* message,
            const char* fileName,
            long line
            ) throw();

        /// This method is in the expansion of the EDDY_ASSERT(cond) macro.
        /**
         * It causes the supplied message \a message which is the text
         * equivolent of the \a cond if the EDDY_ASSERT macro is used to
         * appear.  This method causes a scope trace print and program abort.
         *
         * \param message The message to display.
         * \param fileName The name of the file in which the problem occurred.
         * \param line The line number in the file on which the problem
         *             occurred.
         */
        static
        void
        _Assert(
            const char* message,
            const char* fileName,
            long line
            ) throw();

        /// This method is in the expansion of the EDDY_DEBUGSIGNAL(sig) macro.
        /**
         * The macro registers this function as a callback in the case of
         * the supplied signal.
         *
         * \param val The value of the signal that was caught and sent here.
         */
        static
        void
        _Signal(
            int val
            ) throw();

        /// This method is the expansion of EDDY_DEBUGSIGNAL(a)
        /**
         * This method registers the signal handler for \a sig as
         * _Signal(int).
         *
         * \param sig The value of the signal that will be caught and sent to
         *            _Signal(int).
         */
        static
        void
        _RegisterSignal(
            int sig
            ) throw();

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:





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

    private:

        /// This constructor is private and has no implementation.
        /**
         * All methods of this class are static and thus it should not and
         * can not be instantiated.
         */
        EDDY_Debug(
            );


}; // class EDDY_Debug



#endif // #ifdef EDDY_OPTION_DEBUG




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/EDDY_DebugScope.hpp.inl"




/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_EDDY_DEBUGSCOPE_HPP
