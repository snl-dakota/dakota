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

#include <utilib/std_headers.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

template <class T>
class vector { public: vector() {} };

template <class T>
class set { public: set() {} };

template <class T>
class map { public: map() {} };

template <class T>
class list { public: list() {} };

template <class T>
class queue { public: queue() {} };

int test_namespace(int,char**)
{
#ifdef UTILIB_HAVE_NAMESPACES
cout << "OK" << endl;
#else
std::cout << "OK" << std::endl;
#endif
return 0;
}
