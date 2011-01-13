/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DATA_IO_H
#define DATA_IO_H

#include "system_defs.h"
#include "global_defs.h"  // for Cerr, write_precision
#include "data_types.h"
#include "DakotaBinStream.H"
#include "DakotaString.H"
#include "MPIPackBuffer.H"
#include <boost/foreach.hpp>


namespace Dakota {

// -----------------------
// templated I/O functions
// -----------------------

/// global std::ostream insertion operator for std::set
template <class T>
std::ostream& operator<<(std::ostream& s, const std::set<T>& data)
{
  for (typename std::set<T>::const_iterator cit = data.begin();
       cit != data.end(); ++cit)
    s << "                     " << std::setw(write_precision+7) << *cit <<'\n';
  return s;
}

/// global MPIUnpackBuffer extraction operator for std::set
template <class T>
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, std::set<T>& data)
{
  data.clear();
  size_t len;
  s >> len;
  T val;
  for (size_t i=0; i<len; ++i){
    s >> val; 
    data.insert(val);
  }
  return s;
}

/// global MPIPackBuffer insertion operator for std::set
template <class T>
MPIPackBuffer& operator<<(MPIPackBuffer& s, const std::set<T>& data)
{
  size_t len = data.size();
  s << len;
  for (typename std::set<T>::const_iterator cit = data.begin();
       cit != data.end(); ++cit)
    s << *cit;
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
  OrdinalType len = v.length();
  for (OrdinalType i=0; i<len; i++)
    s >> v[i];
}


/// standard istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArray& label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len)
    label_array.resize(boost::extents[len]);
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArrayView label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in read_data(std::istream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard istream extraction operator for partial SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data_partial(std::istream& s, size_t start_index, size_t num_items,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial(istream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType i=start_index; i<end; i++)
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
  OrdinalType end = start_index + num_items;
  OrdinalType len = v.length();
  if (end > len) { // start_index >= 0 since size_t
    Cerr << "Error: indexing in read_data_partial(std::istream) exceeds "
	 << "length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  if (label_array.size() != len)
    label_array.resize(boost::extents[len]);
  for (OrdinalType i=start_index; i<end; i++)
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
  OrdinalType end = start_index + num_items;
  OrdinalType len = v.length();
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
  for (OrdinalType i=start_index; i<end; i++)
    s >> v[i] >> label_array[i];
}


/// tabular istream extraction operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void read_data_tabular(std::istream& s,
		       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  // differs from read_data(std::istream& s) only in exception handling
  OrdinalType len = v.length();
  for (OrdinalType i=0; i<len; i++) {
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
  OrdinalType end = start_index + num_items;
  if (end > v.length()) {
    Cerr << "Error: indexing in Vector<T>::read_data_partial_tabular(istream) "
	 << "exceeds length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType i=start_index; i<end; i++) {
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
  OrdinalType len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() )
    label_array.resize(boost::extents[len]);
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// annotated istream extraction operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data_annotated(std::istream& s,
  Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
  StringMultiArrayView label_array)
{
  OrdinalType len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data_annotated(std::istream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard ostream insertion operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s, const ScalarType* v, OrdinalType len)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
    s << "                     " << std::setw(write_precision+7) << v[i]
      << '\n';
}


/// standard ostream insertion operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  OrdinalType len = v.length();
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
    s << "                     " << std::setw(write_precision+7) << v[i]
      << '\n';
}


/// standard ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringMultiArray& label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(std::ostream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}


#ifdef __SUNPRO_CC
// custom func needed for SunPro CC 5.10
/* WJB - ToDo (after 1.5 release):  Dig deeper into this new "ambiguity" that
         has surprisingly cropped up on Solaris
"DakotaMinimizer.C", line 691: Could not find a match for Dakota::write_data ...
*/
/// standard ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		StringMultiArrayConstView& label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(std::ostream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
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
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(std::ostream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
    s << "                     " << std::setw(write_precision+7) << v[i] << ' '
      << label_array[i] << '\n';
}


/// aprepro ostream insertion operator for full SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data_aprepro(std::ostream& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
  const StringMultiArray& label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_aprepro(std::ostream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
    s << "                    { " << std::setw(15)
      << std::setiosflags(std::ios::left)
      << label_array[i].data() << std::resetiosflags(std::ios::adjustfield)
      << " = " << std::setw(write_precision+7) << v[i] <<" }\n";
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
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=start_index; i<end; i++)
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
  OrdinalType len = v.length();
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
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=start_index; i<end; i++)
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
  OrdinalType len = v.length();
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
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  for (OrdinalType i=start_index; i<end; i++)
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
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data_annotated(std::ostream) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s.setf(std::ios::scientific);
  s << len << ' ' << std::setprecision(write_precision);
  for (OrdinalType i=0; i<len; i++)
    s << v[i] << ' ' << label_array[i] << ' ';
}


/// tabular ostream insertion operator for full SerialDenseVector
template <typename OrdinalType, typename ScalarType>
void write_data_tabular(std::ostream& s,
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v)
{
  const OrdinalType& len = v.length();
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);
  for (OrdinalType i=0; i<len; i++)
    s << std::setw(write_precision+4) << v[i] << ' ';
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
  for (OrdinalType i=start_index; i<end; i++)
    s << std::setw(write_precision+4) << v[i] << ' ';
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


/// standard binary stream extraction operator for full
/// SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(BiStream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArray& label_array)
{
  OrdinalType len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() )
    label_array.resize(boost::extents[len]);
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard binary stream extraction operator for full
/// SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(BiStream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArrayView label_array)
{
  OrdinalType len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data(BiStream&) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard binary stream insertion operator for full
/// SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(BoStream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringMultiArray& label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(BoStream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << len;
  for (OrdinalType i=0; i<len; i++)
    s << v[i] << label_array[i];
}


/// standard MPI buffer extraction operator for full SerialDenseVector
/// with labels
template <typename OrdinalType, typename ScalarType>
void read_data(MPIUnpackBuffer& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArray& label_array)
{
  OrdinalType len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() )
    label_array.resize(boost::extents[len]);
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard MPI buffer extraction operator for full SerialDenseVector
/// with labels
template <typename OrdinalType, typename ScalarType>
void read_data(MPIUnpackBuffer& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArrayView label_array)
{
  OrdinalType len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data(MPIUnpackBuffer&) does "
	 << "not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType i=0; i<len; i++)
    s >> v[i] >> label_array[i];
}


/// standard MPI buffer insertion operator for full SerialDenseVector
/// with labels
template <typename OrdinalType, typename ScalarType>
void write_data(MPIPackBuffer& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringMultiArray& label_array)
{
  OrdinalType len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(MPIPackBuffer) "
	 << "does not equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << len;
  for (OrdinalType i=0; i<len; i++)
    s << v[i] << label_array[i];
}


/// standard istream extraction operator for std::vector of SerialDenseVectors
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
  std::vector<Teuchos::SerialDenseVector<OrdinalType, ScalarType> >& va)
{
  const OrdinalType& nrows = va.size();
  const OrdinalType  ncols = (nrows > 0) ? va[0].length() : 0;
  for (OrdinalType i=0; i<nrows; ++i)
    for (OrdinalType j=0; j<ncols; ++j)
      s >> va[i][j];
}


/// standard istream extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_data(std::istream& s,
               Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  for (OrdinalType i=0; i<nrows; ++i)
    for (OrdinalType j=0; j<ncols; ++j)
      s >> m(i,j);
}


/// standard binary stream extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_data(BiStream& s,
               Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  for (OrdinalType i=0; i<nrows; ++i)
    for (OrdinalType j=0; j<ncols; ++j)
      s >> m(i,j);
}

/// standard MPI buffer extraction operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void read_data(MPIUnpackBuffer& s,
               Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  for (OrdinalType i=0; i<nrows; ++i)
    for (OrdinalType j=0; j<ncols; ++j)
      s >> m(i,j);
}


/// formatted ostream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m,
                bool brackets, bool row_rtn, bool final_rtn)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  s.setf(std::ios::scientific); // formatting optimized for T = double
  s << std::setprecision(write_precision);
  if (brackets)
    s << "[[ ";
  for (OrdinalType i=0; i<nrows; ++i) {
    for (OrdinalType j=0; j<ncols; ++j)
      s << std::setw(write_precision+7) << m(i,j) << ' ';
    // NOTE: newlines on every 4th component (as in the row vector case)
    // could lead to ambiguity in the matrix case.
    if (row_rtn && i!=m.numRows()-1)
      s << "\n   ";
  }
  if (brackets)
    s << "]] ";
  if (final_rtn)
    s << '\n';
}


/// standard binary stream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(BoStream& s,
                const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  for (OrdinalType i=0; i<nrows; ++i)
    for (OrdinalType j=0; j<ncols; ++j)
      s << m(i,j);
}


/// standard MPI buffer insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(MPIPackBuffer& s,
                const Teuchos::SerialSymDenseMatrix<OrdinalType, ScalarType>& m)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  for (OrdinalType i=0; i<nrows; ++i)
    for (OrdinalType j=0; j<ncols; ++j)
      s << m(i,j);
}


/// formatted ostream insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_data(std::ostream& s,
                const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& m,
                bool brackets, bool row_rtn, bool final_rtn)
{
  const OrdinalType& nrows = m.numRows();
  const OrdinalType& ncols = m.numCols();
  s.setf(std::ios::scientific); // formatting optimized for T = double
  s << std::setprecision(write_precision);
  if (brackets)
    s << "[[ ";
  for (OrdinalType i=0; i<nrows; ++i) {
    for (OrdinalType j=0; j<ncols; ++j)
      s << std::setw(write_precision+7) << m(i,j) << ' ';
    // NOTE: newlines on every 4th component (as in the row vector case)
    // could lead to ambiguity in the matrix case.
    if (row_rtn && i!=m.numRows()-1)
      s << "\n   ";
  }
  if (brackets)
    s << "]] ";
  if (final_rtn)
    s << '\n';
}


/// standard MPI buffer insertion operator for SerialSymDenseMatrix
template <typename OrdinalType, typename ScalarType>
void write_col_vector_trans(std::ostream& s, OrdinalType col,
  OrdinalType num_items, bool brackets, bool break_line,
  bool final_rtn, const Teuchos::SerialDenseMatrix<OrdinalType,ScalarType>& sdm)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  if (brackets)
    s << " [ ";
  for (OrdinalType row=0; row < num_items; ++row) {
    s << std::setw(write_precision+7) << sdm(row,col) << ' ';
    if (break_line && (row+1)%4 == 0)
      s << "\n   "; // Output 4 gradient components per line
  }
  if (brackets)
    s << "] ";
  if (final_rtn)
    s << '\n';
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


/// istream extraction operator for a column vector from a SerialDenseMatrix
template <typename IStreamType, typename OrdinalType, typename ScalarType>
void read_col_vector_trans(IStreamType& s, OrdinalType col,
  Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{ for (OrdinalType row=0; row < sdm.numRows(); ++row) s >> sdm(row,col); }


/// read array from std::istream
template <class ArrayT>
inline void array_read(ArrayT& v, std::istream& s)
{
  typename ArrayT::size_type len = v.size();
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s >> v[i];
}


/// write array to std::ostream
template <class ArrayT>
inline void array_write(const ArrayT& v, std::ostream& s)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  typename ArrayT::size_type len = v.size();
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s << "                     " << std::setw(write_precision+7)
      << v[i] << '\n';
}


/// write list to std::ostream
template <class ListT>
inline void list_write(const ListT& l, std::ostream& s)
{
  BOOST_FOREACH(const typename ListT::value_type& entry, l) {
    s << "                     " << entry << '\n';
  }  
}


/// global std::istream extraction operator for std::vector
template <class T>
inline std::istream& operator>>(std::istream& s, std::vector<T>& data)
{ array_read(data, s); return s; }


/// global std::ostream insertion operator for std::vector
template <class T>
inline std::ostream& operator<<(std::ostream& s, const std::vector<T>& data)
{ array_write(data, s); return s; }


/// global std::ostream insertion operator for std::list
template <class T>
inline std::ostream& operator<<(std::ostream& s, const std::list<T>& data)
{ list_write(data, s); return s; }


/// write array to std::ostream with labels
template <class ArrayT>
inline void array_write(std::ostream& s, const ArrayT& v,
                        const std::vector<String>& label_array)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

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
template <class ArrayT>
inline void array_write_aprepro(std::ostream& s, const ArrayT& v,
                                const std::vector<String>& label_array)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

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
template <class ArrayT>
inline void array_write_annotated(const ArrayT& v, std::ostream& s, bool write_len)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  typename ArrayT::size_type len = v.size();
  if (write_len)
    s << len << ' ';
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s << v[i] << ' ';
}

} // namespace Dakota

#endif // DATA_IO_H
