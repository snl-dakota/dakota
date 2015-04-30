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
 * \file Serialize.cpp
 */

#include <utilib/Serialize.h>
#include <utilib/exception_mngr.h>

using std::cerr;
using std::endl;

using std::map;
using std::pair;
using std::vector;
using std::string;
using std::type_info;


namespace utilib {

namespace { // local namespace for endian detection

const char template_separator = ';';

const int int_bytes = (( 0 << ( 8 * (3 % sizeof(int))) ) + 
                       ( 1 << ( 8 * (2 % sizeof(int))) ) + 
                       ( 2 << ( 8 * (1 % sizeof(int))) ) + 
                       ( 3 << ( 8 * (0 % sizeof(int))) ));

} // namespace utilib::(local)

/** Detect the endian-ness of this platform.
 *
 *  This code should be safe for compilers with both 2-byte and 4-byte
 *  ints.  Common values are:
 *     - e4 (11 10 01 00) = 4-byte little-endian (Intel)
 *     - 1b (00 01 10 11) = 4-byte big-endian
 *     - 22 (00 10 00 10) = 2-byte little endian
 *     - 88 (10 00 10 00) = 2-byte big endian
 */
const unsigned char Serialization_Manager::Endian = 
   (( (3 & reinterpret_cast<const char*>(&int_bytes)[0 % sizeof(int)]) << 6 ) +
    ( (3 & reinterpret_cast<const char*>(&int_bytes)[1 % sizeof(int)]) << 4 ) +
    ( (3 & reinterpret_cast<const char*>(&int_bytes)[2 % sizeof(int)]) << 2 ) +
    ( (3 & reinterpret_cast<const char*>(&int_bytes)[3 % sizeof(int)]) ));


/// local namespace for default POD serialization functions
namespace POD_serializers {  

template<typename T>
int POD_serializer(SerialPOD& serial, Any& data, bool serialize)
{
   if ( serialize )
      serial.set(&data.template expose<T>(), sizeof(T));
   else
   {
      if ( serial.size() != sizeof(T) )
         EXCEPTION_MNGR(serializer_bad_pod_size, "POD_serializer(): "
                        "SerialPOD data size does not match destination type");

      memcpy( &const_cast<T&>(data.template expose<T>()), 
	      serial.data(), serial.size() );
   }
   return 0;
}

template<>
int POD_serializer<std::string>(SerialPOD& serial, Any& data, bool serialize)
{
   const std::string &str = data.expose<std::string>();
   if ( serialize )
      serial.set(str.c_str(), str.size());
   else
      data.set(std::string(serial.data(), serial.size()));
   return 0;
}


template<typename T>
int POD_text_serializer(std::string& serial, Any& data, bool serialize)
{
   std::stringstream s;
   if ( serialize )
   {
      s << data.template expose<T>();
      serial = s.str();
   }
   else
   {
      s.str(serial);
      s >> const_cast<T&>(data.template expose<T>()) >> std::ws;
   }

   if ( s.bad() || s.fail() )
      return error::Serialization::BadPODTextConversion;
   else if ( ! serialize )
      return s.eof() ? 0 : error::Serialization::UnconvertedPODText;
   else
      return 0;
}

template<>
int POD_text_serializer<float>(std::string& serial, Any& data, bool serialize)
{
   std::stringstream s;
   if ( serialize )
   {
      s.setf(std::ios::floatfield);
      s.precision(FLT_DIG+2);
      s << data.expose<float>();
      serial = s.str();
   }
   else
   {
      s.str(serial);
      s >> const_cast<float&>(data.expose<float>()) >> std::ws;
   }

   if ( s.bad() || s.fail() )
      return error::Serialization::BadPODTextConversion;
   else if ( ! serialize )
      return s.eof() ? 0 : error::Serialization::UnconvertedPODText;
   else
      return 0;
}

template<>
int POD_text_serializer<double>(std::string& serial, Any& data, bool serialize)
{
   std::stringstream s;
   if ( serialize )
   {
      s.setf(std::ios::floatfield);
      s.precision(DBL_DIG+2);
      s << data.expose<double>();
      serial = s.str();
   }
   else
   {
      s.str(serial);
      s >> const_cast<double&>(data.expose<double>()) >> std::ws;
   }

   if ( s.bad() || s.fail() )
      return error::Serialization::BadPODTextConversion;
   else if ( ! serialize )
      return s.eof() ? 0 : error::Serialization::UnconvertedPODText;
   else
      return 0;
}

template<>
int POD_text_serializer<long double>( std::string& serial, Any& data, 
                                      bool serialize )
{
   std::stringstream s;
   if ( serialize )
   {
      s.setf(std::ios::floatfield);
      s.precision(LDBL_DIG+2);
      s << data.expose<long double>();
      serial = s.str();
   }
   else
   {
      s.str(serial);
      s >> const_cast<long double&>(data.expose<long double>()) >> std::ws;
   }

   if ( s.bad() || s.fail() )
      return error::Serialization::BadPODTextConversion;
   else if ( ! serialize )
      return s.eof() ? 0 : error::Serialization::UnconvertedPODText;
   else
      return 0;
}

template<>
int POD_text_serializer<std::string>( std::string& serial, Any& data, 
                                      bool serialize )
{
   if ( serialize )
      serial = "\"" + data.expose<std::string>() + "\"";
   else
   {
      string::iterator it = serial.begin();
      string::iterator itEnd = serial.end();
      // test for empty string or no leading "
      if (( it == itEnd ) || ( *it != '"' ))
         return error::Serialization::MissingStringQuote;
      ++it;
      
      // test for string.size() == 1
      if ( it == itEnd )
         return error::Serialization::MissingStringQuote;
      --itEnd;
      if ( *itEnd != '"' )
         return error::Serialization::MissingStringQuote;

      data.set(std::string(it, itEnd));
   }
   return 0;
}

template<>
int POD_text_serializer<char>( std::string& serial, Any& data, 
                               bool serialize )
{
   std::stringstream s;
   if ( serialize )
   {
      char c = data.expose<char>();
      if ( c >= 32 && c <= 126 )
      {
         serial = "\'x\'";
         serial[1] = c;
      }
      else
      {
         s << static_cast<int>(c);
         serial = s.str();
      }
   }
   else
   {
      if ( serial.empty() )
         return error::Serialization::BadPODData;
      if ( serial[0] == '\'' )
      {
         if ( serial.size() != 3 || serial[2] != '\'' )
            return error::Serialization::BadPODData;
         data.set<char>(serial[1]);
      }
      else
      {
         s.str(serial);
         int i = CHAR_MAX + 1;
         s >> i;
         if (( i < CHAR_MIN ) || ( i > CHAR_MAX ))
            return error::Serialization::BadPODTextConversion;
         const_cast<char&>(data.expose<char>()) = static_cast<char>(i);
      }
      s >> std::ws;
   }

   if ( s.bad() || s.fail() )
      return error::Serialization::BadPODTextConversion;
   else if ( ! serialize )
      return s.eof() ? 0 : error::Serialization::UnconvertedPODText;
   else
      return 0;
}

#ifndef QUOTE
#  define QUOTE(x) _QUOTE(x)
#  define _QUOTE(x) #x
#endif

#define REGISTER_POD(TYPE)                                              \
   register_serializer<TYPE >( QUOTE(TYPE),                             \
                               POD_serializers::POD_serializer<TYPE >,  \
                               POD_serializers::POD_text_serializer<TYPE > )


} // namespace utilib::POD_serializers


//--------------------------------------------------------------------
// Serializer singleton management
//
Serialization_Manager& Serializer()
{
   static Serialization_Manager master_serializer;
   return master_serializer;
}


//--------------------------------------------------------------------
// SerialPOD members
//
void SerialPOD::set(const void* buffer_src, const size_t length)
{
   buffer.resize(length);
   memcpy(&buffer[0], buffer_src, length);
   text_mode = false;
}

void SerialPOD::set(const std::string& buffer_src)
{
   size_t size = buffer_src.size();
   buffer.resize(size);
   memcpy(&buffer[0], buffer_src.c_str(), size);
   text_mode = true;
}

void SerialPOD::print(std::ostream& os, std::string indent) const
{
   if ( text_mode )
      os << ": ";
   else
      os << endl << indent << "POD: " << size() << ":";
   for(size_t i = 0; i < size(); ++i)
      if ( text_mode )
         os << buffer[i];
      else
         os << " " << static_cast<int>(static_cast<unsigned char>(buffer[i]));
   os << endl;
}


//--------------------------------------------------------------------
// SerialObject members
//
void SerialObject::print(std::ostream& os, std::string indent) const
{
   os << indent << "type = " << Serializer().get_username(type);
   if ( data.is_type(typeid(SerialObject::elementList_t)) )
   {
      os << endl;
      SerialObject::elementList_t::const_iterator it = 
         data.expose<SerialObject::elementList_t>().begin();
      SerialObject::elementList_t::const_iterator itEnd = 
         data.expose<SerialObject::elementList_t>().end();
      for ( ; it != itEnd; ++it )
         it->print(os, indent + "   ");
   }
   else if ( data.is_type(typeid(SerialPOD)) )
      data.expose<SerialPOD>().print(os, indent + "   ");
   else
      os << endl << indent << "   UNKNOWN DATA! (" 
         << demangledName(data.type()) << ")" << endl;
}


//--------------------------------------------------------------------
// utilib namespace functions
//

int serial_transform(SerialObject::elementList_t& serial, 
                     Any& data,  bool serialize)
{ 
   AnyFixedRef tmp = data;
   int ans = Serializer().transform_impl
      (data.empty() ? typeid(void) : data.type(), serial, tmp, serialize);
   if ( data.empty() || tmp.empty() )
      data = tmp;
   return ans;
}

Any Deserialize(SerialObject &serial)
   { 
   SerialObject::elementList_t tmp(1, serial);
   AnyFixedRef data;
   int ans = Serializer().transform_impl(typeid(void), tmp, data, false);
   if ( ans != 0 )
   {
      EXCEPTION_MNGR(serialization_error, 
                     "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '"
                     << data.type().name() << "' (Error " << ans << ")");
   }
   return data;
}

//--------------------------------------------------------------------
// Serialization_Manager members
//

Serialization_Manager::Serialization_Manager()
   : typename_map(),
     username_map(),
     type_map(),
     functions(),
     rebuild_usernames(true),
     serialize_pod_as_text(false)
{
   // Reserve some space for the standard transformations
   functions.reserve(19);

   // NB: this must be first (so that void gets id 0)
   register_serializer(typeid(void), "void", 0, NULL, NULL, NULL, NULL);

   // Several of these registrations may be duplicates - BUT, since it's
   // all in the same cpp file, the duplicates should be silently
   // ignored.

   // NB: according to the C++ standard, the signedness of char is
   // implementation-specific.  As a result, char, signed char, and
   // unsigned char are distinct types.  This means that the 'signed
   // char' registration is explicitly required (or else signed chars
   // will be silently cast into ints!)
   REGISTER_POD(char);
   REGISTER_POD(signed char);
   REGISTER_POD(unsigned char);

   REGISTER_POD(short);
   REGISTER_POD(signed short);
   REGISTER_POD(unsigned short);

   REGISTER_POD(int);
   REGISTER_POD(signed int);
   REGISTER_POD(unsigned int);

   REGISTER_POD(long);
   REGISTER_POD(signed long);
   REGISTER_POD(unsigned long);

   REGISTER_POD(float);
   REGISTER_POD(double);
   REGISTER_POD(long double);

   REGISTER_POD(bool);

   // manually register wchar_t so the text string transform defaults to binary
   register_serializer<wchar_t>
      ( "wchar_t", POD_serializers::POD_serializer<wchar_t> );
   // manually register strings so the name isn't "std::basic_string<*,*>"
   register_serializer<std::string>
      ( "std::string", 
        POD_serializers::POD_serializer<std::string>,
        POD_serializers::POD_text_serializer<std::string>, -1 );
}


Serialization_Manager::~Serialization_Manager()
{}


const std::type_info* 
Serialization_Manager::get_typeinfo(size_t key)
{
   if ( key >= functions.size() )
      return NULL;
   else
      return functions[key].typeinfo;
}


std::string
Serialization_Manager::get_username(size_t key)
{
   if ( key >= functions.size() )
      return "";
   else
   {
      if ( rebuild_usernames )
         rebuild_user_name_map();
      username_map_t::iterator it = functions[key].username;
      return ( it == username_map.end() ? "" : it->first );
   }
}


bool
Serialization_Manager::is_pod(size_t key)
{
   if ( key >= functions.size() )
      return false;
   else
      return functions[key].pod_transform != NULL;
}


int
Serialization_Manager::get_pod_length(size_t key)
{
   if ( key >= functions.size() )
      return -1;
   else
      return functions[key].pod_size;
}


size_t
Serialization_Manager::get_keyid(std::string user_name)
{
   if ( rebuild_usernames )
      rebuild_user_name_map();

   username_map_t::iterator u_it = username_map.find(user_name);
   if ( u_it == username_map.end() )
      EXCEPTION_MNGR(serializer_unknown_type,
                     "Serialization_Manager::get_keyid(): "
                     "unknown user-defined type name, \""
                     << user_name << "\"");
   if ( u_it->second == typename_map.end() )
      EXCEPTION_MNGR(serializer_unknown_type,
                     "Serialization_Manager::get_keyid(): "
                     "user-defined type name, \""
                     << user_name << "\" maps to multiple types");

   return u_it->second->second;
}

size_t
Serialization_Manager::get_keyid(const std::type_info &type)
{
   type_map_t::iterator t_it = type_map.find(&type);
   if ( t_it == type_map.end() )
   {
      typename_map_t::iterator n_it = typename_map.find(mangledName(type));
      if ( n_it == typename_map.end() )
         EXCEPTION_MNGR(serializer_unknown_type,
                        "Serialization_Manager::get_keyid(): "
                        "unknown type_info name, \""
                        << type.name() << "\"");
      return n_it->second;
   }

   return t_it->second;
}


int
Serialization_Manager::transform_impl( const std::type_info& type, 
                                       SerialObject::elementList_t& serial,
                                       AnyFixedRef& data, 
                                       bool serialize )
{
   if ( rebuild_usernames )
      rebuild_user_name_map();

   type_map_t::iterator it = type_map.end();

   if ( serialize )
   {
      if ( data.empty() )
      {
         // Serialize empty Anys as VOID
         serial.push_back(SerialObject());
         return 0;
      }

      // find this data type...
      it = type_map.find(&type);
      if ( it == type_map.end() )
      {
         // check to see if the mangled typename exists...
         typename_map_t::iterator name_it 
            = typename_map.find(mangledName(type));
         if ( name_it == typename_map.end() )
            EXCEPTION_MNGR(serializer_unknown_type,
                           "Serialization_Manager::transform_impl(): "
                           "cannot serialize unknown type '"
                           << type.name() << "'");

         // insert reference to this typeid for future use
         it = type_map.insert(type_pair_t(&type,name_it->second)).first;
      }

      MappingFunctions &fcns = functions[it->second];

      // Check for a username conflict
      if ( fcns.username == username_map.end() )
         EXCEPTION_MNGR(serialization_error, 
                        "Serialization_Manager::transform_impl(): attempt "
                        "to serialize an object with a nonunique username.");

      // Prep the serialization container.
      serial.push_back(SerialObject(it->second));

      int ans = 0;
      // serialize the object
      if ( fcns.transform != NULL )
         ans = fcns.transform
            ( serial.back().data.set<SerialObject::elementList_t>(), 
              data, serialize );
      else if ( fcns.pod_transform != NULL )
      {
         if (( serialize_pod_as_text ) && ( fcns.pod_txt_xform != NULL ))
         {
            string buf;
            ans = fcns.pod_txt_xform( buf, data, serialize );
            serial.back().data.set<SerialPOD>().set(buf);
         }
         else
            ans = fcns.pod_transform
               ( serial.back().data.set<SerialPOD>(), data, serialize );
      }
      else
         EXCEPTION_MNGR(serialization_error, 
                        "Serialization_Manager::transform_impl(): "
                        "NULL serialization function for type '" 
                        << type.name() << "'");
      return ans;
   }
   else
   {
      if ( serial.empty() )
         EXCEPTION_MNGR(serialization_error, 
                        "Serialization_Manager::transform_impl(): "
                        "SerialObject missing required element.");

      SerialObject& so = serial.front();
      if (so.type == 0 )
      {
         // Deserialize VOID
         data.clear();
         return 0;
      }
      if ( so.type >= functions.size() )
         EXCEPTION_MNGR(serializer_unknown_type, 
                        "Serialization_Manager::transform_impl(): "
                        "SerialObject contains unknown type id.");

      MappingFunctions &fcns = functions[so.type];
      // Check for a username conflict
      if ( fcns.username == username_map.end() )
         EXCEPTION_MNGR(serialization_error, 
                        "Serialization_Manager::transform_impl(): attempt "
                        "to deserialize an object with a nonunique username.");

      // Initialize the result data
      if ( fcns.init == NULL )
         EXCEPTION_MNGR(serialization_error, 
                        "Serialization_Manager::transform_impl(): "
                        "NULL initialization function for id " << so.type);
      fcns.init(data);
      if ( data.empty() )
      {
         cerr << "WARNING: Serialization_Manager::transform_impl(): "
            "initialization function for " << type.name() 
              << " resulted in an empty Any." << endl
              << "         This will likely cause random segmentation "
            "faults." << endl;
      }

      // deserialize the object
      int ans = 0;
      if ( fcns.transform != NULL )
         ans = fcns.transform
            ( const_cast<SerialObject::elementList_t&>
              (so.data.expose<SerialObject::elementList_t>()), 
              data, serialize );
      else if ( fcns.pod_transform != NULL )
      {
         SerialPOD& pod = const_cast<SerialPOD&>(so.data.expose<SerialPOD>());
         if ( pod.is_text_mode() )
         {
            string buf = string(pod.data(), pod.size());
            assert(fcns.pod_txt_xform != NULL);
            ans = fcns.pod_txt_xform( buf, data, serialize );
         }
         else
            ans = fcns.pod_transform( pod, data, serialize );
      }
      else
         EXCEPTION_MNGR(serialization_error, 
                        "Serialization_Manager::transform_impl(): "
                        "NULL deserialization function for id " << so.type);

      serial.pop_front();
      return ans;
   }

   // Cannot get here...
}


void
Serialization_Manager::list_serializers(std::ostream& os)
{
   if ( rebuild_usernames )
      rebuild_user_name_map(); 

   os << "Known serializers:" << endl;
   username_map_t::iterator it = username_map.begin();
   username_map_t::iterator itEnd = username_map.end();
   size_t max = 0;
   for ( ; it != itEnd; ++it )
      max = ( max < it->first.size() ? it->first.size() : max );

   for (it = username_map.begin() ; it != itEnd; ++it )
      os << "   " << std::left << std::setw(max) << it->first << "   [ "
         << ( it->second == typename_map.end() 
              ? "CONFLICT" : it->second->first )
         << " ]" << endl;
}


int
Serialization_Manager::register_serializer( const std::type_info& type, 
                                            std::string name, 
                                            int pod_size,
                                            transform_fcn t_fcn, 
                                            pod_transform_fcn p_fcn,
                                            pod_text_transform_fcn pt_fcn,
                                            initialization_fcn i_fcn)
{
   pair<typename_map_t::iterator,bool> ans = typename_map.insert
      (typename_pair_t(mangledName(type), functions.size()));
   if ( ans.second )
   {
      // New registration
      type_map[&type] = functions.size();

      functions.push_back(MappingFunctions());
      MappingFunctions &m = functions.back();
      m.init = i_fcn;
      m.transform = t_fcn;
      m.pod_transform = p_fcn;
      m.pod_txt_xform = pt_fcn;
      m.typeinfo = &type;
      m.raw_user_name = name;
      m.pod_size = pod_size;
      m.username = username_map.end();
      rebuild_usernames = true;
      return 0;
   }
   else
   {
      MappingFunctions &original = functions[ans.first->second];
      // Should we just check the user-defined name?
      if ( ( original.init == i_fcn ) && 
           ( original.transform == t_fcn ) &&
           ( original.pod_transform == p_fcn ) &&
           ( original.pod_txt_xform == pt_fcn ))
      {
         // A duplicate registration pointing to the same functions...
         return 0;
      }
      cerr << "WARNING: Serialization_Manager::register_serializer(): "
         " discarding duplicate registration for '" << mangledName(type) << 
         "'" << endl;
      return error::Serialization::DuplicateRegistration;
   }
}


void
Serialization_Manager::rebuild_user_name_map()
{
   // clear out everything
   username_map.clear();
   username_map_t::iterator noUserName = username_map.end();

   vector<MappingFunctions>::iterator it = functions.begin();
   vector<MappingFunctions>::iterator itEnd = functions.end();
   for( ; it != itEnd; ++it )
      it->username = noUserName;

   // rebuild everything
   typename_map_t::iterator t_it = typename_map.begin();
   typename_map_t::iterator t_itEnd = typename_map.end();
   for( ; t_it != t_itEnd; ++t_it )
      resolve_user_name(t_it->first);

   rebuild_usernames = false;
}


std::string
Serialization_Manager::resolve_user_name(std::string mangled)
{
   typename_map_t::iterator mangled_it = typename_map.find(mangled);
   // unknown mangled type name...
   if ( mangled_it == typename_map.end() )
      return "*";

   MappingFunctions &m = functions[mangled_it->second];

   // this user name has already been fully resolved
   if ( m.username != username_map.end() )
      return m.username->first;

   // we must resolve the name
   string uname = "";
   size_t sep = m.raw_user_name.find(template_separator);
   uname = m.raw_user_name.substr(0, sep);
   if ( sep != string::npos )
   {
      // This uname is a template (defined in terms of mangled type names)
      uname += "<";
      while ( sep != string::npos )
      {
         ++sep;
         size_t next_sep = m.raw_user_name.find(template_separator, sep);
         uname += resolve_user_name(m.raw_user_name.substr(sep, 
                                                           next_sep-sep));
         uname += ',';
         sep = next_sep;
      }
      *uname.rbegin() = '>';
   }

   // insert the newly-resolved name...
   pair<username_map_t::iterator, bool> ans 
      = username_map.insert(username_pair_t(uname, mangled_it));
   if ( ans.second )
      m.username = ans.first;
   else
   {
      cerr << "WARNING: Serialization_Manager::resolve_user_name(): \n"
"     Multiple mangled type names map to the same user-defined name.  It is\n"
"     likely that you forgot to register a name or a serialization function\n"
"     for a template argument.  If you attempt to serialize either type,\n"
"     you will get an exception.\n"
"  User name: " << uname << endl
            << "  Mangled: " << mangled << endl 
            << "           " << username_map[uname]->first << endl;

      m.username = username_map.end();
      if ( ans.first->second != typename_map.end() )
      {
         functions[ans.first->second->second].username = username_map.end();
         ans.first->second = typename_map.end();
      }
   }

   return uname;
}


} // namespace utilib
