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
 * \file Factory.h
 *
 * Defines the utilib::Factory class.
 */

#ifndef __utilib_Factory_h
#define __utilib_Factory_h

#include <vector>
#include <set>
#include <string>
#include <map>
#include <utilib/exception_mngr.h>
#ifdef UTILIB_HAVE_TINYXML
#include <tinyxml/tinyxml.h>
#endif


namespace utilib {

///
class FactoryBase;

#ifndef UTILIB_HAVE_TINYXML
class TiXmlElement {};
#endif

/// Macro for setting up a container class declaration
#define FACTORY_CONTAINER_CLASS(factory_name, base_t, construction, description)\
namespace utilib {\
class factory_name ## _FactoryContainer : public FactoryContainer< base_t >\
{\
public:\
  factory_name ## _FactoryContainer(const char* _name, const char* _description) : _container_name(_name), _container_description(_description) {}\
  virtual ~factory_name ## _FactoryContainer () {} \
  base_t create(TiXmlElement* elt=0) { construction; elt=0; }\
  const char* container_name() const { return _container_name; }\
  const char* container_description() const { return _container_description; }\
  const char* _container_name;\
  const char* _container_description;\
};\
}

/// Declaration of global factory container
#define FACTORY_REFERENCE(factory_name)\
namespace utilib {\
bool ref_ ## factory_name ## _FactoryContainer();\
}\
namespace {\
const bool tmp_ ## factory_name = utilib::ref_ ## factory_name ## _FactoryContainer();\
}


/// Register a factory constructor
#define FACTORY_NAMED_REGISTER(factory_name_tok, factory_name_str, base_t, construction, description)\
FACTORY_CONTAINER_CLASS(factory_name_tok, base_t, construction, description)\
namespace utilib {\
bool ref_ ## factory_name_tok ## _FactoryContainer() {return true;};\
extern const bool init_ ## factory_name_tok ## _FactoryContainer = factory_register< base_t >( factory_name_str, new factory_name_tok ## _FactoryContainer( factory_name_str, description));\
}

/// Register a factory constructor
#define FACTORY_REGISTER(factory_name, base_t, construction, description)\
FACTORY_NAMED_REGISTER(factory_name, #factory_name, base_t, construction, description)
	

/// Container class used to manage the construction within a factory.
template <class BaseT>
class FactoryContainer
{
public:
  /// Empty virtual destructor to silence compiler warnings
  virtual ~FactoryContainer() {}

  /// Create an object
  virtual BaseT create(TiXmlElement* elt) = 0;

  /// Name of this container
  virtual const char* container_name() const = 0;

  /// Description of this container
  virtual const char* container_description() const = 0;
};


/// An object for creating an arbitrary set of objects with a common base
/// class.
template <class BaseT, class ContainerT=FactoryContainer<BaseT> >
class Factory
{
public:

  /// Constructor.
  Factory()
	{}

  /// Destructor.
  virtual ~Factory()
	{
	typename std::map<std::string, ContainerT*>::iterator curr = name_map.begin();
	typename std::map<std::string, ContainerT*>::iterator end  = name_map.end();
	while (curr != end) {
	  delete curr->second;
	  curr++;
	  }
	}

  /// Register a container
  bool register_container(const char* name, ContainerT* container)
	{
	typename std::map<std::string, ContainerT*>::const_iterator curr = name_map.find(name);
	if (curr == name_map.end()) {
	   name_map[name] = container;
	   return true;
	   }
	EXCEPTION_MNGR(std::runtime_error, "Factory::register_container - Container \"" << name << "\" has already been registered.");
	return false;
	}

  /// Create a object type
  BaseT create(const std::string& name, TiXmlElement* elt=0)
	{
	typename std::map<std::string, ContainerT*>::iterator curr = name_map.find(name);
	if (curr == name_map.end())
	   EXCEPTION_MNGR(std::runtime_error, "Factory::create - unknown type \"" << name << "\"");
	return curr->second->create(elt);
	}

  /// Get the names of the objects in the factory.
  std::vector<const char*> get_names() const
	{
	std::vector<const char*> ans(name_map.size());

	typename std::vector<const char*>::iterator vcurr = ans.begin();
	typename std::map<std::string, ContainerT*>::const_iterator curr = name_map.begin();
	typename std::map<std::string, ContainerT*>::const_iterator end  = name_map.end();
	while (curr != end) {
	  *vcurr = curr->second->container_name();
	  vcurr++;
	  curr++;
	  }

	return ans;
	}
	
  /// Get the descriptions of the objects in the factory.
  std::vector<const char*> get_descriptions() const
	{
	std::vector<const char*> ans(name_map.size());

	typename std::vector<const char*>::iterator vcurr = ans.begin();
	typename std::map<std::string, ContainerT*>::const_iterator curr = name_map.begin();
	typename std::map<std::string, ContainerT*>::const_iterator end  = name_map.end();
	while (curr != end) {
	  *vcurr = curr->second->container_description();
	  vcurr++;
	  curr++;
	  }

	return ans;
	}

protected:

  /// Map of module name to container handle
  std::map<std::string, ContainerT*> name_map;
};


/// Returns the global factory for a specific type
template <class BaseT>
utilib::Factory<BaseT>& global_factory()
{
static utilib::Factory<BaseT> factory;
return factory;
}


/// Register an object
template <class BaseT, class ContainerT>
bool factory_register(const char* name, ContainerT* container)
{ return global_factory<BaseT>().register_container(name,container); }


/// Perform create using the global factory
template <class BaseT>
BaseT factory_create(const char* obj_name)
{ return global_factory<BaseT>().create(obj_name); }


}


#endif
