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
 * \file ParameterSet.h
 * 
 * Defines the utilib::ParameterSet class.
 *
 * TODO: revisit the 'augment' functionality.  This doesn't quite work w.r.t. default parameters like 'debug'.
 * See the use of selection_obj in coliny::PEAbase.
 * TODO: modify to test UTILIB_HAVE_EXCEPTIONS
 */

#ifndef utilib_ParameterSet_h
#define utilib_ParameterSet_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/Parameter.h>
#include <utilib/BitArray.h>

namespace utilib {

class ParameterSet;
class ParamFileCallback;
class ParameterList;



/**
 * A callback class that can be used with a ParameterSet object.
 */
class ParamFileCallback : public std::unary_function<Any,void>
{
public:

  /// Constructor
  ParamFileCallback() : params(0) {}

  /// Apply the callback function
  void operator()(const Any& value);

  /// The parameters used by this function
  ParameterSet* params;

};

/**
 * Defines mechanisms for creating/setting/getting a collection of
 * parameter values.  For example, this can define parameters for a 
 * class.  This class can be used as a subclass to allow transparent
 * initialization of class parameters through a simple interface.
 *
 * Parameter values can be initialized with command-line arguments using the 
 * ParameterSet::process_parameters method.  This method supports
 * the use of the GNU style uses parameter keywords preceded by two hyphens
 * rather than keyword letters. This style is extensible to contexts in
 * which there are two many parameters to use single-letter parameters. This 
 * style is easier to read than the alphabet soup of older styles, and it can be
 * combined with single-letter parameters (for commonly used parameters). 
 * A parameter argument (if any) can be separated by either whitespace or a 
 * single .=. (equal sign) character:
 *
 *    program --param1 paramval --param2=paramval
 *
 * Note: This class relies on the STL map class, which is broken on Janus
 * (the Cougar OS).  On this system, the set/get operators are linear and
 * the parameters are not sorted.
 */
class ParameterSet
{
  #if !defined(DOXYGEN)
  friend class ParameterList;
  #endif

public:

  /// Constructor
  ParameterSet();

  /// Constructor
  ParameterSet(const ParameterSet& params)
	{ *this = params; }

  /// Destructor
  virtual ~ParameterSet();

  /// Copy operator
  ParameterSet& operator=(const ParameterSet& params)
	{
	std::vector<Parameter*>::const_iterator curr = params.param_info.begin();
	std::vector<Parameter*>::const_iterator last = params.param_info.end();
	while (curr != last) {
  	  add_parameter( **curr );
  	  curr++;
  	  }
	return *this;
	}

  /// Set parameters from a parameter list
  virtual void set_parameters(ParameterList& params, bool delete_used=true);

  /// Set a parameter
  template <class T>
  void set_parameter(const string& name, const T& value);

  /// Set a parameter with a string value
  void set_parameter_with_string(const string& name, const string& value);

  /// Alias a parameter
  void alias_parameter(const string& name, const string& alias);

  /// Get a parameter value
  template <class T>
  void get_parameter(const string& name, T& value) const;

  /// Get a parameter value
  template <class T>
  T& get_parameter(const string& name);

  /// Returns true if a parameter value has been initialized
  bool parameter_initialized(const string& name);

  /// Write parameters
  virtual void write_parameters(std::ostream& os, bool verbose=true,
					const char* seperator="\n") const;

  /// Write parameters
  virtual void write_parameters_xml(std::ostream& os) const;

  /// Write parameters
  virtual void write_parameters_html(std::ostream& os) const;

  /// Write the values of the parameters.
  /// If opt_label is given, then this prints a header describing what
  /// these parameter values are for.
  virtual void write_parameter_values(std::ostream& os, 
				const char* opt_label="") const;

  /// Read parameter values
  virtual void read_parameter_values(std::istream& is, const string& terminator);

  /// Read parameter values
  virtual void read_parameter_values(std::istream& is)
		{read_parameter_values(is,"");}

  /// Pack parameter values
  virtual void write_parameter_values(PackBuffer& os) const;

  /// Unpack parameter values
  virtual void read_parameter_values(UnPackBuffer& is);

  /// Returns false if not all of the parameters have been processed
  virtual bool process_parameters(int& argc, char**& argv,
			unsigned int min_num_required_parameters=0);

  /// Reset parameter values to their defaults
  virtual void reset_parameters();

  /// Disable a parameter
  void disable_parameter(const string& name);

  /// Re-enable a paremeter
  void enable_parameter(const string& name);

  /// Returns true if a parameter exists
  bool parameter_exists(const string& name)
	{
#if !defined(COUGAR)
	std::map<std::string,size_type>::iterator curr = parameters.find(name);
	if (curr == parameters.end())
   	   return false;
	return true;
#else
        std::list<std::pair<std::string,size_type> >::iterator curr = parameters.begin();
        std::list<std::pair<std::string,size_type> >::iterator end  = parameters.end();
	while (curr != end) {
          if ((*curr).first == name)
             return true;
          curr++;
          }
	return false;
#endif
	}

#if 0
  ///
  template <class T>
  void create_parameter(const string& name, T& data)
		{create_parameter(name,data,"","","");}

  ///
  template <class T, class FuncT>
  void create_parameter(const string& name, T& data, 
				FuncT validation_func)
		{create_parameter(name,data,"","","",validation_func);}
#endif

  /// Create a parameter
  template <class T>
  void create_parameter(const std::string& name, T& data, const std::string& syntax,
		const std::string& default_value, const std::string& description)
		{create_categorized_parameter(name,data,syntax,default_value,
			description,"General");}

  /// Create a categorized parameter
  template <class T>
  void create_categorized_parameter(const std::string& name, T& data, 
		const std::string& syntax,
		const std::string& default_value, const std::string& description,
		const std::string& category);

  /// Create a parameter
  template <class T, class FuncT>
  void create_parameter(const std::string& name, T& data, 
		const std::string& syntax,
		const std::string& default_value, const std::string& description,
		FuncT validation_func)
		{create_categorized_parameter(name,data,syntax,default_value,
			description, "General",validation_func);}

  /// Create a categorized parameter
  template <class T, class FuncT>
  void create_categorized_parameter(const std::string& name, T& data, 
		const std::string& syntax,
		const std::string& default_value, const std::string& description,
		const std::string& category, FuncT validation_func);

  /// Set a parameter callback that is called when the parameter is set
  void set_parameter_callback(const std::string& name, 
		std::unary_function<Any,void>& func);

  /// Set a parameter default
  void set_parameter_default(const std::string& name, const std::string& default_value);

  /// Get a parameter object
  Parameter& get_parameter_object(const std::string& name);

  /// Returns the number of parameters
  size_type size()
		{return param_info.size();}

  /// The number of default parameters
  static size_type num_default_parameters;

  /// Insert parameters from another parameter set into this parameter
  /// set, keeping ownership in the first set.  If the 'overwrite' flag is
  /// set, these new parameters will replace the existing parameters in the
  /// set with the same name
  void augment_parameters(ParameterSet& pset, bool overwrite=false);

  ///
  const Parameter& get_raw_info(const std::string& param)
        { return find_parameter(param); }

  ///
  const Parameter& get_raw_info(const std::string& param) const
        { return find_parameter(param); }

protected:

  /// Add a parameter
  void add_parameter(const Parameter& param)
	{ add_parameter(param,"General"); }

  /// Add a parameter
  void add_parameter(const Parameter& param, const std::string& category)
	{
	Parameter* tparam = new Parameter(param);
	add_parameter(tparam,category);
	}

  /// Add a parameter
  void add_parameter(Parameter* param, const std::string& category,
							bool ownership=true);

  /// Returns the parameter index
  size_type parameter_index(const std::string& name) const
	{
	std::map<std::string,size_type>::const_iterator curr = parameters.find(name);
	if (curr == parameters.end())
   	   throw std::invalid_argument("missing parameter");
	return (*curr).second;
	}

  /// Find a parameter
  const Parameter& find_parameter(const std::string& str) const
	{ return *(param_info[parameter_index(str)]); }

  /// Find a parameter
  Parameter& find_parameter(const std::string& str)
	{ return *(param_info[parameter_index(str)]); }

  /// The array of parameters
  std::vector<Parameter*> param_info;

  /// Ownership of parameter object pointers
  BitArray param_ownership;

  /// Mapping of parameter names to parameter indeces
  std::map<std::string,size_type> parameters;

  /// Mapping for categorized parameters
  std::map<std::string, std::map<std::string,size_type>* > categorized_parameters;

  /// True if including the help parameter
  bool help_parameter;

  /// Temporary string
  std::string param_file;

  /// Temporary string
  std::string tmp_string;

  /// Temporary callback function
  ParamFileCallback param_file_callback;
};


//============================================================================
//
//
template <class T>
void ParameterSet::set_parameter(const std::string& name, const T& value)
{
try {
  Parameter& param = find_parameter(name);
  anyref_cast<T>(param.info) = value;
  if (!param.valid())
     EXCEPTION_MNGR(runtime_error,
                    "ParameterSet::set_parameter - bad parameter value \"" << 
                    value << "\" for parameter \"" << name.data() << "\".");
  param.initialized=true;
  }
catch (std::invalid_argument& ) {
  EXCEPTION_MNGR(runtime_error,
                 "ParameterSet::set_parameter - missing parameter \"" << 
                 name.data() << "\".");
  }
catch (const bad_any_cast&) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::set_parameter - bad parameter type for \"" <<
                  name << "\".");
  }
}


//============================================================================
//
//
template <class T>
void ParameterSet::get_parameter(const std::string& name, T& value) const
{
try {
  Parameter& param = const_cast<Parameter&>(find_parameter(name));
  value = param.info.template expose<T>();
  param.referenced = true;
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::get_parameter - missing parameter \"" << 
                  name << "\".");
  }
catch (const bad_any_cast&) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::get_parameter - bad parameter type for \"" <<
                  name << "\".");
  }
}


//============================================================================
//
//
template <class T>
T& ParameterSet::get_parameter(const std::string& name)
{
T* ptr = 0;
try {
  Parameter& param = find_parameter(name);
  ptr = &(anyref_cast<T>(param.info));
  param.referenced=true;
  }
catch (std::invalid_argument& ) {
  EXCEPTION_MNGR(runtime_error,
                 "ParameterSet::get_parameter - missing parameter \"" << 
                 name << "\".");
  }
catch (const bad_any_cast&) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::get_parameter - bad parameter type for \"" <<
                  name << "\".");
  }
return *ptr;
}


//============================================================================
//
//
inline bool ParameterSet::parameter_initialized(const std::string& name)
{
try {
  Parameter& param = find_parameter(name);
  return param.initialized;
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::parameter_initialized - missing parameter \"" << 
                  name << "\".");
  }
return false;
}


//============================================================================
//
//
inline void ParameterSet::reset_parameters()
{
std::vector<Parameter*>::iterator curr = param_info.begin();
std::vector<Parameter*>::iterator last = param_info.end();
while (curr != last) {
  if ( !((*curr)->disabled) )
     (*curr)->initialized=false;
     (*curr)->referenced=false;
  curr++;
  }
}


//============================================================================
//
//
inline void ParameterSet::disable_parameter(const std::string& name)
{
try {
  Parameter& param = find_parameter(name);
  param.disabled = true;
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::disable_parameter - missing parameter \"" << 
                  name << "\".");
  }
}


//============================================================================
//
//
inline void ParameterSet::enable_parameter(const std::string& name)
{
try {
  Parameter& param = find_parameter(name);
  param.disabled = false;
  param.initialized = false;
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::enable_parameter - missing parameter \"" << 
                  name << "\".");
  }
}


//============================================================================
//
//
template <class T>
void ParameterSet::create_categorized_parameter(const std::string& name, T& data, 
				const std::string& syntax,
				const std::string& default_value,
				const std::string& description,
				const std::string& category)
{
if (parameter_exists(name))
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::create_categorized_parameter - parameter \"" << 
                  name.data() << "\" already exists!");

Parameter* param = new Parameter;
param->name = name;
param->info.set(data,true);
param->syntax = syntax;
param->default_value = default_value;
param->description = description;
add_parameter(param,category);
}


//============================================================================
//
//
template <class T, class FuncT>
void ParameterSet::create_categorized_parameter(const std::string& name, T& data, 
				const std::string& syntax,
				const std::string& default_value,
				const std::string& description,
				const std::string& category,
				FuncT validation_func)
{
if (parameter_exists(name))
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::create_categorized_parameter - parameters \"" << 
                  name.data() << "\" already exists!");

Parameter* param = new Parameter;
param->name = name;
param->info.set(data,true);
param->syntax = syntax;
param->default_value = default_value;
param->description = description;
param->validator = new ParameterValidator<T,FuncT>(validation_func);
add_parameter(param,category);
}


//============================================================================
//
//
#ifndef DOXYGEN
template <>
inline void ParameterSet::create_categorized_parameter<char*>(const std::string& name, char*& , const std::string&, const std::string&, const std::string&, const std::string& )
{
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::create_categorized_parameter - cannot create parameter \"" <<
                  name.data() << "\" with a char* type.  Use a string type!");
}
#endif


//============================================================================
//
//
inline void ParameterSet::set_parameter_default(const std::string& name, const std::string& value)
{
try {
  Parameter& param = find_parameter(name);
  param.default_value = value;
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::set_parameter_default - parameter \"" << name << "\" does not exist!");
   }
}


//============================================================================
//
//
inline Parameter& ParameterSet::get_parameter_object(const std::string& name)
{
try {
  return find_parameter(name);
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::get_parameter - parameter \"" << name << "\" does not exist!");
  }
/*
** TODO- What test should this be?
*/
#if defined(UTILIB_SGI_CC) || defined(UTILIB_OSF_CC) || defined(UTILIB_AIX_CC) || defined(__PGI)
  return find_parameter(name);
#endif
}


//============================================================================
//
//
inline void ParameterSet::set_parameter_callback(const std::string& name, 
			std::unary_function<Any,void>& func)
{
try {
  Parameter& param = find_parameter(name);
  param.callback = &func;
  }
catch (std::invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::set_parameter_callback - parameter \"" << name << "\" does not exist!");
  }
}


//============================================================================
//
//
inline void ParamFileCallback::operator()(const Any& value)
{
const std::string& tmp = value.expose<std::string>();
std::ifstream ifstr;
ifstr.open(tmp.c_str());
if (ifstr) {
   params->read_parameter_values(ifstr);
   ifstr.close();
   }
else
   EXCEPTION_MNGR(runtime_error, "ParamFileCallback::operator() - unable to open file \"" << tmp << "\"");
}


} // namespace utilib


//============================================================================
//
/// Stream operator to write a ParameterSet object
inline std::ostream& operator<< (std::ostream& ostr, const utilib::ParameterSet& item)
{ item.write_parameter_values(ostr); return ostr; }


//============================================================================
//
/// Stream operator to read a ParameterSet object
inline std::istream& operator>> (std::istream& istr, utilib::ParameterSet& item)
{ item.read_parameter_values(istr); return istr; }


//============================================================================
//
/// Stream operator to pack a ParameterSet object
inline utilib::PackBuffer& operator<< (utilib::PackBuffer& ostr, const utilib::ParameterSet& item)
{ item.write_parameter_values(ostr); return ostr; }


//============================================================================
//
/// Stream operator to unpack a ParameterSet object
inline utilib::UnPackBuffer& operator>> (utilib::UnPackBuffer& istr, utilib::ParameterSet& item)
{ item.read_parameter_values(istr); return istr; }

#endif
