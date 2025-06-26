/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ResultsManager.hpp"

#include "ResultsDBAny.hpp"
#ifdef DAKOTA_HAVE_HDF5
#include "ResultsDBHDF5.hpp"
#endif

namespace Dakota {

void ResultsManager::clear_databases() { resultsDBs.clear(); }

void ResultsManager::add_database(std::unique_ptr<ResultsDBBase> db_ptr) {
  resultsDBs.push_back(std::move(db_ptr));
}

bool ResultsManager::active() const { return !resultsDBs.empty(); }

void ResultsManager::flush() const {
  for (auto &db : resultsDBs) db->flush();
}

void ResultsManager::close() { resultsDBs.clear(); }

// ##############################################################
// Methods to support HDF5
// ##############################################################

void ResultsManager::add_metadata_to_method(const StrStrSizet &iterator_id,
                                            const AttributeArray &attrs) {
  for (auto &db : resultsDBs) db->add_metadata_to_method(iterator_id, attrs);
}

void ResultsManager::add_metadata_to_execution(const StrStrSizet &iterator_id,
                                               const AttributeArray &attrs) {
  for (auto &db : resultsDBs) db->add_metadata_to_execution(iterator_id, attrs);
}

void ResultsManager::add_metadata_to_object(const StrStrSizet &iterator_id,
                                            const StringArray &location,
                                            const AttributeArray &attrs) {
  for (auto &db : resultsDBs)
    db->add_metadata_to_object(iterator_id, location, attrs);
}

void ResultsManager::add_metadata_to_study(const AttributeArray &attrs) {
  for (auto &db : resultsDBs) db->add_metadata_to_study(attrs);
}

void ResultsManager::allocate_vector(const StrStrSizet &iterator_id,
                                     const StringArray &location,
                                     ResultsOutputType stored_type,
                                     const int &len, const DimScaleMap &scales,
                                     const AttributeArray &attrs) {
  for (auto &db : resultsDBs)
    db->allocate_vector(iterator_id, location, stored_type, len, scales, attrs);
}

void ResultsManager::allocate_matrix(const StrStrSizet &iterator_id,
                                     const StringArray &location,
                                     ResultsOutputType stored_type,
                                     const int &num_rows, const int &num_cols,
                                     const DimScaleMap &scales,
                                     const AttributeArray &attrs) {
  for (auto &db : resultsDBs)
    db->allocate_matrix(iterator_id, location, stored_type, num_rows, num_cols,
                        scales, attrs);
}

// ##############################################################
// Methods to support legacy text output
// ##############################################################

// TODO: can't seem to pass SMACV by const ref...
void ResultsManager::insert(const StrStrSizet &iterator_id,
                            const std::string &data_name,
                            StringMultiArrayConstView sma_labels,
                            const MetaDataType metadata) {
  if (active()) {
    std::vector<std::string> vs_labels;
    // convert to standard data type to store
    copy_data(sma_labels, vs_labels);

    for (auto &db : resultsDBs)
      db->insert(iterator_id, data_name, vs_labels, metadata);
  }
}

}  // namespace Dakota
