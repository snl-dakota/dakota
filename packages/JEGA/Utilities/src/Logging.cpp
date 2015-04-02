/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of the Logging setup for JEGA.

    NOTES:

        See notes of Logging.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Dec 16 13:31:15 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the Logging setup for JEGA.
 */

/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <ctime>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>

#ifndef JEGA_LOGGING_ON
#include <iostream>
#endif

using namespace std;

#ifdef JEGA_LOGGING_ON


#ifndef JEGA_LOGGING_NO_FILE
#   define _FILE_ARGS(fname) (fname)
#   define JEGA_LOGGING_IF_FILE_LOGGING(a) a
#   define JEGA_LOGGING_IF_NO_FILE_LOGGING(a)
#   include <fstream>
#else
#   define _FILE_ARGS(fname)
#   define JEGA_LOGGING_IF_FILE_LOGGING(a)
#   define JEGA_LOGGING_IF_NO_FILE_LOGGING(a) a
#endif

#ifndef JEGA_LOGGING_NO_CONSOLE
#   define _CONSOLE_ARGS(str) (str)
#   define JEGA_LOGGING_IF_CONSOLE_LOGGING(a) a
#   define JEGA_LOGGING_IF_NO_CONSOLE_LOGGING(a)
#   include <iostream>

/// A shorthand for the basic output stream type.
typedef
basic_ostream<EDDY_LOGGING_DEF_CHAR_TYPE, EDDY_LOGGING_DEF_CHAR_TRAITS>
ostream_type;

// define the name of the output stream that will get used.  It is either cout
// or wcout depending on whether or not wide characters are in use.
#ifdef JEGA_LOGGING_WIDE_CHARS
#   define CONSOLE_STREAM wcout
#else
#   define CONSOLE_STREAM cout
#endif

#else
#   define _CONSOLE_ARGS(str)
#   define JEGA_LOGGING_IF_CONSOLE_LOGGING(a)
#   define JEGA_LOGGING_IF_NO_CONSOLE_LOGGING(a) a
#endif






/*
================================================================================
Namespace Using Directives
================================================================================
*/





/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Logging {

/// A callback class for use when a fatal logging occurs.
class Logger::fatal_callback :
    public gate_t::callback
{
    public:

        static FatalBehavior onFatal;

    /*
    ===================================================================
    Subclass Overridable Methods
    ===================================================================
    */
    public:

        /// An instance of the fatal callback for convenience use.
        static
        fatal_callback*
        Instance(
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            static fatal_callback instance;
            return &instance;
        }

        /// The activation method of this callback.
        /**
         * This prints an indication that the error was received into
         * the actual log, bypassing the gateway, and then aborts the
         * program.
         *
         * \param e The argument that will be passed when this
         *          callback is invoked.
         */
        virtual
        void
        operator () (
            const gate_t::callback_arg_type& e
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            Logger::Global().Gate().get_log().log(
                JEGA_LOGGING_AS_STRING_LITERAL(
                    "Received a fatal logging callback.  Aborting."
                    )
                );

            EDDY_TRACESCOPE
            if(onFatal == Logger::ABORT) exit(5);
            if(onFatal == Logger::THROW) throw std::runtime_error(
                "Received a fatal callback with message: " +
                static_cast<const std::string&>(e)
                );
        }

}; // class Logger::fatal_callback


/*
================================================================================
Static Member Data Definitions
================================================================================
*/
Logger::FatalBehavior Logger::fatal_callback::onFatal = Logger::ABORT;
auto_ptr<Logger> Logger::_global(0x0);
file_log Logger::_globalFLog;
ostr_log Logger::_globalOLog _CONSOLE_ARGS(CONSOLE_STREAM);

void
Logger::InitGlobalLogger(
    const string_type& filename,
    const LogLevel& def_level
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(_global.get() == 0x0) _global.reset(new Logger("Global Log"));

    JEGA_LOGGING_IF_FILE_LOGGING(
        if(!filename.empty())
            _global->Gate().get_log().get_first_log().set_filename(filename);
        )

    _global->Gate().set_default_level(def_level);

    InitLogger(*_global);
}

void
Logger::Init(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    InitLogger(*this);
}

void
Logger::InitLogger(
    Logger& logger
    )
{
    EDDY_FUNC_DEBUGSCOPE

    logger.Gate().set_callback(lfatal(), fatal_callback::Instance());

    logger.Gate().get_log().log(
        JEGA_LOGGING_AS_STRING_LITERAL(
            "JEGA Genetic Algorithm Optimization Software v" JEGA_VERSION
            )
        );

    time_t lttime;
    time( &lttime );
    logger.Gate().get_log().log(
        string_type(JEGA_LOGGING_AS_STRING_LITERAL("Run started ")) +
        ctime(&lttime)
        );

    logger.Gate().get_log().log(
        logger.GetName() +
        JEGA_LOGGING_AS_STRING_LITERAL(" initialized using \"") +
        LevelClass::to_ostreamable(logger.Gate().get_default_level()) +
        JEGA_LOGGING_AS_STRING_LITERAL("\" as default level.")
        );

    JEGA_LOGGING_IF_FILE_LOGGING(
        logger.Gate().get_log().log(
            logger.GetName() +
            JEGA_LOGGING_AS_STRING_LITERAL(" logging to file ") +
            logger.Gate().get_log().get_first_log().get_filename()
            );
        )

    JEGA_LOGGING_IF_CONSOLE_LOGGING(
        logger.Gate().get_log().log(
            logger.GetName() +
            JEGA_LOGGING_AS_STRING_LITERAL(" logging to console window")
            );
        )
}

Logger::FatalBehavior
Logger::GetFatalBehavior(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return Logger::fatal_callback::onFatal;
}

void
Logger::SetFatalBehavior(
    Logger::FatalBehavior newBehav
    )
{
    EDDY_FUNC_DEBUGSCOPE
    Logger::fatal_callback::onFatal = newBehav;
}

void
Logger::FlushStreams(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_LOGGING_IF_FILE_LOGGING(
        this->_gate.get_log().get_first_log().flush_stream();
        )

    JEGA_LOGGING_IF_CONSOLE_LOGGING(
        this->_gate.get_log().get_second_log().flush_stream();
        )
}

const std::string&
Logger::GetFilename(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_LOGGING_IF_FILE_LOGGING(
        return _global->Gate().get_log().get_first_log().get_filename();
        )

    JEGA_LOGGING_IF_NO_FILE_LOGGING(
        return std::string();
        )
}


Logger::Logger(
    const string_type& loggerName,
    const LogLevel& def_level
    ) :
        _flog(new file_log()),
        _dlog(_globalFLog, _globalOLog),
        _gate(_dlog, def_level),
        _name(loggerName)
{
    EDDY_FUNC_DEBUGSCOPE
}

Logger::Logger(
    const string_type& filename,
    const string_type& loggerName,
    const LogLevel& def_level
    ) :
        _flog(new file_log _FILE_ARGS(filename)),
        _dlog(*(_flog.get()), _globalOLog),
        _gate(_dlog, def_level),
        _name(loggerName)
{
    EDDY_FUNC_DEBUGSCOPE
}

Logger::Logger(
    file_log& flog,
    const string_type& loggerName,
    const LogLevel& def_level
    ) :
        _flog(new file_log()),
        _dlog(flog, _globalOLog),
        _gate(_dlog, def_level),
        _name(loggerName)
{
    EDDY_FUNC_DEBUGSCOPE
}





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Logging
} // namespace JEGA

#else

namespace JEGA {
    namespace Logging {

        Logger::FatalBehavior Logger::fatal_callback::onFatal = Logger::ABORT;

    } // namespace Logging
} // namespace JEGA

#endif // JEGA_LOGGING_ON



/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Logging {


/*
================================================================================
Static Member Data Definitions
================================================================================
*/





/*
================================================================================
Class Method Implementations
================================================================================
*/
text_entry::text_entry(
    const LogLevel& l
    ) :
        eddy::logging::text_entry<>()
{
    EDDY_FUNC_DEBUGSCOPE
    this->reserve(19);
    time_t lttime;
    time( &lttime );
    char* ct = ctime(&lttime);
    this->append((char_type*)ct+11, 8) << ": " << LevelClass::to_ostreamable(l)
        << "- ";
}

text_entry::text_entry(
    const LogLevel& l,
    const string_type& itext
    ) :
        eddy::logging::text_entry<>()
{
    EDDY_FUNC_DEBUGSCOPE
    this->reserve(19 + itext.size());
    time_t lttime;
    time( &lttime );
    char* ct = ctime(&lttime);
    this->append((char_type*)ct+11, 8) << ": " << LevelClass::to_ostreamable(l)
        << "- " << itext;
}

text_entry::text_entry(
    const LogLevel& l,
    const char_type* itext
    ) :
        eddy::logging::text_entry<>()
{
    EDDY_FUNC_DEBUGSCOPE
    this->reserve(19 + 50);
    time_t lttime;
    time( &lttime );
    char* ct = ctime(&lttime);
    this->append((char_type*)ct+11, 8) << ": " << LevelClass::to_ostreamable(l)
        << "- " << itext;
}

ostream_entry::ostream_entry(
    const LogLevel& l
    ) :
        eddy::logging::ostream_entry<>()
{
    EDDY_FUNC_DEBUGSCOPE
    time_t lttime;
    time( &lttime );
    char* ct = ctime(&lttime);
    copy(ct+11, ct+19, std::ostream_iterator<char_type>(this->get_ostream()));
    (*this) << ": " << LevelClass::to_ostreamable(l) << "- ";
    this->get_ostream().flush();
}

ostream_entry::ostream_entry(
    const LogLevel& l,
    const string_type& itext
    ) :
        eddy::logging::ostream_entry<>()
{
    EDDY_FUNC_DEBUGSCOPE
    time_t lttime;
    time( &lttime );
    char* ct = ctime(&lttime);
    copy(ct+11, ct+19, std::ostream_iterator<char_type>(this->get_ostream()));
    (*this) << ": " << LevelClass::to_ostreamable(l) << "- " << itext;
    this->get_ostream().flush();
}


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Logging
} // namespace JEGA

