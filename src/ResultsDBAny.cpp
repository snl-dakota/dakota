/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsDBAny
//- Description:  Implementation of non-templated portion of ResultsDBAny
//- Owner:        Brian Adams
//- Version: $Id:$

#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
#include "dakota_data_io.hpp"
#include "ResultsDBAny.hpp"
//#include <yaml-cpp/emitter.h>

using boost::tuple;
using std::cout;
using std::cerr;

namespace Dakota {

/** Add or update existing entry */
void ResultsDBAny::
insert(const StrStrSizet& iterator_id,
       const std::string& data_name,
       const boost::any& result,
       const MetaDataType& metadata
       )
{
  ResultsKeyType key = make_key(iterator_id, data_name); 

  std::map<ResultsKeyType, ResultsValueType>::iterator data_it = 
    iteratorData.find(key);
    
  // is it better to search then insert, or insert checking status?
  // could just use operator[] since we want to insert

  if (data_it == iteratorData.end()) {
    // new insertion
    ResultsValueType value(result, metadata);
    iteratorData.insert(std::make_pair(key, value));
  }
  else {
    // update the any contained in the result set
    ResultsValueType& result_value = data_it->second;
    result_value.first = result;
    // TODO: allow metadata update?
    //    result_value.second = metadata;
  }  
}


const ResultsValueType& 
ResultsDBAny::lookup_data(const StrStrSizet& iterator_id,
			  const std::string& data_name) const
{
  ResultsKeyType key = make_key(iterator_id, data_name);

  std::map<ResultsKeyType, ResultsValueType>::const_iterator data_it = 
    iteratorData.find(key);
    
  if (data_it == iteratorData.end()) {
    Cerr << "\nError (ResultsDBAny): Could not find requested data"
	 << "\n  Iterator ID: " << iterator_id
	 << "\n  Data name: " << data_name
	 << std::endl;
    abort_handler(-1);
  }

  // extract the stored value (data and metadata)
  return data_it->second;
}


void ResultsDBAny::dump_data(std::ostream& os)
{
  os << "--- Database Dump Begin ---\n";

  std::map<ResultsKeyType, ResultsValueType>::iterator data_it = 
    iteratorData.begin();
  std::map<ResultsKeyType, ResultsValueType>::iterator data_end = 
    iteratorData.end();

  size_t record_index = 0;
  for( ; data_it != data_end; ++data_it, ++record_index) {

    os << "Record " << record_index << ":\n";
    os << "  Key: " << data_it->first << "\n";
    const boost::any& the_any = data_it->second.first;
    const std::type_info& the_any_type = the_any.type();
    bool the_any_empty = the_any.empty();
    os << "  Data: any empty? " << the_any_empty << "\n";
    os << "  Data: any of type " << the_any_type.name() << "\n";
    os << "  Number of MetaData: " << data_it->second.second.size() << "\n";

    extract_data(the_any, os);
  }

  os << "--- Database Dump End ---";
  os << std::endl;

}



void ResultsDBAny::print_data(std::ostream& os)
{
  std::map<ResultsKeyType, ResultsValueType>::iterator data_it = 
    iteratorData.begin();
  std::map<ResultsKeyType, ResultsValueType>::const_iterator data_end = 
    iteratorData.end();


  for( ; data_it != data_end; ++data_it) {

    const ResultsKeyType& key = data_it->first;
    const ResultsValueType& value = data_it->second;

    const std::string& name = key.get<0>();
    const std::string& id = key.get<1>();
    const size_t& run = key.get<2>();
    const std::string& data = key.get<3>();

    // os << name << "," << id << "\n";
    // os << "  " << run << "\n";
    // os << "    " << data << "\n";

    // output iterator name/id/exec/dataname
    os << name << "/" << id << "/";
    os << run << "/\"";
    os << data << "\"\n";

    const MetaDataType& metadata = data_it->second.second;
    print_metadata(os, metadata);

    const boost::any& the_any = data_it->second.first;
    extract_data(the_any, os);
  }

  os << std::endl;

}


/** Extract the data from the held any and map to supported concrete types
    int
    double
    RealVector (Teuchos::SerialDenseVector<int,double)
    RealMatrix (Teuchos::SerialDenseMatrix<int,double)
*/
void ResultsDBAny::extract_data(const boost::any& dataholder, std::ostream& os)
{ 
  // TODO: how to distinguish "array" insert from storing a vector of something

  // if (dataholder.type() == typeid(int)) {
  //   output_data(boost::any_cast<int>(dataholder), os);
  // }
  // else if (dataholder.type() == typeid(double)) {
  //   output_data(boost::any_cast<double>(dataholder), os);
  // }
  if (dataholder.type() == typeid(std::vector<double>)) {
     output_data(boost::any_cast<std::vector<double> >(dataholder), os);
  }
  else if (dataholder.type() == typeid(std::vector<std::string>)) {
    output_data(boost::any_cast<std::vector<std::string> >(dataholder), os);
  }
  // array insert version
  else if (dataholder.type() == typeid(std::vector<std::vector<std::string> >)) {
    output_data(boost::any_cast<std::vector<std::vector<std::string> > >(dataholder), os);
  }
  else if (dataholder.type() == typeid(std::vector<RealVector>)) {
    output_data(boost::any_cast<std::vector<RealVector> >(dataholder), os);
  }
  else if (dataholder.type() == typeid(std::vector<RealMatrix>)) {
    output_data(boost::any_cast<std::vector<RealMatrix> >(dataholder), os);
  }
  else if (dataholder.type() == typeid(RealMatrix)) {
    output_data(boost::any_cast<RealMatrix>(dataholder), os);
  }
  else
    os << "Warning: unknown type of any: " << dataholder.type().name() 
       << std::endl;

}


void ResultsDBAny::print_metadata(std::ostream& os, const MetaDataType& md)
{
  MetaDataType::const_iterator md_it  = md.begin();
  MetaDataType::const_iterator md_end = md.end();
  for ( ; md_it != md_end; ++md_it) {
    os << "  ";
    const MetaDataKeyType& md_key = md_it->first;
    os << md_key << ": ";
    // This is a vector of strings:
    const MetaDataValueType& md_value = md_it->second;
    MetaDataValueType::const_iterator mdv_beg = md_value.begin();
    MetaDataValueType::const_iterator mdv_it  = md_value.begin();
    MetaDataValueType::const_iterator mdv_end = md_value.end();
    for ( ; mdv_it != mdv_end; ++mdv_it) {
      if (mdv_it != mdv_beg)
	os << " ";
      os << '"' << *mdv_it << '"';
    }
    os << std::endl;
  }
}


// void output_data(const std::vector<double>& data, std::ostream& os)
// {
//   std::ostream_iterator<double> spacedelimited(os, " ");
//   std::copy(data.begin(), data.end(), spacedelimited);
// } 


void ResultsDBAny::output_data(const RealMatrix& data, std::ostream& os)
{
  os << "  Data (RealMatrix):\n";
  write_data(os, data, false, true, true);
}

void ResultsDBAny::output_data(const std::vector<RealMatrix>& data, 
				std::ostream& os)
{
  // TODO: check metadata for set labels
  //  "Array Labels: "
  // and write out "Cons1"

  os << "  Data (vector<RealMatrix>):\n";
  for (size_t i=0; i<data.size(); ++i) {
    os << "      Array Entry " << i+1 << ":\n";
    write_data(os, data[i], false, true, true);
    // std::ostream_iterator<double> spacedelimited(os, " ");
    // std::copy(data.begin(), data.end(), spacedelimited);
  }
} 

void ResultsDBAny::output_data(const std::vector<double>& data,
			       std::ostream& os)
{
  os << "  Data (vector<double>):\n";
  for (size_t i=0; i<data.size(); ++i) {
    os << "      "<< data[i] << "\n";

  }
}

void ResultsDBAny::output_data(const std::vector<RealVector>& data,
			       std::ostream& os)
{
  os << "  Data (vector<vector<double>>):\n";
  for (size_t i=0; i<data.size(); ++i) {
    os << "      Array Entry " << i+1 << ":\n";
    write_data(os, data[i]);
    // std::ostream_iterator<double> spacedelimited(os, " ");
    // std::copy(data.begin(), data.end(), spacedelimited);
  }
} 

void ResultsDBAny::output_data(const std::vector<std::string>& data,
			       std::ostream& os)
{
  os << "  Data (vector<string>):\n";
  os << "      ";
  for (size_t i=0; i<data.size(); ++i) {
    if (i>0)
      os << ' ';
    os << '"' << data[i] << '"';
  }
  os << '\n';
}

// array of vector<string>
void ResultsDBAny::
output_data(const std::vector<std::vector<std::string> >& data,
	    std::ostream& os)
{
  os << "  Data (vector<vector<string>>):\n";
  for (size_t i=0; i<data.size(); ++i) {
    os << "      Array Entry " << i+1 << ":\n";
    for (size_t j=0; j<data[i].size(); ++j)
      os << "      \"" << data[i][j] << "\"\n";
    os << '\n';
  }
}

} // namespace Dakota
