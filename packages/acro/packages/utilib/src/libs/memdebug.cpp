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
// memdebug.cpp
//
// Macros that can be used to debug memory allocation
//


#include <utilib_config.h>
#include <utilib/memdebug.h>

#ifdef UTILIB_YES_MEMDEBUG
#include <utilib/std_headers.h>
#include <utilib/CommonIO.h>

namespace utilib {

int memdebug::nbytes = 0;
int memdebug::n_news = 0;
int memdebug::n_dels = 0;
int memdebug::num    = 0;

BasicArray<CharString> memdebug::name(10);
IntVector memdebug::num_new(10);
IntVector memdebug::num_del(10);
IntVector memdebug::memory_allocated(10);
IntVector memdebug::memory_deleted(10);
IntVector memdebug::last_total(10);


void memdebug::print_summary(ostream& os)
{
stdcout << "Fail: " << os.fail() << endl;
os << "MEMDEBUG SUMMARY";
stdcout << "Fail: " << os.fail() << endl;
os << endl;
stdcout << "Fail: " << os.fail() << endl;

os << "  Calls to 'new'            : " << memdebug::n_news << endl;
stdcout << "Fail: " << os.fail() << endl;
os << "  Calls to 'delete'         : " << memdebug::n_dels << endl;
stdcout << "Fail: " << os.fail() << endl;
os << "  Total # of bytes allocated: " << memdebug::nbytes << endl;
stdcout << "Fail: " << os.fail() << endl;
os << endl;
stdcout << "Fail: " << os.fail() << endl;
for (int i=0; i<num; i++) {
  os << "CLASS: " << memdebug::name[i] << endl;
  os << "  Number of objects constructed: " << num_new[i] << endl;
  os << "  Total memory allocated:        " << memory_allocated[i] << endl;
  os << "  Avg object size allocated:     " 
			<< (((double)memory_allocated[i])/num_new[i]) << endl;
  //os << "  Number of objects destructed:  " << num_del[i] << endl;
  //os << "  Total memory deleted:          " << memory_deleted[i] << endl;
  //os << "  Avg object size deleted:       " 
			//<< (((double)memory_deleted[i])/num_del[i]) << endl;
  }
}

}


using namespace utilib;

void my_new_handler()
{
ucout << "\n\nERROR:: Out of memory after allocating " << memdebug::nbytes 
							<< "\n";
ucout << Flush;
abort();
}

/*
** TODO This failed to compile the first time UTILIB_YES_MEMDEBUG was ON
*/

#if defined(UTILIB_HAVE_EXCEPTIONS) && defined(__EDG_STD_NAMESPACE_IS_DEFINED)
void *operator new(size_t size) throw(__EDG_STD_NAMESPACE::bad_alloc)
#else
void *operator new(size_t size)
#endif
{
void* tmp = malloc(size);
cerr << endl << "MEMDEBUG NEW " << size << " " << tmp << endl;

memdebug::nbytes += size;
memdebug::n_news++;

//return ::new (void*) [size];
return tmp;
}


void operator delete(void* ptr)
{
cerr << endl << "MEMDEBUG DELETE " << ptr << endl;
memdebug::n_dels++;
free(ptr);
}


#endif
