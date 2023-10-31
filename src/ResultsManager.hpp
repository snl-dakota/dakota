/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef RESULTS_MANAGER_H
#define RESULTS_MANAGER_H

#include "dakota_results_types.hpp"
#include "ResultsDBBase.hpp"
#include "dakota_data_util.hpp"
#include <memory>
#include <boost/any.hpp>


namespace Dakota {

/// List of valid names for iterator results
/** All data in the ResultsNames class is public, basically just a struct */
class ResultsNames {

public:

  /// Default constructor
  ResultsNames()  { /* no-op */}

  size_t namesVersion = 0; //< Version number of the results names

  // optimization
  std::string best_cv = "Best Continuous Variables";
  std::string best_div = "Best Discrete Integer Variables";
  std::string best_dsv = "Best Discrete std::string Variables";
  std::string best_drv = "Best Discrete Real Variables";
  std::string best_fns = "Best Functions";

  // statistics
  std::string moments_std = "Moments: Standard";
  std::string moments_central = "Moments: Central";
  std::string moments_std_num = "Moments: Standard: Numerical";
  std::string moments_central_num = "Moments: Central: Numerical";
  std::string moments_std_exp = "Moments: Standard: Expansion";
  std::string moments_central_exp = "Moments: Central: Expansion";
  std::string moment_cis = "Moment Confidence Intervals";
  std::string extreme_values = "Extreme Values";
  std::string map_resp_prob = "Response to Probability Mapping";
  std::string map_resp_rel = "Response to Reliability Mapping";
  std::string map_resp_genrel = "Response to Generalized Reliability Mapping";
  std::string map_prob_resp = "Probability to Response Mapping";
  std::string map_rel_resp = "Reliability to Response Mapping";
  std::string map_genrel_resp = "Generalized Reliability to Response Mapping";
  std::string pdf_histograms = "PDF Histograms";

  // correlations
  std::string correl_simple_all = "Simple Correlations (All)";
  std::string correl_simple_io = "Simple Correlations (I/O)";
  std::string correl_partial_io = "Partial Correlations (I/O)";
  std::string correl_simple_rank_all = "Simple Rank Correlations (All)";
  std::string correl_simple_rank_io = "Simple Rank Correlations (I/O)";
  std::string correl_partial_rank_io = "Partial Rank Correlations (I/O)";

  // approximations
  std::string pce_coeffs = "PCE Coefficients: Standardized";
  std::string pce_coeff_labels = "PCE Coefficient Labels";

  // labels for variables/resposes
  std::string cv_labels = "Continuous Variable Labels";
  std::string div_labels = "Discrete Integer Variable Labels";
  std::string dsv_labels = "Discrete std::string Variable Labels";
  std::string drv_labels = "Discrete Real Variable Labels";
  std::string fn_labels = "Function Labels";
};


/// Results manager for iterator final data
/** The results manager provides the API for posting and retrieving
    iterator results data (and eventually run config/statistics).  It
    can manage a set of underlying results databases, in or out of core,
    depending on configuration

    The key for a results entry is documented in results_types.hpp, e.g.,
    tuple<std::string, std::string, size_t, std::string>

    For now, using concrete types for most insertion, since underlying
    databases like HDF5 might need concrete types; though template
    parameter for array allocation and retrieval. 

    All insertions overwrite any previous data.
*/
class ResultsManager
{

  /// ResultsEntry is a friend of ResultsManager
  template <typename StoredType> friend class ResultsEntry;

public:

  /// default constructor: no databases active until they are added
  ResultsManager()
  { /* no-op*/ }

  /// Delete all databases
  void clear_databases();

  /// Add a database
  void add_database(std::unique_ptr<ResultsDBBase>);

  /// whether any databases are active
  bool active() const;

  /// Flush data to the database or disk, if supported
  void flush() const;

  /// Close the database, if supported. This removes it from the active
  /// list of databases.
  void close();

  // ##############################################################
  // Methods to support HDF5
  // ##############################################################


  /// Insert using dimension scales and attributes (DimScaleMap and 
  /// AttributeArray in dakota_results_types.hpp)
  template<typename StoredType>
  void insert(const StrStrSizet& iterator_id,
              const StringArray &location,
              const StoredType& sent_data,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray(),
              const bool &transpose = false) const
  {
    for( auto & db : resultsDBs )
      db->insert(iterator_id, location, sent_data, scales, attrs, transpose);
  }

  /// Pre-allocate a matrix and (optionally) attach dimension scales and attributes. Insert
  /// rows or columns using insert_into(...)
  void allocate_matrix(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &num_rows, const int &num_cols,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray());
 
  /// Pre-allocate a vector and (optionally) attach dimension scales and attributes. Insert
  /// elements insert_into(...)
  void allocate_vector(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &len,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray());
 
  /// Insert a row or column into a matrix that was pre-allocated using allocate_matrix
  template<typename StoredType>
  void insert_into(const StrStrSizet& iterator_id,
              const StringArray &location,
              const StoredType &data,
              const int &index,
              const bool &row = true) const {
    for( auto & db : resultsDBs )
      db->insert_into(iterator_id, location, data, index, row);
  }


  /// Associate key:value metadata with all the results and executions of a method
  void add_metadata_to_method(const StrStrSizet& iterator_id,
                               const AttributeArray &attrs);

  /// Associate key:value metadata with all the results for this execution of a method
  void add_metadata_to_execution(const StrStrSizet& iterator_id,
                                  const AttributeArray &attrs);

  /// Associate key:value metadata with the object at the location
  void add_metadata_to_object(const StrStrSizet& iterator_id,
                               const StringArray &location,
                               const AttributeArray &attrs);

  /// Associate key:value metadata with the object at the location
  void add_metadata_to_study(const AttributeArray &attrs);


  // ##############################################################
  // Methods and variables to support legacy text output
  // ##############################################################

  /// Copy of valid results names for when manager is passed around
  ResultsNames results_names;

  /// allocate an entry with array of StoredType of array_size for
  /// future insertion; likely move to non-templated accessors for these
  template<typename StoredType>
  void
  array_allocate(const StrStrSizet& iterator_id,
		 const std::string& data_name, 
		 size_t array_size,
		 const MetaDataType metadata = MetaDataType())
  {
    for( auto & db : resultsDBs )
      db->array_allocate<StoredType>(iterator_id, data_name, array_size, metadata);
  }

  /// insert into a previously allocated array of StoredType at index
  /// specified; metadata must be specified at allocation
  template<typename StoredType>
  void
  array_insert(const StrStrSizet& iterator_id,
	       const std::string& data_name,
	       size_t index,
	       const StoredType& sent_data)
  {
    for( auto & db : resultsDBs )
      db->array_insert<StoredType>(iterator_id, data_name, index, sent_data);
  }

  /// specialization: insert a SMACV into a previously allocated array
  /// of StringArrayStoredType at index specified; metadata must be
  /// specified at allocation
  template<typename StoredType>
  void
  array_insert(const StrStrSizet& iterator_id,
	       const std::string& data_name,
	       size_t index,
	       StringMultiArrayConstView sent_data)
  {
    if( active() ) {
      // copy the data to native container for storage
      StringArray sent_data_sa;
      copy_data(sent_data, sent_data_sa);

      for( auto & db : resultsDBs )
        db->array_insert<StoredType>(iterator_id, data_name, index, sent_data_sa);
    }
  }


  /// insert data
  template<typename StoredType>
  void insert(const StrStrSizet& iterator_id,
	      const std::string& data_name,
	      const StoredType& sent_data,
	      const MetaDataType metadata = MetaDataType())
  {
    for( auto & db : resultsDBs )
      db->insert(iterator_id, data_name, sent_data, metadata);
  }
  
  // TODO: can't seem to pass SMACV by const ref...
  void insert(const StrStrSizet& iterator_id,
	      const std::string& data_name,
	      StringMultiArrayConstView sma_labels,
	      const MetaDataType metadata = MetaDataType());


private:

  std::vector<std::unique_ptr<ResultsDBBase> > resultsDBs;

  ResultsManager(const ResultsManager&) {return;}

//  /// retrieve in-core entry given by id and name
//  template<typename StoredType>
//  StoredType core_lookup(const StrStrSizet& iterator_id,
//			 const std::string& data_name) const
//  {
//    return coreDB->get_data<StoredType>(iterator_id, data_name);
//  }
//
//  /// retrieve data via pointer to avoid copy; work-around for Boost
//  /// any use of pointer (could use utilib::Any)
//  template<typename StoredType>
//  StoredType* core_lookup_ptr(const StrStrSizet& iterator_id,
//			      const std::string& data_name) const
//  {
//    return coreDB->get_data_ptr<StoredType>(iterator_id, data_name);
//  }
//
//  /// retrieve data from in-core array of StoredType at given index
//  template<typename StoredType>
//  StoredType core_lookup(const StrStrSizet& iterator_id,
//			 const std::string& data_name,
//			 size_t index) const
//  {
//    return coreDB->get_array_data<StoredType>(iterator_id, data_name, index);
//  }
//
//  /// retrieve data via pointer to entry in in-core array
//  template<typename StoredType>
//  const StoredType* core_lookup_ptr(const StrStrSizet& iterator_id,
//				    const std::string& data_name,
//				    size_t index) const
//  {
//    return coreDB->get_array_data_ptr<StoredType>(iterator_id, data_name, index);
//  }
//
//  /// retrieve requested data into provided db_data StoredType
//  template<typename StoredType>
//  void file_lookup(StoredType& db_data,
//		   const StrStrSizet& iterator_id,
//		   const std::string& data_name) const
//  {
//    abort_handler(-1);
//    return;
//    //db_data = hdf5DB.lookup(iterator_id data_name);
//  }


  // ----
  // Data
  // ----

  // TODO: consider removing or renaming flags based on HDF5 needs


};  // class ResultsManager


#if 0 // This code appears to capture ideas but is not used anywhere

// Notes on ResultsEntry: 
// * Would want to generalize this to be able to possibly a templated
//   function that would return arbitrary type using any_cast
// * Would probably need partial specialization to actually load the data
// * Could also use operator[] to index the array
// * Consider utilib::Any
// * Would a factory pattern be better for the ResultsEntry?

/// Class to manage in-core vs. file database lookups
/** ResultsEntry manages database lookups.  If a core database is
    available, will return a reference directly to the stored data; if
    disk, will return reference to a local copy contained in this
    class. Allows disk-stored data to persist for minimum time during
    lookup to support true out-of-core use cases. */
template <typename StoredType>
class ResultsEntry
{

public:

  /// Construct ResultsEntry containing retrieved item of StoredType
  ResultsEntry(const ResultsManager& results_mgr,
	       const StrStrSizet& iterator_id,
	       const std::string& data_name);

  /// Construct ResultsEntry to retrieve item array_index from array
  /// of StoredType
  ResultsEntry(const ResultsManager& results_mgr,
	       const StrStrSizet& iterator_id,
   	       const std::string& data_name,
   	       size_t array_index);

  ~ResultsEntry() { dbDataPtr = NULL; }

  // inactive for now: may cause compile issues w/ Intel C++
  /// return a reference to the stored data, whether from core or file
  //  const StoredType& const_view() const;
  
private:

  /// default construction disallowed: data must be initialized from
  /// DB lookup if needed
  ResultsEntry();

  /// whether the ResultsManager has an active in-core database
  bool coreActive;

  /// data retrieved from file data base
  StoredType dbData;
  
  /// non-const pointer to const data we don't own in the core case
  const StoredType *dbDataPtr;

};  // class ResultsEntry 


template<typename StoredType>
ResultsEntry<StoredType>:: 
ResultsEntry(const ResultsManager& results_mgr,
	     const StrStrSizet& iterator_id,
	     const std::string& data_name): 
  coreActive(results_mgr.core_db_active())
{
  // populate the local dbData object if needed
  if (coreActive)
    dbDataPtr = 
      results_mgr.core_lookup_ptr<StoredType>(iterator_id, data_name);
  else {
    Cerr << "\nResults database not active, can't retrieve." << std::endl; 
    abort_handler(-1);
    // copy for now, eventually file lookup
    //dbData = results_mgr.core_lookup<StoredType>(iterator_id, data_name);
    //results_mgr.file_lookup(dbData, iterator_instance, data_name);
  }
}


template<typename StoredType>
ResultsEntry<StoredType>:: 
ResultsEntry(const ResultsManager& results_mgr,
	     const StrStrSizet& iterator_id,
	     const std::string& data_name,
	     size_t array_index):
  coreActive(results_mgr.core_db_active())
{
  // populate the local dbData object if needed
  if (coreActive)
    dbDataPtr =
      results_mgr.core_lookup_ptr<StoredType>(iterator_id, data_name, 
					      array_index);
  else {
    Cerr << "\nResults database not active, can't retrieve." << std::endl; 
    abort_handler(-1);
    // copy for now, eventually file lookup
    //dbData = 
    //  results_mgr.core_lookup<StoredType>(iterator_id, data_name, array_index);
    //results_mgr.file_lookup(dbData, iterator_instance, data_name);
  }
}

// inactive for now: may cause compile issues w/ Intel C++
// template<typename StoredType>
// const StoredType& ResultsEntry<StoredType>::const_view() const
// {
//   // prefer return of core data
//   if (coreActive)
//     if (dbDataPtr)
//       return *dbDataPtr;
//     else {
//       Cerr << "Unexpected NULL pointer in ResultsManager!" << std::endl;
//       abort_handler(-1);
//     }
//   return dbData;
// }

#endif

}  // namespace Dakota

#endif  // RESULTS_MANAGER_H
