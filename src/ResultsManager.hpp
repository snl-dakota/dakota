/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsManager
//- Description:  Interface to iterator results database(s) and valid results 
//-               names 
//- Owner:        Brian Adams
//- Version: $Id:$

#ifndef RESULTS_MANAGER_H
#define RESULTS_MANAGER_H

#include "dakota_results_types.hpp"
#include "ResultsDBAny.hpp"
#include "dakota_data_util.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#ifdef DAKOTA_HAVE_HDF5
#include "ResultsDBHDF5.hpp"
#else
// forward declaration due to PIMPL in support of conditional compilation HDF5
namespace Dakota {
  class ResultsDBHDF5;
}
#endif


// Design notes (BMA, Fall 2012)

// This API uses concrete data types as underlying out of core DBs may
// require it

// eventually a set of databases, maybe with a list of what they
// want notifications about

// Ideally, individual out-of-core databases can choose when to
// update, for example, HDF might allow continuous insertion, lookup,
// whereas YAML or XML might only allow dump at end

// Need ability to stream any of the databases, even text?

// Could use observer pattern to notify databases of changes

// Challenge: observer may not work if we don't want to store the new
// data in-core

// For efficiency, might want to allow posting of views if they'll
// remain in-scope (dangerous): for now, require out of core for that case

// May want to support heterogeneous storage to core or file, so
// lookup in core, if fails, lookup in file.

// Any inserted object needs to have associated meta data /
// attributes; or does it?

// Should we support stored data managed by someone else?

// other formats generated at end, e.g. YAML from file-based HDF5

// TODO: consider whether a factory pattern would be better for the ResultsEntry


namespace Dakota {

/// Get a globally unique 1-based execution number for a given
/// iterator name (combination of methodName and methodID) for use in
/// results DB.  Each Iterator::run() call creates or increments this
/// count for its string identifier.
class ResultsID {

public:
  /// get the single unique instance of ResultsID
  static ResultsID& instance();
  /// explicitly increment the iterator results ID, init to 1 if needed
  size_t increment_id(const std::string& method_name, 
		      const std::string& method_id);
  /// get (possibly creating) a unique iterator results ID for the passed name
  size_t get_id(const std::string& method_name, 
		const std::string& method_id);
  /// get a unique iterator results ID for the passed name 
  /// (const version errors if not found)
  size_t get_id(const std::string& method_name, 
		const std::string& method_id) const;

private:
  /// Private constructor for ResultsID
  ResultsID() { /* empty */ }
  /// Private destructor for ResultsID
  ~ResultsID() { /* empty */ }
  /// Private copy constructor for ResultsID
  ResultsID(ResultsID const&);
  /// Private assignment operator for ResultsID
  ResultsID& operator=(ResultsID const&);

  /// storage for the results IDs
  std::map<std::pair<std::string, std::string>, size_t> idMap;

};


/// List of valid names for iterator results
/** All data in the ResultsNames class is public, basically just a struct */
class ResultsNames {

public:

  /// Default constructor initializes all valid names
  ResultsNames():
    namesVersion(0),

    // optimization
    best_cv("Best Continuous Variables"),
    best_div("Best Discrete Integer Variables"),
    best_dsv("Best Discrete String Variables"),
    best_drv("Best Discrete Real Variables"),
    best_fns("Best Functions"),

    // statistics
    moments_std("Moments: Standard"),
    moments_central("Moments: Central"),
    moments_std_num("Moments: Standard: Numerical"),
    moments_central_num("Moments: Central: Numerical"),
    moments_std_exp("Moments: Standard: Expansion"),
    moments_central_exp("Moments: Central: Expansion"),
    moment_cis("Moment Confidence Intervals"),
    extreme_values("Extreme Values"),
    map_resp_prob("Response to Probability Mapping"),
    map_resp_rel("Response to Reliability Mapping"),
    map_resp_genrel("Response to Generalized Reliability Mapping"),
    map_prob_resp("Probability to Response Mapping"),
    map_rel_resp("Reliability to Response Mapping"),
    map_genrel_resp("Generalized Reliability to Response Mapping"),
    pdf_histograms("PDF Histograms"),

    // correlations
    correl_simple_all("Simple Correlations (All)"),
    correl_simple_io("Simple Correlations (I/O)"),
    correl_partial_io("Partial Correlations (I/O)"),
    correl_simple_rank_all("Simple Rank Correlations (All)"),
    correl_simple_rank_io("Simple Rank Correlations (I/O)"),	
    correl_partial_rank_io("Partial Rank Correlations (I/O)"),

    // approximations
    pce_coeffs("PCE Coefficients: Standardized"),
    pce_coeff_labels("PCE Coefficient Labels"),

    // labels for variables/resposes
    cv_labels("Continuous Variable Labels"),
    div_labels("Discrete Integer Variable Labels"),
    dsv_labels("Discrete String Variable Labels"),
    drv_labels("Discrete Real Variable Labels"),
    fn_labels("Function Labels")

  { /* no-op */}

  size_t namesVersion; //< Revision number Version of the results names


  // optimization (all used)
  std::string best_cv;
  std::string best_div;
  std::string best_dsv;
  std::string best_drv;
  std::string best_fns;

  // statistics
  std::string moments_std;              // used
  std::string moments_central;
  std::string moments_std_num;
  std::string moments_central_num;
  std::string moments_std_exp;
  std::string moments_central_exp;
  std::string moment_cis;               // used
  std::string extreme_values;           // used
  std::string map_resp_prob;            // used
  std::string map_resp_rel;             // used
  std::string map_resp_genrel;          // used
  std::string map_prob_resp;            // used
  std::string map_rel_resp;             // used
  std::string map_genrel_resp;          // used
  std::string pdf_histograms;           // used

  // correlations (all used)
  std::string correl_simple_all;
  std::string correl_simple_io;
  std::string correl_partial_io;
  std::string correl_simple_rank_all;
  std::string correl_simple_rank_io;
  std::string correl_partial_rank_io;

  // approximations (all used)
  std::string pce_coeffs;
  std::string pce_coeff_labels;

  // labels for variables/resposes (all used)
  std::string cv_labels;
  std::string div_labels;
  std::string dsv_labels;
  std::string drv_labels;
  std::string fn_labels;
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

  /// default constructor: no databases active until initialize called
  ResultsManager(): coreDBActive(false), hdf5DBActive(false)
  { /* no-op*/ }

  /// initialize the results manager to manage an in-core database,
  /// writing to the specified file name
  void initialize(const std::string& base_filename);

  /// whether any databases are active
  bool active() const;

  // TODO: const
  /// Write in-core databases to file
  void write_databases();

  /// Copy of valid results names for when manager is passed around
  ResultsNames results_names;

  // TODO: consider templating on container and scalar type

  /// insert data
  template<typename StoredType>
  void insert(const StrStrSizet& iterator_id,
	      const std::string& data_name,
	      const StoredType& sent_data,
	      const MetaDataType metadata = MetaDataType())
  {
    if (coreDBActive)
      coreDB->insert(iterator_id, data_name, sent_data, metadata);
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->insert(iterator_id, data_name, sent_data, metadata);
#endif
  }
  
  // TODO: can't seem to pass SMACV by const ref...
  void insert(const StrStrSizet& iterator_id,
	      const std::string& data_name,
	      StringMultiArrayConstView sma_labels,
	      const MetaDataType metadata = MetaDataType())
  {
    std::vector<std::string> vs_labels;
    if (coreDBActive || hdf5DBActive) {
      // convert to standard data type to store
      copy_data(sma_labels, vs_labels);
    }
   if (coreDBActive)
      coreDB->insert(iterator_id, data_name, vs_labels, metadata);
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->insert(iterator_id, data_name, vs_labels, metadata);
#endif
  }

  /// allocate an entry with array of StoredType of array_size for
  /// future insertion; likely move to non-templated accessors for these
  template<typename StoredType>
  void
  array_allocate(const StrStrSizet& iterator_id,
		 const std::string& data_name, 
		 size_t array_size,
		 const MetaDataType metadata = MetaDataType())
  {
    if (coreDBActive)
      coreDB->array_allocate<StoredType>(iterator_id, data_name, array_size, 
					 metadata);
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->array_allocate<StoredType>(iterator_id, data_name, array_size, 
					 metadata);
#endif

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
    if (coreDBActive)
      coreDB->array_insert<StoredType>(iterator_id, data_name, index, 
				       sent_data);
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->array_insert<StoredType>(iterator_id, data_name, index, 
				       sent_data);
#endif
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
    // copy the data to native container for storage
    StringArray sent_data_sa;
    copy_data(sent_data, sent_data_sa);

    if (coreDBActive)
      coreDB->array_insert<StoredType>(iterator_id, data_name, index, 
				       sent_data_sa);
#ifdef DAKOTA_HAVE_HDF5
    if (hdf5DBActive)
      hdf5DB->array_insert<StoredType>(iterator_id, data_name, index, 
				       sent_data_sa);
#endif
  }

private:

  /// retrieve in-core entry given by id and name
  template<typename StoredType>
  StoredType core_lookup(const StrStrSizet& iterator_id,
			 const std::string& data_name) const
  {
    return coreDB->get_data<StoredType>(iterator_id, data_name);
  }

  /// retrieve data via pointer to avoid copy; work-around for Boost
  /// any use of pointer (could use utilib::Any)
  template<typename StoredType>
  StoredType* core_lookup_ptr(const StrStrSizet& iterator_id,
			      const std::string& data_name) const
  {
    return coreDB->get_data_ptr<StoredType>(iterator_id, data_name);
  }

  /// retrieve data from in-core array of StoredType at given index
  template<typename StoredType>
  StoredType core_lookup(const StrStrSizet& iterator_id,
			 const std::string& data_name,
			 size_t index) const
  {
    return coreDB->get_array_data<StoredType>(iterator_id, data_name, index);
  }

  /// retrieve data via pointer to entry in in-core array
  template<typename StoredType>
  const StoredType* core_lookup_ptr(const StrStrSizet& iterator_id,
				    const std::string& data_name,
				    size_t index) const
  {
    return coreDB->get_array_data_ptr<StoredType>(iterator_id, data_name, index);
  }

  /// retrieve requested data into provided db_data StoredType
  template<typename StoredType>
  void file_lookup(StoredType& db_data,
		   const StrStrSizet& iterator_id,
		   const std::string& data_name) const
  {
    abort_handler(-1);
    return;
    //db_data = hdf5DB.lookup(iterator_id data_name);
  }


  // ----
  // Data
  // ----

  // TODO: consider removing or renaming flags based on HDF5 needs

  /// whether the in-core database in active
  bool coreDBActive;
  /// filename for the in-core database
  std::string coreDBFilename;

  /// whether the file database is active
  bool hdf5DBActive;

  /// In-core database, with option to flush to file at end
  boost::scoped_ptr<ResultsDBAny> coreDB;

  /// File-based database; using shared_ptr due to potentially incomplete type
  /// and requirements for checked_delete in debug builds
  boost::shared_ptr<ResultsDBHDF5> hdf5DB;

};  // class ResultsManager



// Notes on ResultsEntry: 
// * Would want to generalize this to be able to possibly a templated
//   function that would return arbitrary type using any_cast
// * Would probably need partial specialization to actually load the data
// * Could also use operator[] to index the array
// * Consider utilib::Any

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
  coreActive(results_mgr.coreDBActive)
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
  coreActive(results_mgr.coreDBActive)
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


}  // namespace Dakota

#endif  // RESULTS_MANAGER_H
