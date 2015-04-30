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
 * \file fXMLSerialStream.cpp
 */

#include <utilib/fXMLSerialStream.h>

using std::cerr;
using std::endl;

using std::ios_base;

namespace utilib {


ifXMLSerialStream::ifXMLSerialStream() 
   : filebuf()
{ 
   init(&filebuf); 
   init_buffer();
}

ifXMLSerialStream::
ifXMLSerialStream( const char* fname, std::ios_base::openmode mode )
   : filebuf()
{
   init(&filebuf);
   open(fname, mode);
}

bool
ifXMLSerialStream::is_open()
{
   return filebuf.is_open();
}

void
ifXMLSerialStream::open( const char* fname, std::ios_base::openmode mode )
{
   if ( is_open() )
      close();

   mode |= ios_base::in;
   if ( ! filebuf.open(fname, mode ) )
   {
      setstate(ios_base::failbit);
      return;
   }

   clear();
   init_buffer(mode);
   init_stream();
}

void
ifXMLSerialStream::close()
{
   if ( ! filebuf.close() )
      setstate(ios_base::failbit);

   // reset the file position pointers...
   init_buffer();
}

//------------------------------------------------------------------------

ofXMLSerialStream::ofXMLSerialStream() 
   : filebuf()
{ 
   init(&filebuf); 
   init_buffer();
}

ofXMLSerialStream::
ofXMLSerialStream( const char* fname, 
                   XML::FormatFlags format, 
                   std::ios_base::openmode mode )
   : filebuf()
{
   init(&filebuf);
   open(fname, format, mode);
}

ofXMLSerialStream::~ofXMLSerialStream() 
{ 
   // close the stream (so that finalize_stream() gets called)
   if ( is_open() )
      close();
}

bool
ofXMLSerialStream::is_open()
{
   return filebuf.is_open();
}

void
ofXMLSerialStream::open( const char* fname, 
                         XML::FormatFlags format, 
                         std::ios_base::openmode mode )
{
   if ( is_open() )
      close();

   mode |= ios_base::out;
   if ( ! filebuf.open(fname, mode) )
   {
      setstate(ios_base::failbit);
      return;
   }

   clear();
   set_xml_format(format);
   init_buffer(mode);
   init_stream();
}

void
ofXMLSerialStream::close()
{
   oXMLSerialStream::finalize_stream();
   if ( ! filebuf.close() )
      setstate(ios_base::failbit);

   // reset the file position pointers...
   init_buffer();
}

//------------------------------------------------------------------------

fXMLSerialStream::fXMLSerialStream() 
   : filebuf()
{ 
   init(&filebuf); 
   init_buffer();
}

fXMLSerialStream::
fXMLSerialStream( const char* fname,
                  XML::FormatFlags format, 
                  std::ios_base::openmode mode )
   : filebuf()
{
   init(&filebuf);
   open(fname, format, mode);
}

fXMLSerialStream::~fXMLSerialStream() 
{ 
   // close the stream (so that finalize_stream() gets called)
   if ( is_open() )
      close();
}

bool
fXMLSerialStream::is_open()
{
   return filebuf.is_open();
}

void
fXMLSerialStream::open( const char* fname, 
                        XML::FormatFlags format, 
                        std::ios_base::openmode mode )
{
   if ( is_open() )
      close();

   mode |= ios_base::in | ios_base::out;
   // NB: per the C++ standard, ( in | app ) is not allowed for
   // fstreams, even though *we* allow it.
   openmode fmode = mode & ~ app;

   if ( ! filebuf.open( fname, fmode ) )
   {
      setstate(ios_base::failbit);
      return;
   }

   clear();
   set_xml_format(format);
   init_buffer(mode);
   init_stream();
}

void
fXMLSerialStream::close()
{
   oXMLSerialStream::finalize_stream();
   if ( ! filebuf.close() )
      setstate(ios_base::failbit);

   // reset the file position pointers...
   init_buffer();
}

//------------------------------------------------------------------------


} // namespace utilib
