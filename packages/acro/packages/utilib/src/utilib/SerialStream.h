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
 * \file SerialStream.h
 */

#ifndef utilib_SerialStream_h
#define utilib_SerialStream_h

#include <utilib/Serialize.h>
#include <utilib/TypeManager.h>

#include <ios>

namespace utilib {

/// Generic serial stream error from which all other exceptions will inherit
class serialstream_error : public std::runtime_error
{
public:
   /// Constructor
   serialstream_error(const std::string& msg) 
      : runtime_error(msg)
   {}
};


//----------------------------------------------------------------------
/** SerialStream_base provides a common base class for all serialized
 *  data streams.
 */
class SerialStream_base : public std::basic_ios<char>
{
public:
   typedef std::basic_streambuf<char>      streambuffer_t;
   typedef std::basic_ios<char>            ios_t;
   typedef std::basic_ios<char>::pos_type  pos_t;

   virtual ~SerialStream_base() 
   {}

public:
   /// This has the effect of hiding the "set" form of rdbuf
   streambuffer_t * rdbuf()
   { return ios_t::rdbuf(); }

protected:
   SerialStream_base()
   {}

   static const pos_t NOT_SEEKABLE;
};


//----------------------------------------------------------------------
/** iSerialStream provides a common base class for all serialized
 *  input data streams.
 */
class iSerialStream : virtual public SerialStream_base
{
public:
   struct TypeKeyInfo {
      TypeKeyInfo()
         : key(0), pod_size(-1), is_pod(false) 
      {}

      size_t key;
      int    pod_size;
      bool   is_pod;
   };
   typedef std::vector<TypeKeyInfo>   typeKeyTable_t;

   virtual ~iSerialStream() 
   {}

public:
   /// Return the next SerialObject from the stream
   SerialObject get()
   {
      SerialObject so;
      get_object(so);
      return so;
   }

   pos_t tellg();

   /// (Convenience) Get the next SerialObject, deserialize, and type cast
   template<typename T>
   iSerialStream& operator>>( T& object )
   { 
      SerialObject so;
      if ( get_object(so) )
         TypeManager()->lexical_cast(Deserialize(so), object); 
      return *this;
   }

protected: // methods
   iSerialStream()
      : typeKeyTable(),
        input_pos(pos_t(-1))
   {}

   pos_t tell_bufg() const
   { return input_pos; }

   /// Initialization called when a new buffer is assigned / opened
   void init_buffer(std::ios_base::openmode mode = std::ios_base::openmode());

   /// Initialize the data stream (stream header, type tables, etc.)
   void init_stream();

   iSerialStream& seekg(off_type, ios_base::seekdir ref);

   iSerialStream& seekg(pos_t pos);

   iSerialStream& read(char* s, std::streamsize n);

   /// Parsing method to extract the next object from a stream (returns
   /// false when end-of-stream marker found)
   virtual bool get_object(SerialObject &so) = 0;

   class sentry;
   friend class sentry;

protected: // data
   typeKeyTable_t    typeKeyTable;
private:   // data
   pos_t          input_pos;
};

/// Specialization to avoid type casting Serial Objects
template<>
iSerialStream&
iSerialStream::operator>>( SerialObject& object );

/// Specialization to avoid type casting Serial Objects to Anys
template<>
iSerialStream&
iSerialStream::operator>>( Any& object );


//----------------------------------------------------------------------
/** oSerialStream provides a common base class for all serialized
 *  output data streams.
 */
class oSerialStream : virtual public SerialStream_base
{
public:
   typedef std::set<size_t>      typeKeyWritten_t;

   virtual ~oSerialStream() 
   {}

public:
   virtual void put( const SerialObject& object ) = 0;

   oSerialStream& flush();

   pos_t tellp();

   template<typename T>
   oSerialStream& operator<<( const T& object )
   { 
      put(Serialize(object, pod_text_format)); 
      return *this;
   }

protected:
   oSerialStream()
      : pod_text_format(false),
        typeKeyWritten(),
        output_pos(pos_t(-1))
   {}

   pos_t tell_bufp() const
   { return output_pos; }

   /// Initialization called when a new buffer is assigned / opened
   void init_buffer(std::ios_base::openmode mode = std::ios_base::openmode());

   /// Initialize the data stream (stream header, type tables, etc.)
   void init_stream();

   oSerialStream& seekp(off_type off, ios_base::seekdir ref);

   oSerialStream& seekp(pos_t pos);

   oSerialStream& write(const char* s, const std::streamsize n);

   class sentry;
   friend class sentry;

protected: // data
   bool              pod_text_format;
   typeKeyWritten_t  typeKeyWritten;
private:   // data
   pos_t          output_pos;
};

template<>
oSerialStream& 
oSerialStream::operator<<( const SerialObject& object );


//----------------------------------------------------------------------
/** ioSerialStream provides a common base class for all serialized
 *  input/output data streams.
 */
class ioSerialStream  : virtual public iSerialStream, 
                        virtual public oSerialStream
{
public:
   virtual ~ioSerialStream() 
   {}

protected:
   ioSerialStream()
   {}

   /// Initialization called when a new buffer is assigned / opened
   void init_buffer(std::ios_base::openmode mode = std::ios_base::openmode());

   /// Initialize the data stream (stream header, type tables, etc.)
   void init_stream();
};


//----------------------------------------------------------------------

} // namespace utilib

#endif // utilib_SerialStream_h
