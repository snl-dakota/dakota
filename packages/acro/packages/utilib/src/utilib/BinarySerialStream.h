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
 * \file BinarySerialStream.h
 */

#ifndef utilib_BinarySerialStream_h
#define utilib_BinarySerialStream_h

#include <utilib/SerialStream.h>

namespace utilib {

//----------------------------------------------------------------------

/// Base class for inputting serialized data from a binary stream
class iBinarySerialStream : virtual public iSerialStream
{
public:
   virtual ~iBinarySerialStream() 
   {}

protected:
   iBinarySerialStream(bool delayStreamInit = false)
      : delayInit(delayStreamInit)
   {}

   void init_stream();

   virtual bool get_object(SerialObject &so);

   bool delayInit;
};


/// Base class for outputting serialized data to a binary stream
class oBinarySerialStream : virtual public oSerialStream
{
public:
   virtual ~oBinarySerialStream() 
   {}

   virtual void put(const SerialObject &so);

protected:
   oBinarySerialStream()
   {}

   void init_stream();
};


/// Base class for input & output of serialized data as a binary stream
class BinarySerialStream : virtual public ioSerialStream,
                             public iBinarySerialStream,
                             public oBinarySerialStream
{
public:
   virtual ~BinarySerialStream()
   {}

   void init_stream();

protected:
   BinarySerialStream(bool delayStreamInit = false) 
      : iBinarySerialStream(delayStreamInit),
        oBinarySerialStream()
   {}

   virtual bool get_object(SerialObject &so)
      { return iBinarySerialStream::get_object(so); }

   virtual void put( const SerialObject& object )
      { oBinarySerialStream::put(object); }
};


//----------------------------------------------------------------------

} // namespace utilib

#endif // utilib_BinarySerialStream_h
