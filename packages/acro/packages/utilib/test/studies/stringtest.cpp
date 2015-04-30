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
// string.cpp
//
#include <utilib/std_headers.h>
#include <utilib/CharString.h>
#include <utilib/string_ops.h>

using namespace utilib;


int test_string(int,char**)
{
CharString str = "a.bb.ccc.dddd.eeee.";
std::cout << str << std::endl;
std::cout << upper_case(str) << std::endl;

BasicArray<CharString> strs = split(str,'.');
std::cout << strs << std::endl;

str = join(strs,'-');
std::cout << str << std::endl;

long int a;
int status;
a = aslong("  1",status);
if ((a != 1) || (status != OK)) std::cout << "Error: aslong test 1" << std::endl;
a = aslong("  2e1",status);
if ((a != 20)|| (status != OK))  std::cout << "Error: aslong test 2" << std::endl;
a = aslong("  3e+1",status);
if ((a != 30) || (status != OK)) std::cout << "Error: aslong test 3" << std::endl;
a = aslong("  4.1e+1",status);
if ((a != 41)|| (status != OK))  std::cout << "Error: aslong test 4" << std::endl;
a = aslong("  5.1e+2",status);
if ((a != 510)|| (status != OK))  std::cout << "Error: aslong test 5" << std::endl;
a = aslong("2.4e+7",status);
if ((a != 24000000)|| (status != OK))  std::cout << "Error: aslong test 6" << std::endl;
//cin >> str;
//std::cout << str;

return 0;
}
