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

#include <utilib/SharedValue.h>
#include <iostream>

using std::cout;
using std::endl;
using std::setw;
using std::string;
using utilib::SharedValue;

namespace {

class FOO
  {
  public:
    FOO()
      { cout << "Constructing FOO" << endl; }
    FOO(const FOO &)
      { cout << "Copy Constructing FOO" << endl; }

    // equality test required by SharedVal API
    bool operator==(const FOO &) const
      { return true; }
    // inequality test required by SharedVal API
    bool operator<(const FOO &) const
      { return false; }
    int i;
  };

}

int test_sharedval(int,char**)
  {
  //short s;
  int i;
  //long l;
  //double d;
  SharedValue<int> foo(5);
  SharedValue<int> bar = foo;
  const SharedValue<double> const_bar = 6.5;

  bar << (int)0.0;
  cout << "foo=" << foo << " bar=" << bar << " const_bar=" << const_bar << endl;

  bar = (int)3.5;
  cout << "foo=" << foo << " bar=" << bar << " const_bar=" << const_bar << endl;

  bar << (int) const_bar();
  cout << "foo=" << foo << " bar=" << bar << " const_bar=" << const_bar << endl;

  //const_bar = 0.0;
  //cout << "foo=" << foo << " bar=" << bar << " const_bar=" << const_bar << endl;

  //const_bar << 5.0;
  //cout << "foo=" << foo << " bar=" << bar << " const_bar=" << const_bar << endl;

  foo = bar;
  bar << (int)2.0;
  foo = const_bar;
  cout << "foo=" << foo << " bar=" << bar << " const_bar=" << const_bar << endl;

  // This will call both constructor & copy constructor
  FOO myFOO;
  SharedValue<FOO> anyFoo1(myFOO);
  anyFoo1().i = 0;
  anyFoo1->i = 0;
  // This will only call the constructor
  SharedValue<FOO> anyFoo2;
  anyFoo2 = myFOO;


  // test equality tests
  {
     SharedValue<int> tmp1 = i;
     SharedValue<int> tmp1a = tmp1;
     cout << "test i == i:  " << (tmp1 == tmp1a) << endl;
     SharedValue<int> tmp1b = 3;
     cout << "test i < 3:  " << (tmp1 < tmp1a) << endl;
     SharedValue<double> tmp2 = i;
     cout << "test i == j:  " << (tmp1 == tmp2) << endl;
  }

  return 0;
  }
