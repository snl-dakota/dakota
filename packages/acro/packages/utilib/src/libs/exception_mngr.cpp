/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

//
// exception_mngr.cpp
//

#include <utilib_config.h>
#include <utilib/exception_mngr.h>
#include <utilib/CommonIO.h>

#ifdef UTILIB_HAVE_MPI
#include <mpi.h>
#endif
#ifdef UTILIB_HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#ifdef UTILIB_HAVE_CXXABI_H
#include <cxxabi.h>
#endif
#ifdef _MSC_VER
#ifdef UTILIB_HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef UTILIB_HAVE_DBGHELP_H
#include <dbghelp.h>
#endif
#endif

#include <cstdlib>

using std::endl;
using std::string;

namespace utilib {

/// Default no-op exit function
void default_exitfn()
  { }


std::string demangledName(const char* mangled)
{
#if defined(UTILIB_HAVE_CXXABI_H)
   int stat;
   char *demangled = abi::__cxa_demangle(mangled,0,0,&stat);
   std::string ans = ( demangled == NULL ? mangled : demangled );
   if (demangled)
      free(demangled);

   return ans;
#elif defined(_MSC_VER) && defined(UTILIB_HAVE_WINDOWS_H)
   // I think (hope?!?) this works...
   CHAR buf[1024];
   DWORD len
      = UnDecorateSymbolName( mangled, buf, sizeof(buf), 
                              UNDNAME_COMPLETE );
   return std::string( len == 0 ? mangled : buf );
#else
   return mangled;
#endif
}


namespace exception_mngr {

namespace {

/// local variable to store exception mode
handle_t exception_mode = Standard;

/// local variable to store current exception pointer
null_fn_type exit_function_pointer = &default_exitfn;

/// local variable to store the state of the stack_trace() flag
#ifdef NDEBUG
bool stack_trace_flag = ( getenv("UTILIB_STACK_TRACE") != NULL );
#else
bool stack_trace_flag = true;
#endif

} // namespace (local)


// actually declare the static exception message buffer
std::string exception_message_buffer = string();


handle_t mode()
  { return exception_mode; }

void set_mode(handle_t mode_) 
  { exception_mode = mode_; }


void exit_function()
  { exit_function_pointer(); }

void set_exit_function(null_fn_type fn) 
  { 
  if ( fn == NULL ) 
    { fn = &default_exitfn; }
  exit_function_pointer = fn; 
  }

bool stack_trace()
{
   return stack_trace_flag; 
}

void set_stack_trace(bool stack)
{
   stack_trace_flag = stack;
}



void generate_stack_trace(std::ostringstream &os)
{
#ifdef UTILIB_HAVE_EXECINFO_H
   void * array[25];
   int nSize = backtrace(array, 25);
   char ** symbols = backtrace_symbols(array, nSize);
   if ( symbols == NULL )
      return;
   size_t symbols_found = 0;

   os << "Stack trace:" << endl;
   for (int i = 0; i < nSize; i++)
   {
      os << "     ";
      char* sym_s = strchr(symbols[i], '(');
      char* sym_e = strrchr(symbols[i], ')');
      char* sym_o = ( sym_s == NULL ? NULL : strrchr(sym_s, '+') );
      if (( sym_s != NULL ) && ( sym_o > sym_s+1 ) && ( sym_e > sym_o ))
      {
         ++symbols_found;
         os << string(symbols[i], sym_s-symbols[i]) << ": ";

         // Split symbol & offset into separate C strings (overwrite the "+")
         *sym_o = 0;
         os << demangledName(sym_s+1) << " (+" << string(sym_o+1) << endl;
      }
      else
         os << symbols[i] << endl;
   }
   free(symbols);
   if (( symbols_found < 4 ) && ( nSize > 4 ))
      os << "(Missing trace symbols.  [gcc: Did you forget '-rdynamic'?])" 
         << endl;
#else
   os << "[stack trace disabled: compile with UTILIB_HAVE_EXECINFO_H]" << endl;
#endif
}


/** Process an exception based on the exception manager mode. */
void handle_exception( const ExceptionGenerator_base &exception, 
                       std::ostringstream& msg )
{
#ifdef UTILIB_HAVE_MPI
int mpiActive=0;
MPI_Initialized(&mpiActive);
if (mpiActive) {
   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   msg << "(PN" << rank << ")  ";
   }
#endif


msg << std::endl;
if ( stack_trace() )
   generate_stack_trace(msg);

switch ( utilib::exception_mngr::mode() ) {
  case Standard:
        utilib::exception_mngr::exit_function();
	//
	// Note: if we simply throw msg.str().c_str(), then
	// that may create a temporary char* that is deleted after the
	// throw occurs.  This can lead to the situation where the
	// catch block for this exception processes a deleted array.
	//
        exception_message_buffer = msg.str();
        msg.str("");
        msg << std::endl;
        exception_message_buffer.resize
           (exception_message_buffer.size() - msg.str().size());
        exception.throw_it( exception_message_buffer );

  case Abort:
        #ifdef UTILIB_HAVE_MPI
	/*
        if (mpiActive)
           MPI_Abort(MPI_COMM_WORLD,1);
	*/
        #endif
        utilib::exception_mngr::exit_function();
	ucerr << msg.str();
	CommonIO::end_all();
        std::abort();
        break;

  case Exit:
        #ifdef UTILIB_HAVE_MPI
        if (mpiActive)
           MPI_Abort(MPI_COMM_WORLD,1);
        #endif
        utilib::exception_mngr::exit_function();
	ucerr << msg.str();
	CommonIO::end_all();
        std::exit(-1);
  }
}


} // exception_mngr namespace

} // utilib namespace
