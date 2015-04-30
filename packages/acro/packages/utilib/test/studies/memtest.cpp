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

//#define NEW
#include <utilib_config.h>
#define ARRAY BitArray
#define LEN 256
#include <utilib/std_headers.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

static size_t total=0;

#ifdef NEW

#include <utilib/BitArray.h>
#include <utilib/CharString.h>

using namespace utilib;

#ifdef UTILIB_HAVE_EXCEPTIONS
void *operator new(size_t size) throw(__EDG_STD_NAMESPACE::bad_alloc)
#else
void *operator new(size_t size)
#endif
{
total += size;
return ::new (void*) [size];
}
#endif

extern int profile_total_bytes_used;
extern "C" {
int heap_size();
VOID init_heap_malloc();
VOID init_malloc_sigheap();
int largest_free_block(unsigned int *link, int *largest_free);
int heap_info(int *fragments, int *total_free, int *largest_free,
    int *total_used);
VOID lputs_links();
VOID print_links();
};

static int last_fragments=0, last_total_free=0, last_largest_free=0;
static int last_total_used=0, last_profile=0;

void info(int flag=0)
{
int hsize = 0;
hsize = heap_size();
int fragments, total_free, largest_free, total_used;
int ret = heap_info(&fragments, &total_free, &largest_free, &total_used);
   ucout << "hsize=" << hsize;
   ucout << " Dfrag=" << (fragments-last_fragments);
   ucout << " Dtfree=" << (total_free-last_total_free);
   ucout << " Dlfree=" << (largest_free-last_largest_free);
   ucout << " Dtused=" << (total_used-last_total_used);
   ucout << " Dtbytes=" << (profile_total_bytes_used-last_profile);
   ucout << " ret=" << ret;
   ucout << " total=" << total;
   ucout << endl;
if (flag) {
   ucout << "hsize=" << hsize;
   ucout << " frag=" << fragments;
   ucout << " tfree=" << total_free;
   ucout << " lfree=" << largest_free;
   ucout << " tused=" << total_used;
   ucout << " tbytes=" << profile_total_bytes_used;
   ucout << " ret=" << ret;
   ucout << " total=" << total;
   ucout << endl;
   }
last_fragments=fragments;
last_total_free=total_free;
last_largest_free=largest_free;
last_total_used=total_used;
last_profile=profile_total_bytes_used;
}


int test_memtest(int,char**)
{
//CharString tmp;
BasicArray<char> foo;

ucout << "hsize\tThe heap size (from heap_size())" << endl;
ucout << "Dfrag\tDifferences in fragmentation" << endl;
ucout << "Dtfree\tDifferences in the total number of free bytes" << endl;
ucout << "Dtused\tDifferences in the number of heap bytes used" << endl;
ucout << "Dtbytes\tDifferences in the heap usage (from profile...)." << endl;
info();
info();
info();
ARRAY a1(LEN);
info();
ARRAY a2(LEN);
info();
ARRAY a3(LEN);
info();
ARRAY a4(LEN);
info(1);
ucout.flush();
}
