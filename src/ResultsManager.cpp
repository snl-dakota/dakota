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

void ResultsManager::initialize(const std::string& base_filename)
{
  coreDBFilename = base_filename + ".txt";
  baseDB.reset(new ResultsDBAny());
  //coreDB = std::dynamic_pointer_cast<ResultsDBAny>(baseDB);

#ifdef DAKOTA_HAVE_HDF5
  bool in_core = false;
  hdf5DB.reset(new ResultsDBHDF5(in_core, base_filename));
#endif
}

bool ResultsManager::active() const
{
   return (core_db_active() || hdf5_db_active());
}

bool ResultsManager::core_db_active() const
{
  return baseDB.get() != nullptr;
}

bool ResultsManager::hdf5_db_active() const
{
  return hdf5DB.get() != nullptr;
}


void ResultsManager::write_databases()
{
  if (core_db_active()) {
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
    if (hdf5_db_active())
      hdf5DB->add_metadata_for_method(iterator_id, attrs);
#endif
}

void ResultsManager::add_metadata_for_execution(const StrStrSizet& iterator_id,
                                                const AttributeArray &attrs)  
{
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5_db_active())
      hdf5DB->add_metadata_for_execution(iterator_id, attrs);
#endif
}


} // namespace Dakota
