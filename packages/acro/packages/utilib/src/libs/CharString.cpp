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
// CharString.cpp
//

#include <string>
#include <cstdlib>
#include <utilib/std_headers.h>
#include <utilib/CharString.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace utilib {

int CharString::serializer( SerialObject::elementList_t& serial, 
                            Any& data, bool serialize )
{
   // The following is a trick to cast the incoming Any into the base
   // class (needed until Anys support polymorphism). 
   Any tmp;
   tmp.set<BasicArray<char> >(data.expose<CharString>(), true);
   // NB: directly calling the base class serializer ONLY works
   // because this derived class does not add any additional data.
   // That said, doing this saves a level of indirection in the
   // actual serial stream.
   return BasicArray<char>::serializer(serial, tmp, serialize); 
}

const int CharString::register_serializer = 
   Serializer().register_serializer<CharString>( "utilib::CharString", 
                                                 &CharString::serializer );


CharString::CharString(const char* str, const EnumDataOwned o)
  : BasicArray<char>()
{
  if (str)
    construct(strlen(str), (char*)str, o);
}


CharString::CharString(char* str, const EnumDataOwned o)
  : BasicArray<char>()
{
  if (str)
    construct(strlen(str), str, o);
}


// This copies data, overriding the standard copy from ArrayBase.
// It looks for trailing nulls in the input, and also makes sure that
// that there is a trailing null in the output.  If the source is shorter
// than the target, then the target gets filled out with nulls as well.

void CharString::copy_data(char*     target,
			   size_type target_size,
			   char*     source,
			   size_type source_size)
{
  unsigned int i=0;
  for (; i<target_size && i<source_size && source && *source; i++)
    *(target++) = *(source++);
  for (; i<=target_size; i++)
    *(target++) = '\000';
}


void CharString::dump_data(ostream& str, unsigned int max_elements)
{
  char* pointer = Data;
  unsigned int i=0;
  for (; *pointer && i<max_elements; i++)
    str << *(pointer++);
  if (i > 0)
    str << endl;
}
 

CharString& CharString::set_subvec(const size_type start,
				   const size_type len,
				   const CharString& array, 
				   const size_type offset)
{
  for (size_type i=start; i<(len+start); i++)
    Data[i] = array.Data[offset+i-start];
  Data[len+start] = '\000';
  return *this;
}
 

int compare(const CharString& str1, const CharString& str2)
{
  return str1.compare(str2);
}


CharString& CharString::operator+=(char val)
{
  char tmp[2];
  tmp[0] = val;
  tmp[1] = '\000';
  return ( (*this) += tmp );
}


CharString& CharString::operator+=(const double val)
{
  char tmp[256];
#if (_MSC_VER >= 1400) 
  // sprintf_s is only available starting in VISUAL .NET 2005
  sprintf_s(tmp,256,"%f",val);
#else
  if (val > 10000000000.0)
     sprintf(tmp,"%g",val);
  else
     sprintf(tmp,"%f",val);
#endif
  return ( (*this) += tmp );
}


CharString& CharString::operator+=(const int val)
{
  char tmp[256];
#if (_MSC_VER >= 1400) 
  // sprintf_s is only available starting in VISUAL .NET 2005
  sprintf_s(tmp,256,"%d",val);
#else
  sprintf(tmp,"%d",val);
#endif
  return ( (*this) += tmp );
}


CharString& CharString::operator+=(const unsigned int val)
{
  char tmp[256];
#if (_MSC_VER >= 1400) 
  // sprintf_s is only available starting in VISUAL .NET 2005
  sprintf_s(tmp,256,"%d",val);
#else
  sprintf(tmp,"%d",val);
#endif
  return ( (*this) += tmp );
}


CharString& CharString::operator+=(const char* str)
{
  if (str) {
    size_type curr = Len;
    size_type next = strlen(str);
 
    resize(curr+next);
 
    for (size_type i=curr; i<(curr+next); i++)
    Data[i] = str[i-curr];
    Data[curr+next] = '\000';
  }
 
  return *this;
}


int CharString::compare(const char* s) const
{
  if (Data)
    {
      if (s) {
	int tmp = strcmp(Data,s);  // Both pointers non-null -- use strcmp
	if (tmp < 0) return -1;
	if (tmp > 0) return 1;
        }
      else
	return *Data != 0;      // if s==NULL, return +1 if we have some data
    }
  else if (s)
    return -(*s != 0);          // Return -1 if s points to some data
  return 0;                     // Both pointers NULL: return 0
}


long int aslong(const CharString& str, int& status)
{
#if 0
  /// WEH - I thought that strtol was going to solve this ... but I was wrong

  long int ans=0;
  char *endptr = str.data();   // first invalid character

  status = ERR;

  if (str.data() && str.data()[0]){
    ans = strtol(str.data(), &endptr, 10);
    if (*endptr == NULL) {
       status = OK;
       }
    else if ((endptr[0] == 'e') || (endptr[0] == 'E')) {
       endptr++;
       if (endptr[0] == '+') endptr++;
       char* startptr = endptr;
       long int tmp = strtol(startptr, &endptr, 10);
       if (*endptr == NULL) {
	  for (long int i=0; i<tmp; i++)
	    ans *= 10;
          status = OK;
          }
       }
    }
  
  return ans;
#else
  // This doesn't recognize ints beginning with '-'
  long int ans=0;
  status = ERR;
  size_type i=0;
  int j=0;
  while ((i < str.size()) && str[i] && isspace(str[i])) i++;
  while ((i < str.size())  && str[i] && (isdigit(str[i]) || (str[i] == '.'))) {
    if (isdigit(str[i])) {
       ans = ans*10+ static_cast<long int>(str[i] - '0');
       if (j) j++;
       }
    else
       j++;
    i++;
    }
  if ((i == str.size()) || !str[i]) {
     if (!j) status = OK;
     return ans;
     }
  if (str[i] != 'e') return ans;	// AN ERROR
  i++;
  if (isalpha(str[i])) return ans;	// AN ERROR
  if (str[i] == '-') return ans;	// AN ERROR
  if (str[i] == '+') i++;
  int expn = atoi( &(str[i]) ) - ((j > 0) ? (j - 1) : 0);
  if (expn < 0) return ans;		// AN ERROR
  for (int k=0; k<expn; k++)
    ans = ans*10;
  status = OK;
  return ans;
#endif
}


double asdouble(const CharString& str, int& status)
{
  double ans;
  char* ptr = NULL;
  ans = strtod(str.data(),&ptr);
  if ((ptr == NULL) || (ptr[0] == '\000'))
    status = OK;
  else
    status = ERR;
  return ans;
}


CharString upper_case(CharString str)
{
size_t length = str.size();

for (size_t i=0; i<length; i++) {
  str[i] = std::toupper(str[i]);
  }
return str;
}


} // namespace utilib
