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
 * \file fXMLSerialStream.h
 */

#ifndef utilib_fXMLSerialStream_h
#define utilib_fXMLSerialStream_h

#include <utilib/XMLSerialStream.h>

#include <fstream>

namespace utilib {

class ifXMLSerialStream : public iXMLSerialStream
{
public:
   ifXMLSerialStream();

   ifXMLSerialStream(const char* fname, openmode mode = in );

   bool is_open();

   void open( const char* fname, openmode mode = in );

   void close();

private:
   std::basic_filebuf<char>  filebuf;
};


class ofXMLSerialStream : public oXMLSerialStream
{
public:
   ofXMLSerialStream();

   ofXMLSerialStream( const char* fname, 
                      XML::FormatFlags format = XML::FormatFlags(0),
                      openmode mode = out | trunc );

   virtual ~ofXMLSerialStream();

   bool is_open();

   void open( const char* fname, 
              XML::FormatFlags format = XML::FormatFlags(0),
              openmode mode = out | trunc );

   void close();

private:
   std::basic_filebuf<char>  filebuf;
};


class fXMLSerialStream : public XMLSerialStream
{
public:
   fXMLSerialStream();

   fXMLSerialStream(const char* fname, 
                    XML::FormatFlags format = XML::FormatFlags(0),
                    openmode mode = in | out | app);

   virtual ~fXMLSerialStream();

   bool is_open();

   void open( const char* fname, 
              XML::FormatFlags format = XML::FormatFlags(0),
              openmode mode = in | out | app );

   void close();

private:
   std::basic_filebuf<char>  filebuf;
};

} // namespace utilib

#endif // utilib_fXMLSerialStream_h
