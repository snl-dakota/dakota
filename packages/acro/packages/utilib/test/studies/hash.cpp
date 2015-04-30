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
// hash.cpp
//
#include <utilib/std_headers.h>
#include <utilib/_math.h>
#include <utilib/GenericHashTable.h>
#include <utilib/SimpleHashTable.h>
#include <utilib/OptionParser.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif
using namespace utilib;

namespace {
bool debug = false;
char Dummy;
}

#define LIMIT 100

template <class T>
class A {

public:

  A() {}

  size_type hash(size_type table_size) const
	{
	size_type tmp = (*hash_fn)(val,table_size);
	if (debug) cout << "Hashing: " << val << " with hash value: " << tmp << endl;
	return tmp;
	//return (*hash_fn)(val,table_size);
	}

  int compare(const A<T>& tmp) const;

  T val;

  int write(ostream& os) const
	{os << val; return OK;}

  int read(istream& is)
	{is >> val; return OK;}

  A<T>& operator=(A<T>& tmp)
	{val = tmp.val; return *this;}

  static size_type (*hash_fn)(const T&, size_type);
};


template <class T>
istream& operator>>(istream& is, A<T>& key)
{key.read(is); return is;}

template <class T>
ostream& operator<<(ostream& os, A<T>& key)
{key.write(os); return os;}

template <class T>
size_type (*A<T>::hash_fn)(const T&, size_type) = utilib::hash_fn;


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
void fill_table(SimpleHashTable<BasicArray<T>,char>& table)
{
BasicArray<T> tmp(5);
tmp << (T) (sqrt(2.0)*10000);
T delta= (T) 1.0;
table.add(tmp,Dummy);
//if (debug) cout << 0 << "\t" << tmp << "\t" << (*table.curr_hashfn)(tmp,769) << endl;

int j=0; 
while (j < LIMIT) {
  if (debug)
     cerr << j << "\t" << tmp << "\t" << delta << endl;
  bool flag=false;
  int i=0;
  double curr = test_fn(tmp);
  while ((i<5) && !flag) {
    tmp[i] -= delta;
    if (test_fn(tmp) < curr) {
       table.add(tmp,Dummy);
       //if (debug) cout << j << "\t" << tmp << "\t" << delta << "\t" << (*table.curr_hashfn)(tmp,769) << endl;
       flag=true;
       j++;
       break;
       }
    tmp[i] += 2*delta;
    if (test_fn(tmp) < curr) {
       table.add(tmp,Dummy);
       //if (debug) cout << j << "\t" << tmp << "\t" << delta << "\t" << (*table.curr_hashfn)(tmp,769) << endl;
       flag=true;
       j++;
       break;
       }
    tmp[i] -= delta;
    i++;
    }
  if (!flag)
     delta = (T) (delta*0.9);
  if (delta < 1e-16)
     break;
  }
}



#define FN(x) (fabs(x)*fabs(x))

void fill_table(SimpleHashTable<double,char>& table)
{
double tmp = sqrt(2.0);
double delta=1.0;
table.add(tmp,Dummy);

int j=0; 
while (j < LIMIT) {
  double curr = FN(tmp);
  if (FN(tmp-delta) < curr) {
     tmp = tmp-delta;
     table.add(tmp,Dummy);
     j++;
     }
  else if (FN(tmp+delta) < curr) {
     tmp = tmp+delta;
     table.add(tmp,Dummy);
     j++;
     }
  else {
     delta *= 0.9;
     }
  //cout << delta << "\t" << tmp << endl;
  }
}


void fill_table(SimpleHashTable<int,char>& table)
{
int tmp;
for (int i=0; i<LIMIT; i++) {
  tmp = i*i*i;
  table.add(tmp,Dummy);
  }
}


void fill_table(GenericHashTable<A<int>,char>& table)
{
A<int>* tmp;
for (int i=0; i<LIMIT; i++) {
  tmp = new A<int>;
  tmp->val = i*i*i;
  table.add(*tmp,Dummy);

  tmp = new A<int>;
  tmp->val = i*i*i;
  table.add(*tmp,Dummy);
  }
}


void fill_table(GenericHashTable<A<CharString>,char>& table)
{
ifstream hash_data("hash.data");
A<CharString>* tmp;
tmp = new A<CharString>;
hash_data >> tmp->val;
while (hash_data) {
  table.add(*tmp,Dummy);
  tmp = new A<CharString>;
  hash_data >> tmp->val;
  }
}


template <class T>
void do_stest(const CharString& msg, SimpleHashTable<T,char>& table, ostream& os)
{
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
os << msg << endl;
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
fill_table(table);

// Allocate a temporary table, and set it equal to the current
// table.  If there are errors, then they will probably show up in the
// printing of the original hash table.
{
SimpleHashTable<T,char> tmp_table;
tmp_table = table;
}

if (debug) {
   os << "Sending the hash table to 'os'" << endl;
   os << table << endl;

   os << "Walking through a hash table" << endl;
   typename SimpleHashTable<T,char>::iterator curr = table.begin();
   typename SimpleHashTable<T,char>::iterator end  = table.end();
   while (curr != end) {
     os << curr->key() << endl;
     curr++;
     }
   }

table.statistics(os);
}


template <class T>
void do_test(const CharString& msg, GenericHashTable<A<T>,char>& table, ostream& os)
{
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
os << msg << endl;
os << "\\" << endl;
os << "\\" << endl;
os << "\\" << endl;
fill_table(table);

if (debug) {
   os << "Sending the hash table to 'os'" << endl;
   os << table << endl;

   os << "Walking through a hash table" << endl;
   typename GenericHashTable<A<T>,char>::iterator curr = table.begin();
   typename GenericHashTable<A<T>,char>::iterator end  = table.end();
   while (curr != end) {
     os << curr->key() << endl;
     curr++;
     }
   }

table.statistics(os);
}



int test_hash(int argc, char** argv)
{
utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);

OptionParser params;
params.add("debug",debug);
params.parse_args(argc,argv);
if (params.help_option()) {
      params.write(std::cout);
      return -11;
}

//
// HASHING BasicArray<double>
//
{
//ofstream ofstr("hash-array-double");
SimpleHashTable<BasicArray<double>,char> test_double;
do_stest("Testing BasicArray<double> simpleHT with default function",test_double,cout);
test_double.clear();
test_double.set_hash_fn(hash_fn1);
do_stest("Testing BasicArray<double> simpleHT with function 1",test_double,cout);
test_double.clear();
test_double.set_hash_fn(hash_fn2);
do_stest("Testing BasicArray<double> simpleHT with function 2",test_double,cout);
test_double.clear();
test_double.set_hash_fn(hash_fn3);
do_stest("Testing BasicArray<double> simpleHT with function 3",test_double,cout);
}


//
// HASHING BasicArray<int>
//
{
//ofstream ofstr("hash-array-int");
SimpleHashTable<BasicArray<int>,char> test_int;
do_stest("Testing BasicArray<int> simpleHT with default function",test_int,cout);
test_int.clear();
test_int.set_hash_fn(hash_fn1);
do_stest("Testing BasicArray<int> simpleHT with function 1",test_int,cout);
test_int.clear();
test_int.set_hash_fn(hash_fn2);
do_stest("Testing BasicArray<int> simpleHT with function 2",test_int,cout);
}


//
// HASHING DOUBLES
//
{
//ofstream ofstr("hash-double");
SimpleHashTable<double,char> test_double;
do_stest("Testing double simpleHT with default function",test_double,cout);
test_double.clear();
test_double.set_hash_fn(hash_fn1);
do_stest("Testing double simpleHT with function 1",test_double,cout);
test_double.clear();
test_double.set_hash_fn(hash_fn2);
do_stest("Testing double simpleHT with function 2",test_double,cout);
}


//
// HASHING INTEGERS
//
{
//ofstream ofstr("hash-int");
SimpleHashTable<int,char> test_int;
do_stest("Testing int simpleHT with default function",test_int,cout);
test_int.clear();
test_int.set_hash_fn(hash_fn1);
do_stest("Testing int simpleHT with function 1",test_int,cout);
test_int.clear();
test_int.set_hash_fn(hash_fn2);
do_stest("Testing int simpleHT with function 2",test_int,cout);
}


//
// HASHING STRINGS
//
{
//ofstream ofstr("hash-str");
// GenericHashTable<A<CharString>>
GenericHashTable<A<CharString>,char> foo;

do_test("Testing char* HT with default function",foo,cout);
foo.clear();
A<CharString>::hash_fn = hash_fn1;
do_test("Testing char* HT with function 1",foo,cout);
foo.clear();
A<CharString>::hash_fn = hash_fn2;
do_test("Testing char* HT with function 2",foo,cout);
foo.clear();
A<CharString>::hash_fn = hash_fn3;
do_test("Testing char* HT with function 2",foo,cout);
}

#if 0
// TODO - reinstate this when the packing/unpacking streams
// work properly.
{
SimpleHashTable<int,char> testA;
fill_table(testA);
PackBuffer buff;
buff << testA;
UnPackBuffer ubuff(buff);
SimpleHashTable<int,char> testB;
ubuff >> testB;
}
#endif

utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}
