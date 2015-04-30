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
#include <utilib/stl_auxiliary.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif
using namespace utilib;


namespace tester {

template <class TYPE>
class A {
public:

  void bar()
	{
	TYPE d;
	vector<TYPE> a,b;
	a << d;
	}
};


}



using namespace tester;

void foo ()
{
A<int> a;
a.bar();
}
