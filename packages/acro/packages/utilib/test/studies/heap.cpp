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
// heap.cpp
//
#include <utilib/std_headers.h>
#include <utilib/_math.h>
#include <utilib/CharString.h>
#include <utilib/SimpleHeap.h>
#include <utilib/GenericHeap.h>
#include <utilib/OptionParser.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

using namespace utilib;
#define LIMIT 9

namespace {

bool debug=false;

template <class T>
class A {

public:

  A() {}

  int compare(const A<T>& tmp) const;

  T val;

  int write(ostream& os) const
	{os << val; return OK;}

  int read(istream& is)
	{is >> val; return OK;}

  A<T>& operator=(A<T>& tmp)
	{val = tmp.val; return *this;}
};


template <class T>
int A<T>::compare(const A<T>& tmp) const
{
int foo = utilib::compare(val,tmp.val);
return foo;
}


template <class T>
double test_fn(BasicArray<T>& x)
{
double ans=0.0;
for (size_type i=0; i<x.size(); i++)
  ans += x[i]*x[i];
return ans;
}


template <class T, class Compare>
void fill_table(SimpleHeap<BasicArray<T>,Compare>& table)
{
BasicArray<T> tmp(5);
tmp << (T) (sqrt(2.0)*100);
T delta= (T) 1.0;
table.add(tmp);

int j=0; 
while (j < LIMIT) {
  if (debug) {
     cerr << j << "\t" << tmp << "\t" << delta << endl;
     cerr << table << endl;
     }
  bool flag=false;
  int i=0;
  double curr = test_fn(tmp);
  while ((i<5) && !flag) {
    T foo=tmp[i];
    tmp[i] = foo-delta;
    if (test_fn(tmp) < curr) {
       table.add(tmp);
       flag=true;
       j++;
       break;
       }
    tmp[i] = foo+delta;
    if (test_fn(tmp) < curr) {
       table.add(tmp);
       flag=true;
       j++;
       break;
       }
    tmp[i] = foo;
    i++;
    }
  if (!flag)
     delta = (T) (delta*0.9);
  if (delta < 1e-16)
     break;
  }
}



#define FN(x) (fabs(x)*fabs(x))

template <class Compare>
void fill_table(SimpleHeap<double,Compare>& table)
{
double tmp = sqrt(2.0);
double delta=1.0;
table.add(tmp);

int j=0; 
while (j < LIMIT) {
  double curr = FN(tmp);
  if (FN(tmp-delta) < curr) {
     tmp = tmp-delta;
     table.add(tmp);
     j++;
     }
  else if (FN(tmp+delta) < curr) {
     tmp = tmp+delta;
     table.add(tmp);
     j++;
     }
  else {
     delta *= 0.9;
     }
  }
}


template <class Compare>
void fill_table(SimpleHeap<int,Compare>& table)
{
int tmp;
for (int i=0; i<LIMIT; i++) {
  tmp = i*i*i;
  table.add(tmp);
  }
}

template <class T, class Compare>
void do_stest(const CharString& msg, SimpleHeap<T,Compare>& tree, ostream& os)
{
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
os << msg << endl;
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
fill_table(tree);
fill_table(tree);

os << "Extracting items from heap" << endl;
T tmp;
while (tree) {
  bool status;
  typename SimpleHeap<T,Compare>::item_t* top = tree.top();
  tree.remove(top,tmp,status);
  os << tmp << "\t" << tree.size() << "\t" << status << endl;
  }
}



template <class Compare>
void fill_table(GenericHeap<A<int>,Compare>& table)
{
A<int>* tmp;
for (int i=0; i<LIMIT; i++) {
  tmp = new A<int>;
  tmp->val = i*i*i;
  table.add(*tmp);

  tmp = new A<int>;
  tmp->val = i*i*i;
  table.add(*tmp);
  }
}


template <class Compare>
void fill_table(GenericHeap<A<CharString>,Compare>& table)
{
ifstream hash_data("hash.data");
A<CharString>* tmp;
tmp = new A<CharString>;
hash_data >> tmp->val;
int j=0;
while (hash_data) {
  table.add(*tmp);
  tmp = new A<CharString>;
  hash_data >> tmp->val;
  if (++j > LIMIT) break;
  }
}


template <class T, class Compare>
void do_test(const CharString& msg, GenericHeap<A<T>,Compare>& tree, ostream& os)
{
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
os << msg << endl;
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
fill_table(tree);
fill_table(tree);

os << "Extracting items from heap" << endl;
A<T> tmp;
while (tree) {
  bool status;
  typename GenericHeap<A<T>,Compare>::item_t* top = tree.top();
  tree.remove(top,tmp,status);
  os << tmp.val << "\t" << tree.size() << "\t" << status << endl;
  }
}


void exec_test()
{
////
//// SimpleCompare
////

//
// HEAP BasicArray<double>
//
{
//ofstream ofstr("heap-array-double");
SimpleHeap<BasicArray<double> > test_double;
do_stest("Testing BasicArray<double> simpleST",test_double,cout);
}


//
// HEAP BasicArray<int>
//
{
//ofstream ofstr("heap-array-int");
SimpleHeap<BasicArray<int> > test_int;
do_stest("Testing BasicArray<int> simpleST",test_int,cout);
}


//
// HEAP double
//
{
//ofstream ofstr("heap-double");
SimpleHeap<double> test_double;
do_stest("Testing double simpleST",test_double,cout);
}


//
// HEAP int 
//
{
//ofstream ofstr("heap-int");
SimpleHeap<int> test_int;
do_stest("Testing int simpleST",test_int,cout);
}


//
// HASHING STRINGS
//
{
//ofstream ofstr("heap-str");
GenericHeap<A<CharString> > foo;
do_test("Testing char* HT with default function",foo,cout);
}

////
//// Reverse(SimpleCompare)
////

//
// HEAP BasicArray<double>
//
{
//ofstream ofstr("heap-r-array-double");
SimpleHeap<BasicArray<double>,
		Reverse<SimpleCompare<BasicArray<double> > > > test_double;
do_stest("Testing BasicArray<double> simpleST",test_double,cout);
}


//
// HEAP BasicArray<int>
//
{
//ofstream ofstr("heap-r-array-int");
SimpleHeap<BasicArray<int>, 
		Reverse<SimpleCompare<BasicArray<int> > > > test_int;
do_stest("Testing BasicArray<int> simpleST",test_int,cout);
}


//
// HEAP double
//
{
//ofstream ofstr("heap-r-double");
SimpleHeap<double,
		Reverse<SimpleCompare<double> > > test_double;
do_stest("Testing double simpleST",test_double,cout);
}


//
// HEAP int 
//
{
//ofstream ofstr("heap-r-int");
SimpleHeap<int,
		Reverse<SimpleCompare<int> > > test_int;
do_stest("Testing int simpleST",test_int,cout);
}


//
// HASHING STRINGS
//
{
//ofstream ofstr("heap-r-str");
GenericHeap<A<CharString>,
		Reverse<GenericHeapCompare<A<CharString> > > > foo;
do_test("Testing char* HT with default function",foo,cout);
}

}

}

int test_heap(int argc, char** argv)
{
  OptionParser params;
  params.add("debug",debug);
  params.parse_args(argc,argv);
if (params.help_option())
   {
      params.write(std::cout);
      return -11;
   }

exec_test();
return OK;
}
