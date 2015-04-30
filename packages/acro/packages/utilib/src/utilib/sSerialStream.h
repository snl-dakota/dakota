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
 * \file sSerialStream.h
 */

#ifndef utilib_sSerialStream_h
#define utilib_sSerialStream_h

#include <utilib/BinarySerialStream.h>

#include <sstream>

namespace utilib {

class isSerialStream : public iBinarySerialStream
{
public:
   isSerialStream();

   isSerialStream(const std::string & data);

   void str( const std::string & data );

private:
   std::basic_stringbuf<char>  stringbuf;
};


class osSerialStream : public oBinarySerialStream
{
public:
   osSerialStream();

   osSerialStream(const std::string & data);

   std::string str() const
   { return stringbuf.str(); }

   void str( const std::string & data );

private:
   std::basic_stringbuf<char>  stringbuf;
};


class sSerialStream : public BinarySerialStream
{
public:
   sSerialStream();

   sSerialStream(const std::string & data);

   std::string str() const
   { return stringbuf.str(); }

   void str( const std::string & data );

private:
   std::basic_stringbuf<char>  stringbuf;
};

} // namespace utilib

#endif // utilib_sSerialStream_h
