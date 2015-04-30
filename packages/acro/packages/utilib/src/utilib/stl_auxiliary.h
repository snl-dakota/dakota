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

/**
 * \file stl_auxiliary.h
 *
 * Defines auxiliary functions for STL classes
 */

#ifndef utilib_stl_auxiliary_h
#define utilib_stl_auxiliary_h

#include <utilib/PackBuf.h>
#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/bimap.h>
#include <string>
#include <algorithm>

namespace utilib {

///
inline void toUpper(std::string& str)
{
std::string::iterator curr = str.begin();
std::string::iterator end  = str.end();
while (curr != end) {
  *curr = toupper(*curr);
  curr++;
  }
}

///
inline void toLower(std::string& str)
{
std::string::iterator curr = str.begin();
std::string::iterator end  = str.end();
while (curr != end) {
  *curr = tolower(*curr);
  curr++;
  }
}


//using utilib::NumArray;

/// Compute the max of a vector
template <typename TYPE>
TYPE max(const std::vector<TYPE>& vec)
{
if (vec.size() == 0)
   EXCEPTION_MNGR(std::runtime_error, "max - applied to null vector");
typename std::vector<TYPE>::const_iterator start = vec.begin();
typename std::vector<TYPE>::const_iterator end = vec.end();
TYPE max_val = *start;
start++;
while (start < end) {
  if (max_val < (*start)) max_val = *start;
  start++;
  }
return max_val;
}

///
/// Extensions to STL
///

#if 0
#if !defined(CYGWIN) && !defined(LINUX) && !defined(__GNUC__) && !defined(COUGAR) && !defined(OSF) && !defined(TFLOPS_SERVICE) && !defined(SGI) && !defined(RS6K) && !defined(_MSC_VER)
inline std::ostream& operator<< (std::ostream& os, bool& flag)
{
if (flag)
   os << "true";
else
   os << "false";
return os;
}
#endif

#if 0
template <class TYPE1, class TYPE2>
inline std::basic_ostream<TYPE1,TYPE2>& operator<<(std::basic_ostream<TYPE1,TYPE2>& os, bool& flag)
{
if (flag)
   os << "true";
else
   os << "false";
return os;
}
#endif

#endif

inline std::istream& parse(std::istream& is, bool& flag)
{
std::string str;
is >> str;
toLower(str);
if ((str == "0") || (str == "false") || (str == "f") 
                 || (str == "no")
                 || (str == "n")
    )
   flag = false;
else if ((str == "1") || (str == "true") || (str == "t") 
                 || (str == "yes")
                 || (str == "y")
    )
   flag = true;
else
   EXCEPTION_MNGR(std::runtime_error, "Unknown boolean type: " << str);

return is;
}

} // namespace utilib


/// Stream operator for copying one vector into another
template <class Type1, class Type2>
std::vector<Type1>& operator<< ( std::vector<Type1>& vec, const std::vector<Type2>& old)
{
vec.resize(old.size());
typename std::vector<Type2>::const_iterator oldval = old.begin();
typename std::vector<Type1>::iterator curr = vec.begin();
typename std::vector<Type1>::iterator last = vec.end();

while (curr != last) {
  *curr = *oldval;
  ++curr;
  ++oldval;
  }
return vec;
}

#if defined(UTILIB_SOLARIS_CC)
template <class Type1, class Type2>
std::vector<Type1>& operator<< ( std::vector<Type1>& vec, std::vector<Type2>& old)
{
vec.resize(old.size());
typename std::vector<Type2>::iterator oldval = old.begin();
typename std::vector<Type1>::iterator curr = vec.begin();
typename std::vector<Type1>::iterator last = vec.end();

while (curr != last) {
  *curr = *oldval;
  ++curr;
  ++oldval;
  }
return vec;
}
#endif


/// Stream operator for filling a vector with a given value
template <class TYPE>
std::vector<TYPE>& operator<< (std::vector<TYPE>& vec, const TYPE& val)
{
typename std::vector<TYPE>::iterator curr = vec.begin();
typename std::vector<TYPE>::iterator last = vec.end();
while (curr != last) {
  *curr = val;
  ++curr;
  }
return vec;
}




/// Stream operator for adding one vector with another
template <class TYPE>
std::vector<TYPE>& operator+= (std::vector<TYPE>& vec, const std::vector<TYPE>& val)
{
typename std::vector<TYPE>::iterator curr = vec.begin();
typename std::vector<TYPE>::iterator last = vec.end();
typename std::vector<TYPE>::const_iterator vals = val.begin();
while (curr != last) {
  *curr += *vals;
  ++curr;
  ++vals;
  }
return vec;
}


/// Ampersand-Equal operator for vectors
template <class TYPE>
std::vector<TYPE>& operator&= ( std::vector<TYPE>& vec, const std::vector<TYPE>& old)
{ vec = old; return vec; }



/// Write a map to an output stream
template <class KEY, class VALUE>
std::ostream& operator<<(std::ostream& os, const std::map<KEY,VALUE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   os << " :";
   typename std::map<KEY,VALUE>::const_iterator curr = obj.begin();
   typename std::map<KEY,VALUE>::const_iterator last = obj.end();
   while (curr != last) {
     os << " " << curr->first << " " << curr->second;
     ++curr;
     }
   }
return os;
}


/// Write a map to an output stream
template <class KEY, class VALUE>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, const std::map<KEY,VALUE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   typename std::map<KEY,VALUE>::const_iterator curr = obj.begin();
   typename std::map<KEY,VALUE>::const_iterator last = obj.end();
   while (curr != last) {
     os << curr->first << curr->second;
     ++curr;
     }
   }
return os;
}


/// Write a bimap to an output stream
template <class KEY, class VALUE>
std::ostream& operator<<(std::ostream& os, const utilib::bimap<KEY,VALUE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   os << " :";
   typename utilib::bimap<KEY,VALUE>::const_iterator curr = obj.begin();
   typename utilib::bimap<KEY,VALUE>::const_iterator last = obj.end();
   while (curr != last) {
     os << " " << curr->first << " " << curr->second;
     ++curr;
     }
   }
return os;
}


/// Write a bimap to an output stream
template <class KEY, class VALUE>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, const utilib::bimap<KEY,VALUE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   typename utilib::bimap<KEY,VALUE>::const_iterator curr = obj.begin();
   typename utilib::bimap<KEY,VALUE>::const_iterator last = obj.end();
   while (curr != last) {
     os << curr->first << curr->second;
     ++curr;
     }
   }
return os;
}


/// Write a list to a Pack buffer
/// Write a list to an output stream
template <class TYPE>
std::ostream& operator<<(std::ostream& os, const std::list<TYPE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   os << " :";
   typename std::list<TYPE>::const_iterator curr = obj.begin();
   typename std::list<TYPE>::const_iterator last = obj.end();
   while (curr != last) {
     os << " " << *curr;
     ++curr;
     }
   }
return os;
}


/// Write a list to a Pack buffer
template <class TYPE>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, const std::list<TYPE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   typename std::list<TYPE>::const_iterator curr = obj.begin();
   typename std::list<TYPE>::const_iterator last = obj.end();
   while (curr != last) {
     os << *curr;
     ++curr;
     }
   }
return os;
}


/// Write a set to an output stream
template <class TYPE>
std::ostream& operator<<(std::ostream& os, const std::set<TYPE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   os << " :";
   typename std::set<TYPE>::const_iterator curr = obj.begin();
   typename std::set<TYPE>::const_iterator last = obj.end();
   while (curr != last) {
     os << " " << *curr;
     ++curr;
     }
   }
return os;
}


/// Write a set to an output stream
template <class TYPE>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, const std::set<TYPE>& obj)
{
os << obj.size();
if (obj.size() > 0) {
   typename std::set<TYPE>::const_iterator curr = obj.begin();
   typename std::set<TYPE>::const_iterator last = obj.end();
   while (curr != last) {
     os << *curr;
     ++curr;
     }
   }
return os;
}


/// Write a vector to an output stream
template <class TYPE>
std::ostream& operator<<(std::ostream& os, const std::vector<TYPE>& vec)
{
os << vec.size();
if (vec.size() > 0) {
   os << " :";
   typename std::vector<TYPE>::const_iterator curr = vec.begin();
   typename std::vector<TYPE>::const_iterator last = vec.end();
   while (curr != last) {
     os << " " << *curr;
     ++curr;
     }
   }
return os;
}


/// Write a vector to an output stream
template <class TYPE>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, const std::vector<TYPE>& vec)
{
os << vec.size();
if (vec.size() > 0) {
   typename std::vector<TYPE>::const_iterator curr = vec.begin();
   typename std::vector<TYPE>::const_iterator last = vec.end();
   while (curr != last) {
     os << *curr;
     ++curr;
     }
   }
return os;
}


/// Read a vector from an input stream
template <class TYPE>
std::istream& operator>>(std::istream& is, std::vector<TYPE>& vec)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - istream problem.");

   vec.resize(tmp);
   if (vec.size() > 0) {
      char c;
      is >> c;
      typename std::vector<TYPE>::iterator curr = vec.begin();
      typename std::vector<TYPE>::iterator last = vec.end();
      while (curr != last) {
         EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - istream problem.");
         is >> *curr;
         ++curr;
      }
   }
   return is;
}


/// Read a vector from a PackBuffer
template <class TYPE>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is, std::vector<TYPE>& vec)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - unpack problem.");

   vec.resize(tmp);
   if (vec.size() > 0) {
      typename std::vector<TYPE>::iterator curr = vec.begin();
      typename std::vector<TYPE>::iterator last = vec.end();
      while (curr != last) {
         EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - unpack problem.");
         is >> *curr;
         ++curr;
      }
   }
   return is;
}


/// Read a vector from an input stream
template <class TYPE>
std::istream& operator>>(std::istream& is, std::list<TYPE>& vec)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - istream problem.");

   vec.clear();
      char c;
      is >> c;
      for (size_t i=0; i<tmp; i++) {
         EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - istream problem.");
         TYPE item;
         is >> item;
         vec.push_back(item);
      }
   return is;
}

/// Read a vector from an UnPackBuffer
template <class TYPE>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is, std::list<TYPE>& data)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - unpack problem.");

   data.clear();
   for (size_t i=0; i<tmp; i++) {
         EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - unpack problem.");
         TYPE item;
         is >> item;
         data.push_back(item);
      }
   return is;
}

/// Read a set from an input stream
template <class TYPE>
std::istream& operator>>(std::istream& is, std::set<TYPE>& vec)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - istream problem.");

   vec.clear();
   char c;
   is >> c;
   for (size_t i=0; i<tmp; i++) {
     EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - istream problem.");
     TYPE item;
     is >> item;
     vec.insert(item);
     }
   return is;
}

/// Read a set from an UnPackBuffer
template <class TYPE>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is, std::set<TYPE>& data)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - unpack problem.");

   data.clear();
   for (size_t i=0; i<tmp; i++) {
         EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - unpack problem.");
         TYPE item;
         is >> item;
         data.insert(item);
      }
   return is;
}

/// Read a map from an input stream
template <class KEY, class VALUE>
std::istream& operator>>(std::istream& is, std::map<KEY,VALUE>& data)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - istream problem.");

   data.clear();
   char c;
   is >> c;
   for (size_t i=0; i<tmp; i++) {
     EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - istream problem.");
     KEY item;
     is >> item;
     is >> data[item];
     }
   return is;
}

/// Read a map from an UnPackBuffer
template <class KEY, class VALUE>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is, std::map<KEY,VALUE>& data)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - unpack problem.");

   data.clear();
   for (size_t i=0; i<tmp; i++) {
      EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - unpack problem.");
      KEY item;
      is >> item;
      is >> data[item];
      }
   return is;
}


/// Read a bimap from an input stream
template <class KEY, class VALUE>
std::istream& operator>>(std::istream& is, utilib::bimap<KEY,VALUE>& data)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - istream problem.");

   data.clear();
   char c;
   is >> c;
   for (size_t i=0; i<tmp; i++) {
     EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - istream problem.");
     KEY item;
     is >> item;
     VALUE val;
     is >> val;
     data[item] = val;
     }
   return is;
}


/// Read a bimap from an UnPackBuffer
template <class KEY, class VALUE>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is, utilib::bimap<KEY,VALUE>& data)
{
   size_t tmp;
   is >> tmp;
   EXCEPTION_TEST( !is, std::runtime_error, "operator>> - unpack problem.");

   data.clear();
   for (size_t i=0; i<tmp; i++) {
      EXCEPTION_TEST( !is, std::runtime_error, 
                         "operator>> - unpack problem.");
      KEY item;
      is >> item;
      VALUE val;
      is >> val;
      data[item] = val;
      }
   return is;
}

#endif
