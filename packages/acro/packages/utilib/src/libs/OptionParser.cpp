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

#include <utilib/OptionParser.h>
#include <utilib/TypeManager.h>
#include <utilib/sort.h>
#include <utilib/TinyXML_helper.h>
#include <utilib/Property.h>

#include <sstream>
#include <set>

using std::map;
using std::string;

namespace utilib
{

void OptionParser::merge_options(const OptionParser& options)
{
std::set<data_t>::const_iterator curr = options.parameter_data.begin();
std::set<data_t>::const_iterator end  = options.parameter_data.end();
while (curr != end) {
  add_parameter(*curr);
  curr++;
}
}


void OptionParser::categorize(const std::string& _name, const std::string& category)
{
   map<string, std::set<data_t> >::iterator curr = categories.find(category);
   if (curr == categories.end())
   {
      categories[category] = std::set<data_t>();
      curr = categories.find(category);
   }
   std::string name = standardize(_name);
   bool posix = name.size() == 1;
   data_t tmp = get_param_any(name.c_str(),posix);
   tmp().categories.insert(category);
   curr->second.insert(tmp);
}


void OptionParser::alias(const string& _name, const string& _alias)
   {
   std::string name = standardize(_name);
   std::string alias = standardize(_alias);
   bool posix = name.size() == 1;
   data_t tmp = get_param_any(name.c_str(),posix);

   if (alias.size() == 1)
      posix_parameters[alias[0]] = tmp;
   else
      parameters[alias] = tmp;
   tmp().aliases.insert(alias);
}


bool OptionParser::initialized(const std::string& _name)
{
   std::string name = standardize(_name);
   bool posix = name.size() == 1;
   data_t tmp = get_param_any(name.c_str(),posix);
   return tmp().initialized;
}


void OptionParser::disable(std::string _name)
{
   std::string name = standardize(_name);
   bool posix = name.size() == 1;
   data_t tmp = get_param_any(name.c_str(),posix,false);
   tmp().disabled = true;
}


void OptionParser::enable(std::string _name)
{
   std::string name = standardize(_name);
   bool posix = name.size() == 1;
   data_t tmp = get_param_any(name.c_str(),posix,false);
   tmp().disabled = false;
}


void OptionParser::remove(std::string _name)
{
   std::string name = standardize(_name);
   bool posix = name.size() == 1;
   data_t tmp = get_param_any(name.c_str(),posix);
   Parameter& param = tmp();

   if (param.name.size() == 1)
      posix_parameters.erase(param.short_name);
   else
      parameters.erase(param.name);
   parameter_data.erase(tmp);
   //
   // Remove category data
   //
   std::set<std::string>::iterator curr = param.categories.begin();
   std::set<std::string>::iterator end  = param.categories.end();
   while (curr != end) {
     categories[name].erase(tmp);
     curr++;
   }
}


void OptionParser::remove(const Parameter& param)
{
   if (param.short_name != 0)
   {
      map<char, data_t>::iterator s_it = posix_parameters.find(param.short_name);
      if (s_it != posix_parameters.end()) posix_parameters.erase(s_it);
      else EXCEPTION_MNGR(std::runtime_error, "Expected posix parameter " << param.short_name);
   }
   if (param.aliases.size() > 0)
   {
      std::set<std::string>::const_iterator curr = param.aliases.begin();
      std::set<std::string>::const_iterator end  = param.aliases.end();
      while (curr != end)
      {
         map<std::string, data_t>::iterator s_it = parameters.find(*curr);
         if (s_it != parameters.end()) parameters.erase(s_it);
         else EXCEPTION_MNGR(std::runtime_error, "Expected alias parameter " << *curr);
         curr++;
      }
   }
//
// Erase parameter with long_name
//
   {
      map<std::string, data_t>::iterator s_it = parameters.find(param.name);
      if (s_it != parameters.end()) parameters.erase(s_it);
      else EXCEPTION_MNGR(std::runtime_error, "Expected parameter " << param.name);
   }
//
// Erase parameter data
//
#if 0
   WEH - TODO
   {
      std::set<data_t>::iterator s_it = parameter_data.find(param.name);
      if (s_it != parameter_data.end()) parameter_data.erase(s_it);
      else EXCEPTION_MNGR(std::runtime_error, "Expected parameter " << param.name);
   }
#endif
}


#ifdef UTILIB_HAVE_TINYXML
void OptionParser::process_xml( TiXmlElement* node, bool describe )
{
   if ( describe )
   {
      TiXmlElement *opt = new TiXmlElement("Option");
      opt->SetAttribute("name", "string");
      node->LinkEndChild(opt);
      return;
   }

   TiXmlElement* n = node->FirstChildElement();
   for( ; n != NULL; n = n->NextSiblingElement() )
   {
      if ( n->ValueStr().compare("Option") != 0 )
         EXCEPTION_MNGR(std::runtime_error, "OptionParser:process_xml - invalid element "
                        << n->ValueStr() << " in " << get_element_info(n));
      //std::cerr << "Elment info: " << n->ValueStr() << " in " << get_element_info(n) << std::endl;
      string name = "";
      get_string_attribute(n, "name", name);
      const char* elt_data = n->GetText();
      if (elt_data != 0)
         set_parameter(name, elt_data);
      else
         set_parameter(name, "");
   }
}
#endif


void OptionParser::write(std::ostream& os, const std::set<std::string>& categories_requested, bool categorized) const
{
   //
   // Print usage
   //
   std::string indent = "                              ";
   {
      std::list<std::string>::const_iterator curr = usage.begin();
      std::list<std::string>::const_iterator end  = usage.end();
      while (curr != end)
      {
         std::string _usage = "Usage: ";
         _usage += *curr;
         wordwrap_printline(os, _usage, indent);
         curr++;
      }
      wordwrap_printline(os, description, "");
      os << std::endl;
   }
   if (categorized && (categories.size() > 0))
   {
      //
      // Print options, grouped by categories
      //
      std::vector<std::string> cat;
      cat.reserve(categories.size());
      {
         //
         // Get the names of the categories
         //
         std::map<std::string, std::set<data_t> >::const_iterator curr = categories.begin();
         std::map<std::string, std::set<data_t> >::const_iterator end  = categories.end();
         while (curr != end)
         {
            if ((categories_requested.size() == 0) || (categories_requested.find(curr->first) != categories_requested.end())) {
            if (curr->second.size() > 0)
               cat.push_back(curr->first);
            }
            curr++;
         }
      }
      sort(cat);
      {
         std::vector<std::string>::iterator curr = cat.begin();
         std::vector<std::string>::iterator end  = cat.end();
         while (curr != end)
         {
            os << " " << *curr << ":" << std::endl;
            std::map<std::string, std::set<data_t> >::const_iterator tmp = categories.find(*curr);
            write_parameter_set(os, tmp->second, indent);
            os << std::endl;
            curr++;
         }
      }
      if (categories_requested.size() == 0) {
         //
         // If no categories are requested, then print an 'uncategorized options' category
         //
         std::set<data_t> uncat;
         std::set<data_t>::const_iterator pcurr = parameter_data.begin();
         std::set<data_t>::const_iterator pend  = parameter_data.end();
         while (pcurr != pend) {
        const Parameter& param = (*pcurr)();
        if (param.categories.size() == 0)
            uncat.insert(*pcurr);
        pcurr++;
        }
            os << " uncategorized options:" << std::endl;
            write_parameter_set(os, uncat, indent);
            os << std::endl;
      }

   }
   else
   {
      // WEH - is this always an error?
      if (categories_requested.size() > 0)
            EXCEPTION_MNGR(std::runtime_error, "Although categories were specified for the output, the OptionParser object does not contain categorized options!");
      //
      // Print all options, without categories
      //
      os << "options:" << std::endl << std::endl;
      write_parameter_set(os, parameter_data, indent);
      os << std::endl;
   }
   if (arg_definitions.size() > 0)
   {
      os << "arguments:" << std::endl << std::endl;
      {
         std::list<std::pair<std::string, std::string> >::const_iterator curr = arg_definitions.begin();
         std::list<std::pair<std::string, std::string> >::const_iterator end  = arg_definitions.end();
         while (curr != end)
         {
            std::string tmp;
            tmp = "  ";
            tmp += curr->first;
            tmp += ":  ";
            tmp += curr->second;
            wordwrap_printline(os, tmp, "      ");
            os << std::endl;
            curr++;
         }
      }
   }

   if (epilog.size() > 0)
   {
      wordwrap_printline(os, epilog, "");
   }
}


void OptionParser::write_parameter_set(std::ostream& os, const std::set<data_t>& parameters, const string& indent) const
   {
      std::set<data_t>::const_iterator curr = parameters.begin();
      std::set<data_t>::const_iterator end  = parameters.end();
      while (curr != end)
      {
         const Parameter& param = (*curr)();
         std::ostringstream tmp;
         char sname = param.short_name;
         if (sname == (char)0)
            tmp << "    ";
         else
            tmp << "  -" << sname;
         if (param.name != "")
         {
            if (sname == (char)0)
               tmp << "  --" << param.name;
            else
               tmp << ", --" << param.name;
         }
         string str = tmp.str();
         int tmplen = static_cast<int>(str.size());
         if (tmplen < 30)
         {
            std::string line;
            line = str;
            for (int i = tmplen; i < 30; i++)
               line += " ";
            line += param.description;
            wordwrap_printline(os, line, indent);
         }
         else
         {
            os << str << std::endl << indent;
            wordwrap_printline(os, param.description, indent);
         }
         if (param.aliases.size() > 0) {
            std::string line(30,' ');
            line += "aliases:";
            std::set<std::string>::const_iterator acurr = param.aliases.begin();
            std::set<std::string>::const_iterator aend  = param.aliases.end();
            while (acurr != aend) {
            if (acurr->size() == 1)
                line += " -";
            else
                line += " --";
            line += *acurr;
            acurr++;
            }
            wordwrap_printline(os, line, indent);
         }
         curr++;
      }
   }


//
void OptionParser::write_xml(std::ostream& ) const
{
#ifdef UTILIB_HAVE_TINYXML

  /// TODO - how is this different from the write_values_xml() output?

#else

  EXCEPTION_MNGR(std::runtime_error, "Cannot print XML information unless UTILIB is configured with TinyXML");

#endif
}


//============================================================================
//
//
void OptionParser::write_values(std::ostream& os, const std::string& opt_label) const
{
   if (opt_label != "")
      os << "# ---- Options for " << opt_label << " ----" << std::endl;
//
// Get the length of the longest parameter name
//
   size_type len = 0;
   std::set<data_t>::const_iterator tcurr = parameter_data.begin();
   std::set<data_t>::const_iterator tlast = parameter_data.end();
   while (tcurr != tlast)
   {
      const Parameter& param = (*tcurr)();
      if (len < param.name.size())
         len = param.name.size();
      tcurr++;
   }
   if (len < 10) len = 10;
   char tformat[32];
#ifdef _MSC_VER
   sprintf_s(tformat, 32, "%%-%lds", (long int)len);
#else
   sprintf(tformat, "%%-%lds", (long int)len);
#endif
//
//
//
   std::set<data_t>::const_iterator curr = parameter_data.begin();
   std::set<data_t>::const_iterator last = parameter_data.end();

   char tmp[128];
   while (curr != last)
   {
      const Parameter& param = (*curr)();
      if (!(param.disabled))
      {
         if (param.name.size() > 0)
         {
#ifdef _MSC_VER
            sprintf_s(tmp, 128, tformat, param.name.data());
#else
            sprintf(tmp, tformat, param.name.data());
#endif
         }
         else
         {
            std::string tstr;
            tstr += param.short_name;
            tstr += "_option";
#ifdef _MSC_VER
            sprintf_s(tmp, 128, tformat, tstr.c_str());
#else
            sprintf(tmp, tformat, tstr.c_str());
#endif
         }
         os << tmp << " ";
         std::ostringstream ostr;
         ostr << param.info;
         const std::string& output = ostr.str();
         if (output.size() == 0)
            os << "\"\"";
         else if ( output.find(" ") != string::npos &&
                   ! (( output[0] == '"' ) && ( *output.rbegin() == '"' ) ||
                      ( output[0] == '[' ) && ( *output.rbegin() == ']' )))
            // Instead of explicitly testing for std::string and
            // CharString, we will simply look for data with a space
            // that is not already within quotes or look like a vector.
            os << "\"" << output << "\"";
         else
            os << output;
         if (!param.initialized)
            os << "\t# default" << std::endl;
         else
            os << std::endl;
      }
      curr++;
   }
}


#ifdef UTILIB_HAVE_TINYXML
void OptionParser::write_values_xml(std::ostream& os) const
{

  TiXmlElement* root = new TiXmlElement("Options");
  std::set<data_t>::const_iterator curr = parameter_data.begin();
  std::set<data_t>::const_iterator end  = parameter_data.end();
  while (curr != end) {
      const Parameter& param = (*curr)();
      if (!(param.disabled)) {
      TiXmlElement* option = new TiXmlElement("Option");
      if (param.name != "")
         option->SetAttribute("name",param.name);
      else {
         std::string tmp;
         tmp += param.short_name;
         option->SetAttribute("name",tmp);
         }
      int default_val = !(param.initialized);
      option->SetAttribute("default",default_val);
      //option->SetAttribute("type",param.info.type().name());
      std::ostringstream ostr;
      ostr << param.info;
      option->LinkEndChild(new TiXmlText(ostr.str()));
      root->LinkEndChild(option);
      }
    curr++;
  }

  os << *root;
  delete root;
}
#else
void OptionParser::write_values_xml(std::ostream& ) const
{
  EXCEPTION_MNGR(std::runtime_error, "Cannot print XML information unless UTILIB is configured with TinyXML");
}
#endif



//
// 1. Flag errors if an option appears after the first non-option?
// 2. Flag error when invalid value is given
//
OptionParser::args_t& OptionParser::parse_args(int argc, char* _argv[])
{
   std::vector<std::string> argv(_argv, _argv + argc);
   int argc_limit = argc - (int)min_num_required_args;
   //
   // Go through the argument list
   //
   int i = 1;
   while ((i < argc_limit) && (argv[i][0] == '-'))
   {
      //
      // Split the current string at the '=' character
      //
      char* tmp = const_cast<char*>(strchr(argv[i].c_str(), '='));
      bool using_equal = false;
      if (tmp)
      {
         using_equal = true;
         tmp++;
         *(tmp - 1) = '\000';
      }
      bool posix = argv[i][1] != '-';
      Parameter& param = get_param(argv[i].c_str(),posix);
      if (param.is_bool)
      {
         if (tmp != '\000')
            param.set_value_with_string(tmp);
         else
            param.set_value_with_string("");
      }
      else
      {
         if (!using_equal && required_equals)
            EXCEPTION_MNGR(std::runtime_error, "Nonboolean parameter '" << argv[i] << "' specified without required argument.  Option parsing configured to require --option=value syntax.");
         if (tmp != '\000')
            param.set_value_with_string(tmp);
         else
         {
            i++;
            if (i == argc_limit)
               EXCEPTION_MNGR(std::runtime_error, "Expected argument for parameter '" << argv[i-1] << "' but ran out of available arguments");
            if ((argv[i][0] == '-') && (argv[i].size() > 1) && (isalpha(argv[i][1])))
               EXCEPTION_MNGR(std::runtime_error, "Expected argument for parameter '" << argv[i-1] << "' but the next argument is an option.");
            param.set_value_with_string(argv[i]);
         }
      }
      i++;
   }
   //
   // Check to ensure that first 'required arg' does not look like a parameter
   //
   if ((i < argc) && (argv[i][0] == '-'))
   {
      std::string tmp = argv[i];
      if ((tmp == "--help") || (tmp == "--version"))
      {
         bool posix = argv[i][1] != '-';
         Parameter& param = get_param(argv[i].c_str(), posix);
         param.set_value_with_string("");
      }
      else if ((argv[i].size() > 1) && (isalpha(argv[i][1])))
         EXCEPTION_MNGR(runtime_error,
                        "OptionParser::parse_args "
                        "- first required argument looks "
                        "like a parameter flag: " << argv[i]);
   }
#if 0
   // WEH - This error checking seems useful, but it can get in the way.
   //       There are weird contexts where we should expect to find an option
   //       after a non-option.  For example, the command-line for an AMPL
   //       solver has the syntax: <solver> <nl-file> -AMPL
   int j = i + 1;
   while (j < argc)
   {
      if (argv[j][0] == '-')
         EXCEPTION_MNGR(runtime_error,
                        "OptionParser::parse_args "
                        "- argument '" << argv[j] << "' looks "
                        "like a parameter flag after all parameters have been parsed.  Argument '" << argv[i] << "' is the first argument.");
      j++;
   }
#endif

   //
   // Collect the remaining arguments
   //
   processed_args.push_back(argv[0]);
   while (i < argc)
   {
      processed_args.push_back(argv[i++]);
   }

   //
   // Return the list of arguments
   //
   return processed_args;
}


void OptionParser::set_parameter(std::string _name, const std::string& value)
{
   std::string name = standardize(_name);
   bool posix= name.size() == 1;
   Parameter& param = get_param(name.c_str(), posix);
   param.set_value_with_string(value);
}

void OptionParser::set_parameter(std::string _name, Any value)
{
   std::string name = standardize(_name);
   bool posix= name.size() == 1;
   Parameter& param = get_param(name.c_str(), posix);
   if ( param.info.is_type<Property>() )
      param.info.expose<Property>() = value;
   else if ( param.info.is_type<Privileged_Property>() )
      param.info.expose<Privileged_Property>() = value;
   else
      TypeManager()->lexical_cast(value, param.info);
}

int OptionParser::has_parameter(std::string _name)
{
   if ( _name.empty() )
      return 0;
   const char* c = _name.c_str();
   if ( c[0] == '-') c++;
   if ( c[0] == '-') c++;
   std::string name = standardize(c);
   
   // as with get_param_any(), we will assume all single-character parameters are POSIX.
   if ( name.size() == 1 )
   {
      std::map<char, data_t>::iterator curr = posix_parameters.find(name[0]);
      if ( curr == posix_parameters.end() )
         return 0;
      else
         return curr->second().disabled ? -1 : 1;
   }
   else
   {
      std::map<std::string, data_t>::iterator curr = parameters.find(name);
      if ( curr == parameters.end() )
         return 0;
      else
         return curr->second().disabled ? -1 : 1;
   }
}


Parameter& OptionParser::get_param(const char* name, bool posix)
{
data_t tmp = get_param_any(name,posix);
return tmp();
}


OptionParser::data_t OptionParser::get_param_any(const char* name, bool posix, bool test_if_enabled)
{
   if ((name == 0) || (name[0] == '\000'))
      EXCEPTION_MNGR(std::runtime_error, "OptionParser - "
                     "cannot access a parameter with an empty name.");
   if (name[0] == '-') name++;
   if (name[0] == '-') name++;

   std::string param = name;
   if (posix) {
      if (param.size() > 1)
         EXCEPTION_MNGR(std::runtime_error, "Multiple posix options cannot be specified at once.");
      std::map<char, data_t>::iterator curr = posix_parameters.find(param[0]);
      if (curr == posix_parameters.end())
         EXCEPTION_MNGR(std::runtime_error, "Unknown posix parameter '" << param << "'");
      if (curr->second().disabled && test_if_enabled)
         EXCEPTION_MNGR(std::runtime_error, "Parameter '" << param << "' is disabled.");
      return curr->second;
   }
   else
   {
      std::map<std::string, data_t>::iterator curr = parameters.find(param);
      if (curr == parameters.end())
         EXCEPTION_MNGR(std::runtime_error, "Unknown parameter '" << param << "'");
      if (curr->second().disabled && test_if_enabled)
         EXCEPTION_MNGR(std::runtime_error, "Parameter '" << param << "' is disabled.");
      return curr->second;
   }

}


void OptionParser::add_parameter(data_t any_param)
      {
      Parameter& param = any_param();
      //
      // Setup parameters, posix_parameters and parameter_data 
      //
      if (param.name != "")
         parameters[param.name] = any_param;
      if (param.short_name != 0)
      {
         posix_parameters[param.short_name] = any_param;
         if (param.name == "")
         {
            std::string tmp;
            tmp += "_" + param.short_name;;
            parameters[tmp] = any_param;
         }
      }
      parameter_data.insert(any_param);
      //
      // Insert aliases
      //
      {
      std::set<std::string>::iterator curr = param.aliases.begin();
      std::set<std::string>::iterator end  = param.aliases.end();
      while (curr != end) {
        if (curr->size() == 1)
           posix_parameters[(*curr)[0]] = any_param;
        else
           parameters[*curr] = any_param;
      curr++;
      }
      }
      //
      // Setup categories
      //
      {
      std::set<std::string>::iterator curr = param.categories.begin();
      std::set<std::string>::iterator end  = param.categories.end();
      while (curr != end) {
         if (param.name == "") {
            std::string tmp;
            tmp += param.short_name;
            categorize(tmp, *curr);
            }
         else
            categorize(param.name, *curr);
      curr++;
      }
   }
}

std::string OptionParser::standardize(const std::string& _name)
{
std::string name = _name;
std::string::iterator curr = name.begin();
std::string::iterator end  = name.end();
while (curr != end) {
  if (*curr == '_')
     *curr = '-';
  curr++;
  }
return name;
}

//template <>
//bool CachedAllocator<SmartPtrInfo<Parameter> >::cache_enabled = false;

}
