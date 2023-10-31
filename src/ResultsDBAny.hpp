/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef RESULTS_DB_ANY_H
#define RESULTS_DB_ANY_H

#include "ResultsDBBase.hpp"

// TODO: try/catch around any_cast

namespace Dakota {

/**
 * Class: ResultsDBAny
 *
 * Description: A map-based container to store DAKOTA Iterator
 * results in underlying boost::anys, with optional metadata
 */
class ResultsDBAny : public ResultsDBBase
{

public:

  ResultsDBAny(const String &filename) : fileName(filename) {}; 

  /// record addition with metadata map
  void 
  insert(const StrStrSizet& iterator_id,
	 const std::string& data_name,
	 const boost::any& result,
	 const MetaDataType& metadata
	 ) override;

  // NOTE: removed accessors to add metadata only or record w/o metadata

  /// Write data to file
  void flush() const;


  // ##############################################################
  // Methods to support HDF5 (no-op for Any DB)
  // ##############################################################

  /// insert an arbitrary type (RealMatrix) with metadata
  void
  insert(const StrStrSizet& iterator_id,
         const StringArray &location,
         const boost::any& data,
         const DimScaleMap &scales = DimScaleMap(),
         const AttributeArray &attrs = AttributeArray(),
         const bool &transpose = false) override
  {
    return;
  }


  /// Pre-allocate a vector and (optionally) attach dimension scales and attributes. Insert
  /// elements using insert_into(...)
  void allocate_vector(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &len,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray()) {
      return;
   }

  /// Pre-allocate a matrix and (optionally) attach dimension scales and attributes. Insert
  /// rows or columns using insert_into(...)
  void allocate_matrix(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &num_rows, const int &num_cols,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray()) {
      return;
   }

  /// Insert a row or column into a pre-allocated matrix 
  void insert_into(const StrStrSizet& iterator_id,
                   const StringArray &location,
                   const boost::any& data,
                   const int &index, const bool &row) {
      return;
  }

  /// Add key:value metadata to method
  void add_metadata_to_method(
              const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override
  { return; }

  /// Add key:value metadata to execution
  void add_metadata_to_execution(
              const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override
  { return; }
 
  /// Associate key:value metadata with the object at the location
  void add_metadata_to_object(const StrStrSizet& iterator_id,
                              const StringArray &location,
                              const AttributeArray &attrs) override
  { return; }
 
  /// Associate key:value metadata with the study
  void add_metadata_to_study(const AttributeArray &attrs) override
  { return; }

private:

  /// print metadata to ostream
  void print_metadata(std::ostream& os, const MetaDataType& md) const;

  /// determine the type of contained data and output it to ostream
  void extract_data(const boost::any& dataholder, std::ostream& os) const;

  /// output data to ostream
  void output_data(const std::vector<double>& data, std::ostream& os) const;
  /// output data to ostream
  void output_data(const std::vector<RealVector>& data, std::ostream& os) const;
  /// output data to ostream
  void output_data(const std::vector<std::string>& data, std::ostream& os) const;
  /// output data to ostream
  void output_data(const std::vector<std::vector<std::string> >& data,
		   std::ostream& os) const;

  /// output data to ostream
  void output_data(const std::vector<RealMatrix>& data, std::ostream& os) const;
  /// output data to ostream
  void output_data(const RealMatrix& data, std::ostream& os) const;

  /// name of database file
  String fileName;
}; // class ResultsDBAny


} // namespace Dakota

#endif  // RESULTS_DB_ANY_H
