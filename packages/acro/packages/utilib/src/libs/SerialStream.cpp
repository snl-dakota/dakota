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
 * \file SerialStream.cpp
 */

#include <utilib/SerialStream.h>

using std::cerr;
using std::endl;

using std::string;

//=======================================================================

namespace utilib {

const SerialStream_base::pos_t SerialStream_base::NOT_SEEKABLE
    = SerialStream_base::pos_t(-2);


iSerialStream::pos_t
iSerialStream::tellg() 
{
   pos_t ans = pos_t(-1);
   try
   {
      if ( ! fail() )
         ans = rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in);
   }
   catch(...)
   {
      setstate(ios_base::badbit); 
   }
   return ans;
}


template<>
iSerialStream& 
iSerialStream::operator>>( SerialObject& object )
{ 
   object = get();
   return *this;
}


template<>
iSerialStream& 
iSerialStream::operator>>( Any& object )
{ 
   SerialObject so = get();
   object = Deserialize(so); 
   return *this;
}


void
iSerialStream::init_buffer(std::ios_base::openmode mode)
{
   if ( good() && mode & ios_base::in )
   {
      if ( mode & ate )
         input_pos = seekg(0, end).tellg();
      else
         input_pos = seekg(0, beg).tellg();

      if ( ! good() )
      {
         clear(); // Clear the stream error state
         input_pos = NOT_SEEKABLE;
      }
   }
   else
      input_pos = pos_t(-1);
   //cerr << "input_pos = " << input_pos << endl;
}

void
iSerialStream::init_stream()
{
   typeKeyTable.clear();
}


iSerialStream&
iSerialStream::seekg(off_type off, ios_base::seekdir ref)
{
   try
   {
      if ( ! fail() )
      {
         const pos_t p = rdbuf()->pubseekoff(off, ref, ios_base::in);
         if ( p == pos_t(off_type(-1)) )
            setstate(ios_base::failbit);
         else
            input_pos = p;
      }
   }
   catch(...)
   { 
      setstate(ios_base::badbit); 
   }

   return *this;
}

iSerialStream&
iSerialStream::seekg(pos_t pos)
{
   try
   {
      if ( ! fail() )
      {
         const pos_t p = rdbuf()->pubseekpos(pos, ios_base::in);
         if ( p == pos_t(off_type(-1)) )
            setstate(ios_base::failbit);
         else
            input_pos = p;
      }
   }
   catch(...)
   { 
      setstate(ios_base::badbit); 
   }

   return *this;
}


class iSerialStream::sentry 
{
public:
   explicit sentry(iSerialStream & is )
      : ok(false)
   {
      if ( is.good() )
      {
         if ( is.tie() )
	    is.tie()->flush();
      }

      if ( is.good() )
         ok = true;
      else
         is.setstate(ios_base::iostate(ios_base::failbit));
   }

   operator bool() const { return ok; }

private:
   bool ok;
};


iSerialStream&
iSerialStream::read(char* s, std::streamsize n)
{
   if ( sentry(*this) )
   {
      try
      {
         if (( input_pos != NOT_SEEKABLE ) && ( input_pos != tellg() ))
            seekg(input_pos);

         std::streamsize count = rdbuf()->sgetn(s, n);

         if ( input_pos != NOT_SEEKABLE )
            input_pos += count;
         if ( count != n )
            setstate(ios_base::eofbit | ios_base::failbit);
      }
      catch( ... )
      { 
         setstate(ios_base::badbit); 
      }
   }
   return *this;
}


//------------------------------------------------------------------------

oSerialStream& 
oSerialStream::flush()
{
   try
   {
      if ( rdbuf() && rdbuf()->pubsync() == -1 )
         setstate(ios_base::badbit); 
   }
   catch(...)
   { 
      setstate(ios_base::badbit); 
   }
   return *this;
}

oSerialStream::pos_t
oSerialStream::tellp() 
{
   pos_t ans = pos_t(-1);
   try
   {
      if ( ! fail() )
         ans = rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
   }
   catch(...)
   {
      setstate(ios_base::badbit); 
   }
   return ans;
}


template<>
oSerialStream& 
oSerialStream::operator<<( const SerialObject& object )
{ 
   put(object); 
   return *this;
}


void
oSerialStream::init_buffer(std::ios_base::openmode mode)
{
   if ( good() && mode & ios_base::out )
   {
      if ( mode & trunc )
         output_pos = seekp(0, beg).tellp();
      else if ( mode & (ate | app) )
         output_pos = seekp(0, end).tellp();
      else
         output_pos = tellp();

      if ( ! good() )
      {
         clear(); // Clear the stream error state
         output_pos = NOT_SEEKABLE;
      }
   }
   else
      output_pos = pos_t(-1);
   //cerr << "output_pos = " << output_pos << endl;
}

void
oSerialStream::init_stream()
{
   typeKeyWritten.clear();
}



oSerialStream&
oSerialStream::seekp(off_type off, ios_base::seekdir ref)
{
   try
   {
      if ( ! fail() )
      {
         const pos_t p = rdbuf()->pubseekoff(off, ref, ios_base::out);
         if ( p == pos_t(off_type(-1)) )
            setstate(ios_base::failbit); 
         else
            output_pos = p;
      }
   }
   catch(...)
   { 
      setstate(ios_base::badbit); 
   }

   return *this;
}


oSerialStream&
oSerialStream::seekp(pos_t pos)
{
   try
   {
      if ( ! fail() )
      {
         const pos_t p = rdbuf()->pubseekpos(pos, ios_base::out);
         if ( p == pos_t(off_type(-1)) )
            setstate(ios_base::failbit);
         else
            output_pos = p;
      }
   }
   catch(...)
   { 
      setstate(ios_base::badbit); 
   }

   return *this;
}


class oSerialStream::sentry 
{
public:
   explicit sentry(oSerialStream & os )
      : ok(false), oss(os)
   {
      if ( oss.tie() && oss.good() )
         oss.tie()->flush();

      if ( oss.good() )
         ok = true;
      else
         oss.setstate(ios_base::failbit);
   }

   ~sentry()
   {
      if ( (oss.flags() & ios_base::unitbuf) && ! std::uncaught_exception() )
      {
         // Can't call flush directly or else will get into recursive lock.
         if ( oss.rdbuf() && oss.rdbuf()->pubsync() == -1 )
	      oss.setstate(ios_base::badbit);
      }
   }

   operator bool() const { return ok; }

private:
   bool           ok;
   oSerialStream  &oss;
};


oSerialStream&
oSerialStream::write(const char* s, const std::streamsize n)
{
   sentry Sentry(*this);
   if ( Sentry )
   {
      try
      {
         if ((output_pos != NOT_SEEKABLE ) && ( output_pos != tellp() ))
            seekp(output_pos);
         //cerr << "writing at " << tellp() << " (len " << n << ")" << endl;

         std::streamsize count = rdbuf()->sputn(s, n);
         if ( output_pos != NOT_SEEKABLE )
            output_pos += count;
         if ( count != n )
            setstate(ios_base::badbit);
      }
      catch ( ... )
      { 
         setstate(ios_base::badbit); 
      }
   }
   //cerr << "   bufinfo (after): " 
   //     << ( rdstate() & goodbit ? "good " : "" )
   //     << ( rdstate() & badbit  ? "bad " : "" )
   //     << ( rdstate() & failbit ? "fail " : "" )
   //     << ( rdstate() & eofbit  ? "eof " : "" )
   //     << endl;
   return *this;
}


//------------------------------------------------------------------------

void
ioSerialStream::init_buffer(std::ios_base::openmode mode)
{
   iSerialStream::init_buffer(mode);
   oSerialStream::init_buffer(mode);
}

void
ioSerialStream::init_stream()
{
   iSerialStream::init_stream();
   oSerialStream::init_stream();
}


//========================================================================

} // namespace utilib
