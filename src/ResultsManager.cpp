/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsManager
//- Description:  Implementation of non-templated portion of ResultsManager
//- Owner:        Brian Adams
//- Version: $Id:$

#include "ResultsManager.hpp"

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
      result = idMap.insert(make_pair<pair<string,string>, size_t>(name_id, 0));
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
      result = idMap.insert(make_pair<pair<string,string>, size_t>(name_id, 1));
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


void ResultsManager::initialize(const std::string& text_filename)
{
  coreDBActive = true;
  coreDBFilename = text_filename;
}


bool ResultsManager::active() const
{
  return (coreDBActive || fileDBActive);
}

void ResultsManager::write_databases()
{
  if (!coreDBActive) return;

  //  coreDB.dump_data(Cout);
  //  coreDB.print_data(Cout);
  std::ofstream results_file(coreDBFilename.c_str());
  coreDB.print_data(results_file);
}


void ResultsManager::insert(const StrStrSizet& iterator_id,
			    const std::string& data_name,
			    StringMultiArrayConstView sma_labels,
			    const MetaDataType metadata)
{
  if (!coreDBActive) return;

  // convert to standard data type
  // consider adding to data_utils
  size_t size_sma = sma_labels.size();
  std::vector<std::string> vs_labels(size_sma);
  for (size_t i=0; i<size_sma; ++i)
    vs_labels[i] = sma_labels[i];

  coreDB.add_data(iterator_id, data_name, vs_labels, metadata);
}


void ResultsManager::insert(const StrStrSizet& iterator_id,
			    const std::string& data_name,
			    const StringArray& sa_labels,
			    const MetaDataType metadata)
{
  if (!coreDBActive) return;

  // convert to standard data type
  size_t size_sa = sa_labels.size();
  std::vector<std::string> vs_labels(size_sa);
  for (size_t i=0; i<size_sa; ++i)
    vs_labels[i] = sa_labels[i];

  coreDB.add_data(iterator_id, data_name, vs_labels, metadata);
}


void ResultsManager::insert(const StrStrSizet& iterator_id,
			    const std::string& data_name,
			    const RealMatrix& matrix,
			    const MetaDataType metadata)
{
  if (!coreDBActive) return;

  coreDB.add_data(iterator_id, data_name, matrix, metadata);
}


} // namespace Dakota
