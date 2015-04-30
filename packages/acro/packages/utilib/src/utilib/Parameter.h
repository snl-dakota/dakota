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
 * \file Parameter.h
 *
 * Defines the utilib::Parameter class.
 *
 * \todo revise how strings are written/read
 *		empty strings do not generate anything!
 * \todo depricated the 'referenced' value.
 */

#ifndef utilib_Parameter_h
#define utilib_Parameter_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/ValidateParameter.h>
#include <utilib/SmartPtr.h>
#include <utilib/PackObject.h>

namespace utilib
{


/**
 * Defines the information for a parameter included in a ParameterSet object.
 *
 * This class stores the value of a parameter using an Any
 * object.  Thus when the value of a Parameter object is set,
 * the result is that the value of the \a referenced object is
 * actually set.
 */
class Parameter : public PackObject
{

   class ListParameterFunctorBase
   {
   public:
      virtual ~ListParameterFunctorBase() {}
      virtual void operator()(Any& list_any, Any& item_any) = 0;
   };

   template <class T>
   class ListParameterFunctor : public ListParameterFunctorBase
   {
   public:
      void operator()(Any& list_any, Any& item_any)
      {
         std::list<T>& list_val = anyref_cast<std::list<T> > (list_any);
         T& item_val = anyref_cast<T>(item_any);
         list_val.push_back(item_val);
      }
   };


public:

   enum action_t {store_value = 0, store_true = 1, store_false = 2, append_value = 3};

   /// Constructor
   Parameter()
      : name(""), short_name(0), action(store_value), initialized(false),
        disabled(false), referenced(false), validator(0), callback(0),
        is_bool(false) 
   {}

   /// Constructor
   Parameter(const std::string& _long_name, char _short_name)
      : name(_long_name), short_name(_short_name), 
        action(store_value), initialized(false),
        disabled(false), referenced(false), validator(0), callback(0),
        is_bool(false) 
   { setup_comp_name(); }

   /// Copy Constructor
   Parameter(const Parameter& param)
      : PackObject(param)
   { copy(param); }

   /// Copy operator
   Parameter& operator=(const Parameter& param)
   { copy(param); return *this; }

   /// Destructor
   virtual ~Parameter()
   {}

   /// Write a parameter
   void write(std::ostream& os) const
   {
      os << "Parameter: " << name <<
      " Value: " << info <<
      " Initialized: " << initialized <<
      " Disabled: " << disabled <<
      " Referenced: " << referenced << std::endl;
   }

   /// Read a parameter
   void read(std::istream& is)
   {
      string tmp;
      is >> tmp >> name >> tmp >> info;
      is >> tmp >> initialized;
      is >> tmp >> disabled;
      is >> tmp >> referenced;
   }

   /// Pack a parameter into a PackBuffer object
   void write(utilib::PackBuffer& os) const
      {os << info << initialized << disabled << referenced;}

   /// Unpack a parameter out of a PackBuffer object
   void read(utilib::UnPackBuffer& is)
   {is >> info >> initialized >> disabled >> referenced;}

   /// Returns true if the parameter is valid
   bool valid()
   {
      if (!validator)
         return true;
      return validator->validate(info);
   }

   ///
   void set_value_with_string(const std::string& value);

   template <class T>
   void initialize(T& data)
   { info.set(data, true, true); }

   template <class T>
   void initialize(std::list<T>& data)
   {
      info.set(data, true, true);
      list_info.template set<T>();
      list_functor = new ListParameterFunctor<T>();
   }

   /// A reference to the value of the parameter
   Any info;

   /// Contains data used to fill a list of parameter values
   Any list_info;

   /// The parameter name
   string name;

   /// The single-character name
   char short_name;

   /// A textual description of the parameter type
   string syntax;

   /// A textual description of the default value
   string default_value;

   /// A general description of the parameter
   string description;

   /// A flag that controls how values are stored
   action_t action;

   /**
    * True if the parameter has been initialized after creation.
    * Default value: false.
    */
   bool initialized;

   /**
    * True if the parameter has been disabled.
    * Default value: false.
    */
   bool disabled;

   /**
    * True if the value of this parameter has been accessed.
    * Default value: false.
    */
   bool referenced;

   /**
    * The set of aliases that provide alternative names for the parameter.
    * \note A hash table would be a better choice here, but this is not part of
    * the STL yet.
    */
   std::set<string> aliases;

   /**
    * The set of categories that can be used to organize parameters in 
    * the OptionParser object. 
    */
   std::set<string> categories;

   /**
    * Pointer to an object that contains a unary_function that is
    * used to validate a parameter value.
    */
   SmartPtr<ParameterValidatorBase> validator;

   /**
    * Pointer to an object that contains a unary_function that is
    * used to validate a parameter value.
    */
   SmartPtr<ListParameterFunctorBase> list_functor;

   /**
    * Pointer to an object that contains a unary_function that performs a
    * callback when the parameter is set.
    */
   std::unary_function<Any, void>* callback;

   ///
   bool is_bool;

   ///
   bool operator==(const Parameter& param) const
    { return comp_name == param.comp_name; }

   ///
   bool operator<(const Parameter& param) const
    {
    if ((name == "") && (param.name == ""))
        return comp_name < param.comp_name;
    if (name == "") return true;
    if (param.name == "") return false;
    return comp_name < param.comp_name;
    }

   ///
   void setup_comp_name()
        {
        comp_name = name;
        comp_name += "_";
        comp_name += short_name;
        }

protected:

   ///
   std::string comp_name;

   /// Copy function
   void copy(const Parameter& param)
   {
      info = param.info;
      name = param.name;
      short_name = param.short_name;
      action = param.action;
      syntax = param.syntax;
      default_value = param.default_value;
      description = param.description;
      initialized = param.initialized;
      disabled = param.disabled;
      referenced = param.referenced;
      aliases = param.aliases;
      validator = param.validator;
      callback = param.callback;
      setup_comp_name();
   }

};

DEFINE_DEFAULT_ANY_COMPARATOR(Parameter);

} // namespace utilib

#endif
