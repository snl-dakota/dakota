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
// splay.cpp
//
#include <utilib/std_headers.h>
#include <utilib/_math.h>
#include <utilib/CharString.h>
#include <utilib/SimpleSplayTree.h>
#include <utilib/GenericSplayTree.h>
#include <utilib/OptionParser.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

#define LIMIT 100

using namespace utilib;

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


template <class T>
void fill_table(SimpleSplayTree<BasicArray<T> >& table)
{
BasicArray<T> tmp(5);
tmp << (T) (sqrt(2.0)*100);
T delta= (T) 1.0;
table.add(tmp);

int j=0; 
while (j < LIMIT) {
  if (debug)
     cerr << j << "\t" << tmp << "\t" << delta << endl;
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

void fill_table(SimpleSplayTree<double>& table)
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


void fill_table(SimpleSplayTree<int>& table)
{
int tmp;
for (int i=0; i<LIMIT; i++) {
  tmp = i*i*i;
  table.add(tmp);
  }
}


void fill_table(GenericSplayTree<A<int> >& table, std::list<A<int>*>& items)
{
A<int>* tmp;
for (int i=0; i<LIMIT; i++) {
  tmp = new A<int>;
  tmp->val = i*i*i;
  table.add(*tmp);
  items.push_back(tmp);

  tmp = new A<int>;
  tmp->val = i*i*i;
  table.add(*tmp);
  items.push_back(tmp);
  }
}


void fill_table(GenericSplayTree<A<CharString> >& table, std::list<A<CharString>* >& items)
{
ifstream hash_data("hash.data");
A<CharString>* tmp;
tmp = new A<CharString>;
hash_data >> tmp->val;
while (hash_data) {
  table.add(*tmp);
  items.push_back(tmp);
  tmp = new A<CharString>;
  hash_data >> tmp->val;
  }
items.push_back(tmp);
}


template <class T>
void do_stest(const CharString& msg, ostream& os)
{
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
os << msg << endl;
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
SimpleSplayTree<T> mytree;
fill_table(mytree);

os << "Sending the splay tree to 'os'" << endl;
os << mytree << endl;
}


template <class T>
void do_test(const CharString& msg, ostream& os)
{
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
os << msg << endl;
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
std::list<A<T>* > items;
{
   GenericSplayTree<A<T> > mytree;
   fill_table(mytree,items);

   os << "Sending the splay tree to 'os'" << endl;
   os << mytree << endl;
}

typename std::list<A<T>* >::iterator curr = items.begin();
typename std::list<A<T>* >::iterator end  = items.end();
while (curr != end) {
   typename std::list<A<T>* >::iterator tmp = curr;
   ++curr;
   delete *tmp;
  }
}

}

namespace utilib {
// Overload the write function for printing doubles to use
// pscientific
template <>
void SimpleSplayTreeItem<double>::write(std::ostream& os) const {
       os << pscientific(Key);
}
}

int test_splay(int argc, char** argv)
{
  OptionParser params;
  params.add("debug",debug);
  params.parse_args(argc,argv);
     if (params.help_option())
   {
      params.write(std::cout);
      return -11;
   }

//
// HASHING BasicArray<double>
//
{
//ofstream ofstr("splay-array-double");
do_stest<BasicArray<double> >("Testing BasicArray<double> simpleST",cout);
}


//
// HASHING BasicArray<int>
//
{
//ofstream ofstr("splay-array-int");
do_stest<BasicArray<int> >("Testing BasicArray<int> simpleST",cout);
}


//
// SPLAY TREE OF DOUBLES
//
{
//ofstream ofstr("splay-double");
do_stest<double>("Testing double simpleST",cout);
}


//
// SPLAY TREE OF INTEGERS
//
{
//ofstream ofstr("splay-int");
do_stest<int>("Testing int simpleST",cout);
}


//
// HASHING STRINGS
//
{
//ofstream ofstr("splay-str");
do_test<CharString>("Testing char* HT with default function",cout);
}

return OK;
}
