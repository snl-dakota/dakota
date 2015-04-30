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
 * \file XMLSerialStream.cpp
 */

/**
 * XMLSerialStream format notes:
 *
 * Version 1:
 *
 * \verbatim
 *   (define the schema here)
 * \endverbatim
 */

#include <utilib/XMLSerialStream.h>

#ifdef UTILIB_HAVE_EXPAT_H
extern "C" {
#include <expat.h>
}
#endif

using std::cerr;
using std::endl;

using std::string;
using std::vector;
using std::list;
using std::map;
using std::pair;

using std::stringstream;

//=======================================================================

namespace utilib {

namespace {

const string  xmlElementFile    = "UtilibSerialStream";
const string  xmlElementType    = "TypeDef";
const string  xmlElementVoid    = "Void";
const string  xmlElementObject  = "Object";
const string  xmlElementPod     = "Pod";
const string  xmlAttrVersion    = "version";
const string  xmlAttrEndian     = "endian";
const string  xmlAttrTypeName   = "type";
const string  xmlAttrTypeId     = "typeid";
const string  xmlAttrPodSize    = "pod";
const string  xmlAttrFormat     = "format";
const string  xmlAttrValHex     = "hex";
const string  xmlIndent         = "   ";
const int     xmlFileVersion    = 1;


class XMLCharMap
{
public:
   XMLCharMap()
      : encode(256)
   {
      cmap.insert(cmap.end(), pair<char,string>('"',  "&quot;"));
      cmap.insert(cmap.end(), pair<char,string>('&',  "&amp;"));
      cmap.insert(cmap.end(), pair<char,string>('\'', "&apos;"));
      cmap.insert(cmap.end(), pair<char,string>('<',  "&lt;"));
      cmap.insert(cmap.end(), pair<char,string>('>',  "&gt;"));

      map<char,string>::iterator it;
      for( it = cmap.begin(); it != cmap.end(); ++it )
      {
         decode.insert(pair<string,char>(it->second.substr(1,2), it->first));
         encode[it->first].first = it->second.size();
         encode[it->first].second = it->second;
      }
   }
    
   vector<pair<size_t,string> > encode;
   map<char,string>  cmap;
   map<string,char>  decode;
};


string
XMLify(const string& str)
{
   static const XMLCharMap cmap;
   size_t size = str.size();

   list<string::const_iterator> special;
   int i = 0;
   string::const_iterator it = str.begin();
   string::const_iterator itEnd = str.end();
   for(; it != itEnd; ++it )
   {
      i = cmap.encode[*it].first;
      if ( i > 0 )
      {
         size += i;
         special.push_back(it);
      }
   }

   string s;
   s.reserve(size);
   it = str.begin();
   while ( ! special.empty() )
   {
      s.append(it, special.front());
      s.append(cmap.encode[*special.front()].second);
      it = special.front();
      ++it;
      special.pop_front();
   }
   if ( it != str.end() )
      s.append(it, str.end());
   return s;
}

SerialStream_base::pos_t
verify_xml_stream_header(SerialStream_base::streambuffer_t *buf)
{ return buf->pubseekoff(0, std::ios_base::cur, std::ios_base::in); }

} // namespace utilib::(local)

//========================================================================

#ifdef UTILIB_HAVE_EXPAT_H

class iXMLSerialStream::ParserImplementation 
{
public:
   ParserImplementation(typeKeyTable_t &_typeKeyTable)
      : typeKeyTable(_typeKeyTable),
        parser(XML_ParserCreate(NULL)),
        initialized(false),
        after_main_element(false),
        in_pod(false),
        skipWS(false),
        binary(false),
        terminalElement(false),
        completeObjReady(false),
        fileEndian(0)
   {
      XML_SetUserData(parser, this);
      XML_SetElementHandler(parser, start_element, end_element);
      XML_SetCharacterDataHandler(parser, char_data);
   }

   ~ParserImplementation()
   {
      XML_ParserFree(parser);
   }

   bool inMainElement() const
   { return initialized; }

   bool endOfMainElement() const
   { return after_main_element; }

   bool objectReady() const
   { return completeObjReady; }

   SerialObject get_object()
   {
      completeObjReady = false;
      return completeObj;
   }

   char* get_buffer() const
   { return static_cast<char*>(XML_GetBuffer(parser, 1)); }

   int parse_char(bool isFinal)
   { return XML_ParseBuffer(parser, (isFinal ? 0 : 1), isFinal); }

   string get_error() const
   {
      stringstream s;
      s << XML_GetErrorCode(parser) << ": "
        << XML_ErrorString(XML_GetErrorCode(parser))
        << " at line " << XML_GetCurrentLineNumber(parser);
      return s.str();
   }

   bool is_error(XML_Error e) const
   { return XML_GetErrorCode(parser) == e; }

private:
   static void
   start_element(void *data, const char *el, const char **attr);

   static void
   end_element(void *data, const char *el);

   static void
   char_data(void *data, const char *txt, int txtlen);

private:
   iSerialStream::typeKeyTable_t &typeKeyTable;
   XML_Parser     parser;
   bool           initialized;
   bool           after_main_element;
   bool           in_pod;
   bool           skipWS;
   bool           binary;
   bool           terminalElement;
   bool           completeObjReady;
   unsigned char  fileEndian;
   stringstream   pod_data;
   std::list<SerialObject>  objStack;
   SerialObject   completeObj;
};


void
iXMLSerialStream::ParserImplementation::
start_element(void *data, const char *el, const char **attr) 
{
   ParserImplementation &p = *static_cast<ParserImplementation*>(data);

   // Check the top-level element
   string element = string(el);
   if (( ! p.initialized ) && ( element != xmlElementFile ))
      EXCEPTION_MNGR(xmlserialstream_error,
                     "iXMLSerialStream::ParserImplementation: "
                     "File begins with an invalid element (" << element <<")");

   if ( p.terminalElement )
      EXCEPTION_MNGR(xmlserialstream_error, 
                     "iXMLSerialStream::ParserImplementation: "
                     "sub-element found where none are allowed");

   // process any element attributes
   map<string, string> attrib;
   while ( *attr != NULL )
   {
      attrib[*attr] = *(attr+1);
      attr += 2;
   }

   // convert the type name and type id elements into numerical form
   size_t key = 0;
   size_t fileKey = 0;
   map<string,string>::iterator it = attrib.find(xmlAttrTypeId);
   if ( it != attrib.end() )
   {
      stringstream s;
      s.str(it->second);
      s >> fileKey;
      if ( fileKey >= p.typeKeyTable.size() )
         p.typeKeyTable.resize(fileKey+1);
   }
   it = attrib.find(xmlAttrTypeName);
   if ( it != attrib.end() )
      try {
         key = Serializer().get_keyid(it->second);
      }
      catch ( serializer_unknown_type& )
      {
         cerr << "ERROR: iXMLSerialStream::ParserImplementation: "
              << "stream specifies user-defined name that "
              << "we can't resolve." << endl << "     Name = \"" 
              << it->second << "\"" << endl;
         throw;
      }

   // process each of the element types we know about
   p.terminalElement = true;
   if ( element == xmlElementPod )
   {
      if ( it == attrib.end() )
      {
         key = p.typeKeyTable[fileKey].key;
         if (( key == 0 ) && ( fileKey != 0 ))
            EXCEPTION_MNGR(xmlserialstream_error, 
                           "iXMLSerialStream::ParserImplementation: "
                           "typeid refers to undefined ID");
      }
      if ( Serializer().is_pod(key) != true )
         EXCEPTION_MNGR(xmlserialstream_error, 
                        "iXMLSerialStream::ParserImplementation: "
                        "POD flag disagrees with registered type");
      p.objStack.push_back(SerialObject(key));
      p.binary = ( attrib[xmlAttrFormat] == xmlAttrValHex );
      if (( p.binary ) && ( Serialization_Manager::Endian != p.fileEndian ))
         EXCEPTION_MNGR(xmlserialstream_error, 
                        "iXMLSerialStream::ParserImplementation: "
                        "POD contains binary data that does not match this "
                        "platform's endian flag: " 
                        << (int)Serialization_Manager::Endian << "," 
                        << (int)p.fileEndian);
      p.in_pod = true;
      p.skipWS = true;
      p.pod_data.str("");
   }
   else if ( element == xmlElementObject )
   {
      if ( it == attrib.end() )
      {
         key = p.typeKeyTable[fileKey].key;
         if (( key == 0 ) && ( fileKey != 0 ))
            EXCEPTION_MNGR(xmlserialstream_error, 
                           "iXMLSerialStream::ParserImplementation: "
                           "typeid refers to undefined ID");
      }
      if ( Serializer().is_pod(key) != false )
         EXCEPTION_MNGR(xmlserialstream_error, 
                        "iXMLSerialStream::ParserImplementation: "
                        "POD flag disagrees with registered type");
      p.objStack.push_back(SerialObject(key));
      p.objStack.back().data.set<SerialObject::elementList_t>();
      p.terminalElement = false;
   }
   else if ( element == xmlElementType )
   {
      p.typeKeyTable[fileKey].key = key;
   }
   else if ( element == xmlElementVoid )
   {
      p.objStack.push_back(SerialObject());
   }
   else if ( element == xmlElementFile )
   {
      int ver = atoi(attrib[xmlAttrVersion].c_str());
      if ( ver != xmlFileVersion )
         EXCEPTION_MNGR(xmlserialstream_error, 
                        "iXMLSerialStream::ParserImplementation::"
                        "start_element(): inconsistent file version (file=" 
                        << ver << ", parser=" << xmlFileVersion <<")");

      string::iterator a_it = attrib[xmlAttrEndian].begin();
      string::iterator a_itEnd = attrib[xmlAttrEndian].end();
      if ( a_it == a_itEnd )
         EXCEPTION_MNGR(xmlserialstream_error,
                        "iXMLSerialStream::ParserImplementation::"
                        "start_element(): endian marker not found.");
      for(p.fileEndian=0; a_it != a_itEnd; ++a_it)
      {
         int value = 0;
         if (( *a_it <= '9' ) && ( *a_it >= '0' ))
            value = (*a_it - '0');
         else if (( *a_it >= 'a' ) && ( *a_it <= 'f' ))
            value = 10 + (*a_it - 'a');
         else if (( *a_it >= 'A' ) && ( *a_it <= 'F' ))
            value = 10 + (*a_it - 'A');
         else
            EXCEPTION_MNGR(xmlserialstream_error,
                           "iXMLSerialStream::ParserImplementation::"
                           "start_element(): invalid endian marker character ("
                           << *a_it << ").");
         p.fileEndian <<= 4;
         p.fileEndian += value;
      }
      if ( ! p.fileEndian )
         EXCEPTION_MNGR(xmlserialstream_error,
                        "iXMLSerialStream::ParserImplementation::"
                        "start_element(): endian marker not valid.");

      // NB: we will skip the endian test unless we actually have to
      // read in binary data.
      p.initialized = true;
      p.terminalElement = false;
   }
   else
      EXCEPTION_MNGR(xmlserialstream_error,
                     "iXMLSerialStream::ParserImplementation::start_element():"
                     " unknown element (" << element << ").");
}


void
iXMLSerialStream::ParserImplementation::
end_element(void *data, const char *el) 
{
   ParserImplementation &p = *static_cast<ParserImplementation*>(data);
   p.terminalElement = false;

   string element = string(el);
   if ( element == xmlElementPod )
   {
      p.in_pod = false;
      string str = p.pod_data.str();
      // Strip off the trailing whitespace
      size_t endIndex = str.size();
      while ( endIndex > 0 )
      {
         char c = str[--endIndex];
         if (( c != ' ' ) && ( c != '\n' ) && ( c != '\r' ) && ( c != '\t' ))
            break;
      }
      str.resize(++endIndex);

      // Process the string
      if ( p.binary )
      {
         // "Binary" mode - we need to convert the hex characters back
         // into a binary stream.
         unsigned char pod_char = 0;
         bool whole_char = true;
         string binData;
         binData.reserve(str.size()/2);

         // process a character at a time...
         const char *c = str.c_str();
         while ( endIndex > 0 )
         {
            // filter out non-hex characters
            int value = -1;
            if (( *c <= '9' ) && ( *c >= '0' ))
               value = (*c - '0');
            else if (( *c >= 'a' ) && ( *c <= 'f' ))
               value = 10 + (*c - 'a');
            else if (( *c >= 'A' ) && ( *c <= 'F' ))
               value = 10 + (*c - 'A');
            else if ((*c!=' ') && (*c!='\n') && (*c!='\r') && (*c!='\t'))
               EXCEPTION_MNGR(xmlserialstream_error, 
                              "iXMLSerialStream::ParserImplementation: "
                              "invalid character in hexidecimal binary data");
            if ( value != -1 )
            {
               pod_char <<= 4;
               pod_char += value;
               whole_char = ! whole_char;
               if ( whole_char )
                  binData.append(1, pod_char);
            }
            ++c;
            --endIndex;
         }
         if ( ! whole_char )
            EXCEPTION_MNGR(xmlserialstream_error,
                           "iXMLSerialStream::ParserImplementation: "
                           "hexidecimal binary data ended with a partial "
                           "byte");
                           
         p.objStack.back().data.set<SerialPOD>()
            .set(binData.c_str(), binData.size());
      }
      else
      {
         // text-mode... easy!
         p.objStack.back().data.set<SerialPOD>().set(str);
      }
   }
   else if (( element != xmlElementObject ) && ( element != xmlElementVoid ))
   {
      if ( element == xmlElementFile )
         p.after_main_element = true;
      return;
   }

   // POD, Object, and Void are all processed the same now...
   if ( p.completeObjReady )
      EXCEPTION_MNGR(xmlserialstream_error, 
                     "iXMLSerialStream::ParserImplementation: "
                     "Driver did not retrieve processed SerialObject.  "
                     "Data lost.");

   // insert this object into the parent's object list (if there is a
   // parent object)
   p.completeObj = p.objStack.back();
   p.objStack.pop_back();
   if ( p.objStack.empty() )
      p.completeObjReady = true;
   else
      const_cast<SerialObject::elementList_t&>
         (p.objStack.back().data.expose<SerialObject::elementList_t>())
         .push_back(p.completeObj);
}


void
iXMLSerialStream::ParserImplementation::
char_data(void *data, const char *txt, int txtlen) 
{
   ParserImplementation &p = *static_cast<ParserImplementation*>(data);
   if ( ! p.in_pod )
      return;
   while ( p.skipWS )
   {
      if ( txtlen == 0 )
         return;
      if (( *txt==' ' ) || ( *txt=='\n' ) || ( *txt=='\r' ) || ( *txt=='\t' ))
      {
         ++txt;
         --txtlen;
      }
      else
         p.skipWS = false;
   }

   p.pod_data.write(txt, txtlen);
}

#else // (not defined) UTILIB_HAVE_EXPAT_H

enum XML_Error {
   XML_ERROR_NO_ELEMENTS,
   // ...
};

class iXMLSerialStream::ParserImplementation 
{
public:
   ParserImplementation(typeKeyTable_t&)
   {
      EXCEPTION_MNGR(std::logic_error, 
                     "UTILIB compiled without Expat support.  "
                     "iXMLSerialStream is not available.");
   }

   bool inMainElement() const
   { return false; }

   bool endOfMainElement() const
   { return false; }

   bool objectReady() const
   { return false; }

   SerialObject get_object()
   { return SerialObject(); }

   char* get_buffer() const
   { return NULL; }

   int parse_char(bool isFinal)
   { return 0; }

   string get_error() const
   { return "UTILIB compiled without Expat support."; }

   bool is_error(XML_Error e) const
   { return false; }
};

#endif // UTILIB_HAVE_EXPAT_H



//------------------------------------------------------------------------

iXMLSerialStream::iXMLSerialStream()
   : iSerialStream(),
     pimpl(NULL)
{}


iXMLSerialStream::~iXMLSerialStream()
{
   delete pimpl;
   pimpl = NULL;
}


void
iXMLSerialStream::init_stream()
{
   iSerialStream::init_stream();

   delete pimpl;
   pimpl = new ParserImplementation(typeKeyTable);
}


bool
iXMLSerialStream::get_object(SerialObject &so)
{
   if ( ! pimpl )
      EXCEPTION_MNGR(xmlserialstream_error, "iXMLSerialStream::get_object() "
                     "called on an uninitialized stream.");

   while( ! pimpl->objectReady() )
   {
      //std::cerr << "eof: " << eof();
      read(pimpl->get_buffer(), 1);
      //std::cerr << ", " << eof() << std::endl;
      if ( ! pimpl->parse_char(eof()) )
      {
         // Special case: reading past the EOF returns an empty ANY and
         // sets eof().
         if ( eof() && pimpl->is_error(XML_Error(XML_ERROR_NO_ELEMENTS)) )
            return true;

         EXCEPTION_MNGR(xmlserialstream_error,
                        "iXMLSerialStream::get_object(): parser threw error "
                        << pimpl->get_error());
      }
      if ( pimpl->endOfMainElement() )
      {
         // Create a new parser to read the next XML chunk by
         // re-initializing the input side of this SerialStream
         iXMLSerialStream::init_stream();
      }
   }
   so = pimpl->get_object();
   return true;
}


//------------------------------------------------------------------------

void
oXMLSerialStream::init_stream()
{
   oSerialStream::init_stream();

   // If we are actually *writing* a buffer (even if there is
   // pre-existing data) we need to start with the file marker
   stringstream s;
   s << "<" << xmlElementFile << " " 
     << xmlAttrVersion << "=\"" << std::hex << xmlFileVersion << "\" "
     << xmlAttrEndian << "=\"" 
     << static_cast<int>(Serialization_Manager::Endian) << "\">" 
     << endl;
   string str = s.str();
   write(str.c_str(), str.size());
}


void
oXMLSerialStream::finalize_stream()
{
   stringstream s;
   s << "</" << xmlElementFile << ">" << endl;
   string str = s.str();
   write(str.c_str(), str.size());
}


void
oXMLSerialStream::put_xml(const SerialObject &so, std::string indent)
{
   indent += (( m_format & XML::noindent ) ? "" : xmlIndent );
   size_t key = so.type;

   if (( m_format & XML::typeids ) && ( typeKeyWritten.count(key) == 0 ))
   {
      stringstream s;
      s << indent << "<" << xmlElementType << " " << xmlAttrTypeName 
        << "=\"" << XMLify(Serializer().get_username(so.type)) << "\" "
        << xmlAttrTypeId << "=\"" << so.type;
      if ( Serializer().is_pod(so.type) )
         s << "\" " << xmlAttrPodSize << "=\"" 
           << Serializer().get_pod_length(so.type);
      s << "\" />" << endl;

      string str = s.str();
      write(str.c_str(), str.size());

      // remember that we have writen this out...
      typeKeyWritten.insert(so.type);
   }


   stringstream type;
   if ( m_format & XML::typeids )
      type << xmlAttrTypeId << "=\"" << so.type;
   else
      type << xmlAttrTypeName << "=\"" 
           << XMLify(Serializer().get_username(so.type));

   stringstream s;
   s << indent << "<";
   if ( so.data.empty() )
   {
      s << xmlElementVoid << " " << type.str() << "\" />" << endl;
      string str = s.str();
      write(str.c_str(), str.size());
      return;
   }
   else if ( so.data.is_type(typeid(SerialPOD)) )
      s << xmlElementPod << " " << type.str();
   else
      s << xmlElementObject << " " << type.str();

   if ( so.data.is_type(typeid(SerialPOD)) )
   {
      string subIndent = indent+((m_format & XML::noindent) ? "" : xmlIndent);
      const SerialPOD& pod = so.data.expose<SerialPOD>();

      if ( pod.is_text_mode() )
      {
         //s << "\" format=\"text\">";
         s << "\">";
         if (( m_format & XML::noindent ) == 0 )
            s << endl << subIndent;
         s << XMLify(string(pod.data(), pod.size()));
         if (( m_format & XML::noindent ) == 0 )
            s << endl << indent;
         s << "</" << xmlElementPod << ">" << endl;
      }
      else
      {
         s << "\" " << xmlAttrFormat << "=\"" << xmlAttrValHex << "\">" 
           << std::hex << std::setfill('0');
         const unsigned char *c 
            = reinterpret_cast<const unsigned char*>(pod.data()) - 1;
         size_t mod = ( (m_format & XML::noindent) ? 1 : 25 );
         size_t pod_size = pod.size();
         for( size_t i = 1; i <= pod_size; ++i )
         {
            if ( i % mod == 1 )
               s << endl << subIndent;
            s << std::setw(2) << (int)*(c+i);
         }
         if ( mod > 1 )
            s << endl << indent;
         s << "</" << xmlElementPod << ">" << endl;
      }

      string str = s.str();
      write(str.c_str(), str.size());
   }
   else
   {
      s << "\">" << endl;
      string str = s.str();
      write(str.c_str(), str.size());

      // write out the Object data
      const SerialObject::elementList_t& objectList
         = so.data.expose<SerialObject::elementList_t>();
      SerialObject::elementList_t::const_iterator it = objectList.begin();
      SerialObject::elementList_t::const_iterator itEnd = objectList.end();
      for( ; it != itEnd; ++it )
         put_xml(*it, indent);

      s.str("");
      s << indent << "</" << xmlElementObject << ">" << endl;
      str = s.str();
      write(str.c_str(), str.size());
   }
}

//------------------------------------------------------------------------

void
XMLSerialStream::init_stream()
{
   // NB: i/o XMLSerialStream::init_stream() each call i/o
   // SerialStream::init_stream(), so we don't need to call this here.
   //ioSerialStream::init_stream(mode);

   // it is *critical* to initialize the output side FIRST so that if
   // this is a new file, the output side will create the binary header
   // before the input side reads it.
   oXMLSerialStream::init_stream();
   flush();
   iXMLSerialStream::init_stream();
}

//=======================================================================

} // namespace utilib
