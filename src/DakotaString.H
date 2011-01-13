/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Dakota::String
//- description: 
//- Owner:       
//- Checked by:
//- Version: $Id: DakotaString.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef DAKOTA_STRING_H
#define DAKOTA_STRING_H

#include "system_defs.h"
#include <string>

namespace Dakota {

class MPIPackBuffer;
class MPIUnpackBuffer;
class String;

// global operators and methods
/// Reads String from buffer 
MPIPackBuffer& operator<<(MPIPackBuffer& s, const String& data);
/// Writes String to buffer
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, String& data);
 
/// Dakota::String class, used as main string class for Dakota

/** The Dakota::String class is the common string class for Dakota.  It
    provides a common interface for string operations whether using
    the std::string interface or the (legacy) RogueWave RWCString API */

class String : public std::string
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Default constructor
  inline String();
  /// Copy constructor for incoming String
  inline String(const String& a);
  /// Copy constructor for portion of incoming String
  inline String(const String& a, size_t start_index, size_t num_items);
  /// Copy constructor for incoming char* array
  inline String(const char* c_string);
  /// Copy constructor for incoming base string
  inline String(const std::string& a);

  /// Destructor
  inline ~String();

  //
  //- Heading: operators
  //

  /// Assignment operator for incoming String
  inline String& operator=(const String&);
  /// Assignment operator for incoming base string
  inline String& operator=(const std::string&);
  /// Assignment operator for incoming char* array
  inline String& operator=(const char*);

  // operators and member fns needed to mimic Rogue Wave RWCString Class

  /// The operator() returns pointer to standard C char array
  inline operator const char*() const;

  //
  //- Heading: Member functions
  //
 
  /// Convert to upper case string
  inline String& toUpper();
  void upper();

  /// Convert to lower case string
  inline String& toLower();
  void lower();

  /// Returns true if String contains char* substring
  inline bool contains(const char* sub_string) const;
  /// Returns true if String starts with char* substring
  inline bool begins(const char* sub_string) const;
  /// Returns true if String ends with char* substring
  inline bool ends(const char* sub_string) const;

  /// Returns pointer to standard C char array
  inline char* data() const;
};


/* Default constructor, defaults to base class constructor */
inline String::String()
{ }


/* Destructor, defaults to base class destructor */
inline String::~String()
{ }


/* Default copy constructor, calls base class copy constructor */
inline String::String(const String& a): std::string(a)
{ }


/* Partial copy constructor, calls base class partial copy constructor */
inline String::String(const String& a, size_t start_index, size_t num_items):
  std::string(a, start_index, num_items)
{ }


/* Copy constructor from char* (standard C char array) */
inline String::String(const char* c_string):
  std::string(c_string)
{ }

/* Copy constructor from std::string */
inline String::String(const std::string& a):
    std::string(a)
{ }

/* Default assignment operator, calls base class assignment operator */
inline String& String::operator=(const String& a)
{
  if (this != &a) // verify that these are different objects
    std::string::operator=(a); // base class assignment operator
  return *this;
}


/* Assignment operator using std::string */
inline String& String::operator=(const std::string& a)
{
  if (this != &a) // verify that these are different objects
    std::string::operator=(a); // base class assignment operator
  return *this;
}


/* Assignment operator using char* (standard C char array) */
inline String& String::operator=(const char* a)
{
  std::string::operator=(a);
  return *this;
}


// These methods needed to mimic RW string class

/** The operator () returns a pointer to a char string.  Uses the STL
    c_str() method.  This allows for the String to be used in method
    calls without having to call the data() or c_str() methods.  */
inline String::operator const char*() const 
{ return std::string::c_str(); }


inline String& String::toUpper()
{ upper(); return *this; }


inline String& String::toLower()
{ lower(); return *this; }


/** Returns true if the String contains the char* sub_string.
    Uses the STL find() method. */
inline bool String::contains(const char* sub_string) const
{
  size_t val = std::string::find(sub_string);
  return ( val < size() ) ? true : false;
}


/** Returns true if the String begins with the char* sub_string.
    Uses the STL compare() method. */
inline bool String::begins(const char* sub_string) const
{
  size_t len = size(), sub_str_len = std::strlen(sub_string);
  if (len >= sub_str_len)
    return ( std::string::compare(0, sub_str_len, sub_string) == 0 )
      ? true : false;
  else
    return false;
}


/** Returns true if the String ends with the char* sub_string.
    Uses the STL compare() method. */
inline bool String::ends(const char* sub_string) const
{
  size_t len = size(), sub_str_len = std::strlen(sub_string);
  if (len >= sub_str_len)
    return (std::string::compare(len - sub_str_len, sub_str_len, sub_string)
				 == 0) ? true : false;
  else
    return false;
}


/** Returns a pointer to C style char array.  Needed to mimic the
    Rogue Wave string class.  USE WITH CARE.  */
inline char* String::data() const
{ return (char *)std::string::c_str(); }


// global function inlines

/// Concatenate two Strings and return the resulting String
inline String operator+(const String& s1, const String& s2)
{ String newStr(s1); newStr += s2; return newStr; }


/// Append a String to a char* and return the resulting String
inline String operator+(const char* s1, const String& s2)
{ String newStr(s1); newStr += s2; return newStr; }


/// Append a char* to a String and return the resulting String
inline String operator+(const String& s1, const char* s2)
{ String newStr(s1); newStr += s2; return newStr; }

/// Append a String to a std::string and return the resulting String
inline String operator+(const std::string& s1, const String& s2)
{ String newStr(s1); newStr += s2; return newStr; }

/// Append a std::string to a String and return the resulting String
inline String operator+(const String& s1, const std::string& s2)
{ String newStr(s1); newStr += s2; return newStr; }

/// Returns a String converted to upper case. Calls String::upper().
inline String toUpper(const String& str)
{ String newStr(str); newStr.upper(); return newStr; }


/// Returns a String converted to lower case. Calls String::lower().
inline String toLower(const String& str)
{ String newStr(str); newStr.lower(); return newStr; }

} // namespace Dakota

#endif  // DAKOTA_STRING_H
