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
 * \file CharString.h
 *
 * Defines the utilib::CharString class
 */

#ifndef utilib_CharString_h
#define utilib_CharString_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/BasicArray.h>
#include <utilib/comments.h>

namespace utilib {


/**
 * A class to manipulate strings that contain a terminating null at 
 * the end, without requiring the user to explicitly know that this exists.
 *
 * Potential bug:  when a CharString is initialized with a 
 * const char*, the class does not explicitly prohibit the
 * modification of the character array.  This was done to
 * keep all of the basic operations relatively efficient.
 *
 * This version was modified by Jonathan Eckstein to use the features
 * of the new ArrayBase class.  In particular, the 'Len' member stores
 * the "size()" of the array perceived by the outside world.  The 
 * alloc_size method is overridden to make sure that there is room
 * for a trailing null.
 *
 */
class CharString : public BasicArray<char> {

  public:
 
    /// Null constructor.
    CharString() : BasicArray<char>() { }

    /**
     * Constructor that initializes the length and possibly data.
     * The \a len parameter specifies the length of the array.
     * If the \a d parameter is not null, then this array is
     * assumed to have length \a len.  The initialization of the
     * array then depends upon the value of the \a o parameter. If
     * \a o is \b DataNotOwned (the default), then the data is
     * copied from \a d.  Otherwise, the internal point is set to
     * \a d and the internal ownership flag is set to \a o.
     */
    explicit 
    CharString(const size_type len, 
               char* str=(char*)0, 
               const EnumDataOwned own=DataNotOwned) :
      BasicArray<char>() 
      { construct(len,str,own); }

    /**
     * Constructor that initializes the data using the array 
     * \a str.  The length is determined by computing the length
     * of \a str, and ownership is handled as above.
     */
    CharString(char* str, const EnumDataOwned own=DataNotOwned);

    /**
     * Constructor that initializes the data using the array 
     * \a str.  The length is determined by computing the length
     * of \a str, and ownership is handled as above.
     */
    CharString(const char* str, const EnumDataOwned own=DataNotOwned);

    /// Copy constructor.
    CharString(const CharString& array) :
      BasicArray<char>()
        { construct(array.Len, array.Data, DataOwned); }

    /// Return a pointer to the string's data
    char* c_str() {return Data;}

    /// Return a const pointer to the string's const data
    const char* c_str() const {return Data;}


    /// Compute how big a char[] to allocate for an array.  If the
    /// string is nonempty, allocate an extra byte for a trailing null.
    size_type alloc_size(size_type l) const
      {
        if (l == 0)
          return 0;
        else
          return l + 1;
      }

    /// Copy operator=
    CharString& operator=(const CharString& str)
      {
        if (Data != str.Data) {
          free();
          construct(str.size(),str.data(),DataOwned);
        }
        return *this;
      }

    /// Basic comparison method: returns 1 if greater than \b s, 0 if equal,
    /// and -1 if less than \b s.  Comparisons are lexicographic.
    int compare(const char* s) const;

    /// Compare to another CharString
    inline int compare(const CharString& otherString) const
      { return compare(otherString.Data); }

    /// Checks to see if the current array equals \b array.
    inline bool operator==(const char* array) const 
        { return compare(array) == 0; }

    /// Check for equality with another string
    inline bool operator==(const CharString& otherString) const
        { return compare(otherString.Data) == 0; }

    /// Checks to see if the current array is not equal to \b array.
    inline bool operator!= (const char* array) const 
        { return compare(array) != 0; }

    /// Check for inequality with another string
    inline bool operator!=(const CharString& otherString) const
        { return compare(otherString.Data) != 0; }

    /// Checks to see if the current array equals \b array.
    inline bool operator==(char* array) const
        { return operator==((const char*)array); }

    /// Checks to see if the current array is not equal to \b array.
    inline bool operator!= (char* array) const
        { return operator!=((const char*)array); }

    /// Checks to see if the current array is lexicographically 
    /// less than \b array.
    inline bool operator<(const char* array) const
        { return compare(array) < 0; }

    /// Check whether less than another string
    inline bool operator<(const CharString& otherString) const
        { return compare(otherString.Data) < 0; }

    /// Checks to see if the current array is lexicographically 
    /// greater than \b array.
    inline bool  operator>(const char* array) const
        { return compare(array) > 0; }

    /// Check whether greater than another string
    inline bool operator>(const CharString& otherString) const
        { return compare(otherString.Data) > 0; }

    /// Checks to see if the current array is lexicographically 
    /// less than or equal to \b array.
    inline bool operator<=(const char* array) const
        { return compare(array) <= 0; }

    /// Check whether less than or equal to another string
    inline bool operator<=(const CharString& otherString) const
        { return compare(otherString.Data) <= 0; }

    /// Checks to see if the current array is lexicographically 
    /// greater than or equal to \b array.
    inline bool  operator>=(const char* array) const
        { return compare(array) >= 0; }

    /// Check whether greater than or equal to another string
    inline bool operator>=(const CharString& otherString) const
        { return compare(otherString.Data) >= 0; }

    /// Used if you want to make GenericHeap<CharString> and the like.
    bool equivalent(CharString& array) const
      { return compare(array.Data) == 0; }

    /// Appends the string with \a array
    CharString& operator+=(const char* array);

    /// Appends the string with \a array
    CharString& operator+=(char* array)
                {return operator+=((const char*)array);}

    /// Appends the string with \a array
    CharString& operator+=(const CharString& array)
                {return operator+=((const char*)array.data());}

    /// Appends the string with \a array
    CharString& operator+=(const std::string& array)
                {return operator+=((const char*)array.data());}

    /// Appends the string with \a array
    CharString& operator+=(CharString& array)
                {return operator+=((const char*)array.data());}

    /// Appends the string with a single character 
    CharString& operator+=(char val);

    /// Appends the string with a string representation of \a val
    CharString& operator+=(const int val);

    /// Appends the string with a string representation of \a val
    CharString& operator+=(const unsigned int val);

    /// Appends the string with a string representation of \a val
    CharString& operator+=(const double val);

    /// Appends the string with a string representation of \a val
    template <class Type>
    CharString& operator+=(const Ereal<Type>& val)
        {return this->operator+=( static_cast<double>(val) );}

    /**
     * Copy a substring from \a datavec.
     * The substring beginning at \a offset in \a datavec and 
     * ending at \a offset + \a len - 1 is copied to the substring
     * in the current array beginning at \a start.
     */
    CharString& set_subvec(const size_type start, const size_type len,
                           const CharString& datavec, 
                           const size_type offset=0);

    /// Write the string to an output stream.
    void write(std::ostream& output) const;

    /// Pack the array into a PackBuffer class.
    void write(PackBuffer& output) const ;

  /**
   * Read a string from an input stream.
   * This method operates like a normal stream operator would.
   * Whitespace is ignored, and characters are read until a
   * standard delimiter is read: whitespace, '\\t' or '\\n'.
   */
  void read(std::istream& input);

  /// Unpack the string from an UnPackBuffer class.
  void read(UnPackBuffer& input);

  #if !defined(DOXYGEN)
  friend int compare(const CharString& str1, const CharString& str2);
  #endif

protected:

    /// Override the usual copy_data method from ArrayBase to handle
    /// trailing nulls.
    void copy_data(char*     target,
                   size_type target_size,
                   char*     source,
                   size_type source_size);

    /// Override the usual initialize to write nulls.
    void initialize(char* data,  
                    const size_type start, 
                    const size_type stop)
                {
                for(size_type i=start; i<=stop; i++)
                  data[i] = 0;
                }

    /// For debug output, the string will get written in the 
    /// earlier part of debug_print, anyway, so don't write.
    void dump_data(std::ostream& str, unsigned int max_elements=0);

private:
   ///
   static const int register_serializer;
   ///
   static int serializer( SerialObject::elementList_t& serial, 
                          Any& data, bool serialize );
};



/// Convert a string to a long integer
long int aslong(const CharString& str, int& status);


/// Convert a string to a double
double asdouble(const CharString& str, int& status);

/// Generate an upper-case version of this string
CharString upper_case(CharString string);

//============================================================================
//
//
inline void CharString::write(std::ostream& os) const
{
if (size() == 0) return;
assert(data()[size()] == '\000');
os << data();
}


//============================================================================
//
//
inline void CharString::read(std::istream& is)
{
  char buf[256];
  int ndx=0;

  is >> utilib::whitespace;

  char c, pc=' ';
  bool flag=true;
  bool str_flag=false;

  while ((flag==true) && is) {
    if (ndx == 255)
       EXCEPTION_MNGR(std::runtime_error, "CharString::read - Reading a string longer than 256 characters is not supported yet!");
    is.get(c);
    if (is) {
      if ((str_flag == false) && ((c == ' ') || (c == '\t') || (c == '\n')))
        flag = false;
      else {
        if (str_flag && (c == '"')) {
          if (pc == '\\')
            ndx--;
          else
            flag = false;
        }
        else {
          if (c == '"') str_flag=true;
          else buf[ndx++] = c;
        }
        pc=c;
      }
    }
  }
  buf[ndx] = '\000';

  resize(ndx);
  if (ndx > 0)
#if (_MSC_VER >= 1400) 
    // strncpy_s is only available starting in VISUAL .NET 2005
    strncpy_s(data(),ndx+1,buf,ndx+1);
#else
    std::strncpy(data(),buf,ndx+1);
#endif
}


//============================================================================
//
//
inline void CharString::write(utilib::PackBuffer& os) const
{
size_t len = size();
os << len;
for (size_type i=0; i<size(); i++)
  os << Data[i];
}


//============================================================================
//
//
inline void CharString::read(utilib::UnPackBuffer& is)
{
size_type len=0;
is >> len;
resize(len);
if (len > 0)
  {
    char* data = Data;
    for (size_type i=0; i<len; i++)
      is >> *(data++);
    *data = '\000';
  }
}

} // namespace utilib

/// Out-stream operator for writing the contents of a CharString
inline std::ostream& operator<<(std::ostream& os, const utilib::CharString& obj)
{ obj.write(os); return(os); }

/// Out-stream operator for writing the contents of a CharString
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
                                        const utilib::CharString& obj)
{ obj.write(os); return(os); }

/// In-stream operator for reading the contents of a CharString
inline std::istream& operator>>(std::istream& is, utilib::CharString& obj)
{ obj.read(is); return(is); }

/// In-stream operator for reading the contents of a CharString
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
                                        utilib::CharString& obj)
{ obj.read(is); return(is); }

namespace utilib {
DEFINE_FULL_ANY_EXTENSIONS(utilib::CharString);
} // namespace utilib

#endif
