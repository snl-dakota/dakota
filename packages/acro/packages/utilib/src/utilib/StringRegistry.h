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
 * \file StringRegistry.h
 *
 * Defines the utilib::StringRegistry class.  
 * 
 */
#ifndef utilib_StringRegistry_h
#define utilib_StringRegistry_h

#include <map>
#include <list>
#include <utilib/exception_mngr.h>

namespace utilib {

/**
 *  A class to register string, and associate them with a unique integer ID.
 */
class StringRegistry
{
public:

  /// Typedef for registry id's
  typedef long int registry_id_type;

  /// Constructor
  StringRegistry() {}

  /// Destructor
  ~StringRegistry() {}

  /// The size of the registry
  size_t size()
	{ return name_map.size(); }

  /// The first (lowest wrt operator<()) ID that the registry can assign
  registry_id_type lowest_id() const
	{ return 1; }

  /// Get the contextID for a given name
  const std::string& name(registry_id_type id) const
	{
	std::map<registry_id_type, std::string>::const_iterator curr = name_map.find(id);
	if (curr == name_map.end())
	   EXCEPTION_MNGR(runtime_error, "StringRegistry::name - no id with value " << id);
	return curr->second;
	}

  /// Get the contextID for a given name
  registry_id_type id(const std::string& name) const
	{
  	std::map<std::string, registry_id_type>::const_iterator curr = id_map.find(name);
        if (curr == id_map.end())
           EXCEPTION_MNGR(runtime_error, "StringRegistry::id - no id with name " << name);
	return curr->second;
	}

  /// Register a new context name (generates new contextID)
  registry_id_type add(const std::string& name)
	{
	registry_id_type id = static_cast<registry_id_type>(id_map.size()+1);
	id_map[name] = id;
	name_map[id] = name;
	names.push_back(name);
	return id;
	}

  /// Return the list of registered string names
  std::list<std::string>& get_names()
	{return names;}

protected:

  /// The registered string names
  std::list<std::string> names;

  /// map of names to id's
  std::map<std::string, registry_id_type> id_map;

  /// map of id's to names
  std::map<registry_id_type, std::string> name_map;

  };


}; // namespace utilib

#endif
