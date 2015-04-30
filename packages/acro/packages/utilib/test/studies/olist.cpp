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
// olist.cpp
//
// Test routine for ordered lists
//
#include <utilib_config.h>
#include <utilib/CommonIO.h>
#include <utilib/OrderedList.h>

using namespace utilib;
#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif


void do_write(const OrderedList<void*,double>& olist)
{
ucout << "LIST: " << Flush;
OrderedListItem<void*,double>* item=olist.head();
while (item) {
      ucout << "(" << item->key()  << ") ";
      item = olist.next(item);
      }
ucout << endl;
}


int test_olist(int,char**)
{
int i=0;

while (i++ < 100) {
OrderedList<void*,double>* olist = new OrderedList<void*,double>();
void* ptr;
double key;

key=1.0;
olist->add(ptr,key);
do_write(*olist);

key=2.0;
olist->add(ptr,key);
do_write(*olist);

key=3.0;
olist->add(ptr,key);
do_write(*olist);

delete olist;
}

return 0;
}
