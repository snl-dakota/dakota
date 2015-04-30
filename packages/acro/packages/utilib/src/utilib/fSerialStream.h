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
 * \file fSerialStream.h
 */

#ifndef utilib_fSerialStream_h
#define utilib_fSerialStream_h

#include <utilib/BinarySerialStream.h>

#include <fstream>

namespace utilib {

class ifSerialStream : public iBinarySerialStream
{
public:
   ifSerialStream();

   ifSerialStream( const char* fname, openmode mode = in | binary );

   bool is_open();

   void open( const char* fname, openmode mode = in | binary );

   void close();

private:
   std::basic_filebuf<char>  filebuf;
};


class ofSerialStream : public oBinarySerialStream
{
public:
   ofSerialStream();

   ofSerialStream( const char* fname, openmode mode = out | binary | trunc );

   bool is_open();

   void open( const char* fname, openmode mode = out | binary | trunc );

   void close();

private:
   std::basic_filebuf<char>  filebuf;
};


class fSerialStream : public BinarySerialStream
{
public:
   fSerialStream();

   fSerialStream( const char* fname, openmode mode = in | out | binary | app );

   bool is_open();

   void open( const char* fname, openmode mode = in | out | binary | app );

   void close();

private:
   std::basic_filebuf<char>  filebuf;
};

} // namespace utilib

#endif // utilib_fSerialStream_h
