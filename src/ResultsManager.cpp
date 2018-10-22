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

void ResultsManager::initialize(const std::string& base_filename, const unsigned short &format)
{
  // Could allow the various backends to self-register ... RWH
  resultsDBs.clear();
  if(format & RESULTS_OUTPUT_TEXT)
    resultsDBs.push_back(std::make_shared<ResultsDBAny>(base_filename));
  if(format & RESULTS_OUTPUT_HDF5) {
  #ifdef DAKOTA_HAVE_HDF5
    resultsDBs.push_back(std::make_shared<ResultsDBHDF5>(false /* in_core */, base_filename));
  #else
    Cerr << "WARNING: HDF5 results output was requested, but is not available in this build.\n";
  #endif
  }
}

bool ResultsManager::active() const
{
   return !resultsDBs.empty();
}


void ResultsManager::flush() const
{
  for( auto & db : resultsDBs )
  {
    db->flush();
  }
}

// ##############################################################
// Methods to support HDF5
// ##############################################################



void ResultsManager::add_metadata_to_method(const StrStrSizet& iterator_id,
                                            const AttributeArray &attrs)  
{
  for( auto & db : resultsDBs )
    db->add_metadata_to_method(iterator_id, attrs);
}

void ResultsManager::add_metadata_to_execution(const StrStrSizet& iterator_id,
                                               const AttributeArray &attrs)  
{
  for( auto & db : resultsDBs )
    db->add_metadata_to_execution(iterator_id, attrs);
}

void ResultsManager::add_metadata_to_object(const StrStrSizet& iterator_id,
                                            const StringArray &location,
                                            const AttributeArray &attrs)  
{
  for( auto & db : resultsDBs )
    db->add_metadata_to_object(iterator_id, location, attrs);
}

void ResultsManager::allocate_vector(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &len,
              const DimScaleMap &scales,
              const AttributeArray &attrs)
{
  for( auto & db : resultsDBs )
    db->allocate_vector(iterator_id, location, stored_type, len, 
                          scales, attrs);
}

void ResultsManager::allocate_matrix(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &num_rows, const int &num_cols,
              const DimScaleMap &scales,
              const AttributeArray &attrs)
{
  for( auto & db : resultsDBs )
    db->allocate_matrix(iterator_id, location, stored_type, num_rows, num_cols, 
                          scales, attrs);
}

// ##############################################################
// Methods to support legacy text output
// ##############################################################

// TODO: can't seem to pass SMACV by const ref...
void ResultsManager::insert(const StrStrSizet& iterator_id,
	      const std::string& data_name,
	      StringMultiArrayConstView sma_labels,
	      const MetaDataType metadata)
{
  if( active() )
  {
    std::vector<std::string> vs_labels;
    // convert to standard data type to store
    copy_data(sma_labels, vs_labels);

    for( auto & db : resultsDBs )
      db->insert(iterator_id, data_name, vs_labels, metadata);
  }
}


} // namespace Dakota
