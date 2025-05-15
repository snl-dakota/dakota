/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaVariables.hpp"

#include <boost/tokenizer.hpp>
#include <boost/filesystem/operations.hpp>
#include "boost/filesystem/path.hpp"

namespace Dakota {

//
//- Utilities for reading functional data
//

//----------------------------------------------------------------

void 
read_sized_data(std::istream& s,
                RealVectorArray& va,
                size_t num_rows,
                int num_cols)
{
  if( va.size() != num_rows )
    va.resize(num_rows);

  for(size_t i=0; i<num_rows; ++i)
  {
    RealVector & v = va[i];
    if( v.length() != num_cols )
      v.sizeUninitialized(num_cols);
    read_data_tabular(s, v);
  }
}

//----------------------------------------------------------------

void 
read_fixed_rowsize_data(std::istream& s,
                        RealVectorArray& va,
                        int num_cols,
                        bool row_major)
{
  if( !va.empty() )
    va.clear();

  RealVectorArray working_va;
  RealVector v;
  s >> std::ws;
  while ( !s.eof() )
  {
    v.sizeUninitialized(num_cols);
    read_data_tabular(s, v);
    working_va.push_back(v);
    s >> std::ws;
  }

  if( row_major )
    va = working_va;
  else {
    int num_rows = (int) working_va.size();
    va.resize(num_cols);
    for( int i=0; i<num_cols; ++i ) {
      v.sizeUninitialized(num_rows);
      for( int j=0; j<num_rows; ++j )
        v[j] = working_va[j][i];
      va[i] = v;
    }
  }
}

//----------------------------------------------------------------

void 
read_unsized_data(std::istream& s,
                  RealVectorArray& va,
                  bool row_major)
{
  if( !va.empty() )
    va.clear();

  // Use the first line to ascertain number of columns
  std::string first_line;
  std::getline( s >> std::ws, first_line);
  boost::char_separator<char> sep(", \t"); // allow comma and tab delimited too
  boost::tokenizer< boost::char_separator<char> > tokens(first_line, sep);
  int num_tokens = 0;
  for(const std::string& t : tokens)
    num_tokens++;

  // Rewind input stream 
  s.seekg(0);
  read_fixed_rowsize_data(s, va, num_tokens, row_major);
}

//----------------------------------------------------------------

// This version uses multiple configuration files
void 
read_config_vars_multifile(const std::string& basename, int num_expts, int ncv,
			   std::vector<Variables>& config_vars){
  assert(num_expts == config_vars.size());
  for( int i = 0; i < num_expts; ++i ) {
    std::string filename = basename + "." + std::to_string(i+1) + ".config";
    if( !std::filesystem::exists(filename) ) {
      Cerr << "Could not find expected experiment config file '"
	   << filename << "'.\n";
      abort_handler(IO_ERROR);
    }

    std::ifstream s;
    TabularIO::open_file(s, filename, "read_config_vars_multifile");
    try {
      config_vars[i].read_tabular(s, INACTIVE_VARS);
    }
    catch (const std::exception& e) {
      // could catch TabularDataTruncated, but message would be the same
      Cerr << "\nError: Could not read configuration (state) variable values "
	   << "for experiment " << i + 1 << "\nfrom file '"
	   << filename << "'; details:\n" << e.what()
	   << std::endl;
      abort_handler(IO_ERROR);
    }
  }
}

//----------------------------------------------------------------

// This version uses a single configuration file adhering to an expected num_expts X ncv format
void 
read_config_vars_singlefile(const std::string& basename, int num_expts, int ncv,
			    std::vector<Variables>& config_vars){
  assert(num_expts == config_vars.size());
  std::string filename = basename + ".config";
  if( !std::filesystem::exists(filename) ) {
    Cerr << "Could not find expected experiment config file '" << filename
	 << "'.\n";
    abort_handler(IO_ERROR);
  }

  std::ifstream s;
  TabularIO::open_file(s, filename, "read_config_vars_singlefile");
  for( int i = 0; i < num_expts; ++i ) {
    try {
      config_vars[i].read_tabular(s, INACTIVE_VARS);
    }
    catch (const std::exception& e) {
      // could catch TabularDataTruncated, but message would be the same
      Cerr << "\nError: Could not read configuration (state) variable values "
	   << "for experiment " << i + 1 << "\nfrom file '"
	   << filename << "'; details:\n" << e.what()
	   << std::endl;
      abort_handler(IO_ERROR);
    }
  }
}

//----------------------------------------------------------------

void 
read_field_values(const std::string& basename, int expt_num, RealVectorArray& field_vars){

  std::ifstream s;
  std::string filename = basename + "." + convert_to_string(expt_num) + ".dat";
  TabularIO::open_file(s, filename, "read_field_values");
  bool row_major = false;
  read_unsized_data(s, field_vars, row_major);
}

//----------------------------------------------------------------

void 
read_field_values(const std::string& basename, int expt_num, RealVector& field_vars){

  RealVectorArray va;
  read_field_values(basename, expt_num, va);
  field_vars = va[0];
}

//----------------------------------------------------------------

void 
read_coord_values(const std::string& basename, int expt_num, RealMatrix& coords){

  std::ifstream s;
  std::string filename = basename + "." + convert_to_string(expt_num) + ".coords";
  TabularIO::open_file(s, filename, "read_coord_values");
  RealVectorArray va;
  read_unsized_data(s, va);
  copy_data(va, coords);
}

//----------------------------------------------------------------

void 
read_coord_values(const std::string& basename, RealMatrix& coords){

  std::ifstream s;
  TabularIO::open_file(s, basename, "read_coord_values");
  RealVectorArray va;
  read_unsized_data(s, va);
  copy_data(va, coords);
}

//----------------------------------------------------------------

void 
read_covariance(const std::string& basename, int expt_num, RealMatrix& cov_vals){

  std::ifstream s;
  std::string filename = basename + "." + convert_to_string(expt_num) + ".sigma";
  TabularIO::open_file(s, filename, "read_sigma_values");
  RealVectorArray va;
  read_sized_data(s, va, 1, 1);
  cov_vals.shapeUninitialized(1,1);
  cov_vals[0][0] = va[0][0];
}

//----------------------------------------------------------------

void 
read_covariance(const std::string& basename, int expt_num,
                Dakota::CovarianceMatrix::FORMAT format, int num_vals,
                RealMatrix& cov_vals){

  std::ifstream s;
  std::string filename = basename + "." + convert_to_string(expt_num) + ".sigma";
  TabularIO::open_file(s, filename, "read_sigma_values");
  RealVectorArray va;
  try {
    if( format == Dakota::CovarianceMatrix::VECTOR )
      read_sized_data(s, va, 1, num_vals);
    else
      read_sized_data(s, va, num_vals, num_vals);
  }
  catch(const FileReadException& fr_except) {
        throw FileReadException("Error(s) in sigma file " + filename +
            ":\n" + fr_except.what());
  }
  copy_data(va, cov_vals);
}

//----------------------------------------------------------------

int
count_rows(String file_name){

  int nbOfLines = 0;
  std::ifstream file(file_name);
  String line;
  while (std::getline(file, line))
    ++nbOfLines;
  return nbOfLines;
}

//----------------------------------------------------------------

int
count_columns(String file_name){
  int nbOfColumns = 0;
  std::ifstream file(file_name);
  String line;
  std::getline(file, line);
  std::stringstream stream(line);
  String word;
  while ( stream >> word )
    ++nbOfColumns;
  return nbOfColumns;
}

} // namespace Dakota
