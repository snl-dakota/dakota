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
 * \file BinarySerialStream.cpp
 */

/** \class utilib::BinarySerialStream BinarySerialStream.h
 *
 * BinarySerialStream format notes:
 *
 * Version 0 (obsolete):
 * 
 * \verbatim
   BinarySerialStream ::= <stream-header> <data-stream>
   stream-header ::= <file-marker> <version> <endian-flag>
   file-marker   ::= "UBSS"
   version       ::= {byte}
   endian-flag   ::= {byte}
   data-stream   ::= <data> [ <data-stream> ]
   data          ::= <void> | <type-def> | <container> | <pod>
   id            ::= {size_t}
   void          ::= <id=END>
   type-def      ::= <id=TypeKey> <id> <name>
   name          ::= {size_t} {char[size_t]}
   container     ::= <id> <id=ObjectStart> <data-stream> <id=ObjectEnd>
   pod           ::= <id> <id=POD> <pod-data>
   pod-data      ::= {size_t} {char[size_t]} 
 \endverbatim
 * 
 * Version 1:
 *
 * \verbatim
   BinarySerialStream ::= <stream_header> <data_stream>
   stream_header ::= <file_marker> <version> <endian_flag> <size_t_size>
   file_marker   ::= "UBSS"
   version       ::= {char}
   endian_flag   ::= {char}
   size_t_size   ::= {char}
   data_stream   ::= <data> [ <data_stream> ]
   data          ::= <void> | <type_def> | <container> | <pod>
   id            ::= {size_t}
   void          ::= <id=END>
   type_def      ::= <id=TypeKey> <id> <name> <pod_flag>
   pod_flag      ::= {char=0} | {char=1} <pod_length>
   pod_length    ::= {size_t+1}
   name          ::= <char_buffer>
   char_buffer   ::= {size_t} {char[size_t]} 
   container     ::= <id> <data_stream> <id=ObjectEnd>
   pod           ::= <id> [<pod_length>] {char[pod_length]}
 \endverbatim
 */

#include <utilib/BinarySerialStream.h>

using std::cerr;
using std::endl;

using std::string;

//=======================================================================

namespace utilib {

namespace {

const string  binFileMarker  = "UBSS";
const char    binFileVersion = 1;


enum ReservedCodes {
   TypeKey,     // Type ID definition (see <type-def> above)
   ObjectEnd,   // [no data]
   StreamHeader, // mark the start of a new section (with full stream header)
   END
};
const int  ReservedCodesEnd = ReservedCodes(END);

} // namespace utilib::(local)


//========================================================================

void
iBinarySerialStream::init_stream()
{
   iSerialStream::init_stream();

   // If we are actually *reading* a buffer, read in and test the file marker
   if ( delayInit )
      return;

   // test the format marker
   string typeMarker(binFileMarker.size(), ' ');
   read( const_cast<char*>(typeMarker.c_str()), 
         static_cast<std::streamsize>(binFileMarker.size()) );
   if ( !good() || typeMarker != binFileMarker )
   {
      std::stringstream ss;
      for(size_t i = 0; i < typeMarker.size(); ++i)
         ss << " " << std::setw(2) << std::setfill('0') << std::hex
            << static_cast<int>(typeMarker[i]);
      EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::init_stream(): "
                     "stream does not begin with the correct format marker "
                     "(found" << ss.str() << " [hex])");
   }

   // test the format version
   char tmp = 0;
   read(&tmp, 1);
   if ( !good() || tmp != binFileVersion )
      EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::init_stream(): "
                     "stream version (" << (int)(unsigned char)tmp << 
                     ") does not match the current version (" << 
                     (int)(unsigned char)binFileVersion 
                     << ").");

   // test the endian-ness
   read(&tmp, 1);
   if ( !good() || 
        reinterpret_cast<unsigned char&>(tmp) != Serialization_Manager::Endian)
      EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::init_stream(): "
                     "stream endian flag (" << (int)tmp << ") does not "
                     "match this platform (" << 
                     (int)Serialization_Manager::Endian << ").");

   // test the size of size_t
   read(&tmp, 1);
   if ( !good() || tmp != sizeof(size_t) )
      EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::init_stream(): "
                     "stream size_t (" << (int)tmp << ") does not match "
                     "this platform (" << (int)sizeof(size_t) << ").");
}


/** Parse the next object from the stream.  Returns false when the
 *  end-of-stream marker is found.  NB: the end-of-stream marker is not
 *  the same as EOF.
 */
bool
iBinarySerialStream::get_object(SerialObject &so)
{
   if ( delayInit )
   {
      delayInit = false;
      iBinarySerialStream::init_stream();
   }

   // Get the next key in the stream
   size_t key = 0;
   do 
   {
      read(reinterpret_cast<char*>(&key), sizeof(key));
      if ( eof() )
         return true;

      if ( key == ReservedCodes(TypeKey) )
      {
         // get the file key
         size_t type = 0;
         read(reinterpret_cast<char*>(&type), sizeof(type));
         // get the user name
         size_t len = 0;
         read(reinterpret_cast<char*>(&len), sizeof(len));
         string user_name(len,' ');
         read( const_cast<char*>(user_name.c_str()), 
               static_cast<std::streamsize>(len) );
         if ( ! good() )
            EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::"
                           "get_object(): encountered EOF during type "
                           "definition.");
         
         // reserve space in the lookup table...
         if ( type >= typeKeyTable.size() )
            typeKeyTable.resize(type+1);

         // NB: this will allow files to implicitly re-define the lookup
         // table during the file read.  This is important for buffers
         // that have been appended to.
         TypeKeyInfo& tki = typeKeyTable[type];
         // figure out what *our* type id is for this type
         try {
            tki.key = Serializer().get_keyid(user_name);
         }
         catch ( serializer_unknown_type& )
         {
            cerr << "ERROR: iBinarySerialStream::get_object(): "
                 << "stream specifies typeid/user-defined name that "
                 << "we can't resolve." << endl << "     TypeID: "
                 << type << ", Name = \"" << user_name << "\"" << endl;
            throw;
         }

         // make sure that the file's idea of this type is the same as our's
         char is_pod = 0;
         read(&is_pod, sizeof(is_pod));
         tki.is_pod = ( is_pod != 0 );
         if ( Serializer().is_pod(tki.key) != tki.is_pod )
            EXCEPTION_MNGR(serialstream_error, 
                           "iBinarySerialStream::get_object(): SerialStream "
                           "POD flag disagrees with registered type for " << 
                           user_name);
         if ( tki.is_pod )
         {
            size_t pod_size_buf = 0;
            read(reinterpret_cast<char*>(&pod_size_buf), sizeof(pod_size_buf));
            tki.pod_size = static_cast<int>(pod_size_buf) - 1;
            if ( Serializer().get_pod_length(tki.key) != tki.pod_size )
               EXCEPTION_MNGR(serialstream_error, 
                              "iBinarySerialStream::get_object(): "
                              "SerialStream POD size disagrees with "
                              "registered type size for " << user_name);
         }
         else
            tki.pod_size = -1;

         if ( ! good() )
            EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::"
                           "get_object(): encountered EOF during type "
                           "definition POD flags.");
      }
      // Starting a new section in the stream
      else if ( key == ReservedCodes(StreamHeader) )
      {
         // re-initialize the input side of this SerialStream
         iBinarySerialStream::init_stream();
         // Mark this as a TypeKey so that the loop continues and we
         // pull in another key code
         key = ReservedCodes(TypeKey);
      }

   } while ( key == ReservedCodes(TypeKey) );

   // Case 1: if the key marks the End-Of-Object
   if ( key == ReservedCodes(ObjectEnd) )
      return false;

   // Simple stupidity checking (tests if there is a reserved code that
   // is not checked for & handled in the above code.  This code should
   // never be reachable)
   if ( key < ReservedCodes(END) )
      EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::get_object(): "
                     "Stream returned an invalid type key (" << key << ")");

   // Make sure that this key has een previously registered with a
   // TypeKey declaration
   if ( ( key >= typeKeyTable.size() ) || 
        (( typeKeyTable[key].key == 0 ) && ( key != ReservedCodes(END) )) )
      EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::get_object(): "
                     "Stream returned an unknown type key (" << key << ")");

   // Case 2: if the key is void
   //   By definition, void's key will ALWAYS be equal to ReservedCodes(END)
   if ( key == ReservedCodes(END) )
   {
      so = SerialObject();
      return true;
   }

   // Cases 3 & 4 - either a POD or Object
   TypeKeyInfo& tki = typeKeyTable[key];
   so = SerialObject(tki.key);
   if ( tki.is_pod )
   {
      // Read in the POD data
      size_t len = 0;
      if ( tki.pod_size > 0 )
         len = tki.pod_size;
      else
         read(reinterpret_cast<char*>(&len), sizeof(len));
      SerialPOD &pod = so.data.set<SerialPOD>(SerialPOD(len));
      read(const_cast<char*>(pod.data()), static_cast<std::streamsize>(len));
      if ( eof() )
         EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::"
                        "get_object(): encountered EOF in middle of "
                        "SerialPOD.");
   }
   else
   {
      // Read in all of the container's objects (until the ObjectEnd key)
      SerialObject::elementList_t &objectList 
         = so.data.set<SerialObject::elementList_t>();
      SerialObject tmp_so;
      while ( get_object(tmp_so) )
      {
         if ( eof() )
            EXCEPTION_MNGR(serialstream_error, "iBinarySerialStream::"
                           "get_object(): encountered EOF in middle of "
                           "SerialObject container.");
         objectList.push_back(tmp_so);
      }
   }
   return true;
}


//------------------------------------------------------------------------

void
oBinarySerialStream::init_stream()
{
   oSerialStream::init_stream();

   // If we are appending to a non-empty file, we *should* check the
   // initial file header to make sure it is compatible.  Unfortunately,
   // as this is an output stream, we can only seek back and read from
   // the data if this is actually an i/o stream.  If that is the case,
   // the INPUT side init may check the header for us (so we don't have
   // to do it here -- provided the file is not opened ios::ate).  
   //
   // As for output-only streams, there is nothing we can really do to
   // check the initial data.  We could toss a warning, but that would
   // just get annoying if the user was doing this intentionally.  Our
   // current solution is to just ASSUME that the first part of the file
   // is a valid UBSS file.  We will start our section with a new stream
   // header, so in case the previous data was written by a different
   // process or on a different platform, the reader can be notified
   // that all the definitions and parameters may change.


   // If this stream is appending to an existing stream with data, then
   // we need to write out the Key Code to mark the beginning of a new
   // Stream Header section.
   if ( tell_bufp() > pos_t(0) )
   {
      size_t initMarker = ReservedCodes(StreamHeader);
      write(reinterpret_cast<char*>(&initMarker), sizeof(size_t));
   }

   // If we are actually *writing* a buffer (even if there is
   // pre-existing data) we need to start with the file marker

   // write the format marker
   write( binFileMarker.c_str(), 
          static_cast<std::streamsize>(binFileMarker.size()) );
   // write the format version
   write(&binFileVersion, 1);
   // write the endian-ness flag
   write(reinterpret_cast<const char*>(&Serialization_Manager::Endian), 1);
   // write the size of the size_t type
   char tmp = sizeof(size_t);
   write(&tmp, 1);
}


void
oBinarySerialStream::put(const SerialObject &so)
{
   size_t key = so.type;

   if ( typeKeyWritten.count(key) == 0 )
   {
      // Key code: new type
      key = ReservedCodes(TypeKey);
      write(reinterpret_cast<char*>(&key), sizeof(key));
      // key (adjusted)
      key = so.type + ReservedCodes(END);
      write(reinterpret_cast<char*>(&key), sizeof(key));
      // username name
      string name = Serializer().get_username(so.type);
      size_t len = name.size();
      write(reinterpret_cast<char*>(&len), sizeof(len));
      write( name.c_str(),  static_cast<std::streamsize>(len) );
      // the POD flag
      char is_pod = ( Serializer().is_pod(so.type) ? 1 : 0 );
      write(&is_pod, sizeof(is_pod));
      if ( is_pod != 0 )
      {
         // the POD size
         size_t pod_size = Serializer().get_pod_length(so.type) + 1;
         write(reinterpret_cast<char*>(&pod_size), sizeof(pod_size));
      }

      // remember that we have writen this out...
      typeKeyWritten.insert(so.type);
   }

   // write out the key code 
   //   NB: we must shift all codes to make room for the Protocol's
   // reserved codes
   key = so.type + ReservedCodes(END);
   write(reinterpret_cast<char*>(&key), sizeof(key));

   if ( so.data.empty() )
   {
      // VOID (nothing to do)
   }
   else if ( so.data.is_type(typeid(SerialPOD)) )
   {
      // write out the POD data
      const SerialPOD& pod = so.data.expose<SerialPOD>();
      int len = Serializer().get_pod_length(so.type);
      size_t pod_size;
      if ( len < 0 )
      {
         pod_size = pod.size();
         write(reinterpret_cast<char*>(&pod_size), sizeof(pod_size));
      }
      else
         pod_size = len;
      write(pod.data(),  static_cast<std::streamsize>(pod_size));
   }
   else
   {
      // write out the Object data
      const SerialObject::elementList_t& objectList
         = so.data.expose<SerialObject::elementList_t>();
      SerialObject::elementList_t::const_iterator it = objectList.begin();
      SerialObject::elementList_t::const_iterator itEnd = objectList.end();
      for( ; it != itEnd; ++it )
         put(*it);

      // and the Object closer
      key = ReservedCodes(ObjectEnd);
      write(reinterpret_cast<char*>(&key), sizeof(key));
   }
}

//------------------------------------------------------------------------

void
BinarySerialStream::init_stream()
{
   // NB: i/o BinarySerialStream::init_stream() each call i/o
   // SerialStream::init_stream(), so we don't need to call this here.
   //ioSerialStream::init_stream(mode);

   // it is *critical* to initialize the output side FIRST so that if
   // this is a new file, the output side will create the binary header
   // before the input side reads it.
   oBinarySerialStream::init_stream();
   if ( ! delayInit )
   {
      flush();
      iBinarySerialStream::init_stream();
   }
}

//=======================================================================

} // namespace utilib
