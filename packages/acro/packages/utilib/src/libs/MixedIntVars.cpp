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
// MixedIntVars.cpp
//

#include <utilib/MixedIntVars.h>

#include <limits>


using namespace std;

namespace utilib {

namespace {

int cast_real_to_miv(const Any& from, Any& to)
{
   MixedIntVars &ans = to.set<MixedIntVars>();
   ans.Real() << from.expose<NumArray<double> >();
   ans.Integer().resize(0);
   ans.Binary().resize(0);
   return OK;
}

int cast_int_to_miv(const Any& from, Any& to)
{
   MixedIntVars &ans = to.set<MixedIntVars>();
   ans.Real().resize(0);
   ans.Integer() << from.expose<NumArray<int> >();
   ans.Binary().resize(0);
   return OK;
}

} // namespace utilib::(local)

// register the serializer and casting functions
const volatile bool MixedIntVars::registrations_complete = 
   MixedIntVars::register_aux_functions();

bool MixedIntVars::register_aux_functions()
{
   Serializer().register_serializer<MixedIntVars>
      ( "utilib::MixedIntVars", MixedIntVars::serializer );

   TypeManager()->register_lexical_cast
      ( typeid(NumArray<double>), typeid(MixedIntVars), &cast_real_to_miv );
   TypeManager()->register_lexical_cast
      ( typeid(NumArray<int>), typeid(MixedIntVars), &cast_int_to_miv );

   return true;
}


MixedIntVars::MixedIntVars(const int numBits, const int numGenInts, 
					const int numDoubles)
	: a(0)
{
construct(numBits, numGenInts, numDoubles);
}


MixedIntVars::MixedIntVars()
	: a(0)
{
construct(0,0,0);
}


void MixedIntVars::resize(const size_type numBits, const size_type numGenInts,
                                        const size_type numDoubles)
{
a->bitVars.resize(numBits);
a->gintVars.resize(numGenInts);
a->doubleVars.resize(numDoubles);

}     


void MixedIntVars::write(ostream& os) const
{
if (a->bitVars.size() > 0) {
   os << "b(" << a->bitVars.size() << " :";
   for (size_t i=0; i<a->bitVars.size(); i++) {
     if (a->bitVars(i))
        os << " 1";
     else
        os << " 0";
     }
   os << ")  ";
   }
if (a->gintVars.size() > 0) {
   os << "i(" << a->gintVars.size() << " :";
   for (size_t i=0; i<a->gintVars.size(); i++) {
     os << " " << a->gintVars[i];
     }
   os << ")  ";
   }
if (a->doubleVars.size() > 0) {
   os << "r(" << a->doubleVars.size() << " :";
   for (size_t i=0; i<a->doubleVars.size(); i++) {
     os << " " << a->doubleVars[i];
     }
   os << ")";
   }
}


void MixedIntVars::read(istream& is)
{
char c;
int len;
std::vector<char> tmp;
std::string str;
while (is) {
  is.get(c);
  while (is && (c == ' ')) is.get(c);
  //cerr << "Delim " << c << endl;
  if (!is) break;
  if (c == 'b') {
     is >> c;
     is >> len;
     is >> c;     
     a->bitVars.resize(len);
     is.get(c);
     while (c != ')') {
       tmp.push_back(c);
       is.get(c);
     }
     tmp.push_back(0);
     istringstream tis(&tmp[0]);
     //cerr << tis.str().c_str() << std::endl;
     //cerr << &tmp[0] << std::endl;
     for (size_t i=0; i<a->bitVars.size(); i++) {
       bool tmp;
       tis >> tmp;
       if (tmp)
          a->bitVars.set(i);
       else
          a->bitVars.reset(i);
     }
     tmp.resize(0);
  } else if (c == 'i') { 
     is >> c;
     is >> len;
     is >> c;     
     a->gintVars.resize(len);
     is.get(c);
     while (c != ')') {
       tmp.push_back(c);
       is.get(c);
     }
     tmp.push_back(0);
     istringstream tis(&tmp[0]);
     //cerr << tis.str().c_str() << std::endl;
     //cerr << &tmp[0] << std::endl;
     for (size_t i=0; i<a->gintVars.size(); i++) {
       tis >> a->gintVars[i];
     }
     tmp.resize(0);
  } else if (c == 'r') {
     is >> c;
     is >> len;
     is >> c;     
     a->doubleVars.resize(len);
     is.get(c);
     while (c != ')') {
       tmp.push_back(c);
       is.get(c);
     }
     tmp.push_back(0);
     istringstream tis(&tmp[0]);
     //cerr << tis.str().c_str() << std::endl;
     //cerr << &tmp[0] << std::endl;
     for (size_t i=0; i<a->doubleVars.size(); i++) {
       tis >> a->doubleVars[i];
     }
     tmp.resize(0);
  } else {
     EXCEPTION_MNGR(std::runtime_error, "MixedIntVars::read - Wrong data delimiter");
  }
  if (!is) {
     EXCEPTION_MNGR(std::runtime_error, "MixedIntVars::read - Error parsing string");
  }
  }
}


void MixedIntVars::write(PackBuffer& os) const
{
os << a->bitVars << a->gintVars << a->doubleVars;
}


void MixedIntVars::read(UnPackBuffer& is)
{
is >> a->bitVars >> a->gintVars >> a->doubleVars;
}


void MixedIntVars::construct(const size_type nbits, const size_type nints, const size_type ndlbs)
{
free();
a = new MixedIntVarsRep(nbits, nints, ndlbs);
}

void MixedIntVars::free()
{
if (a) {
   delete a;
   a = 0;
   }
}


int MixedIntVars::compare(const MixedIntVars& array) const
{
   int ans = a->bitVars.compare(array.a->bitVars);
   if (ans != 0) 
      return ans;

   ans = a->gintVars.compare(array.a->gintVars);
   if (ans != 0) 
      return ans;

   return a->doubleVars.compare(array.a->doubleVars);
}


bool MixedIntVars::operator==(const MixedIntVars& array) const
{
   return compare(array) == 0;
}


bool MixedIntVars::operator<(const MixedIntVars& array) const
{
   return compare(array) < 0;
}


MixedIntVars& MixedIntVars::operator=(const MixedIntVars& array)
{
if (this != &array) {
   free();
   construct(array.numIntegerVars(), array.numGeneralIntVars(), 
	     array.numDoubleVars());
   (*this) << array;
   }
return *this;
}


MixedIntVars& MixedIntVars::operator&=(const MixedIntVars& array)
{
if (array.a == a) return *this;

free();
a = array.a;
a->ref++;
return *this;
}


double MixedIntVars::get(const size_type ndx) const
{
   if ( a == NULL ) 
   {
      EXCEPTION_MNGR(std::runtime_error, "MixedIntVars::get(): "
                     "NULL MixedIntVarsRep");
   }

   size_t offset = a->bitVars.size();
   if ( ndx < offset )
      return a->bitVars[ndx]; 

   if ( ndx - offset < a->gintVars.size() )
      return a->gintVars[ndx - offset];

   offset += a->gintVars.size();
   if ( ndx - offset < a->doubleVars.size() )
      return a->doubleVars[ndx - offset];

   EXCEPTION_MNGR(std::runtime_error, "MixedIntVars::get(): "
                  "index out of range");

   // Return *something* so compilers don't complain
   return std::numeric_limits<double>::quiet_NaN();
}


int MixedIntVars::serializer( SerialObject::elementList_t& serial, 
                              Any& data, bool serialize )
{
   MixedIntVars& tmp = const_cast<MixedIntVars&>(data.expose<MixedIntVars>());
   if ( serialize && ( tmp.a == NULL ))
      return 0;
   if (( ! serialize ) && serial.empty() )
   {
      tmp.free();
      return 0;
   }

   if ( tmp.a == NULL )
      tmp.a = new MixedIntVarsRep(0,0,0);

   int ans = 0;
   if ( 0 != (ans = serial_transform(serial, tmp.a->bitVars, serialize)) )
      return ans;
   if ( 0 != (ans = serial_transform(serial, tmp.a->gintVars, serialize)) )
      return ans;
   return serial_transform(serial, tmp.a->doubleVars, serialize);
}


} // namespace utilib
