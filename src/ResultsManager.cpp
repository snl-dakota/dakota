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
  resultsDBs.clear();

  // Could allow the various backends to self-register ... RWH
  resultsDBs.push_back(std::make_shared<ResultsDBAny>(base_filename));
#ifdef DAKOTA_HAVE_HDF5
  resultsDBs.push_back(std::make_shared<ResultsDBHDF5>(false /* in_core */, base_filename));
#endif
}

bool ResultsManager::active() const
{
   return !resultsDBs.empty();
}


void ResultsManager::flush()
{
  for( auto & db : resultsDBs )
  {
    db->flush();
  }
}

void ResultsManager::add_metadata_for_method(const StrStrSizet& iterator_id,
                                             const AttributeArray &attrs)  
{
  for( auto & db : resultsDBs )
    db->add_metadata_for_method(iterator_id, attrs);
}

void ResultsManager::add_metadata_for_execution(const StrStrSizet& iterator_id,
                                                const AttributeArray &attrs)  
{
  for( auto & db : resultsDBs )
    db->add_metadata_for_execution(iterator_id, attrs);
}

} // namespace Dakota
