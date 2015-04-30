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
 * \file fSerialStream.cpp
 */

#include <utilib/fSerialStream.h>

using std::cerr;
using std::endl;

using std::ios_base;

namespace utilib {


ifSerialStream::ifSerialStream() 
   : iBinarySerialStream(true),
     filebuf()
{ 
   init(&filebuf); 
   init_buffer();
}

ifSerialStream::
ifSerialStream( const char* fname, std::ios_base::openmode mode )
   : filebuf()
{
   init(&filebuf);
   open(fname, mode);
}

bool
ifSerialStream::is_open()
{
   return filebuf.is_open();
}

void
ifSerialStream::open( const char* fname, std::ios_base::openmode mode )
{
   if ( is_open() )
      close();

   delayInit = false;
   mode |= ios_base::in | ios_base::binary;
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
ifSerialStream::close()
{
   if ( ! filebuf.close() )
      setstate(ios_base::failbit);

   // reset the file position pointers...
   init_buffer();
}

//------------------------------------------------------------------------

ofSerialStream::ofSerialStream() 
   : filebuf()
{ 
   init(&filebuf); 
   init_buffer();
}

ofSerialStream::
ofSerialStream( const char* fname, std::ios_base::openmode mode )
   : filebuf()
{
   init(&filebuf);
   open(fname, mode);
}

bool
ofSerialStream::is_open()
{
   return filebuf.is_open();
}

void
ofSerialStream::open( const char* fname, std::ios_base::openmode mode )
{
   if ( is_open() )
      close();

   mode |= ios_base::out | ios_base::binary;
   if ( ! filebuf.open(fname, mode) )
   {
      setstate(ios_base::failbit);
      return;
   }

   clear();
   init_buffer(mode);
   init_stream();
}

void
ofSerialStream::close()
{
   if ( ! filebuf.close() )
      setstate(ios_base::failbit);

   // reset the file position pointers...
   init_buffer();
}

//------------------------------------------------------------------------

fSerialStream::fSerialStream() 
   : BinarySerialStream(true),
     filebuf()
{ 
   init(&filebuf); 
   init_buffer();
}

fSerialStream::
fSerialStream( const char* fname, std::ios_base::openmode mode )
   : filebuf()
{
   init(&filebuf);
   open(fname, mode);
}

bool
fSerialStream::is_open()
{
   return filebuf.is_open();
}

void
fSerialStream::open( const char* fname, std::ios_base::openmode mode )
{
   if ( is_open() )
      close();

   mode |= ios_base::in | ios_base::out | ios_base::binary;
   // NB: per the C++ standard, ( in | app ) is not allowed for
   // fstreams, even though *we* allow it.
   openmode fmode = mode & ~ app;

   if ( ! filebuf.open( fname, fmode ) )
   {
      setstate(ios_base::failbit);
      return;
   }

   clear();
   delayInit = false;
   init_buffer(mode);
   if ( tell_bufg() <= pos_t(0) )
      init_stream();
   else
   {
      // Very special case: if we are appending to a pre-existing file,
      // but opening the input stream at the end instead of at the
      // beginning, we will need to eat the StreamHeader reserved code
      // before we start parsing the stream header.
      oBinarySerialStream::init_stream();
      flush();
      
      // no real use checking this ... the key would have just been
      // written by the oBinarySerialStream::init_stream() above -- we
      // just need to eat it so that the file position pointer gets
      // advanced.
      size_t key = 0;
      read(reinterpret_cast<char*>(&key), sizeof(key));
      iBinarySerialStream::init_stream();
   }
}

void
fSerialStream::close()
{
   if ( ! filebuf.close() )
      setstate(ios_base::failbit);

   // reset the file position pointers...
   init_buffer();
}

//------------------------------------------------------------------------


} // namespace utilib
