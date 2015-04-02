/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Implementation of class EDDY_DebugScope.

    NOTES:

        See notes of EDDY_DebugScope.hpp.

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
 * \brief Contains the implementation of the EDDY_DebugScope class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include "../include/EDDY_DebugScope.hpp"

#ifdef EDDY_OPTION_DEBUG

#   include <csignal>
#   include <cstdlib>
#   include <iostream>

#define STREAM std::cerr

#   ifdef EDDY_THREADSAFE
#       include "../../threads/include/mutex_lock.hpp"
        using namespace eddy::threads;
#   endif

#endif






/*
================================================================================
Namespace Using Directives
================================================================================
*/
#ifdef EDDY_OPTION_DEBUG
using namespace std;
EDDY_IF_THREADSAFE(using namespace eddy::threads;)
#endif






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
Global Utility Functions
================================================================================
*/
EDDY_IF_THREADSAFE(
    inline
    mutex&
    stack_mutex() {
        // do not allocate this on the stack.  Put it on the heap.  Don't worry
        // about the leak.  It is small and it will be cleaned up when the
        // process is exited.  If we put it on the stack, it may be cleaned up
        // before other static items that use it.
        static mutex* m = new mutex(PTHREAD_MUTEX_RECURSIVE);
        return *m;
    }
)

/*
================================================================================
Static Member Data Definitions
================================================================================
*/
EDDY_DebugScope* EDDY_DebugScope::_top = 0x0;







/*
================================================================================
Mutators
================================================================================
*/








/*
================================================================================
Accessors
================================================================================
*/








/*
================================================================================
Public Methods of EDDY_DebugScope
================================================================================
*/
void
EDDY_DebugScope::_PrintScopeTrace(
    ) throw()
{
    EDDY_IF_THREADSAFE(mutex_lock l(stack_mutex());)
    STREAM << "SCOPE TRACE:";
    for(EDDY_DebugScope* iter=_top; iter!=0x0; iter=iter->_next)
        STREAM << '\n' << iter->_data;

    STREAM << '\n';
}







/*
================================================================================
Public Methods of EDDY_Debug
================================================================================
*/

void
EDDY_Debug::_Report(
    const char* message,
    const char* fileName,
    long line
    ) throw()
{
    STREAM << "\nEDDY REPORT:\n" << message
           << "\n... in file \"" << fileName << "\", line " << line << "\n\n";

    EDDY_DebugScope::_PrintScopeTrace();
    STREAM << '\n';
    ::abort();
}

void
EDDY_Debug::_Warning(
    const char* message,
    const char* fileName,
    long line
    ) throw()
{
    STREAM << "\nEDDY WARNING:\n" << message
           << "\n... in file \"" << fileName << "\", line " << line << "\n\n";

    EDDY_DebugScope::_PrintScopeTrace();
    STREAM << '\n';
}

void
EDDY_Debug::_Message(
    const char* message,
    const char* fileName,
    long line
    ) throw()
{
    STREAM << "\nEDDY MESSAGE:\n" << message
           << "\n... in file \"" << fileName << "\", line " << line << "\n\n";
}

void
EDDY_Debug::_Assert(
    const char* message,
    const char* fileName,
    long line
    ) throw()
{
    STREAM << "\nEDDY ASSERTION FAILURE:\n" << message
           << "\n... in file \"" << fileName << "\", line " << line << "\n\n";

    EDDY_DebugScope::_PrintScopeTrace();
    STREAM << '\n';
    ::abort();
}

void
EDDY_Debug::_Signal(
    int val
    ) throw()
{
    STREAM << "\nEDDY DEBUGGING SIGNAL CAUGHT:\nvalue = " << val << ' ';

    switch(val)
    {
        case SIGSEGV: STREAM << "(SIGSEGV)"; break;
        case SIGINT : STREAM << "(SIGINT)"; break;
        case SIGFPE : STREAM << "(SIGFPE)"; break;
        case SIGILL : STREAM << "(SIGILL)"; break;
        case SIGTERM: STREAM << "(SIGTERM)"; break;
        case SIGABRT: STREAM << "(SIGABRT)"; break;
        default: STREAM << "(UNKNOWN = " << val << ')';
    }

    STREAM << "\n\n";
    EDDY_DebugScope::_PrintScopeTrace();
    STREAM << '\n';

    ::exit(3);
}

void
EDDY_Debug::_RegisterSignal(
    int a
    ) throw()
{
    ::signal(a, &EDDY_Debug::_Signal);

} // EDDY_Debug::_RegisterSignal







/*
================================================================================
Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Subclass Overridable Methods
================================================================================
*/








/*
================================================================================
Private Methods
================================================================================
*/








/*
================================================================================
Structors
================================================================================
*/


EDDY_DebugScope::EDDY_DebugScope(
    const char* name
    ) throw() :
        _data(name)
{
    EDDY_IF_THREADSAFE(mutex_lock l(stack_mutex());)
    _next = _top;
    _top = this;
}

EDDY_DebugScope::~EDDY_DebugScope() throw()
{
    EDDY_IF_THREADSAFE(mutex_lock l(stack_mutex());)
    _top = _next;
}




#endif // #ifdef EDDY_OPTION_DEBUG

/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy
