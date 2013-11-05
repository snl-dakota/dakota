/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DATA_IO_H
#define DATA_IO_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"  // for Cerr, write_precision
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"
#include <boost/foreach.hpp>


namespace Dakota {

// -----------------------
// templated I/O functions
// -----------------------


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


/// global MPIUnpackBuffer extraction operator for std::set
template <typename T>
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, std::set<T>& data)
{
  data.clear();
  size_t i, len;
  s >> len;
  T val;
  for (i=0; i<len; ++i){
    s >> val; 
    data.insert(val);
  }
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


/// standard istream extraction operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType i, len = v.length();
  for (i=0; i<len; ++i)
    s >> v[i];
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


/// standard istream extraction operator for partial SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data_partial(std::istream& s, size_t start_index, size_t num_items,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType i, end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial(istream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=start_index; i<end; ++i)
    s >> v[i];
}


/// standard istream extraction operator for partial SerialDenseVector
/// with labels
template <typename OrdinalType, typename ScalarType>
void read_data_partial(std::istream& s,
		       size_t start_index, size_t num_items,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		       StringMultiArray& label_array)
{
  OrdinalType i, len = v.length(), end = start_index + num_items;
  if (end > len) { // start_index >= 0 since size_t
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
/// with labels
template <typename OrdinalType, typename ScalarType>
void read_data_partial(std::istream& s,
		       size_t start_index, size_t num_items,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		       StringMultiArrayView label_array)
{
  OrdinalType i, len = v.length(), end = start_index + num_items;
  if (end > len) { // start_index >= 0 since size_t
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


/// tabular istream extraction operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data_tabular(std::istream& s,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  // differs from read_data(std::istream& s) only in exception handling
  OrdinalType i, len = v.length();
  for (i=0; i<len; ++i) {
    if (s)
      s >> v[i];
    else {
      char err[80];
      std::sprintf(err,
	      "At EOF: insufficient tabular data for SerialDenseVector[%d]", i);
      throw String(err);
    }
  }
}


/// tabular istream extraction operator for partial SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data_partial_tabular(std::istream& s, size_t start_index,
  size_t num_items, Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType i, end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial_tabular(istream) "
	 << "exceeds length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=start_index; i<end; ++i) {
    if (s)
      s >> v[i];
    else {
      char err[80];
      std::sprintf(err,
	      "At EOF: insufficient tabular data for SerialDenseVector[%d]", i);
      throw String(err);
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


/// standard ostream insertion operator for full SerialDenseVector
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


#ifdef __SUNPRO_CC
// custom func needed for SunPro CC 5.10
/* WJB - ToDo (after 1.5 release):  Dig deeper into this new "ambiguity" that
         has surprisingly cropped up on Solaris
"DakotaMinimizer.cpp", line 691: Could not find a match for Dakota::write_data ...
*/
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
#endif


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


/// standard ostream insertion operator for partial SerialDenseVector
template <typename ScalarType>
void write_data_partial(std::ostream& s, size_t start_index, size_t num_items,
			const std::vector<ScalarType>& v)
{
  size_t i, end = start_index + num_items;
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
template <typename OrdinalType, typename ScalarType>
void write_data_partial(std::ostream& s, size_t start_index, size_t num_items,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  size_t end = start_index + num_items;
  if (end > v.length()) { 
    Cerr << "Error: indexing in write_data_partial(std::ostream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::scientific << std::setprecision(write_precision);
  for (OrdinalType i=start_index; i<end; ++i)
    s << "                     " << std::setw(write_precision+7) << v[i] <<'\n';
}


/// standard ostream insertion operator for partial SerialDenseVector
/// with labels
template <typename OrdinalType, typename ScalarType>
void write_data_partial(std::ostream& s, size_t start_index, size_t num_items,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v, 
  const StringMultiArray& label_array)
{
  size_t end = start_index + num_items;
  OrdinalType i, len = v.length();
  if (end > len) { // start_index >= 0 since size_t
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


/// aprepro ostream insertion operator for partial SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data_partial_aprepro(std::ostream& s, size_t start_index,
  size_t num_items,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v, 
  const StringMultiArray& label_array)
{
  size_t end = start_index + num_items;
  OrdinalType i, len = v.length();
  if (end > len) { // start_index >= 0 since size_t
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


/// tabular ostream insertion operator for full SerialDenseVector
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
template <typename OrdinalType, typename ScalarType>
void write_data_partial_tabular(std::ostream& s, size_t start_index,
  size_t num_items,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  size_t end = start_index + num_items;
  if (end > v.length()) { // start_index >= 0 since size_t
    Cerr << "Error: indexing in write_data_partial_tabular(std::ostream) "
	 << "exceeds length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
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


/// standard MPI buffer extraction operator for full SerialDenseVector
/// with labels
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


/// standard MPI buffer extraction operator for full SerialDenseVector
/// with labels
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


/// standard MPI buffer insertion operator for full SerialDenseVector
/// with labels
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


/// formatted ostream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m,
                bool brackets, bool row_rtn, bool final_rtn)
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


/// formatted ostream insertion operator for SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& m,
                bool brackets, bool row_rtn, bool final_rtn)
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


/// ostream version for writing the lower triangle of a SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_lower_triangle(std::ostream& s,
  const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& sm,
  bool row_rtn)
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


/// ostream insertion operator for a column vector from a SerialDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_col_vector_trans(std::ostream& s, OrdinalType col,
  OrdinalType num_items, bool brackets, bool break_line, bool final_rtn,
  const Teuchos::SerialDenseMatrix<OrdinalType,ScalarType>& sdm)
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
  bool brackets, bool break_line, bool final_rtn,
  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{
  write_col_vector_trans(s, col, sdm.numRows(), brackets, break_line,
                         final_rtn, sdm);
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


/// read array from std::istream
template <typename ArrayT>
inline void array_read(std::istream& s, ArrayT& v)
{
  typename ArrayT::size_type len = v.size();
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s >> v[i];
}


/// write array to std::ostream
template <typename ArrayT>
inline void array_write(std::ostream& s, const ArrayT& v)
{
  s << std::scientific << std::setprecision(write_precision);
  typename ArrayT::size_type len = v.size();
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7)
      << v[i] << '\n';
}


/// write list to std::ostream
template <typename ListT>
inline void list_write(const ListT& l, std::ostream& s)
{
  BOOST_FOREACH(const typename ListT::value_type& entry, l) {
    s << "                     " << entry << '\n';
  }  
}


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
{ list_write(data, s); return s; }


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
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7)
      << v[i] << ' ' << label_array[i] << '\n';
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
  for (register typename ArrayT::size_type i=0; i<len; ++i)
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
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s << v[i] << ' ';
}

} // namespace Dakota

#endif // DATA_IO_H
