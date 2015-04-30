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

#include <utilib/StringRegistry.h>

int test_sregistry(int,char**)
{
utilib::StringRegistry reg;

for (int i=0; i<10; i++) {
  char tmp[2];
  tmp[1] = '\000';
  tmp[0] = (char)(i+65);
  int val = reg.add(tmp);
  std::cout << "ID: " << val << " Name: " << tmp << std::endl;
  }

std::list<std::string>::iterator curr = reg.get_names().begin();
std::list<std::string>::iterator end  = reg.get_names().end();
while (curr != end) {
  std::cout << "Name: " << *curr << " ID: " << reg.id(*curr) << " RegName: " << reg.name(reg.id(*curr)) << std::endl;
  curr++;
  }

return 0;
}
