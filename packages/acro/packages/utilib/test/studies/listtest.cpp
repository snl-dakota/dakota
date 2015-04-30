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
// list.cpp
//
// Test routine for linked lists
//
#include <utilib_config.h>
#include <utilib/CommonIO.h>
#include <utilib/LinkedList.h>

using namespace utilib;


void do_write(const LinkedList<double>& llist)
{
{
ucout << "LIST: " << Flush;
LinkedList<double>::const_iterator curr = llist.begin();
LinkedList<double>::const_iterator end = llist.end();
while (curr != end) {
      ucout << "(" << *curr  << ") ";
      curr++;
      }
ucout << std::endl;
}
{
ucout << "LIST: " << Flush;
ListItem<double>* curr = llist.head();
while (curr) {
  ucout << "[" << curr->data() << "] ";
  curr = llist.next(curr);
  }
ucout << std::endl;
}

ucout << std::endl;
}


int test_listtest(int,char**)
{
	utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
try {

  int i=0;
  
  while (i++ < 10) {
  LinkedList<double>* llist = new LinkedList<double>();
  //double key;

  llist->add(1.0);
  do_write(*llist);
  
  llist->add(2.0);
  do_write(*llist);
  
  llist->add(3.0);
  do_write(*llist);
  
  llist->push_back(10.0);
  do_write(*llist);
  
  llist->push_front(9.0);
  do_write(*llist);
  
  delete llist;
  }
  }

catch (std::exception err) {
  std::cerr << err.what() << std::endl;
  }


utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}
