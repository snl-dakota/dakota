/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_DATA_IO_H
#define DAKOTA_DATA_IO_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"  // for Cerr, write_precision
#include "dakota_data_types.hpp"
#include "ExperimentDataUtils.hpp"
#include "MPIPackBuffer.hpp"
#include "ActiveKey.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {

// this is NOT the recommend way to read contiguous arrays, but it's a start
// http://www.boost.org/doc/libs/1_54_0/libs/serialization/doc/wrappers.html#arrays
/// Load a Teuchos::SerialDenseVector from an archive
template <class Archive, typename OrdinalType, typename ScalarType>
void load(Archive& ar, Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v, 
     const unsigned int version)
{
  OrdinalType i, len;
  ar & len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  for (i=0; i<len; ++i)
    ar & v[i];
}

/// Save a Teuchos::SerialDenseVector  an archive
template <class Archive, typename OrdinalType, typename ScalarType>
void save(Archive& ar, 
	  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v, 
	  const unsigned int version)
{
  OrdinalType i, len = v.length();
  ar & len;
  // make a temporary array for serialization
  //  ar & make_array(v.values(), len);
  for (i=0; i<len; ++i)
    ar & v[i];
}

/// Serialize a symmetric matrix (pre-sized) to/from an archive
template<class Archive, typename OrdinalType, typename ScalarType>
void serialize(Archive& ar,
	       Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& sm,
	       const unsigned int version)
{
  OrdinalType i, j, nrows = sm.numRows();
  for (i=0; i<nrows; ++i)
    for (j=0; j<=i; ++j)
      ar & sm(i,j);
}

/// serialization save for 1-D boost::multi_array
template<typename T, class Archive>
void save(Archive& ar, const boost::multi_array<T, 1>& ma_1d,
	  const unsigned int version)
{
  typename boost::multi_array<T, 1>::size_type size0 = ma_1d.shape()[0];
  ar << size0;
  ar << boost::serialization::make_array(ma_1d.data(), ma_1d.num_elements()); 
}

/// serialization load for 1-D boost::multi_array
template<typename T, class Archive>
void load(Archive& ar, boost::multi_array<T,1>& ma_1d,
	  const unsigned int version)
{
  typename boost::multi_array<T, 1>::size_type size0;
  ar >> size0;
  ma_1d.resize(boost::extents[size0]);
  ar >> boost::serialization::make_array(ma_1d.data(), ma_1d.num_elements()); 
}


/// save a boost dynamic bitset, size, then contents
template <class Archive, typename Block, typename Allocator>
inline void save(Archive &ar, 
		 const boost::dynamic_bitset<Block, Allocator>& bs,
		 const unsigned int version)
{
  size_t size = bs.size();
  ar & size;

  // create a vector of blocks and serialize it
  std::vector<Block> vec_block(bs.num_blocks());
  to_block_range(bs, vec_block.begin());
  ar & vec_block;
}

/// load a boost dynamic bitset, size, then contents
template <class Archive, typename Block, typename Allocator>
inline void load(Archive &ar,
		 boost::dynamic_bitset<Block, Allocator> &bs,
		 const unsigned int version)
{
  size_t size;
  ar & size;

  bs.resize(size);

  // Load vector
  std::vector<Block> vec_block;
  ar & vec_block;

  // Convert vector into a bitset
  from_block_range(vec_block.begin(), vec_block.end(), bs);
}

}  // namespace serialization
}  // namespace boost

/// Register separate load/save for IntVector type
BOOST_SERIALIZATION_SPLIT_FREE(Dakota::IntVector)
/// Register separate load/save for RealVector type
BOOST_SERIALIZATION_SPLIT_FREE(Dakota::RealVector)
/// Register separate load/save for BitArray type
BOOST_SERIALIZATION_SPLIT_FREE(Dakota::BitArray)
/// Register separate load/save for StringMultiArray type
BOOST_SERIALIZATION_SPLIT_FREE(Dakota::StringMultiArray)


namespace Dakota {


// Forward declarations since these operators are used in read/write data and vice-versa

/// global std::istream extraction operator for std::vector
template <typename T>
std::istream& operator>>(std::istream& s, std::vector<T>& data);

/// global std::ostream insertion operator for std::vector
template <typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& data);

/// global std::ostream insertion operator for std::list
template <typename T>
std::ostream& operator<<(std::ostream& s, const std::list<T>& data);

/// global ostream insertion operator for std::pair
template <typename U, typename V>
std::ostream& operator<<(std::ostream& s, const std::pair<U,V>& data);

/// global std::ostream insertion operator for std::set
template <typename T>
std::ostream& operator<<(std::ostream& s, const std::set<T>& data);

/// global std::ostream insertion operator for std::map
template <typename KeyT, typename ValueT>
std::ostream& operator<<(std::ostream& s, const std::map<KeyT, ValueT>& data);

/// global std::istream extraction operator for SerialDenseVector
template <typename OrdinalType, typename ScalarType>
std::istream& operator>>(std::istream& s,
			 Teuchos::SerialDenseVector<OrdinalType, ScalarType>& data);

/// global std::ostream insertion operator for SerialDenseVector
template <typename OrdinalType, typename ScalarType>
std::ostream& operator<<(std::ostream& s,
			 const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& data);

/// global std::istream extraction operator for SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
std::istream& operator>>(std::istream& s,
			 Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& data);

/// global std::ostream insertion operator for SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
std::ostream& operator<<(std::ostream& s,
			 const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& data);

/// global std::istream extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
std::istream& operator>>(std::istream& s,
			 Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& data);

/// global std::ostream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
std::ostream& operator<<(std::ostream& s,
			 const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& data);


/// simple utility to convert intrinsics to strings 
template <typename T>
  inline std::string convert_to_string(const T &value) {
    std::ostringstream out;
    out << value;
    return out.str();
  }

/// istream extraction operator for configuration data of known dim and length
void read_sized_data(std::istream& s,
                     RealVectorArray& va,
                     size_t num_experiments,
                     int num_state_variables);

/// istream extraction operator for response data of known dim and
/// unknown length
void read_fixed_rowsize_data(std::istream& s,
                             RealVectorArray& va,
                             int num_responses,
                             bool row_major = true);

/// istream extraction operator for coordinate data of unknown dim and
/// unknown length
void read_unsized_data(std::istream& s,
                       RealVectorArray& va,
                       bool row_major = true);

/// file reader for configuration data supplied via multiple files
void read_config_vars_multifile(const std::string& basename,
                                int num_expts,
                                int ncv,
				std::vector<Variables>& config_vars);

/// file reader for configuration data supplied via a single file
void read_config_vars_singlefile(const std::string& basename,
                                 int num_expts,
                                 int ncv,
				 std::vector<Variables>& config_vars);

/// file reader for vector field (response) data
void read_field_values(const std::string& basename,
                       int expt_num,
                       RealVectorArray& field_vars);

/// file reader for scalar field (response) data
void read_field_values(const std::string& basename,
                       int expt_num,
                       RealVector& field_vars);

/// file reader for simulation coordinate data
void read_coord_values(const std::string& basename,
                       RealMatrix& coords);

/// file reader for experimental coordinate data
void read_coord_values(const std::string& basename,
                       int expt_num,
                       RealMatrix& coords);

/// file reader for CONSTANT covariance data
void read_covariance(const std::string& basename,
                     int expt_num,
                     RealMatrix& cov_vals);

/// file reader for VECTOR and MATRIX covariance data
void read_covariance(const std::string& basename,
                     int expt_num,
                     Dakota::CovarianceMatrix::FORMAT,
                     int num_vals,
                     RealMatrix& cov_vals);

/// Count number of rows in given file (for LowDiscrepancySequence)
int count_rows(String file_name);

/// Count number of columns in given file (for LowDiscrepancySequence)
int count_columns(String file_name);

// --------------------------------
// templated istream read functions (some called from operator>>)
// --------------------------------


/// standard istream extraction operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType i, len = v.length();
  std::string token;
  for (i=0; i<len; ++i)
    { s >> token; v[i] = std::atof(token.c_str()); }
}


/// standard istream extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
               Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& m)
{
  // for istream, read full matrix
  OrdinalType i, j, nrows = m.numRows(), ncols = m.numCols();
  std::string token;
  for (i=0; i<nrows; ++i)
    for (j=0; j<ncols; ++j)
      { s >> token; m(i,j) = std::atof(token.c_str()); }
}


/// standard istream extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
               Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m)
{
  // for istream, read full matrix
  OrdinalType i, j, nrows = m.numRows();
  std::string token;
  for (i=0; i<nrows; ++i)
    for (j=0; j<nrows; ++j)
      { s >> token; m(i,j) = std::atof(token.c_str()); }
}


/// standard istream extraction operator for std::vector of SerialDenseVectors
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
  std::vector<Teuchos::SerialDenseVector<OrdinalType, ScalarType> >& va)
{
  OrdinalType i, j, nrows = va.size(), ncols = (nrows > 0) ? va[0].length() : 0;
  std::string token;
  for (i=0; i<nrows; ++i)
    for (j=0; j<ncols; ++j)
      { s >> token; va[i][j] = std::atof(token.c_str()); }
}


/// standard istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len)
    label_array.resize(boost::extents[len]);
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArrayView label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in read_data(std::istream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for StringMultiArray with labels
inline void read_data(std::istream& s, StringMultiArray& v,
		      StringMultiArrayView label_array)
{
  size_t i, len = v.size();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in read_data(std::istream) does not "
	 << "equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for partial SerialDenseVector
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void read_data_partial(std::istream& s,
		       OrdinalType2 start_index, OrdinalType2 num_items,
		       Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v)
{
  OrdinalType2 i, end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial(istream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=start_index; i<end; ++i)
    s >> v[i];
}


/// standard istream extraction operator for partial SerialDenseVector
/// with labels (as StringMultiArray&)
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void read_data_partial(std::istream& s,
		       OrdinalType2 start_index, OrdinalType2 num_items,
		       Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v,
		       StringMultiArray& label_array)
{
  OrdinalType1 len = v.length();
  OrdinalType2 i, end = start_index + num_items;
  if (end > len) {
    Cerr << "Error: indexing in read_data_partial(std::istream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len)
    label_array.resize(boost::extents[len]);
  for (i=start_index; i<end; ++i)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for partial SerialDenseVector
/// with labels (as StringMultiArrayView)
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void read_data_partial(std::istream& s,
		       OrdinalType2 start_index, OrdinalType2 num_items,
		       Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v,
		       StringMultiArrayView label_array)
{
  OrdinalType1 len = v.length();
  OrdinalType2 i, end = start_index + num_items;
  if (end > len) {
    Cerr << "Error: indexing in read_data_partial(std::istream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in read_data_partial(std::istream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=start_index; i<end; ++i)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for partial StringMultiArray
/// with labels (as StringMultiArrayView)
template <typename OrdinalType>
void read_data_partial(std::istream& s, OrdinalType start_index,
		       OrdinalType num_items, StringMultiArray& v,
		       StringMultiArrayView label_array)
{
  OrdinalType i, len = v.size(), end = start_index + num_items;
  if (end > len) {
    Cerr << "Error: indexing in read_data_partial(std::istream) exceeds "
	 << "length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in read_data_partial(std::istream) "
	 << "does not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  for (i=start_index; i<end; ++i)
    s >> v[i] >> label_array[i];
}


/// tabular istream extraction operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data_tabular(std::istream& s,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  // differs from read_data(std::istream& s) only in exception handling
  OrdinalType i, len = v.length();
  s >> std::ws;
  for (i=0; i<len; ++i) {
    if (s && !s.eof()) {
      s >> v[i];
      s >> std::ws;
    }
    else {
      std::string err = 
	"At EOF: insufficient tabular data for SerialDenseVector[" +
	std::to_string(i) + "]";
      throw TabularDataTruncated(err);
    }
  }
}


/// tabular istream extraction operator for partial SerialDenseVector
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void read_data_partial_tabular(std::istream& s,
  OrdinalType2 start_index, OrdinalType2 num_items,
  Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v)
{
  OrdinalType2 i, end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial_tabular(istream) "
	 << "exceeds length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s >> std::ws;
  for (i=start_index; i<end; ++i) {
    if (s && !s.eof()) {
      s >> v[i];
      s >> std::ws;
    }
    else {
      std::string err =
	"At EOF: insufficient tabular data for SerialDenseVector[" + 
	std::to_string(i) + "]";
      throw TabularDataTruncated(err);
    }
  }
}


/// tabular istream extraction operator for partial SerialDenseVector
template <typename OrdinalType>
void read_data_partial_tabular(std::istream& s, OrdinalType start_index,
			       OrdinalType num_items, StringMultiArray& v)
{
  OrdinalType i, end = start_index + num_items;
  if (end > v.size()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial_tabular(istream) "
	 << "exceeds length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  for (i=start_index; i<end; ++i) {
    if (s)
      s >> v[i];
    else {
      std::string err = 
	"At EOF: insufficient tabular data for StringMultiArray[" + 
	std::to_string(i) + "]";
      throw TabularDataTruncated(err);
    }
  }
}


/// annotated istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data_annotated(std::istream& s,
  Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
  StringMultiArray& label_array)
{
  OrdinalType i, len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() )
    label_array.resize(boost::extents[len]);
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// annotated istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data_annotated(std::istream& s,
  Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
  StringMultiArrayView label_array)
{
  OrdinalType i, len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data_annotated(std::istream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// annotated istream extraction operator for StringMultiArray with labels
inline void read_data_annotated(std::istream& s, StringMultiArray& v,
				StringMultiArrayView label_array)
{
  size_t i, len;
  s >> len;
  if( len != v.size() )
    v.resize(boost::extents[len]);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data_annotated(std::istream) "
	 << "does not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// istream partial specialization for reading the lower triangle of a
/// SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_lower_triangle(std::istream& s,
  Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& sm)
{
  OrdinalType i, j, nrows = sm.numRows();
  std::string token;
  for (i=0; i<nrows; ++i)
    for (j=0; j<=i; ++j)
      { s >> token; sm(i,j) = std::atof(token.c_str()); }
}


/// generic input stream template for reading the lower triangle of a
/// SerialSymDenseMatrix
template <typename IStreamType, typename OrdinalType, typename ScalarType>
void read_lower_triangle(IStreamType& s,
  Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& sm)
{
  OrdinalType i, j, nrows = sm.numRows();
  for (i=0; i<nrows; ++i)
    for (j=0; j<=i; ++j)
      s >> sm(i,j);
}


/// istream partial specialization for reading a column vector of a
/// SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_col_vector_trans(std::istream& s, OrdinalType col,
  Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{
  OrdinalType nr = sdm.numRows();
  ScalarType* sdm_c = sdm[col]; // column vector
  std::string token; // handles NaN and +/-Inf
  for (OrdinalType row=0; row<nr; ++row)
    { s >> token; sdm_c[row] = std::atof(token.c_str()); }
}


/// generic input stream template for reading a column vector of a
/// SerialDenseMatrix
template <typename IStreamType, typename OrdinalType, typename ScalarType>
void read_col_vector_trans(IStreamType& s, OrdinalType col,
  Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{
  OrdinalType nr = sdm.numRows();
  ScalarType* sdm_c = sdm[col]; // column vector
  for (OrdinalType row=0; row<nr; ++row)
    s >> sdm_c[row];
}


/// read array from std::istream
template <typename ArrayT>
inline void array_read(std::istream& s, ArrayT& v)
{
  typename ArrayT::size_type len = v.size();
  for (typename ArrayT::size_type i=0; i<len; ++i)
    s >> v[i];
}


// ---------------------------------
// templated ostream write functions (some called from operator<<)
// ---------------------------------


/// standard ostream insertion operator for pointer
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s, const ScalarType* v, OrdinalType len)
{
  s << std::scientific << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i]
      << '\n';
}


/// standard ostream insertion operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType i, len = v.length();
  s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i]
      << '\n';
}


/// standard ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringMultiArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(std::ostream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}


// BMA: Also needed for MSVS 2008, and probably in general as a
// const_array_view shouldn't be convertible to const multi_array&
// Could consider a single implementation with a forward from const &
// to const view

/// standard ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		StringMultiArrayConstView label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(std::ostream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}
//#endif


/// standard ostream insertion operator for full SerialDenseVector
/// with alternate labels
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(std::ostream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}


/// standard ostream insertion operator for StringMultiArray
inline void write_data(std::ostream& s, const StringMultiArray& v)
{
  size_t i, len = v.size();
  //s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] <<'\n';
}


/// aprepro ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data_aprepro(std::ostream& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
  const StringMultiArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_aprepro(std::ostream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << "                    { " << std::setw(15)
      << std::setiosflags(std::ios::left)
      << label_array[i].data() << std::resetiosflags(std::ios::adjustfield)
      << " = " << std::setw(write_precision+7) << v[i] <<" }\n";
}


/// standard ostream insertion operator for partial std::vector
template <typename OrdinalType, typename ScalarType>
void write_data_partial(std::ostream& s, OrdinalType start_index,
			OrdinalType num_items, const std::vector<ScalarType>& v)
{
  OrdinalType i, end = start_index + num_items;
  if (end > v.size()) { 
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of std::vector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] <<'\n';
}


/// standard ostream insertion operator for partial SerialDenseVector
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void write_data_partial(std::ostream& s,
  OrdinalType2 start_index, OrdinalType2 num_items,
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v)
{
  OrdinalType2 end = start_index + num_items;
  if (end > v.length()) { 
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (OrdinalType2 i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] <<'\n';
}


/// standard ostream insertion operator for partial SerialDenseVector
/// with labels (StringMultiArray or StringArray)
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType,
	  typename LabelArrayType>
void write_data_partial(std::ostream& s,
  OrdinalType2 start_index, OrdinalType2 num_items,
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v, 
  const LabelArrayType& label_array)
{
  OrdinalType2 i, end = start_index + num_items;
  OrdinalType1 len = v.length();
  if (end > len) {
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_partial(std::ostream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}


/// standard ostream insertion operator for partial StringMultiArray
template <typename OrdinalType>
void write_data_partial(std::ostream& s, OrdinalType start_index,
			OrdinalType num_items, const StringMultiArray& v)
{
  OrdinalType i, end = start_index + num_items, len = v.size();
  if (end > len) {
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  //s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] <<'\n';
}


// BMA: Implementation accepting a const_array_view since can't
// convert to const& prototype above.  Could convert const & to a view
// and pass to this function to have a single implementation...

/// standard ostream insertion operator for partial StringMultiArray
template <typename OrdinalType>
void write_data_partial(std::ostream& s, OrdinalType start_index,
			OrdinalType num_items, StringMultiArrayConstView v)
{
  OrdinalType i, end = start_index + num_items, len = v.size();
  if (end > len) {
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  //s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] <<'\n';
}


/// standard ostream insertion operator for partial StringMultiArray
/// with labels
template <typename OrdinalType>
void write_data_partial(std::ostream& s, OrdinalType start_index,
			OrdinalType num_items, const StringMultiArray& v,
			StringMultiArrayConstView label_array)
{
  OrdinalType i, end = start_index + num_items, len = v.size();
  if (end > len) {
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_partial(std::ostream) "
	 << "does not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  //s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}


/// aprepro ostream insertion operator for partial SerialDenseVector with labels
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void write_data_partial_aprepro(std::ostream& s, OrdinalType2 start_index,
  OrdinalType2 num_items,
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v, 
  const StringMultiArray& label_array)
{
  OrdinalType2 i, end = start_index + num_items;
  OrdinalType1 len = v.length();
  if (end > len) {
    Cerr << "Error: indexing in write_data_partial_aprepro(std::ostream) "
	 << "exceeds length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_partial_aprepro"
	 << "(std::ostream) does not equal length of Vector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                    { " << std::setw(15)
      << std::setiosflags(std::ios::left)
      << label_array[i].data() << std::resetiosflags(std::ios::adjustfield)
      << " = " << std::setw(write_precision+7) << v[i] <<" }\n";
}


/// aprepro ostream insertion operator for partial StringMultiArray with labels
/// (string variables must be quoted for use with aprepro)
template <typename OrdinalType>
void write_data_partial_aprepro(std::ostream& s, OrdinalType start_index,
  OrdinalType num_items, const StringMultiArray& v,
  StringMultiArrayConstView label_array)
{
  OrdinalType i, end = start_index + num_items, len = v.size();
  if (end > len) {
    Cerr << "Error: indexing in write_data_partial_aprepro(std::ostream) "
	 << "exceeds length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_partial_aprepro(std::"
	 << "ostream) does not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  //s << std::scientific << std::setprecision(write_precision);
  for (i=start_index; i<end; ++i)
    s << "                    { " << std::setw(15)
      << std::setiosflags(std::ios::left)
      << label_array[i].data() << std::resetiosflags(std::ios::adjustfield)
      << " = " << std::setw(write_precision+7) << '"' << v[i] << '"' << " }\n";
}


/// annotated ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data_annotated(std::ostream& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
  const StringMultiArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_annotated(std::ostream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << len << ' ' << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << v[i] << ' ' << label_array[i] << ' ';
}


/// annotated ostream insertion operator for StringMultiArray with labels
inline void write_data_annotated(std::ostream& s, const StringMultiArray& v,
				 StringMultiArrayConstView label_array)
{
  size_t i, len = v.size();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_annotated(std::ostream) "
	 << "does not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  s << len << ' ';// << std::scientific << std::setprecision(write_precision);
  for (i=0; i<len; ++i)
    s << v[i] << ' ' << label_array[i] << ' ';
}


/// tabular ostream insertion operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void write_data_tabular(std::ostream& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType i, len = v.length();
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
  for (i=0; i<len; ++i)
    s << std::setw(write_precision+4) << v[i] << ' ';
}


/// tabular ostream insertion operator for pointer
template <typename OrdinalType, typename ScalarType>
void write_data_tabular(std::ostream& s, const ScalarType* ptr,
			OrdinalType num_items)
{
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
  for (OrdinalType i=0; i<num_items; ++i)
    s << std::setw(write_precision+4) << ptr[i] << ' ';
}


/// tabular ostream insertion operator for partial SerialDenseVector
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void write_data_partial_tabular(std::ostream& s,
  OrdinalType2 start_index, OrdinalType2 num_items,
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& v)
{
  OrdinalType2 end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in write_data_partial_tabular(std::ostream) "
	 << "exceeds length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
  for (OrdinalType2 i=start_index; i<end; ++i)
    s << std::setw(write_precision+4) << v[i] << ' ';
}


/// tabular ostream insertion operator for partial StringMultiArray (const &)
template <typename OrdinalType>
void write_data_partial_tabular(std::ostream& s,
  OrdinalType start_index, OrdinalType num_items, const StringMultiArray& v)
{
  OrdinalType end = start_index + num_items;
  if (end > v.size()) {
    Cerr << "Error: indexing in write_data_partial_tabular(std::ostream) "
	 << "exceeds length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  //s << std::setprecision(write_precision) 
  //  << std::resetiosflags(std::ios::floatfield);
  for (OrdinalType i=start_index; i<end; ++i)
    s << std::setw(write_precision+4) << v[i] << ' ';
}


// BMA: Implementation accepting a const_array_view since can't
// convert to const& prototype above.  Could convert const & to a view
// and pass to this function to have a single implementation...

/// tabular ostream insertion operator for partial StringMultiArray (const view)
template <typename OrdinalType>
void write_data_partial_tabular(std::ostream& s,
  OrdinalType start_index, OrdinalType num_items, StringMultiArrayConstView v)
{
  OrdinalType end = start_index + num_items;
  if (end > v.size()) {
    Cerr << "Error: indexing in write_data_partial_tabular(std::ostream) "
	 << "exceeds length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  //s << std::setprecision(write_precision) 
  //  << std::resetiosflags(std::ios::floatfield);
  for (OrdinalType i=start_index; i<end; ++i)
    s << std::setw(write_precision+4) << v[i] << ' ';
}


/// tabular ostream insertion operator for vector of strings
inline void write_data_tabular(std::ostream& s, const StringArray& sa)
{
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
  size_t size_sa = sa.size();
  for (size_t i=0; i<size_sa; ++i)
    s << std::setw(write_precision+4) << sa[i] << ' ';
}


/// tabular ostream insertion operator for view of StringMultiArray
inline void write_data_tabular(std::ostream& s, StringMultiArrayConstView ma)
{
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
  size_t size_ma = ma.size();
  for (size_t i=0; i<size_ma; ++i)
    s << std::setw(write_precision+4) << ma[i] << ' ';
}


/// formatted ostream insertion operator for SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& m,
                bool brackets = true, bool row_rtn = true,
		bool final_rtn = true)
{
  OrdinalType i, j, nrows = m.numRows(), ncols = m.numCols();
  s << std::scientific << std::setprecision(write_precision);
  if (brackets) s << "[[ ";
  else          s << "   ";
  for (i=0; i<nrows; ++i) {
    for (j=0; j<ncols; ++j)
      s << std::setw(write_precision+7) << m(i,j) << ' ';
    // NOTE: newlines on every 4th component (as in the row vector case)
    // could lead to ambiguity in the matrix case.
    if (row_rtn && i!=m.numRows()-1)
      s << "\n   ";
  }
  if (brackets)  s << "]] ";
  if (final_rtn) s << '\n';
}


/// formatted ostream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m,
                bool brackets = true, bool row_rtn = true,
		bool final_rtn = true)
{
  OrdinalType i, j, nrows = m.numRows();
  s << std::scientific << std::setprecision(write_precision);
  if (brackets) s << "[[ ";
  else          s << "   ";
  for (i=0; i<nrows; ++i) {
    for (j=0; j<nrows; ++j)
      s << std::setw(write_precision+7) << m(i,j) << ' ';
    // NOTE: newlines on every 4th component (as in the row vector case)
    // could lead to ambiguity in the matrix case.
    if (row_rtn && i!=nrows-1)
      s << "\n   ";
  }
  if (brackets)  s << "]] ";
  if (final_rtn) s << '\n';
}


/// ostream insertion operator for SerialDenseMatrix with row/col labels
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& m,
                const StringArray& row_labels, const StringArray& col_labels)
{
  OrdinalType i, j, nrows = m.numRows(), ncols = m.numCols();
  s << std::scientific << std::setprecision(write_precision)
    << "                 ";
  for (j=0; j<ncols; ++j)
    s << std::setw(write_precision+7) << col_labels[j] << ' ';
  s << '\n';
  for (i=0; i<nrows; ++i) {
    s << std::setw(15) << row_labels[i] << "  ";
    for (j=0; j<ncols; ++j)
      s << std::setw(write_precision+7) << m(i,j) << ' ';
    s << '\n';
  }
}


/// ostream version for writing the lower triangle of a SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_lower_triangle(std::ostream& s,
  const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& sm,
  bool row_rtn = true)
{
  OrdinalType i, j, nrows = sm.numRows();
  s << std::scientific << std::setprecision(write_precision);
  for (i=0; i<nrows; ++i) {
    for (j=0; j<=i; ++j)
      s << std::setw(write_precision+7) << sm(i,j) << ' ';
    if (row_rtn)
      s << '\n';
  }
}


/// generic output stream template for writing the lower triangle of a
/// SerialSymDenseMatrix
template <typename OStreamType, typename OrdinalType, typename ScalarType>
void write_lower_triangle(OStreamType& s,
  const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& sm)
{
  OrdinalType i, j, nrows = sm.numRows();
  for (i=0; i<nrows; ++i)
    for (j=0; j<=i; ++j)
      s << sm(i,j);
}


/// ostream insertion operator for a column vector from a SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_col_vector_trans(std::ostream& s, OrdinalType col,
  OrdinalType num_items, 
  const Teuchos::SerialDenseMatrix<OrdinalType,ScalarType>& sdm,
  bool brackets = true, bool break_line = true, bool final_rtn = true)
{
  s << std::scientific << std::setprecision(write_precision);
  if (brackets) s << " [ ";
  else          s << "   ";
  for (OrdinalType row=0; row < num_items; ++row) {
    s << std::setw(write_precision+7) << sdm(row,col) << ' ';
    if (break_line && (row+1)%4 == 0)
      s << "\n   "; // Output 4 gradient components per line
  }
  if (brackets)  s << "] ";
  if (final_rtn) s << '\n';
}


/// ostream insertion operator for a column vector from a SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_col_vector_trans(std::ostream& s, OrdinalType col,
  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm,
  bool brackets = true, bool break_line = true, bool final_rtn = true)
{
  write_col_vector_trans(s, col, sdm.numRows(), sdm, brackets, break_line,
                         final_rtn);
}


/// ostream insertion operator for a column vector from a SerialDenseMatrix
template <typename OStreamType, typename OrdinalType, typename ScalarType>
void write_col_vector_trans(OStreamType& s, OrdinalType col,
  OrdinalType num_items,
  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{ for (OrdinalType row=0; row < num_items; ++row) s << sdm(row,col); }


/// ostream insertion operator for a column vector from a SerialDenseMatrix
template <typename OStreamType, typename OrdinalType, typename ScalarType>
void write_col_vector_trans(OStreamType& s, OrdinalType col,
  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{ write_col_vector_trans(s, col, sdm.numRows(), sdm); }


/// write array to std::ostream
template <typename ArrayT>
inline void array_write(std::ostream& s, const ArrayT& v)
{
  s << std::scientific << std::setprecision(write_precision);
  typename ArrayT::size_type len = v.size();
  for (typename ArrayT::size_type i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7)
      << v[i] << '\n';
}


/// write array to std::ostream with labels
template <typename ArrayT>
inline void array_write(std::ostream& s, const ArrayT& v,
                        const std::vector<String>& label_array)
{
  s << std::scientific << std::setprecision(write_precision);
  typename ArrayT::size_type len = v.size();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in vector<T>::write() does not equal "
	 << "length of vector." << std::endl;
    abort_handler(-1);
  }
  for (typename ArrayT::size_type i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7)
      << v[i] << ' ' << label_array[i] << '\n';
}


/// write array to std::ostream (APREPRO format)
/// specialize for StringArray: values need quoting
inline void array_write_aprepro(std::ostream& s, const StringArray& v,
                                const std::vector<String>& label_array)
{
  s << std::scientific << std::setprecision(write_precision);
  StringArray::size_type len = v.size();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in vector<T>::write() does not equal "
	 << "length of vector." << std::endl;
    abort_handler(-1);
  }
  for (StringArray::size_type i=0; i<len; ++i)
    s << "                    { "
      << std::setw(15) << std::setiosflags(std::ios::left)
      << label_array[i].c_str() << std::resetiosflags(std::ios::adjustfield)
      << " = " << std::setw(write_precision+7)
      << '"' << v[i] << '"' << " }\n";
}


/// write array to std::ostream (APREPRO format)
template <typename ArrayT>
inline void array_write_aprepro(std::ostream& s, const ArrayT& v,
                                const std::vector<String>& label_array)
{
  s << std::scientific << std::setprecision(write_precision);
  typename ArrayT::size_type len = v.size();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in vector<T>::write() does not equal "
	 << "length of vector." << std::endl;
    abort_handler(-1);
  }
  for (typename ArrayT::size_type i=0; i<len; ++i)
    s << "                    { "
      << std::setw(15) << std::setiosflags(std::ios::left)
      << label_array[i].c_str() << std::resetiosflags(std::ios::adjustfield)
      << " = " << std::setw(write_precision+7)
      << v[i] << " }\n";
}


// WJB: confer with MSE -- e.g. desired API? naming style of read/write funcs,
//                              order of the argument list; mv to data_io.h??
/// Write array to ostream as a row vector; precede with length if
/// write_len = true
template <typename ArrayT>
inline void array_write_annotated(std::ostream& s, const ArrayT& v,
				  bool write_len)
{
  s << std::scientific << std::setprecision(write_precision);
  typename ArrayT::size_type len = v.size();
  if (write_len)
    s << len << ' ';
  for (typename ArrayT::size_type i=0; i<len; ++i)
    s << v[i] << ' ';
}


// ------------------------------------------------------
// templated MPIPackBuffer insertion/extraction operators (in namespace Dakota)
// ------------------------------------------------------


// MSE 1/29/2021: introduce random-access conditional with C++11:
// > one option is iterator-based "tag-dispatch"
// > another option is enable_if<>, but this approach seems more complex
// Thanks to stackoverflow.com post 23848011 for sample code adapted below

// helper alias:
template<typename ContainerT>
using IteratorCategoryOf = typename
  std::iterator_traits<typename ContainerT::iterator>::iterator_category;


template<typename ContainerT>
void container_read(MPIUnpackBuffer& s, ContainerT& c,
		    std::forward_iterator_tag) // generic version
{
#ifdef DAKOTA_HAVE_MPI
  c.clear();
  typename ContainerT::size_type i, len;
  s >> len;
  for (i=0; i<len; ++i) {
    typename ContainerT::value_type data;// fresh alloc in case T is ref-counted
    s >> data;
    c.push_back(data);
  }
#endif
}

template<typename ContainerT>
void container_read(MPIUnpackBuffer& s, ContainerT& c,
		    std::random_access_iterator_tag) // random-access version
{
  // Container types with random-access iterators include vector<T>, deque<T>.
  // While the generic version above could be augmented with reserve(len) for
  // vector, deque does not support this.  Therefore, we use resize() with
  // operator[] instead of reserve() + push_back():
#ifdef DAKOTA_HAVE_MPI
  c.clear(); // ensures fresh allocations in resize() (see note above)
  typename ContainerT::size_type i, len;
  s >> len;
  c.resize(len); // deque<T> supports resize() but not reserve()
  for (i=0; i<len; ++i)
    s >> c[i];
#endif
}

template<typename ContainerT>
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, ContainerT& c) // two versions
{ container_read(s, c, IteratorCategoryOf<ContainerT>()); return s; }


template<typename ContainerT>
MPIPackBuffer& operator<<(MPIPackBuffer& s, const ContainerT& c) // one version
{
#ifdef DAKOTA_HAVE_MPI
  typename ContainerT::size_type len = c.size();
  s << len;
  for (const typename ContainerT::value_type& entry : c)
    s << entry;
#endif
  return s;
}


/// stream insertion for BitArray
template <typename Block, typename Allocator>
inline MPIPackBuffer& 
operator<<(MPIPackBuffer& s, const boost::dynamic_bitset<Block, Allocator>& bs)
{ 
  size_t size = bs.size();
  s << size;

  // create a vector of blocks and insert it it
  std::vector<Block> vec_block(bs.num_blocks());
  to_block_range(bs, vec_block.begin());
  s << vec_block;

  return s; 
}


/// stream extraction for BitArray
template <typename Block, typename Allocator>
inline MPIUnpackBuffer& 
operator>>(MPIUnpackBuffer& s, boost::dynamic_bitset<Block, Allocator>& bs)
{ 
  size_t size;
  s >> size;

  bs.resize(size);

  // Load vector
  std::vector<Block> vec_block;
  s >> vec_block;

  // Convert vector into a bitset
  from_block_range(vec_block.begin(), vec_block.end(), bs);

  return s;
}


/// global MPIUnpackBuffer extraction operator for std::pair
template <typename U, typename V>
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, std::pair<U,V>& data)
{
  U first; V second;
  s >> first >> second;
  data.first = first; data.second = second;
  return s;
}


/// global MPIPackBuffer insertion operator for std::pair
template <typename U, typename V>
MPIPackBuffer& operator<<(MPIPackBuffer& s, const std::pair<U,V>& data)
{
  s << data.first << data.second;
  return s;
}


/// global MPIUnpackBuffer extraction operator for std::set
template <typename T>
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, std::set<T>& data)
{
  size_t i, len;  T val;
  s >> len;

  data.clear();
  for (i=0; i<len; ++i)
    { s >> val; data.insert(val); }
  return s;
}


/// global MPIPackBuffer insertion operator for std::set
template <typename T>
MPIPackBuffer& operator<<(MPIPackBuffer& s, const std::set<T>& data)
{
  size_t len = data.size();
  s << len;
  for (typename std::set<T>::const_iterator cit = data.begin();
       cit != data.end(); ++cit)
    s << *cit;
  return s;
}


/// global MPIUnpackBuffer extraction operator for std::map
template <typename KeyT, typename ValueT>
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, std::map<KeyT, ValueT>& data)
{
  data.clear();
  size_t i, len; KeyT key; ValueT val;
  s >> len;
  for (i=0; i<len; ++i){
    s >> key >> val; 
    data[key] = val;
  }
  return s;
}


/// global MPIPackBuffer insertion operator for std::map
template <typename KeyT, typename ValueT>
MPIPackBuffer& operator<<(MPIPackBuffer& s, const std::map<KeyT, ValueT>& data)
{
  size_t len = data.size();
  s << len;
  for (typename std::map<KeyT, ValueT>::const_iterator cit = data.begin();
       cit != data.end(); ++cit)
    s << cit->first << cit->second;
  return s;
}


/// global MPIPackBuffer insertion operator for Teuchos::SerialDenseVector
template <typename OrdinalType, typename ScalarType> 
MPIPackBuffer& operator<<(MPIPackBuffer& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& data)
{
  OrdinalType i, n = data.length();
  s << n;
  for (i=0; i<n; ++i)
    s << data[i];
  return s;
}


/// global MPIPackBuffer insertion operator for Teuchos::SerialDenseMatrix
template <typename OrdinalType, typename ScalarType> 
MPIPackBuffer& operator<<(MPIPackBuffer& s,
  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& data)
{
  OrdinalType i, j, n = data.numRows(), m = data.numCols();
  s << n << m;
  for (i=0; i<n; ++i)
    for (j=0; j<m; ++j)
      s << data(i,j);
  return s;
}


/// global MPIPackBuffer insertion operator for Teuchos::SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType> 
MPIPackBuffer& operator<<(MPIPackBuffer& s,
  const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& data)
{
  OrdinalType i, j, n = data.numRows();
  s << n;
  for (i=0; i<n; ++i)
    for (j=0; j<=i; ++j)
      s << data(i,j);
  return s;
}


/// global MPIUnpackBuffer extraction operator for Teuchos::SerialDenseVector
template <typename OrdinalType, typename ScalarType> 
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s,
  Teuchos::SerialDenseVector<OrdinalType, ScalarType>& data)
{
  OrdinalType i, n;
  s >> n;
  data.sizeUninitialized(n);
  for(i=0; i<n; ++i)
    s >> data[i];
  return s;
}


/// global MPIUnpackBuffer extraction operator for Teuchos::SerialDenseMatrix
template <typename OrdinalType, typename ScalarType> 
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s,
  Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& data)
{
  OrdinalType i, j, n, m;
  s >> n >> m;
  data.shapeUninitialized(n, m);
  for (i=0; i<n; ++i)
    for (j=0; j<m; ++j)
      s >> data(i,j);
  return s;
}


/// global MPIUnpackBuffer extraction operator for Teuchos::SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType> 
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s,
  Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& data)
{
  OrdinalType i, j, n;
  s >> n;
  data.shapeUninitialized(n);
  for (i=0; i<n; ++i)
    for (j=0; j<=i; ++j)
      s >> data(i,j);
  return s;
}


// -----------------------------------------------------
// templated MPI{Pack,Unpack}Buffer read,write functions (in namespace Dakota)
// -----------------------------------------------------


/// MPI buffer extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(MPIUnpackBuffer& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArray& label_array)
{
  OrdinalType i, len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() )
    label_array.resize(boost::extents[len]);
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// MPI buffer extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(MPIUnpackBuffer& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArrayView label_array)
{
  OrdinalType i, len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data(MPIUnpackBuffer&) does "
	 << "not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// MPI buffer extraction operator for StringMultiArray with labels
inline void read_data(MPIUnpackBuffer& s, StringMultiArray& v,
		      StringMultiArrayView label_array)
{
  size_t i, len;
  s >> len;
  if( len != v.size() )
    v.resize(boost::extents[len]);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data(MPIUnpackBuffer&) does "
	 << "not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// MPI buffer insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(MPIPackBuffer& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringMultiArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(MPIPackBuffer) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << len;
  for (i=0; i<len; ++i)
    s << v[i] << label_array[i];
}


/// MPI buffer insertion operator for StringMultiArray with labels
inline void write_data(MPIPackBuffer& s, const StringMultiArray& v,
		       StringMultiArrayConstView label_array)
{
  size_t i, len = v.size();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(MPIPackBuffer) "
	 << "does not equal length of StringMultiArray." << std::endl;
    abort_handler(-1);
  }
  s << len;
  for (i=0; i<len; ++i)
    s << v[i] << label_array[i];
}


// -----------------------------------------------------------------------
// templated iostream insertion/extraction operators (in namespace Dakota)
// -----------------------------------------------------------------------


/// global std::istream extraction operator for std::vector
template <typename T>
inline std::istream& operator>>(std::istream& s, std::vector<T>& data)
{ array_read(s, data); return s; }


/// global std::ostream insertion operator for std::vector
template <typename T>
inline std::ostream& operator<<(std::ostream& s, const std::vector<T>& data)
{ array_write(s, data); return s; }


/// global std::ostream insertion operator for std::list
template <typename T>
inline std::ostream& operator<<(std::ostream& s, const std::list<T>& data)
{
  for(const typename std::list<T>::value_type& entry : data)
    s << "                     " << entry << '\n';
  return s;
}


/// global ostream insertion operator for std::pair
template <typename U, typename V>
std::ostream& operator<<(std::ostream& s, const std::pair<U,V>& data)
{
  size_t width = write_precision+7;
  s << "                     " << std::setw(width) 
    << data.first << ' ' << data.second << '\n';
  return s;
}


/// global std::ostream insertion operator for std::set
template <typename T>
std::ostream& operator<<(std::ostream& s, const std::set<T>& data)
{
  size_t width = write_precision+7;
  for (typename std::set<T>::const_iterator cit = data.begin();
       cit != data.end(); ++cit)
    s << "                     " << std::setw(width) << *cit << '\n';
  return s;
}


/// global std::ostream insertion operator for std::map
template <typename KeyT, typename ValueT>
std::ostream& operator<<(std::ostream& s, const std::map<KeyT, ValueT>& data)
{
  size_t width = write_precision+7;
  for (typename std::map<KeyT, ValueT>::const_iterator cit = data.begin();
       cit != data.end(); ++cit)
    s << "                     " << std::setw(width) << cit->first
      << "  " << std::setw(width) << cit->second << '\n';
  return s;
}


/// global std::istream extraction operator for SerialDenseVector
template <typename OrdinalType, typename ScalarType>
inline std::istream& operator>>(std::istream& s,
  Teuchos::SerialDenseVector<OrdinalType, ScalarType>& data)
{ read_data(s, data); return s; }


/// global std::ostream insertion operator for SerialDenseVector
template <typename OrdinalType, typename ScalarType>
inline std::ostream& operator<<(std::ostream& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& data)
{ write_data(s, data); return s; }


/// global std::istream extraction operator for SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
inline std::istream& operator>>(std::istream& s,
  Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& data)
{ read_data(s, data); return s; }


/// global std::ostream insertion operator for SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
inline std::ostream& operator<<(std::ostream& s,
  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& data)
{ write_data(s, data, true, true, true); return s; }


/// global std::istream extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
inline std::istream& operator>>(std::istream& s,
  Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& data)
{ read_data(s, data); return s; }


/// global std::ostream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
inline std::ostream& operator<<(std::ostream& s,
  const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& data)
{ write_data(s, data, true, true, true); return s; }

// Note: these operators for Pecos types have to be promoted to the Dakota
//       namespace for MPI buffer streams to resolve ambiguity between the
//       Pecos templated stream operators and the generic ContainerT templates
//       in this file.

/// stream extraction operator for ActiveKeyData.  Calls read(Stream&).
//template <typename Stream>
//Stream& operator>>(Stream& s, Pecos::ActiveKeyData& key_data)
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s,
				   Pecos::ActiveKeyData& key_data)
{ key_data.read(s); return s; }


/// stream insertion operator for ActiveKeyData.  Calls write(Stream&).
//template <typename Stream>
//Stream& operator<<(Stream& s, const Pecos::ActiveKeyData& key_data)
inline MPIPackBuffer& operator<<(MPIPackBuffer& s,
				 const Pecos::ActiveKeyData& key_data)
{ key_data.write(s); return s; }


/// stream extraction operator for ActiveKey.  Calls read(Stream&).
//template <typename Stream>
//Stream& operator>>(Stream& s, Pecos::ActiveKey& key)
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, Pecos::ActiveKey& key)
{ key.read(s); return s; }


/// stream insertion operator for ActiveKey.  Calls write(Stream&).
//template <typename Stream>
//Stream& operator<<(Stream& s, const Pecos::ActiveKey& key)
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const Pecos::ActiveKey& key)
{ key.write(s); return s; }

} // namespace Dakota

#endif // DAKOTA_DATA_IO_H
