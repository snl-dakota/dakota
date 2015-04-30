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

#include <utilib/Factory.h>
#include <utilib/NumArray.h>
#include <utilib/CharString.h>

typedef utilib::ArrayBase<int,utilib::BasicArray<int> > intarray_base;
typedef utilib::ArrayBase<char,utilib::BasicArray<char> > string_base;

FACTORY_REGISTER(intarray, intarray_base*, return new utilib::BasicArray<int>; ,"test1")
FACTORY_REGISTER(numarray, intarray_base*, return new utilib::NumArray<int>;,"test2")
FACTORY_REGISTER(string, string_base*, return new utilib::CharString; ,"test3")

int test_factory(int,char**)
{
intarray_base* a = utilib::factory_create<intarray_base*>("intarray");
std::cout << "Status: " << (a != 0) << std::endl;
intarray_base* c = utilib::factory_create<intarray_base*>("numarray");
std::cout << "Status: " << (c != 0) << std::endl;
string_base* b = utilib::factory_create<string_base*>("string");
std::cout << "Status: " << (b != 0) << std::endl;

std::vector<const char*> names, descriptions;

names = utilib::global_factory<intarray_base*>().get_names();
descriptions = utilib::global_factory<intarray_base*>().get_descriptions();
std::cout << "intarray_base factory: " << names.size() << std::endl;
for (unsigned int i=0; i<names.size(); i++)
  std::cout << names[i] << " " << descriptions[i] << std::endl;

names = utilib::global_factory<string_base*>().get_names();
descriptions = utilib::global_factory<string_base*>().get_descriptions();
std::cout << "string_base factory: " << names.size() << std::endl;
for (unsigned int i=0; i<names.size(); i++)
  std::cout << names[i] << " " << descriptions[i] << std::endl;


return 0;
}
