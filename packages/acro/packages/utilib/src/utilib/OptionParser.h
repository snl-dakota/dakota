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

#ifndef utilib_OptionParser_h
#define utilib_OptionParser_h

#include <utilib/string_ops.h>
#include <utilib/Parameter.h>
#include <utilib/SmartHandle.h>

#ifdef UTILIB_HAVE_TINYXML
#include <tinyxml/tinyxml.h>
#endif
#include <vector>
#include <utility>
#include <string>

namespace utilib
{

///
/// This is a simple option parser that is based on the UTILI ParameterSet
/// class.  This provides a simpler API for setting up command-line options.
///
class OptionParser
{
public:

   ///
   typedef SmartHandle<Parameter> data_t;

   ///
   typedef std::vector<std::string> args_t;

   /// Constructor
   OptionParser(const char* usage_ = 0, const char* description_ = 0)
   {
      _help_option = false;
      _version_option = false;
      min_num_required_args = 0;
      required_equals = false;

      if (usage_ != 0)
         add_usage(usage_);
      if (description_ != 0)
         description = description_;
      add('h', "help", _help_option, "Display usage information");
   }

   /// Merge options from an OptionParser object into the current
   /// OptionParser.  This merge ignores arguments, the usage, etc.
   void merge_options(const OptionParser& options);
    
   ///
   unsigned int min_num_required_args;

   ///
   bool required_equals;

   ///
   std::string description;

   ///
   std::string epilog;

   ///
   void add_usage(const std::string& usage_)
    {
    usage.push_back(usage_);
    }

   ///
   void version(const std::string& _version)
    {
    std::map<std::string,data_t>::iterator curr = parameters.find(_version);
    if (curr == parameters.end())
       add("version", _version_option, "Display version information");
    _version_text = _version;
    }

   // - Add standard parameters

   ///
   template <class T>
   void add(const char& short_name, const std::string& long_name, T& data, const std::string& description_="", Parameter::action_t action = Parameter::store_value)
   { add_parameter(short_name, long_name, data, description_, action); }

   ///
   template <class T>
   void add(const char& short_name, T& data, const std::string& description_="", Parameter::action_t action = Parameter::store_value)
   {
      std::string tmp = "";
      add_parameter(short_name, tmp, data, description_, action);
   }

   ///
   template <class T>
   void add(const std::string& long_name, T& data, const std::string& description_="", Parameter::action_t action = Parameter::store_value)
   {
      char tmp = 0;
      add_parameter(tmp, long_name, data, description_, action);
   }

   // - Add list parameters

   ///
   template <class T>
   void add(const char& short_name, const std::string& long_name, std::list<T>& data, const std::string& description_="")
   { add_parameter(short_name, long_name, data, description_, Parameter::append_value); }

   ///
   template <class T>
   void add(const char& short_name, std::list<T>& data, const std::string& description_="")
   {
      std::string tmp = "";
      add_parameter(short_name, tmp, data, description_, Parameter::append_value);
   }

   ///
   template <class T>
   void add(const std::string& long_name, std::list<T>& data, const std::string& description_="")
   {
      char tmp = 0;
      add_parameter(tmp, long_name, data, description_, Parameter::append_value);
   }

   // - Add boolean parameters

   ///
   void add(const char& short_name, const std::string& long_name, bool& data, const std::string& description_="", Parameter::action_t action = Parameter::store_true)
   { add_parameter(short_name, long_name, data, description_, action); }

   ///
   void add(const char& short_name, bool& data, const std::string& description_="", Parameter::action_t action = Parameter::store_true)
   {
      std::string tmp = "";
      add_parameter(short_name, tmp, data, description_, action);
   }

   ///
   void add(const std::string& long_name, bool& data, const std::string& description_="", Parameter::action_t action = Parameter::store_true)
   {
      char tmp = 0;
      add_parameter(tmp, long_name, data, description_, action);
   }

   ///
   void categorize(const std::string& name, const std::string& category);

   ///
   void categorize(char short_name, const std::string& category)
    {
    std::string tmp;
    tmp += short_name;
    categorize(tmp,category);
    }

   ///
   template <class Type, class FuncT>
   void validate(const std::string& name_, const Type&, FuncT fn)
    {
    std::string name = standardize(name_);
    bool posix = name.size() == 1;
    Parameter& param = get_param(name.c_str(),posix);
    param.validator = new ParameterValidator<Type, FuncT>(fn);
    }

   /// Disable parameter, but don't delete it from 
   void disable(std::string name);

   /// Enable a parameter that was previously disabled
   void enable(std::string name);

   /// Delete parameter
   void remove(std::string name);

   ///
   bool initialized(const std::string& name);

   #ifdef UTILIB_HAVE_TINYXML
   ///
   void process_xml(TiXmlElement* elt, bool describe=false);
   #endif

   ///
   void write(std::ostream& os, bool categorized=true) const
    {
    std::set<std::string> tmp;
    write(os, tmp, categorized);
    }

   ///
   void write(std::ostream& os, const std::set<std::string>& categories_requested, bool categorized=true) const;

   ///
   void write_xml(std::ostream& os) const;

   ///
   void write_values(std::ostream& os, const std::string& opt_label="") const;

   ///
   void write_values_xml(std::ostream& os) const;

   ///
   void write_parameters(std::ostream& os) const
        {
        std::string indent = "                              ";
        write_parameter_set(os, parameter_data, indent);
        }

   ///
   void print_version(std::ostream& os) const
      { wordwrap_printline(os, _version_text, ""); }

   ///
   void add_argument(const std::string& name_, const std::string& description_)
   { 
    std::string name = standardize(name_);
    arg_definitions.push_back(std::pair<std::string, std::string>(name, description_));
    }

   ///
   args_t& parse_args(int argc, char* argv[]);

   ///
   void set_parameter(std::string name, Any value);

   ///
   void set_parameter(std::string name, const std::string& value);

   ///
   void set_parameter(std::string name, const char * const &value)
    {
    if ( value == NULL )
    {
       //EXCEPTION_MNGR(std::logic_error, "OptionParser::set_parameter(): "
       //               "called with NULL char*");
       set_parameter(name, Any(false));
       return;
    }
    std::string tmp = value;
    set_parameter(name,tmp);
    }

   /// returns 0 if DNE, 1 if enabled, -1 if disabled
   int has_parameter(std::string name);

   ///
   args_t& args()
   { return processed_args; }

   ///
   const args_t& args() const
      { return processed_args; }

   ///
   void alias(const string& name, const string& alias);

   ///
   bool help_option() const
   { return _help_option; }

   ///
   bool version_option() const
      { return _version_option; }

protected:

   ///
   template <class T>
   void add_parameter(const char& short_name, const std::string& long_name_, T& data, const std::string& description_, Parameter::action_t action)
   {
      std::string long_name = standardize(long_name_);
      if (long_name != "")
      {
         std::map<std::string,data_t>::iterator curr = parameters.find(long_name);
         if (curr != parameters.end())
            EXCEPTION_MNGR(runtime_error,
                           "OptionParser::add_parameter - parameter \"" << long_name << "\" already exists!");
         if (long_name.size() == 1) {
            EXCEPTION_MNGR(runtime_error, "OptionParser::add_parameter - cannot specify a non-posix option with a single-character name: " << long_name);
            }
      }
      if (short_name != 0)
      {
         std::map<char,data_t>::iterator curr = posix_parameters.find(short_name);
         if (curr != posix_parameters.end())
            EXCEPTION_MNGR(runtime_error,
                           "OptionParser::add_parameter - posix parameter \"" << short_name << "\" already exists!");
      }

      std::ostringstream buf;
      buf << data;
      data_t param_handle(true);
      Parameter& param = param_handle();
      param.name = long_name.c_str();
      param.short_name = short_name;
      param.setup_comp_name();
      param.initialize(data);
      param.syntax = typeid(data).name();
      param.default_value = buf.str();
      param.description = description_;
      param.action = action;
      param.is_bool = param.info.is_type(typeid(bool));
      add_parameter(param_handle);
      }

   ///
   void add_parameter(data_t any_param);

   ///
   void remove(const Parameter& param);

   /// A utility function for finding a parameter given a string,
   /// which may contain one or more characters.
   Parameter& get_param(const char* name, bool posix);

   /// Return the data_t that contains a parameter.
   data_t get_param_any(const char* name, bool posix, bool test_if_enabled=true);

   ///
   void write_parameter_set(std::ostream& os, const std::set<data_t>& parameters, const std::string& indent) const;

   /// Standardize the parameter name, by replacing '_' chars with '-'
   std::string standardize(const std::string& name_);

   /// This is where the parameter data is actually stored
   std::set<data_t> parameter_data;

   /// Category info for parameters.
   std::map<std::string,std::set<data_t> > categories;

   /// A map from a string to a parameter
   std::map<std::string, data_t> parameters;

   /// A map from a single character to a parameter
   std::map<char, data_t> posix_parameters;

   /// A list of argument/description pairs that describe the
   /// arguments of a command
   std::list< std::pair<std::string, std::string> > arg_definitions;

   /// The list of arguments returned after parsing a command line
   /// NOTE: the first argument is the name of the executable that was
   /// run.
   args_t processed_args;

   /// The bool used to define the 'help' option
   bool _help_option;

   /// The bool used to define the 'version' option
   bool _version_option;

   ///
   std::string _version_text;

   ///
   std::list<std::string> usage;

};

}

#endif
