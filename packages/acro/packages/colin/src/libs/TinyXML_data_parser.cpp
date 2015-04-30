/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#include<colin/TinyXML_data_parser.h>
#include<colin/real.h>

#include<utilib/TinyXML_helper.h>
#include<utilib/TypeManager.h>
#include<utilib/MixedIntVars.h>

using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;

using utilib::Any;
using utilib::MixedIntVars;
using utilib::TypeManager;

using colin::real;

namespace colin {

namespace {

template<typename T>
Any parse(stringstream &ss)
{
   Any ans;
   T& data = ans.set<T>();
   ss >> data;
   if ( ! ss.eof() )
      ss >> std::ws;
   return ss.fail() ? Any() : ans;
}

template<typename T>
Any parse_vector(char type, stringstream &ss)
{
   Any ans;
   vector<T>& data = ans.set<vector<T> >();

   char c = 0;
   int len = -1;
   // this checks for the formal "r(#: x, x, x)" format
   if ( ss.peek() == type )
   {
      ss >> c >> std::ws >> c >> std::ws;
      if (( c != '(' ) || ( ss.fail() ))
         return Any();
      ss >> len >> std::ws;
      if (( len < 0 ) || ( ss.fail() ))
         return Any();
      data.reserve(len);
      ss >> c >> std::ws;
      if (( c != ':' ) || ( ss.fail() ))
         return Any();
   }
   
   // parse the vector data
   T item;
   while ( ss.good() )
   {
      if ( len == 0 )
      {
         ss >> c;
         if ( ! ss.eof() )
            ss >> std::ws;
         if (( c != ')' ) || ss.fail())
            return Any();
         else
            return ans;
      }

      ss >> item >> std::ws;
      if ( ss.fail() )
         return Any();
      if ( !ss.eof() && ( ss.peek() == ',' ))
         ss >> c >> std::ws;
      
      data.push_back(item);
      if ( len > 0 )
         --len;
   }

   return ss.fail() ? Any() : ans;
}

template<typename T>
Any parse_matrix(stringstream &ss)
{
   static_cast<void>(ss);
   EXCEPTION_MNGR(std::logic_error, "parse_matrix not implemented.");
   return Any();
}

Any parse_mixed_int(stringstream &ss)
{
   Any ans;
   MixedIntVars& data = ans.set<MixedIntVars>();

   while ( ss.good() )
   {
      char c = ss.peek();
      if ( c == 'r' )
      {
         Any tmp = parse_vector<real>('r', ss);
         if ( tmp.empty() ) 
            return tmp;
         TypeManager()->lexical_cast(tmp, data.Real());
      }
      else if ( c == 'i' )
      {
         Any tmp = parse_vector<int>('i', ss);
         if ( tmp.empty() ) 
            return tmp;
         TypeManager()->lexical_cast(tmp, data.Integer());
      }
      else if ( c == 'b' )
      {
         Any tmp = parse_vector<bool>('b', ss);
         if ( tmp.empty() ) 
            return tmp;
         TypeManager()->lexical_cast(tmp, data.Binary());
      }
      else 
         return Any();
   }

   return ss.fail() ? Any() : ans;
}


Any Parsers(string type, const string& data)
{ 
   if ( type.compare("string") == 0 )
      return data;

   stringstream ss(data);
   ss >> std::ws;
   Any ans;

   if (( type.compare("real") == 0 ) || 
       ( type.compare("double") == 0 ))
      ans = parse<double>(ss);
   else if (( type.compare("int") == 0 ) || 
            ( type.compare("integer") == 0 ))
      ans = parse<int>(ss);
   else if ( type.compare("bool") == 0 )
      ans = parse<bool>(ss);
   else if ( type.compare("MixedInt") == 0 )
      ans = parse_mixed_int(ss);
   else if ( type.find("vector<") == 0 )
   {
      if (( type.compare(6, string::npos, "<real>") == 0 ) || 
          ( type.compare(6, string::npos, "<double>") == 0 ))
         ans = parse_vector<double>('r', ss);
      else if (( type.compare(6, string::npos, "<int>") == 0 ) || 
               ( type.compare(6, string::npos, "<integer>") == 0 ))
         ans = parse_vector<int>('i', ss);
      else if ( type.compare(6, string::npos, "<bool>") == 0 )
         ans = parse_vector<bool>('b', ss);
   }
   else if ( type.find("matrix<") == 0 )
   {
      if (( type.compare(6, string::npos, "<real>") == 0 ) || 
          ( type.compare(6, string::npos, "<double>") == 0 ))
         ans = parse_matrix<double>(ss);
      else if (( type.compare(6, string::npos, "<int>") == 0 ) || 
               ( type.compare(6, string::npos, "<integer>") == 0 ))
         ans = parse_matrix<int>(ss);
      else if ( type.compare(6, string::npos, "<bool>") == 0 )
         ans = parse_matrix<bool>(ss);
   }
   else
      EXCEPTION_MNGR(std::runtime_error, "Parsers(): "
                     "unknown data type, '"  << type << "'.");


#if 0
   if ( ans.empty() )
      EXCEPTION_MNGR( std::runtime_error, "Parsers(): error encountered "
                      "parsing data: no value returned" );
   else if ( ss.fail() )
   {
      ans.clear();
      EXCEPTION_MNGR( std::runtime_error, "Parsers(): error encountered "
                      "parsing data: data stream failure." );
   }
   else if ( ! ss.eof() )
   {
      ans.clear();
      EXCEPTION_MNGR( std::runtime_error, "Parsers(): error encountered "
                      "parsing data: unparsed data remaining in stream." );
   }
#endif
   // If the parser failed, return the raw string
   if ( ans.empty() || ss.fail() || ! ss.eof() )
      return data;
   return ans;
}

} // namespace colin::(local)


utilib::Any
parse_xml_data(TiXmlElement* elt, std::string datatype)
{
   Any ans;
   if ( elt == NULL )
      return ans;

   string data;
   if ( ! utilib::get_string_attribute(elt, "value", data, "") )
      data = utilib::get_element_text(elt);

   // Is someone telling us what to expect?
   if ( datatype.empty() )
      utilib::get_string_attribute(elt, "type", datatype, "");

   try {
      ans = parse_data(data, datatype);
   } catch ( std::exception& e ) {
      EXCEPTION_MNGR(std::runtime_error, "parse_xml_data(): "
                     "Error encountered  at " << utilib::get_element_info(elt)
                     << ":" << endl << e.what() );
   }
   return ans;
}


utilib::Any
parse_data(const std::string &data, std::string datatype)
{
   if ( ! datatype.empty() )
      return Parsers(datatype, data);

   // OK - we need to "guess" the data type to parse.  Right now we will
   // assume that the element will always have only a single child
   // element: the text.

   // 1) look for marked vectors
   int vtype_r = ( data.find("r(") == string::npos ) ? 0 : 1;
   int vtype_i = ( data.find("i(") == string::npos ) ? 0 : 1;
   int vtype_b = ( data.find("b(") == string::npos ) ? 0 : 1;

   if ( vtype_r + vtype_i + vtype_b > 1 )
      return Parsers("MixedInt", data);
   else if ( vtype_r )
      return Parsers("vector<real>", data);
   else if ( vtype_i )
      return Parsers("vector<int>", data);
   else if ( vtype_b )
      return Parsers("vector<bool>", data);

   // 2) guess the core data type
   if ( data.find_first_of(".eE") != string::npos )
      datatype = "real";
   else if ( data.find_first_not_of(" 01,\n") == string::npos )
   {
      if ( data.find("00") == string::npos &&
           data.find("10") == string::npos &&
           data.find("11") == string::npos &&
           data.find("01") == string::npos )
         datatype = "bool";
      else
         datatype = "int";
   }
   else if ( data.find_first_not_of(" 0123456789,\n") == string::npos )
      datatype = "int";
   else
      return data; // string

   // 3) is this a matrix / vector?
   if ( data.find('\n') != string::npos )
      return Parsers(string("matrix<") + datatype + ">", data);
   if ( data.find_first_of(" ,") != string::npos )
      return Parsers(string("vector<") + datatype + ">", data);
   else
      return Parsers(datatype, data);
}


} // namespace colin
