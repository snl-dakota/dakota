/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#include <acro_config.h>
#include <colin/XMLProcessor.h>
#include <utilib/TinyXML_helper.h>

// This is a bit aggressive, but a first cut
#if defined(_WIN32) || defined(HAVE_DLOPEN)

#ifdef _WIN32
  #include <windows.h>
  typedef HINSTANCE LIB_POINTER;
  #define LIB_EXTENSION ".dll"
#else
  #include <unistd.h>
  #include <dlfcn.h>
  typedef void* LIB_POINTER;
  #define LIB_EXTENSION ".so"
#endif

#ifdef _WIN32
#include <direct.h>
#define IS_REG_FILE(STAT) (( _S_IFREG & (STAT).st_mode ) > 0 )
#else
#define IS_REG_FILE(STAT) S_ISREG((STAT).st_mode)
#endif


#define DEBUG_LIBLOADER 0 

#include <iostream>
#include <sstream>
#include <cctype>
#include <sys/stat.h>
#include <iostream>

using std::cerr;
using std::endl;
using std::string;


namespace colin {


LIB_POINTER load(string name)
{
   LIB_POINTER lib_handle;

   // Ideally, we should look in common paths and for platform-specific
   // extensions, but for now, we will assume that the full (correct)
   // path is being provided.
   struct stat fs;
   if ( ! (( stat( name.c_str(), &fs ) == 0 ) && IS_REG_FILE(fs) ) )
   {
      cerr << "ERROR: load(): cannot find library '"
                << name << "'" << endl;
      return NULL;
   }

   // The actual loading stuff
#ifdef _WIN32
   unsigned int err_code = SetErrorMode(SEM_FAILCRITICALERRORS);
   lib_handle = LoadLibrary(name.c_str());
#else
   lib_handle = dlopen(name.c_str(), RTLD_LAZY);
#endif
   if (!lib_handle) 
   {
      cerr << endl << "Module " << name << endl;
      cerr << "Error during load(): ";
#ifdef _WIN32
      const int buflen = 1024;
      char buf[buflen];
      err_code = GetLastError();
      FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_code, 
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                     (LPTSTR)buf, buflen, NULL );
      cerr << buf << endl;
#else
      cerr << dlerror() << endl;
#endif
      return NULL;
   }
#ifdef _WIN32
   else
   {
      SetLastError(0); 
   }
#endif

   return lib_handle;
}


class LibraryLoadElement : public XML_Processor::ElementFunctor
{
public:
   LibraryLoadElement()
   {}

   virtual void process(TiXmlElement* root, int version)
   {
      string lib_name;
      utilib::get_string_attribute(root, "file", lib_name);
      if ( ! load(lib_name) )
         EXCEPTION_MNGR(std::runtime_error, "ERROR: Loading library '"
                        << lib_name << "' failed.");
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement LibraryLoadElement::describe" << endl;
   }
  
};


//---------------------------------------------------------------------

namespace StaticInitializers {
namespace {

bool RegisterFunctor()
{
   XMLProcessor().register_element("LoadLibrary", 1, new LibraryLoadElement);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool library_loader = RegisterFunctor();

} // namespace colin::StaticInitializers
} // namespace colin

#else  // defined(_WIN32) || defined(HAVE_DLOPEN)
// keep this separate to avoid intermingling the namespaces with the ifdefs
namespace colin {
  namespace StaticInitializers {
    extern const volatile bool library_loader = false;
  } // namespace colin::StaticInitializers
} // namespace colin

#endif  // defined(_WIN32) || defined(HAVE_DLOPEN)
