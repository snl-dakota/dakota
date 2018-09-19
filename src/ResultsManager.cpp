/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsManager
//- Description:  Implementation of non-templated portion of ResultsManager
//- Owner:        Brian Adams
//- Version: $Id:$

#include "ResultsManager.hpp"
#include "ResultsDBAny.hpp"
#ifdef DAKOTA_HAVE_HDF5
#include "ResultsDBHDF5.hpp"
#endif

namespace Dakota {

ResultsID& ResultsID::instance()
{
  static ResultsID resultsIDInstance;
  return resultsIDInstance;
}

// TODO for ResultsID: 
// * problem with this design is might need to use non-const
//   version in print_results, which should be const...

// run_identifier should probably be const
// what happens if an iterator runs nested inside the same iterator, with same ID -- bad things.  Therefore decided to archive identifier in class instance data.

using std::map;  using std::string;  using std::pair; using std::make_pair;

size_t ResultsID::increment_id(const std::string& method_name, 
			       const std::string& method_id)
{
  pair<string, string> name_id(method_name, method_id);
  map<pair<string, string>, size_t>::iterator it = idMap.find(name_id);
  // if not found initialize to zero, then increment
  if (it == idMap.end()) {
    pair<map<pair<string, string>, size_t>::iterator, bool>
      result = idMap.insert(make_pair(name_id, 0));
    it = result.first;
  }
  return ++it->second;
}

size_t ResultsID::get_id(const std::string& method_name, 
			 const std::string& method_id)
{
  pair<string, string> name_id(method_name, method_id);
  map<pair<string, string>, size_t>::iterator it = idMap.find(name_id);
  // if not found initialize to one
  if (it == idMap.end()) {
    pair<map<pair<string, string>, size_t>::iterator, bool>
      result = idMap.insert(make_pair(name_id, 1));
    it = result.first;
  }
  return it->second;
}

size_t ResultsID::get_id(const std::string& method_name, 
			 const std::string& method_id) const
{
  pair<string, string> name_id(method_name, method_id);
  map<pair<string, string>, size_t>::const_iterator it = idMap.find(name_id);
  // if not found error in const version
  if (it == idMap.end()) {
    Cerr << "\nError: ResultsID::get_id Couldn't find requested iterator '"
	 << method_name << ", " << method_id << "'" << std::endl;
    abort_handler(-1);
  }
  return it->second;
}


void ResultsManager::initialize(const std::string& base_filename)
{
  coreDBActive = true;
  coreDBFilename = base_filename + ".txt";
  baseDB.reset(new ResultsDBAny());
  //coreDB = std::dynamic_pointer_cast<ResultsDBAny>(baseDB);

#ifdef DAKOTA_HAVE_HDF5
  hdf5DBActive = true;
  bool in_core = false;
  hdf5DB.reset(new ResultsDBHDF5(in_core, base_filename));
#endif
}

bool ResultsManager::active() const
{
   return (coreDBActive || hdf5DBActive);
}

bool ResultsManager::isCoreDBActive() const
{
  return coreDBActive;
}

bool ResultsManager::isHDF5DBActive() const
{
  return hdf5DBActive;
}


void ResultsManager::write_databases()
{
  if (coreDBActive) {
    //  coreDB->dump_data(Cout);
    //  coreDB->print_data(Cout);
    std::shared_ptr<ResultsDBAny> coreDB = std::dynamic_pointer_cast<ResultsDBAny>(baseDB);
    std::ofstream results_file(coreDBFilename.c_str());
    coreDB->print_data(results_file);
  }
}

void ResultsManager::add_metadata_for_method(const StrStrSizet& iterator_id,
                                             const AttributeArray &attrs)  
{
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->add_metadata_for_method(iterator_id, attrs);
#endif
}

void ResultsManager::add_metadata_for_execution(const StrStrSizet& iterator_id,
                                                const AttributeArray &attrs)  
{
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->add_metadata_for_execution(iterator_id, attrs);
#endif
}


} // namespace Dakota
