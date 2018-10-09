/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsDBAny
//- Description:  Any-based in-core results database
//- Owner:        Brian Adams
//- Version: $Id:$

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

  ResultsDBAny(const String& base_filename) :
    ResultsDBBase( base_filename + ".txt" )
  { }

  void add_metadata_for_method(
              const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override
  {
    std::cout << "ResultsDBAny needs to implement add_metadata_for_method(...)."
              << std::endl;
  }

  void add_metadata_for_execution(
              const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override
  {
    std::cout << "ResultsDBAny needs to implement "
              << "add_metadata_for_execution(...)."
              << std::endl;
  }
  
  /// record addition with metadata map
  void 
  insert(const StrStrSizet& iterator_id,
	 const std::string& data_name,
	 const boost::any& result,
	 const MetaDataType& metadata
	 ) override;

  /// insert an arbitrary type (RealMatrix) with metadata
  void
  insert(const StrStrSizet& iterator_id,
         const std::string& result_name,
         const std::string& response_name,
         const boost::any& data,
         const DimScaleMap &scales = DimScaleMap(),
         const AttributeArray &attrs = AttributeArray(),
         const bool &transpose = false) override
  {
    std::cout << "ResultsDBAny needs to implement insert(...) w/o metadata."
              << std::endl;
  }

  // NOTE: removed accessors to add metadata only or record w/o metadata

  /// Write data to file
  void flush();

private:

  /// print metadata to ostream
  void print_metadata(std::ostream& os, const MetaDataType& md);

  /// determine the type of contained data and output it to ostream
  void extract_data(const boost::any& dataholder, std::ostream& os);

  /// output data to ostream
  void output_data(const std::vector<double>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const std::vector<RealVector>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const std::vector<std::string>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const std::vector<std::vector<std::string> >& data,
		   std::ostream& os);

  /// output data to ostream
  void output_data(const std::vector<RealMatrix>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const RealMatrix& data, std::ostream& os);

}; // class ResultsDBAny


} // namespace Dakota

#endif  // RESULTS_DB_ANY_H
