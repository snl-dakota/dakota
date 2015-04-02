/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        All configuration for logging in the JEGA project.

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

        Wed Dec 14 11:10:14 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the configuration for logging in the JEGA project.
 *
 * This file provides the interface to the
 * <a href="../../Logging/doc/html/index.html">Eddy C++ Logging Project</a>,
 * henceforth referred to as the "base project", used by JEGA for all output to
 * the user.  This includes all informational as well as error messages.  The
 * interface is implemented using a class that holds the logs and log manager
 * which can be instantiated as many times as necessary.  In terms of macros,
 * the interface is a reduced version of the interface available
 * through the base project.  The main difference is the shortening of the
 * macro names from for example EDDY_LOGGING_* to JEGALOG* with extensions
 * for using the global log and for issuing fatal entries with reduced syntax.
 *
 * One special difference in this implementation is the introduction of the
 * *_F macros.  These macros are meant specifically for use when a log entry
 * is being made at the fatal level.  They are necessary because these entries
 * are meant to cause a program abort.  If logging is not enabled at compile
 * time (JEGA_LOGGING_ON not defined) then the regular
 * EDDY_LOGGING macros simply make the code go away and no abort occurs.  The
 * *_F macros behave differently in that when logging is disabled, they
 * still cause a program abort and still include a detailed description of the
 * cause made possible by the specialized entry types used here.
 * When logging is enabled, the *_F macros behave just as the regular macros.
 *
 * Note that if logging is disabled, absolutely no output will appear to the
 * user unless a fatal error occurs and then the output will be minimal.  It is
 * recommended that JEGA not be used without logging unless it is absolutely
 * necessary.
 *
 * If JEGA_LOGGING_ON is defined, the logging project allows logging to both
 * console and to files.  These behaviors can be controlled individually with
 * preprocessor defines JEGA_LOGGING_NO_CONSOLE and JEGA_LOGGING_NO_FILE.
 * Defining both will cause an undef of JEGA_LOGGING_ON and turn logging off
 * completely as well as strip out all logging code (with the exception of
 * fatal entry handling).  Defining just JEGA_LOGGING_NO_FILE will cause all
 * file logging code to be ignored and definition of JEGA_LOGGING_NO_CONSOLE
 * will cause no log entries to appear on the console.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_LOGGING_HPP
#define JEGA_LOGGING_HPP

// If both JEGA_LOGGING_NO_FILE and JEGA_LOGGING_NO_CONSOLE are defined, then
// logging is effectively off.  So to avoid the overhead of all the logging
// code, we will simply turn logging off.
#if defined(JEGA_LOGGING_ON)
#   if defined(JEGA_LOGGING_NO_FILE) && defined(JEGA_LOGGING_NO_CONSOLE)
#       undef JEGA_LOGGING_ON
#   endif
#endif

// If we are using wide characters, then we must define our default level type
// to indicate that.  Leave the Eddy Logging project to define the character
// traits appropriately for our choice.
#ifdef JEGA_LOGGING_WIDE_CHARS
#define EDDY_LOGGING_DEF_CHAR_TYPE wchar_t
#endif

// No matter what, because we need to deal with levels whether or not logging
// is on, we should define our default level class.  This way we don't always
// have to specify it in our template argument lists.  It will be used as the
// default argument.  We can leave the default char type and traits type
// as they are (char and std::char_traits<char> unless changed above).
#define EDDY_LOGGING_DEF_LEVEL_CLASS eddy::logging::dakota_levels<>


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

// Include the JEGA configuration information header.


// prepare the Logging project for possible shared object creation.
#ifdef JEGA_SL
#define EDDY_LOGGING_SL
#endif

// prepare the logging project for shared library exporting.
#ifdef JEGA_EXPORTING
#define EDDY_LOGGING_EXPORTING
#endif

// Now prepare the Logging project for possible thread safety.
#ifdef JEGA_THREADSAFE
#define EDDY_LOGGING_THREADSAFE
#endif

// Now we handle the case where logging is off.  We still want our entry types
// and our level type so that our fatal loggings can display reasoning.  In
// order to accomplish this, we have to include both the entry type headers and
// the level type headers with EDDY_LOGGGING_ON defined so the code is not
// stripped.
#define EDDY_LOGGING_ON
#include <logging/level_classes/dakota_levels.hpp>
#include <logging/entries/text_entry.hpp>
#include <logging/entries/ostream_entry.hpp>

// if JEGA Logging is off, we must now undef EDDY_LOGGING_ON.
// Otherwise we keep it.
#ifndef JEGA_LOGGING_ON
#undef EDDY_LOGGING_ON
#endif

// We must include the macros file from Eddy logging no matter what.  At this
// point EDDY_LOGGING_ON is only defined if JEGA_LOGGING_ON is.
#include <logging/include/macros.hpp>

// This block defines most of what happens if logging is on.
#ifdef JEGA_LOGGING_ON

// include memory for the auto_ptr.
#include <memory>

// For convenience, we will provide functions for creating string literals.
#define JEGA_LOGGING_STRING_LITERAL const EDDY_LOGGING_DEF_CHAR_TYPE*

#define JEGA_LOGGING_AS_STRING_LITERAL(a)                                   \
    static_cast<JEGA_LOGGING_STRING_LITERAL>(a)

// Define the JEGA Logging string type which is a basic_string of the char
// type.
typedef
std::basic_string<EDDY_LOGGING_DEF_CHAR_TYPE, EDDY_LOGGING_DEF_CHAR_TRAITS>
string_type;


// JEGA Logging is on, so we need to start defining our logging capabilities.
// To start with, we will only log to a file if the user has not defined
// JEGA_LOGGING_NO_FILE.  If they have not, then we need to include our
// file log header.

#ifndef JEGA_LOGGING_NO_FILE
#include <logging/logs/file_log.hpp>
#endif

// Now, we will only log to console if the user has not defined
// JEGA_LOGGING_NO_CONSOLE.  If they have not, then we need to include our
// ostream log header.
#ifndef JEGA_LOGGING_NO_CONSOLE
#include <logging/logs/ostream_log.hpp>
#endif

// If either log type is turned off via JEGA_LOGGING_NO_*, then we need
// a null log to take the place of the unused log.  So include the null
// log header.  It is much simpler to use the null_log object instead
// of trying to replace the decorator log and inclusion and use of the
// null_log is practically free.
#if defined(JEGA_LOGGING_NO_FILE) || defined(JEGA_LOGGING_NO_CONSOLE)
#include <logging/logs/null_log.hpp>
#endif

// No matter what else is happening, if JEGA logging is on, we need our
// decorator log and our log gateway so include those headers now.
#include <logging/logs/decorator_log.hpp>
#include <logging/log_gateways/cb_level_log_gateway.hpp>






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
namespace JEGA {
    namespace Logging {








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

// If the user has not defined JEGA_LOGGING_NO_FILE, then our file log type is
// the eddy::logging::file_log<>.  If not it is the eddy::logging::null_log.
#ifndef JEGA_LOGGING_NO_FILE

/// The type of log that writes entries into a file.
typedef eddy::logging::file_log<> file_log;

#else // defined(JEGA_LOGGING_NO_FILE)

/// The null log takes the place of the file_log if file logging is disabled.
typedef eddy::logging::null_log file_log;

#endif


// If the use has not defined JEGA_LOGGING_NO_CONSOLE, then our file log type is
// the eddy::logging::ostream_log<>.  If not it is the eddy::logging::null_log.
#ifndef JEGA_LOGGING_NO_CONSOLE

/// The type of log that writes entries into an ostream.
typedef eddy::logging::ostream_log<> ostr_log;

#else // defined(JEGA_LOGGING_NO_FILE)

/**
 * \brief The null log takes the place of the ostream_log if console logging
 *        is disabled.
 */
typedef eddy::logging::null_log ostr_log;

#endif



// Now we decorate our two log types together.  One or the other may be the
// null log but this introduces virtually no additional overhead.  We then go
// on to define our level and gateway classes.  Our LevelClass will be the
// default level class which we set above to be the dakota_levels.

/// The type of log that writes entries into a file and an ostream.
typedef eddy::logging::decorator_log<file_log, ostr_log> deco_log;

/// The class that defines the levels available for this project.
typedef EDDY_LOGGING_DEF_LEVEL_CLASS LevelClass;

/// The type of the actual levels at which log entries may be issued.
typedef LevelClass::level LogLevel;

/// The class type for the log gateway which passes entries to logs.
typedef eddy::logging::cb_level_log_gateway<deco_log> gate_t;



/*
================================================================================
Class Declarations
================================================================================
*/
/**
 * \brief A class to serve as a logger for the JEGA project.
 *
 * This class contains a log gateway and potentially a unique file log.  There
 * is also a static "global" file log for use if desired.  The console log is
 * the same for all instances.
 */
class JEGA_SL_IEDECL Logger
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /// A shorthand for the type of a string used by this class.
        typedef
        std::basic_string<
            EDDY_LOGGING_DEF_CHAR_TYPE,
            EDDY_LOGGING_DEF_CHAR_TRAITS
            >
        string_type;

        enum FatalBehavior
        {
            ABORT = 0,

            THROW = 1

        }; // enum FatalBehavior

    protected:


    private:


    /*
    ============================================================================
    Nested Subclass Forward Declares
    ============================================================================
    */
    private:

        /// A callback class for use when a fatal logging occurs.
        class fatal_callback;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The one Logger instance that serves as the global logger.
        static std::auto_ptr<Logger> _global;

        /// The one instance that serves as the global file log.
        static file_log _globalFLog;

        /// The one instance that serves as the global ostream (console) log.
        static ostr_log _globalOLog;

        /**
         * \brief A pointer to the new file_log created for this logger.
         *
         * If no file stream was created (this Logger is using the global
         * file stream log) then the pointer is null.
         */
        std::auto_ptr<file_log> _flog;

        /**
         * \brief The decorator log that combines the file stream of this
         *        Logger with the global std err log.
         */
        deco_log _dlog;

        /// The gateway through which all entries to this logger must pass.
        gate_t _gate;

        /// The human readable name of this logger.
        string_type _name;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Does everything necessary to initialize the Global Logger object.
        /**
         * This should only be called once per program execution.
         *
         * I know it is stupid, but if I don't explicitly qualify the
         * LevelClass usage with the namespaces, Visual Studio .NET 2003
         * has problems.
         *
         * \param filename The name of the file for the Global file_log to log
         *                 into.
         * \param def_level The default level for the Global Logger to use.
         */
        static
        void
        InitGlobalLogger(
            const string_type& filename =
                string_type(JEGA_LOGGING_AS_STRING_LITERAL("JEGAGlobal.log")),
            // Not using our typedef b/c vc7.1 doesn't like it.
            const LogLevel& def_level = EDDY_LOGGING_DEF_LEVEL_CLASS::Default
            );

        /**
         * \brief This method initializes the supplied logger with leading
         *        information.
         *
         * The information written to the beginning of the log is something
         * like:
         * \verbatim
            JEGA Genetic Algorithm Optimization Software v2.0.0
            Run started Wed Feb 15 12:08:01 2006

            <log name> initialized using <def level> as default level.
            <log name> logging to file <log file name>
            <log name> logging to console window
           \endverbatim
         *
         * Certain information may not be relevant, etc.
         *
         * \param logger The logger to initialize.
         */
        static
        void
        InitLogger(
            Logger& logger
            );

        /// Initializes this log by calling InitLogger(*this).
        void
        Init(
            );

        /// Provides access to the global Logger object.
        /**
         * \return The Logger that writes to the Global log objects.
         */
        inline static
        Logger&
        Global(
            )
        {
            return *(_global.get());
        }

        /// Provides mutable access to the log_gateway used by this Logger.
        /**
         * \return The log gateway through which all entries to this Logger
         *         must pass.
         */
        inline
        gate_t&
        Gate(
            )
        {
            return _gate;
        }

        const std::string&
        GetFilename(
            );

        /// Provides immutable access to the log_gateway used by this Logger.
        /**
         * \return The log gateway through which all entries to this Logger
         *         must pass.
         */
        inline
        const gate_t&
        Gate(
            ) const
        {
            return _gate;
        }

        /// Provides access to the name given to this Logger object.
        /**
         * \return The name given to this Logger.
         */
        inline
        const string_type&
        GetName(
            ) const
        {
            return _name;
        }

        static
        FatalBehavior
        GetFatalBehavior(
            );

        static
        void
        SetFatalBehavior(
            FatalBehavior newBehav
            );

        void
        FlushStreams(
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a Logger with the supplied name and default level.
         *
         * This constructor will cause the resulting Logger to use the
         * global file log and as with all Loggers, the global console window
         * log.
         *
         * \param loggerName The name for this new Logger object.
         * \param def_level The default level for the log gateway used by this
         *                  Logger.
         */
        Logger(
            const string_type& loggerName,
            // Not using our typedef b/c vc7.1 doesn't like it.
            const LogLevel& def_level = EDDY_LOGGING_DEF_LEVEL_CLASS::Default
            );

        /**
         * \brief Constructs a Logger with the supplied filename, logger name,
         *        and default level.
         *
         * This constructor will cause the resulting Logger to use a newly
         * created file log and as with all Loggers, the global console window
         * log.
         *
         * \param filename The name of the file for the new file_log object to
         *                 log into.
         * \param loggerName The name for this new Logger object.
         * \param def_level The default level for the log gateway used by this
         *                  Logger.
         */
        Logger(
            const string_type& filename,
            const string_type& loggerName,
            // Not using our typedef b/c vc7.1 doesn't like it.
            const LogLevel& def_level = EDDY_LOGGING_DEF_LEVEL_CLASS::Default
            );

        /**
         * \brief Constructs a Logger using the supplied file_log, logger name,
         *        and default level.
         *
         * This constructor will cause the resulting Logger to use the supplied
         * file log and as with all Loggers, the global console window log.
         *
         * \param flog The existing file_log object for this Logger to log to.
         * \param loggerName The name for this new Logger object.
         * \param def_level The default level for the log gateway used by this
         *                  Logger.
         */
        Logger(
            file_log& flog,
            const string_type& loggerName,
            // Not using our typedef b/c vc7.1 doesn't like it.
            const LogLevel& def_level = EDDY_LOGGING_DEF_LEVEL_CLASS::Default
            );

}; // class Logger


/*
================================================================================
Global Functions
================================================================================
*/






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Logging
} // namespace JEGA

#else // !defined (JEGA_LOGGING_ON)

#include <iostream>
#include <utilities/include/EDDY_DebugScope.hpp>

namespace JEGA {
    namespace Logging {




// If logging isn't on, we will still declare a class type for Logger
// but it will only deal with fatal error handling.
class JEGA_SL_IEDECL Logger {

    public:

        enum FatalBehavior
        {
            ABORT = 0,

            THROW = 1

        }; // enum FatalBehavior

    /*
    ============================================================================
    Nested Subclass Forward Declares
    ============================================================================
    */
    public:

        /// A callback class for use when a fatal logging occurs.
        class fatal_callback
        {
            public:

                static FatalBehavior onFatal;

                template <typename CBT>
                static
                void
                Invoke(
                    const CBT& e
                    )
                {
                    EDDY_FUNC_DEBUGSCOPE
                    EDDY_TRACESCOPE
                    if(onFatal = Logger::ABORT)
                    {
                        std::cerr << static_cast<const std::string&>(e) << '\n';
                        exit(5);
                    }
                    if(onFatal = Logger::THROW) throw std::runtime_error(
                        "Received a fatal callback with message: " +
                        static_cast<const std::string&>(e)
                        );
                }

        }; // class Logger::fatal_callback

        static
        FatalBehavior
        GetFatalBehavior(
            )
        {
            return fatal_callback::onFatal;
        }

        static
        void
        SetFatalBehavior(
            FatalBehavior newBehav
            )
        {
            fatal_callback::onFatal = newBehav;
        }

        void
        FlushStreams(
            )
        {
        }

}; // class Logger




    } // namespace Logging
} // namespace JEGA

#endif // JEGA_LOGGING_ON


// No matter what, we declare our entry types so that fatal logging
// can be descriptive even when logging is off.

/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Logging {

/// The class that defines the levels available for this project.
typedef EDDY_LOGGING_DEF_LEVEL_CLASS LevelClass;

/// The type of the actual levels at which log entries may be issued.
typedef LevelClass::level LogLevel;

/*
================================================================================
Instantiations
================================================================================
*/
/// Simple function to allow shorthand reference to the debug logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::debug logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
ldebug(
    )
{
    return LevelClass::debug;
}

/// Simple function to allow shorthand reference to the verbose logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::verbose logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
lverbose(
    )
{
    return LevelClass::verbose;
}

/// Simple function to allow shorthand reference to the quiet logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::quiet logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
lquiet(
    )
{
    return LevelClass::quiet;
}

/// Simple function to allow shorthand reference to the normal logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::normal logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
lnormal(
    )
{
    return LevelClass::normal;
}

/// Simple function to allow shorthand reference to the fatal logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::fatal logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
lfatal(
    )
{
    return LevelClass::fatal;
}

/// Simple function to allow shorthand reference to the silent logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::silent logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
lsilent(
    )
{
    return LevelClass::silent;
}

/// Simple function to allow shorthand reference to the Default logging level.
/**
 * This is here to make dll creation easier.
 *
 * \return The LevelClass::Default logging level.
 */
inline
JEGA_SL_IEDECL
const LogLevel&
ldefault(
    )
{
    return LevelClass::Default;
}


/*
================================================================================
Class Declarations
================================================================================
*/
/// An entry that can only handle various kinds of text.
/**
 * This is a specialization of the eddy::logging::text_entry with default
 * template parameters that automatically inserts date, time, and level
 * information in front of all entries.
 */
class JEGA_SL_IEDECL text_entry :
    public eddy::logging::text_entry<>
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
        * \brief Constructs a text_entry with time and level initial text.
        *
        * These entries will always begin like:
        * \verbatim
            DDD MMM DD HH:MM:SS YYYY: LEVEL-
          \endverbatim
        *
        * \param l The level at which this entry is being issued.
        */
        text_entry(
            const LogLevel& l
            );

        /**
        * \brief Constructs a text_entry initial text plus supplied text.
        *
        * These entries will always begin like:
        * \verbatim
            DDD MMM DD HH:MM:SS YYYY: LEVEL- <itext>
          \endverbatim
        *
        * \param l The level at which this entry is being issued.
        * \param itext The leading initial text of this entry.  More text can
        *              be added on if necessary.
        */
        text_entry(
            const LogLevel& l,
            const string_type& itext
            );

        /**
        * \brief Constructs a text_entry initial text plus supplied text.
        *
        * These entries will always begin like:
        * \verbatim
            DDD MMM DD HH:MM:SS YYYY: LEVEL- <itext>
        \endverbatim
        *
        * \param l The level at which this entry is being issued.
        * \param itext The leading initial text of this entry.  More text can
        *              be added on if necessary.
        */
        text_entry(
            const LogLevel& l,
            const char_type* itext
            );

}; // class text_entry

/// An entry that can handle all ostreamable content.
/**
 * This is a specialization of the eddy::logging::ostream_entry with default
 * template parameters that automatically inserts date, time, and level
 * information in front of all entries.
 */
class JEGA_SL_IEDECL ostream_entry :
    public eddy::logging::ostream_entry<>
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
        * \brief Constructs an ostream_entry with time and level initial text.
        *
        * These entries will always begin like:
        * \verbatim
            DDD MMM DD HH:MM:SS YYYY: LEVEL-
          \endverbatim
        *
        * \param l The level at which this entry is being issued.
        */
        ostream_entry(
            const LogLevel& l
            );

        /**
        * \brief Constructs an ostream_entry initial text plus supplied text.
        *
        * These entries will always begin like:
        * \verbatim
            DDD MMM DD HH:MM:SS YYYY: LEVEL- <itext>
          \endverbatim
        *
        * \param l The level at which this entry is being issued.
        * \param itext The leading initial text of this entry.  More text can
        *              be added on if necessary.
        */
        ostream_entry(
            const LogLevel& l,
            const string_type& itext
            );

}; // class ostream_entry

/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Logging
} // namespace JEGA


/*
================================================================================
Macro Interface
================================================================================
*/

/// A macro to execute the statement (a) if JEGA_LOGGING_ON is defined.
/**
 * See EDDY_LOGGING_IF_ON for more details.
 *
 * \param a The statement to conditionally create.
 */
#define JEGA_LOGGING_IF_ON(a)                                               \
    EDDY_LOGGING_IF_ON(a)

/// A macro to execute the statement (a) if JEGA_LOGGING_ON is not defined.
/**
 * See EDDY_LOGGING_IF_OFF for more details.
 *
 * \param a The statement to conditionally create.
 */
#define JEGA_LOGGING_IF_OFF(a)                                              \
    EDDY_LOGGING_IF_OFF(a)

/**
 * \brief Logs entry (e) at level (ll) checked against the default with
 *        the suplied Logger class object.
 *
 * See documentation of EDDY_LOGGING_LEVEL_LOG_D for more information.
 *
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param e The entry to log.
 */
#define JEGALOG(logger, ll, e)                                              \
    EDDY_LOGGING_LEVEL_LOG_D(logger.Gate(), ll, e)

/**
 * \brief Logs entry (e) at level (ll) checked against the default with
 *        the Logger class object "Logger::Global()".
 *
 * This macro uses the JEGALOG macro with the global logger instance.
 *
 * \param ll The level to supply to the gateway at which to log (e).
 * \param e The entry to log.
 */
#define JEGALOG_G(ll, e)                                                    \
    JEGALOG(JEGA::Logging::Logger::Global(), ll, e)

/**
 * \brief Logs entry (e) at level (ll) checked against the default with
 *        the suplied Logger class object if cond evaluates to true.
 *
 * See documentation of EDDY_LOGGING_LEVEL_IFLOG_D for more information.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG(cond, logger, ll, e)                                      \
    EDDY_LOGGING_LEVEL_IFLOG_D(cond, logger.Gate(), ll, e)

/**
 * \brief Logs entry (e) at level (ll) checked against the default with
 *        the Logger class object "Logger::Global()" if cond evaluates to true.
 *
 * This macro uses the JEGAIFLOG macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ll The level to supply to the gateway at which to log (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_G(cond, ll, e)                                            \
    JEGAIFLOG(cond, JEGA::Logging::Logger::Global(), ll, e)

/**
 * \brief Logs entry (e) at level (ll) checked against the default with
 *        the suplied Logger class object if cond evaluates to true.
 *
 * See documentation of EDDY_LOGGING_LEVEL_IFLOG_CF_D for more information.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF(cond, logger, ll, e)                                      \
    EDDY_LOGGING_LEVEL_IFLOG_CF_D(cond, logger.Gate(), ll, e)

/**
 * \brief Logs entry (e) at level (ll) checked against the default with
 *        the Logger class object "Logger::Global()" if cond evaluates to true.
 *
 * This macro uses the JEGAIFLOG_CF macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ll The level to supply to the gateway at which to log (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_G(cond, ll, e)                                            \
    JEGAIFLOG_CF(cond, JEGA::Logging::Logger::Global(), ll, e)

/**
 * \brief Logs entry (e) at level (ll) if the level for issuer (ii) allows
 *        with the suplied Logger class object.
 *
 * See documentation of EDDY_LOGGING_LEVEL_LOG_II for more information.
 *
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_II(logger, ll, ii, e)                                       \
    EDDY_LOGGING_LEVEL_LOG_II(logger.Gate(), ll, ii, e)

/**
 * \brief Logs entry (e) at level (ll) if the level for issuer (ii) allows
 *        with the Logger class object "Logger::Global()".
 *
 * This macro uses the JEGALOG_II macro with the global logger instance.
 *
 * \param ll The level to supply to the gateway at which to log (e).
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_II_G(ll, ii, e)                                             \
    JEGALOG_II(JEGA::Logging::Logger::Global(), ll, ii, e)

/**
 * \brief Logs entry (e) at level (ll) with the supplied Logger class object
 *        if the level for issuer (ii) allows and cond evaluates to true.
 *
 * See documentation of EDDY_LOGGING_LEVEL_IFLOG_II for more information.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_II(cond, logger, ll, ii, e)                               \
    EDDY_LOGGING_LEVEL_IFLOG_II(cond, logger.Gate(), ll, ii, e)

/**
 * \brief Logs entry (e) at level (ll) with the Logger class object
 *        "Logger::Global()" if the level for issuer (ii) allows and cond
 *        evaluates to true.
 *
 * This macro uses the JEGAIFLOG_II macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ll The level to supply to the gateway at which to log (e).
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_II_G(cond, ll, ii, e)                                     \
    JEGAIFLOG_II(cond, JEGA::Logging::Logger::Global(), ll, ii, e)

/**
 * \brief Logs entry (e) at level (ll) with the supplied Logger class object
 *        if the level for issuer (ii) allows and cond evaluates to true.
 *
 * See documentation of EDDY_LOGGING_LEVEL_IFLOG_CF_II for more information.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_II(cond, logger, ll, ii, e)                               \
    EDDY_LOGGING_LEVEL_IFLOG_CF_II(cond, logger.Gate(), ll, ii, e)

/**
 * \brief Logs entry (e) at level (ll) with the Logger class object
 *        "Logger::Global()" if the level for issuer (ii) allows and cond
 *        evaluates to true.
 *
 * This macro uses the JEGAIFLOG_CF_II macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ll The level to supply to the gateway at which to log (e).
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_II_G(cond, ll, ii, e)                                  \
    JEGAIFLOG_CF_II(cond, JEGA::Logging::Logger::Global(), ll, ii, e)

/**
 * \brief Logs entry (e) at level (ll) with the supplied Logger class object
 *             if the level for issuer type (it) allows.
 *
 * See documentation of EDDY_LOGGING_LEVEL_LOG_IT for more information.
 *
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_IT(logger, ll, it, e)                                       \
    EDDY_LOGGING_LEVEL_LOG_IT(logger.Gate(), ll, it, e)

/**
 * \brief Logs entry (e) at level (ll) with the Logger class object
 *             "Logger::Global()" if the level for issuer type (it) allows.
 *
 * This macro uses the JEGALOG_IT macro with the global logger instance.
 *
 * \param ll The level to supply to the gateway at which to log (e).
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_IT_G(ll, it, e)                                             \
    JEGALOG_IT(JEGA::Logging::Logger::Global(), ll, it, e)

/**
 * \brief Logs entry (e) at level (ll) with the supplied Logger class object
 *        if the level for issuer type (it) allows and cond evaluates to true.
 *
 * See documentation of EDDY_LOGGING_LEVEL_IFLOG_IT for more information.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_IT(cond, logger, ll, it, e)                               \
    EDDY_LOGGING_LEVEL_IFLOG_IT(cond, logger.Gate(), ll, it, e)

/**
 * \brief Logs entry (e) at level (ll) with the Logger class object
 *        "Logger::Global()" if the level for issuer type (it) allows and cond
 *        evaluates to true.
 *
 * This macro uses the JEGAIFLOG_IT macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ll The level to supply to the gateway at which to log (e).
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_IT_G(cond, ll, it, e)                                     \
    JEGAIFLOG_IT(cond, JEGA::Logging::Logger::Global(), ll, it, e)

/**
 * \brief Logs entry (e) at level (ll) with the supplied Logger class object
 *        if the level for issuer type (it) allows and cond evaluates to true.
 *
 * See documentation of EDDY_LOGGING_LEVEL_IFLOG_CF_IT for more information.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ll The level to supply to the gateway at which to log (e).
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_IT(cond, logger, ll, it, e)                            \
    EDDY_LOGGING_LEVEL_IFLOG_CF_IT(cond, logger.Gate(), ll, it, e)

/**
 * \brief Logs entry (e) at level (ll) with the Logger class object
 *        "Logger::Global()" if the level for issuer type (it) allows and cond
 *        evaluates to true.
 *
 * This macro uses the JEGAIFLOG_CF_IT macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ll The level to supply to the gateway at which to log (e).
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_IT_G(cond, ll, it, e)                                  \
    JEGAIFLOG_CF_IT(cond, JEGA::Logging::Logger::Global(), ll, it, e)




/*
================================================================================
Logging Initialization Interface
================================================================================
*/
#ifdef JEGA_LOGGING_ON

/// Calls Logger::InitGlobalLogger with the supplied default level
#define JEGA_LOGGING_INIT(filename, def_level)                              \
    JEGA::Logging::Logger::InitGlobalLogger(filename, def_level);

#else // if !defined JEGA_LOGGING_ON

/// Does nothing because JEGA_LOGGING_ON is not defined.
#define JEGA_LOGGING_INIT(fileName, def_level)

#endif // JEGA_LOGGING_ON



/*
================================================================================
Fatal Logging Interface
================================================================================
*/
#ifdef JEGA_LOGGING_ON

/**
 * \brief A special macro for logging fatal level entries to the supplied
 *        Logger class object in the JEGA project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGALOG macro with a fatal logging
 * level.
 *
 * \param logger The Logger class object to log this entry to.
 * \param e The entry to log.
 */
#define JEGALOG_F(logger, e)                                                \
    JEGALOG(logger, JEGA::Logging::LevelClass::fatal, e)

/**
 * \brief A special macro for logging fatal level entries to the
 *        "Logger::Global" Logger class object in the JEGA project.
 *
 * This macro uses the JEGALOG_F macro with the global logger instance.
 *
 * \param e The entry to log.
 */
#define JEGALOG_G_F(e)                                                      \
    JEGALOG_F(JEGA::Logging::Logger::Global(), e)

/**
 * \brief A special macro for if-logging fatal level entries to the
 *        supplied Logger class object in the JEGA project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param e The entry to log.
 */
#define JEGAIFLOG_F(cond, logger, e)                                        \
    JEGAIFLOG(cond, logger, JEGA::Logging::LevelClass::fatal, e)

/**
 * \brief A special macro for if-logging fatal level entries to the
 *        "Logger::Global" Logger class object in the JEGA project.
 *
 * This macro uses the JEGAIFLOG_F macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param e The entry to log.
 */
#define JEGAIFLOG_G_F(cond, e)                                              \
    JEGAIFLOG_F(cond, JEGA::Logging::Logger::Global(), e)

/**
 * \brief A special macro for if-logging fatal level entries to the
 *        supplied Logger class object in the JEGA project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_F(cond, logger, e)                                     \
    JEGAIFLOG_CF(cond, logger, JEGA::Logging::LevelClass::fatal, e)

/**
 * \brief A special macro for if-logging fatal level entries to the
 *        "Logger::Global" Logger class object in the JEGA project.
 *
 * This macro uses the JEGAIFLOG_CF_F macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_G_F(cond, e)                                           \
    JEGAIFLOG_CF_F(cond, JEGA::Logging::Logger::Global(), e)

/**
 * \brief A special macro for logging fatal level entries from a particular
 *        issuer instance to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGALOG_II macro with a fatal logging
 * level.
 *
 * \param logger The Logger class object to log this entry to.
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_II_F(logger, ii, e)                                         \
    JEGALOG_II(logger, JEGA::Logging::LevelClass::fatal, ii, e)

/**
 * \brief A special macro for logging fatal level entries from a particular
 *        issuer instance to the "Logger::Global" Logger class object in the
 *        JEGA project.
 *
 * This macro uses the JEGALOG_II_F macro with the global logger instance.
 *
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_II_G_F(ii, e)                                               \
    JEGALOG_II_F(JEGA::Logging::Logger::Global(), ii, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer instance to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_II macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_II_F(cond, logger, ii, e)                                 \
    JEGAIFLOG_II(cond, logger, JEGA::Logging::LevelClass::fatal, ii, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer instance to the "Logger::Global" Logger class object in the
 *        JEGA project.
 *
 * This macro uses the JEGAIFLOG_II_F macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_II_G_F(cond, ii, e)                                       \
    JEGAIFLOG_II_F(cond, JEGA::Logging::Logger::Global(), ii, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer instance to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF_II macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_II_F(cond, logger, ii, e)                              \
    JEGAIFLOG_CF_II(cond, logger, JEGA::Logging::LevelClass::fatal, ii, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer instance to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF_II_F macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_II_G_F(cond, ii, e)                                    \
    JEGAIFLOG_CF_II_F(cond, JEGA::Logging::Logger::Global(), ii, e)

/**
 * \brief A special macro for logging fatal level entries from a particular
 *        issuer type to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGALOG_IT macro with a fatal logging
 * level.
 *
 * \param logger The Logger class object to log this entry to.
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_IT_F(logger, it, e)                                         \
    JEGALOG_IT(logger, JEGA::Logging::LevelClass::fatal, it, e)

/**
 * \brief A special macro for logging fatal level entries from a particular
 *        issuer type to the "Logger::Global" Logger class object in the JEGA
 *        project.
 *
 * This macro uses the JEGALOG_IT_F macro with the global logger instance.
 *
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGALOG_IT_G_F(it, e)                                               \
    JEGALOG_IT_F(JEGA::Logging::Logger::Global(), it, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer type to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_IT macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_IT_F(cond, logger, it, e)                                 \
    JEGAIFLOG_IT(cond, logger, JEGA::Logging::LevelClass::fatal, it, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer type to the "Logger::Global" Logger class object in the JEGA
 *        project.
 *
 * This macro uses the JEGAIFLOG_IT_F macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_IT_G_F(cond, it, e)                                       \
    JEGAIFLOG_IT_F(cond, JEGA::Logging::Logger::Global(), it, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer type to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF_IT macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_IT_F(cond, logger, it, e)                              \
    JEGAIFLOG_CF_IT(cond, logger, JEGA::Logging::LevelClass::fatal, it, e)

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer type to the "Logger::Global" Logger class object in the JEGA
 *        project.
 *
 * This macro uses the JEGAIFLOG_CF_IT_F macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_IT_G_F(cond, it, e)                                    \
    JEGAIFLOG_CF_IT_F(cond, JEGA::Logging::Logger::Global(), it, e)


#else // if !defined JEGA_LOGGING_ON

// If JEGA_LOGGING_ON is not defined, we must none the less make sure that our
// fatal log entries get issued and provide meaningful feedback.  That's what
// these alternate log macros do.

/// A macro to produce a non descript failure output message.
#define JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e)                          \
    JEGA::Logging::Logger::fatal_callback::Invoke(e);

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param logger The Logger class object to log this entry to. Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGALOG_F(logger, e)                                                \
    { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGALOG_G_F(e)                                                      \
    JEGALOG_F(nolog, e)

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to abort.
 * \param logger The Logger class object to log this entry to. Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGAIFLOG_F(cond, logger, e)                                        \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to abort.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGAIFLOG_G_F(cond, e)                                              \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_F(cond, logger, e)                                     \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_G_F(cond, e)                                           \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param logger The Logger class object to log this entry to. Ignored.
 * \param ii The issuer instance responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGALOG_II_F(logger, ii, e)                                         \
    { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param ii The issuer instance responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGALOG_II_G_F(ii, e)                                               \
    { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to abort.
 * \param logger The Logger class object to log this entry to. Ignored.
 * \param ii The issuer instance responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGAIFLOG_II_F(cond, logger, ii, e)                                 \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to abort.
 * \param ii The issuer instance responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGAIFLOG_II_G_F(cond, ii, e)                                       \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer instance to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF_II_G macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_II_F(cond, logger, ii, e)                              \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer instance to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF_II_G macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param ii The issuer instance responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_II_G_F(cond, ii, e)                                    \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param logger The Logger class object to log this entry to. Ignored.
 * \param it The issuer type responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGALOG_IT_F(logger, it, e)                                         \
    { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param it The issuer type responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.
 */
#define JEGALOG_IT_G_F(it, e)                                               \
    { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to abort.
 * \param logger The Logger class object to log this entry to. Ignored.
 * \param it The issuer type responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.S
 */
#define JEGAIFLOG_IT_F(cond, logger, it, e)                                 \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief Logging is off, so this prints an error message to stderr and aborts
 *        the program or throws an exception.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * The error message presented is the contents of the entry which must be
 * "ostream-able".
 *
 * \param cond The condition based on which to abort.
 * \param it The issuer type responsible for entry (e). Ignored.
 * \param e The entry whose contents will be displayed prior to program exit.S
 */
#define JEGAIFLOG_IT_G_F(cond, it, e)                                       \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer type to the supplied Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort in the case where logging is turned off.
 *
 * Otherwise, it is the same as the JEGAIFLOG_CF_IT macro with a fatal logging
 * level.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param logger The Logger class object to log this entry to.
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_IT_F(cond, logger, it, e)                              \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

/**
 * \brief A special macro for if-logging fatal level entries from a particular
 *        issuer type to the "Logger::Global" Logger class object in the JEGA
 *        project.
 *
 * This macro exists so that fatal entries can be replaced by program
 * abort or an exception in the case where logging is turned off.
 *
 * This macro uses the JEGAIFLOG_CF_IT_F macro with the global logger instance.
 *
 * \param cond The condition based on which to log "e" on "logger".
 * \param it The issuer type responsible for entry (e).
 * \param e The entry to log.
 */
#define JEGAIFLOG_CF_IT_G_F(cond, it, e)                                    \
    { if(cond) { JEGA_LOGGING_NO_LOG_FATAL_ERROR_HANDLE(e) } }

#endif // JEGA_LOGGING_ON

/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// no inline file for configuration file.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_LOGGING_HPP
