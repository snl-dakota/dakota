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
 * \file sSerialStream.cpp
 */

#include <utilib/sSerialStream.h>

using std::cerr;
using std::endl;

using std::ios; // convenient (shorter) alias for ios_base:: constants

namespace utilib {


isSerialStream::isSerialStream() 
   : iBinarySerialStream(true),
     stringbuf()
{ 
   init(&stringbuf);
   init_buffer( ios::in | ios::binary );
   init_stream();
}

isSerialStream::isSerialStream(const std::string &data) 
   : iBinarySerialStream(false),
     stringbuf(data)
{ 
   init(&stringbuf); 
   init_buffer( ios::in | ios::binary );
   init_stream();
}

void
isSerialStream::str( const std::string & data )
{
   stringbuf.str(data);
   clear();
   delayInit = false;
   init_buffer( ios::in | ios::binary );
   init_stream();
}

//------------------------------------------------------------------------

osSerialStream::osSerialStream() 
   : stringbuf()
{ 
   init(&stringbuf); 
#if 1
   // Partial workaround for bug in GCC 3.3: if the stringbuffer is
   // empty, tellp() returns -1 and not 0.  To "fix" this, we will start
   // with a non-empty string and leverage "ios::trunc" mode.
   //
   stringbuf.str(" ");
   init_buffer( ios::out | ios::binary | ios::trunc );
#else
   init_buffer( ios::out | ios::binary );
#endif
   init_stream();
}

osSerialStream::osSerialStream(const std::string &data) 
   : stringbuf(data)
{ 
   init(&stringbuf); 
#if 1
   // Partial workaround for bug in GCC 3.3: if the stringbuffer is
   // empty, tellp() returns -1 and not 0.  To "fix" this, we will start
   // with a non-empty string and leverage "ios::trunc" mode.
   //
   if ( data.empty() )
   {
      stringbuf.str(" ");
      init_buffer( ios::out | ios::binary | ios::trunc );
   }
   else
      init_buffer( ios::out | ios::binary | ios::app );
#else
   init_buffer( ios::out | ios::binary | ios::app );
#endif
   init_stream();
}

void
osSerialStream::str( const std::string & data )
{
   stringbuf.str(data);
   clear();
#if 1
   // Partial workaround for bug in GCC 3.3: if the stringbuffer is
   // empty, tellp() returns -1 and not 0.  To "fix" this, we will start
   // with a non-empty string and leverage "ios::trunc" mode.
   //
   if ( data.empty() )
   {
      stringbuf.str(" ");
      init_buffer( ios::out | ios::binary | ios::trunc );
   }
   else
      init_buffer( ios::out | ios::binary | ios::app );
#else
   init_buffer( ios::out | ios::binary | ios::app );
#endif
   init_stream();
}

//------------------------------------------------------------------------

sSerialStream::sSerialStream() 
   : stringbuf()
{ 
   init(&stringbuf); 
#if 1
   // Partial workaround for bug in GCC 3.3: if the stringbuffer is
   // empty, tellp() returns -1 and not 0.  To "fix" this, we will start
   // with a non-empty string and leverage "ios::trunc" mode.
   stringbuf.str(" ");
   init_buffer( ios::in | ios::out | ios::binary | ios::trunc );
#else
   init_buffer( ios::in | ios::out | ios::binary );
#endif
   init_stream();
}

sSerialStream::sSerialStream( const std::string &data )
   : stringbuf(data)
{
   init(&stringbuf);
#if 1
   // Partial workaround for bug in GCC 3.3: if the stringbuffer is
   // empty, tellp() returns -1 and not 0.  To "fix" this, we will start
   // with a non-empty string and leverage "ios::trunc" mode.
   //
   if ( data.empty() )
   {
      stringbuf.str(" ");
      init_buffer( ios::in | ios::out | ios::binary | ios::trunc );
   }
   else
      init_buffer( ios::in | ios::out | ios::binary | ios::app );
#else
   init_buffer( ios::in | ios::out | ios::binary | ios::app );
#endif
   init_stream();
}

void
sSerialStream::str( const std::string & data )
{
   stringbuf.str(data);
   clear();
   // delayInit = false [delayInit is never true for a sSerialStream]
#if 1
   // Partial workaround for bug in GCC 3.3: if the stringbuffer is
   // empty, tellp() returns -1 and not 0.  To "fix" this, we will start
   // with a non-empty string and leverage "ios::trunc" mode.
   //
   if ( data.empty() )
   {
      stringbuf.str(" ");
      init_buffer( ios::in | ios::out | ios::binary | ios::trunc );
   }
   else
      init_buffer( ios::in | ios::out | ios::binary | ios::app );
#else
   init_buffer( ios::in | ios::out | ios::binary | ios::app );
#endif
   init_stream();
}


//------------------------------------------------------------------------


} // namespace utilib
