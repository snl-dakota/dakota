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
 * \file XMLSerialStream.h
 */

#ifndef utilib_XMLSerialStream_h
#define utilib_XMLSerialStream_h

#include <utilib/SerialStream.h>

namespace utilib {

namespace XML {

enum FormatFlags { 
   Default              = 0,
   binary 		= 1L << 0,
   noindent 		= 1L << 1,
   typeids 		= 1L << 2,
   formatflags_end 	= 1L << 16 
};

inline FormatFlags
operator&(FormatFlags __a, FormatFlags __b)
{ return FormatFlags(static_cast<int>(__a) & static_cast<int>(__b)); }

inline FormatFlags
operator|(FormatFlags __a, FormatFlags __b)
{ return FormatFlags(static_cast<int>(__a) | static_cast<int>(__b)); }

inline FormatFlags
operator^(FormatFlags __a, FormatFlags __b)
{ return FormatFlags(static_cast<int>(__a) ^ static_cast<int>(__b)); }

inline FormatFlags&
operator|=(FormatFlags& __a, FormatFlags __b)
{ return __a = __a | __b; }

inline FormatFlags&
operator&=(FormatFlags& __a, FormatFlags __b)
{ return __a = __a & __b; }

inline FormatFlags&
operator^=(FormatFlags& __a, FormatFlags __b)
{ return __a = __a ^ __b; }

inline FormatFlags
operator~(FormatFlags __a)
{ return FormatFlags(~static_cast<int>(__a)); }

} // namespace utilib::XML

//----------------------------------------------------------------------


/// Generic XML serial stream error 
class xmlserialstream_error : public serialstream_error
{
public:
   /// Constructor
   xmlserialstream_error(const std::string& msg) 
      : serialstream_error(msg)
   {}
};


//----------------------------------------------------------------------

class iXMLSerialStream : virtual public iSerialStream
{
public:
   virtual ~iXMLSerialStream();
   
protected:
   iXMLSerialStream();

   void init_stream();

   virtual bool get_object(SerialObject &so);

private:
   class ParserImplementation;

   ParserImplementation* pimpl;
};


class oXMLSerialStream : virtual public oSerialStream
{
public:
   virtual ~oXMLSerialStream() 
   {}

   virtual void put(const SerialObject &so)
   { put_xml(so, ""); }

protected:
   oXMLSerialStream(XML::FormatFlags format = XML::Default)
   {
      set_xml_format(format);
   }

   void init_stream();

   void 
   finalize_stream();

   void
   set_xml_format(XML::FormatFlags format)
   {
      m_format = format;
      pod_text_format = (( format & XML::binary ) == 0 );
   }

private:
   void put_xml(const SerialObject &so, std::string indent);

   XML::FormatFlags m_format;
};


class XMLSerialStream : virtual public ioSerialStream,
                          public iXMLSerialStream,
                          public oXMLSerialStream
{
public:
   virtual ~XMLSerialStream()
   {}

   void init_stream();

protected:
   XMLSerialStream( XML::FormatFlags format = XML::Default )
      : iXMLSerialStream(),
        oXMLSerialStream(format)
   {}

   virtual bool get_object(SerialObject &so)
      { return iXMLSerialStream::get_object(so); }

   virtual void put( const SerialObject& object )
      { oXMLSerialStream::put(object); }
};


//----------------------------------------------------------------------

} // namespace utilib

#endif // utilib_XMLSerialStream_h
