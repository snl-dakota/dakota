/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_io.hpp"

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

namespace Dakota {

//
//- Utilities for reading functional data
//

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
                        int num_rows)
{
  if( !va.empty() )
    va.clear();

  RealVector v;
  s >> std::ws;
  while ( !s.eof() )
  {
    v.sizeUninitialized(num_rows);
    read_data_tabular(s, v);
    va.push_back(v);
    s >> std::ws;
  }
}

//----------------------------------------------------------------

void 
read_unsized_data(std::istream& s,
                  RealVectorArray& va)
{
  if( !va.empty() )
    va.clear();

  // Use the first line to ascertain number of columns
  std::string first_line;
  std::getline( s >> std::ws, first_line);
  boost::char_separator<char> sep(", \t"); // allow comma and tab delimited too
  boost::tokenizer< boost::char_separator<char> > tokens(first_line, sep);
  int num_tokens = 0;
  BOOST_FOREACH (const std::string& t, tokens) {
    num_tokens++;
  }

  // Rewind input stream 
  s.seekg(0);
  read_fixed_rowsize_data(s, va, num_tokens);
}

// ----------------------------------------
// Assignment/Copy functions for data types
// ----------------------------------------

void copy_data(const RealMatrix& rmat, RealVectorArray& rvarray)
{
  rvarray.clear();
  int num_rows = rmat.numRows();
  int num_cols = rmat.numCols();
  rvarray.resize(num_rows);

  for( int i=0; i<num_rows; ++i )
  {
    RealVector & row_vec = rvarray[i];
    row_vec.sizeUninitialized(num_cols);
    for( int j=0; j<num_cols; ++j )
      rvarray[i][j] = rmat(i,j);
  }
}

//----------------------------------------------------------------

void copy_data(const RealVectorArray& rvarray, RealMatrix& rmat)
{
  if( rvarray.empty() )
    return;

  size_t num_rows = rvarray.size();
  int num_cols = rvarray[0].length();
  rmat.shapeUninitialized(num_rows, num_cols);

  for( int i=0; i<num_rows; ++i )
    for( int j=0; j<num_cols; ++j )
      rmat(i,j) = rvarray[i][j];
}


} // namespace Dakota
