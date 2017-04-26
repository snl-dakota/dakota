/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DATA_UTIL_H
#define DATA_UTIL_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"  // for Cerr
#include "dakota_data_types.hpp"
#include "pecos_data_types.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash/hash.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <algorithm>
#include "Teuchos_SerialDenseHelpers.hpp"


// --------------
// hash functions
// --------------
namespace boost {

inline size_t hash_value(const Dakota::StringMultiArray& sma)
{ return boost::hash_range( sma.begin(), sma.end() ); }

} // namespace boost

namespace Teuchos {

template<typename OrdinalType, typename ScalarType>
size_t hash_value(const SerialDenseVector<OrdinalType, ScalarType>& sdv)
{ return boost::hash_range( sdv.values(), sdv.values() + sdv.length() ); }

} // namespace Teuchos

namespace Dakota {


// --------------------------------------------
// Equality operator definitions for data types
// --------------------------------------------

/// tolerance-based equality operator for RealVector
bool nearby(const RealVector& rv1, const RealVector& rv2, Real rel_tol);
/// equality operator for IntArray
bool operator==(const IntArray& dia1, const IntArray& dia2);
/// equality operator for ShortArray
bool operator==(const ShortArray& dsa1, const ShortArray& dsa2);
/// equality operator for StringArray
bool operator==(const StringArray& dsa1, const StringArray& dsa2);

/// equality operator for std::vector and boost::multi_array::const_array_view
template <typename T>
bool operator==(const std::vector<T>& vec,
	        typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav)
{
  // Check for equality in array lengths
  size_t len = vec.size();
  if ( mav.size() != len )
    return false;
  // Check each size_t
  for (size_t i=0; i<len; ++i)
    if ( mav[i] != vec[i] )
      return false;
  return true;
}
  
/// equality operator for boost::multi_array::const_array_view and std::vector
template <typename T>
bool operator==(typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav,
		const std::vector<T>& vec)
{ return (vec == mav); } // flip order

/// equality operator for boost::multi_array and
/// boost::multi_array::const_array_view
template <typename T>
bool operator==(const boost::multi_array<T, 1>& ma,
		typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav)
{
  if (ma.size() != mav.size())
    return false;
  // Note: template dependent names require disambiguation
  typename boost::multi_array<T, 1>::template
    const_array_view<1>::type::const_iterator cit1 = mav.begin(),
    end1 = mav.end();
  typename boost::multi_array<T, 1>::const_iterator cit2 = ma.begin();
  for ( ; cit1 != end1; ++cit1, ++cit2)
    if (*cit1 != *cit2)
      return false;
  return true;
}

/// equality operator for boost::multi_array::const_array_view and
/// boost::multi_array
template <typename T>
bool operator==(typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav,
		const boost::multi_array<T, 1>& ma)
{ return (ma == mav); } // flip order


// ----------------------------------------------
// Inequality operator definitions for data types
// ----------------------------------------------

/// inequality operator for IntArray
inline bool operator!=(const IntArray& dia1, const IntArray& dia2)
{ return !(dia1 == dia2); }

/// inequality operator for ShortArray
inline bool operator!=(const ShortArray& dsa1, const ShortArray& dsa2)
{ return !(dsa1 == dsa2); }

/// inequality operator for StringArray
inline bool operator!=(const StringArray& dsa1, const StringArray& dsa2)
{ return !(dsa1 == dsa2); }

/// inequality operator for std::vector and boost::multi_array::const_array_view
template <typename T>
bool operator!=(const std::vector<T>& vec,
		typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav)
{ return !(vec == mav); }

/// inequality operator for boost::multi_array::const_array_view and std::vector
template <typename T>
bool operator!=(typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav,
		const std::vector<T>& vec)
{ return !(vec == mav); } // flip order

/// inequality operator for boost::multi_array and
/// boost::multi_array::const_array_view
template <typename T>
bool operator!=(const boost::multi_array<T, 1>& ma,
		typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav)
{ return !(ma == mav); }

/// inequality operator for boost::multi_array::const_array_view and
/// boost::multi_array
template <typename T>
bool operator!=(typename boost::multi_array<T, 1>::template
		  const_array_view<1>::type mav,
		const boost::multi_array<T, 1>& ma)
{ return !(ma == mav); } // flip order

// ---------------------------------
// miscellaneous numerical utilities
// ---------------------------------

/// Computes relative change between RealVectors using Euclidean L2 norm
Real rel_change_L2(const RealVector& curr_rv, const RealVector& prev_rv);

/// Computes relative change between Real/int/Real vector triples using
/// Euclidean L2 norm
Real rel_change_L2(const RealVector& curr_rv1, const RealVector& prev_rv1,
		   const IntVector&  curr_iv,  const IntVector&  prev_iv,
		   const RealVector& curr_rv2, const RealVector& prev_rv2);
// ---------------------
// Misc matrix utilities 
// ---------------------

/// Removes column from matrix
void remove_column(RealMatrix& matrix, int index);

// -----
// Utility functions for manipulating or searching strings
// -----

/// Return lowercase copy of string s
inline std::string strtolower(const std::string& s)
{ return boost::to_lower_copy(s); }

/// Return true if input string begins with string test
inline bool strbegins(const std::string& input, const std::string& test)
{ return(boost::starts_with(input, test)); }

/// Return true if input string ends with string test
inline bool strends(const std::string& input, const std::string& test)
{ return(boost::ends_with(input, test)); }

/// Return true if input string contains string test
inline bool strcontains(const std::string& input, const std::string& test)
{ return(boost::contains(input, test)); }


// --------------------------------------------
// Utility functions for creating string arrays
// --------------------------------------------

/// create a label by appending a numerical tag to the root_label, o
inline void build_label(String& label, const String& root_label, size_t tag, 
			const String& separator = "")
{
  label = root_label + separator + boost::lexical_cast<std::string>(tag);
}

/// create an array of labels by tagging root_label for each entry in
/// label_array.  Uses build_label().
inline void build_labels(StringArray& label_array, const String& root_label)
{
  size_t len = label_array.size();
  for (size_t i=0; i<len; ++i)
    build_label(label_array[i], root_label, i+1);
}

/// create an array of labels by tagging root_label for each entry in
/// label_array.  Uses build_label().
inline void build_labels(StringMultiArray& label_array,
			 const String& root_label)
{
  size_t len = label_array.size();
  for (size_t i=0; i<len; ++i)
    build_label(label_array[i], root_label, i+1);
}

/// create a partial array of labels by tagging root_label for a subset
/// of entries in label_array.  Uses build_label().
inline void build_labels_partial(StringArray& label_array,
				 const String& root_label, size_t start_index,
				 size_t num_items)
{
  for (size_t i=0; i<num_items; ++i)
    build_label(label_array[start_index+i], root_label, i+1);
}


// ----------------------------
// non-templated copy functions
// ----------------------------

// ------------------------
// templated copy functions
// ------------------------

// Templated conversion functions between/among DAKOTA data types and
// built in/pointer data types


///// copy Array<T> to T*
//template <typename T>
//void copy_data(const std::vector<T>& vec, T* ptr, const size_t ptr_len)
//{
//  if (ptr_len != vec.size()) { // could use <, but enforce exact match
//    Cerr << "Error: bad ptr_len in copy_data(Dakota::Array<T>, T* ptr)."
//	 << std::endl;
//    abort_handler(-1);
//  }
//  for (size_t i=0; i<ptr_len; ++i)
//    ptr[i] = vec[i];
//}
//
//template <typename ScalarType1> //, typename ScalarType1, typename ScalarType2>
//void copy_data(const RealVector& vec, ScalarType1* Sptr, const size_t ptr_len, size_t dummy)
//{
//  if (ptr_len != vec.length()) { // could use <, but enforce exact match
//    Cerr << "Error: bad ptr_len in copy_data(Dakota::Array<T>, T* ptr)."
//	 << std::endl;
//    abort_handler(-1);
//  }
//  for (size_t i=0; i<ptr_len; ++i)
//    Sptr[i] = vec[i];
//}
//
///// copy T* to Array<T>
//template <typename T>
//void copy_data(const T* ptr, const size_t ptr_len, std::vector<T>& vec)
//{
//  if (ptr_len != vec.size())
//    vec.resize(ptr_len);
//  for (size_t i=0; i<ptr_len; ++i)
//    vec[i] = ptr[i];
//}
//
//
///// copy Array<Teuchos::SerialDenseVector<OT,ST> > to ST*
//template <typename OrdinalType1, typename OrdinalType2, typename ScalarType> 
//void copy_data(
//  const std::vector<Teuchos::SerialDenseVector<OrdinalType1, ScalarType> >& va,
//  ScalarType* ptr, const OrdinalType2 ptr_len, const String& ptr_type)
//{
//  bool c_type;
//  if (strtolower(ptr_type) == "c") // case insensitive
//    c_type = true;
//  else if (strtolower(ptr_type) == "fortran") // case insensitive
//    c_type = false;
//  else {
//    Cerr << "Error: invalid ptr_type in copy_data(Dakota::Array<SDV<OT,ST> >, "
//	 << "ST* ptr)" << std::endl;
//    abort_handler(-1);
//  }
//  OrdinalType2 i, j, num_vec = va.size(), total_len = 0, max_vec_len = 0;
//  for (i=0; i<num_vec; ++i) { // loop over vectors in array
//    OrdinalType1 vec_len = va[i].length();
//    total_len += vec_len;
//    if (!c_type && vec_len > max_vec_len)
//      max_vec_len = vec_len;
//  }
//  if (ptr_len != total_len) {
//    Cerr << "Error: bad ptr_len in copy_data(Dakota::Array<Vector<T> >, T* "
//	 << "ptr)." << std::endl;
//    abort_handler(-1);
//  }
//  int cntr = 0;
//  if (c_type) {
//    for (i=0; i<num_vec; ++i) { // loop over rows
//      OrdinalType1 vec_len = va[i].length(); // allowed to vary
//      for (j=0; j<vec_len; ++j) // loop over columns
//        ptr[cntr++] = va[i][j];
//    }
//  }
//  else {
//    for (j=0; j<max_vec_len; ++j) // loop over longest column
//      for (i=0; i<num_vec; ++i) // loop over rows
//	if (j < va[i].length())
//	  ptr[cntr++] = va[i][j];
//  }
//}


/// copy Array<Teuchos::SerialDenseVector<OT,ST> > to
/// Teuchos::SerialDenseMatrix<OT,ST> - used by read_data_tabular - RWH
template <typename OrdinalType, typename ScalarType> 
void copy_data(
  const std::vector<Teuchos::SerialDenseVector<OrdinalType, ScalarType> >& sdva,
        Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{
  OrdinalType i, j, num_vec = sdva.size(), max_vec_len = 0;
  for (i=0; i<num_vec; ++i) { // loop over vectors in array
    OrdinalType vec_len = sdva[i].length();
    if (vec_len > max_vec_len)
      max_vec_len = vec_len;
  }

  // each vector in array becomes a row in the matrix, with shorter
  // rows padded with trailing zeros
  sdm.shape(num_vec, max_vec_len);
  for (i=0; i<num_vec; ++i) {
    const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& vec_i = sdva[i];
    OrdinalType vec_len = vec_i.length(); // allowed to vary
    for (j=0; j<vec_len; ++j)
      sdm(i,j) = vec_i[j];
  }
}


/// copy Array<Teuchos::SerialDenseVector<OT,ST> > to transposed
/// Teuchos::SerialDenseMatrix<OT,ST> - used by read_data_tabular - RWH
template <typename OrdinalType, typename ScalarType> 
void copy_data_transpose(
  const std::vector<Teuchos::SerialDenseVector<OrdinalType, ScalarType> >& sdva,
  Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{
  OrdinalType i, j, num_vec = sdva.size(), max_vec_len = 0;
  for (i=0; i<num_vec; ++i) { // loop over vectors in array
    OrdinalType vec_len = sdva[i].length();
    if (vec_len > max_vec_len)
      max_vec_len = vec_len;
  }

  // each vector in array becomes a column in the matrix, with shorter
  // columns padded with trailing zeros
  sdm.shape(max_vec_len, num_vec);
  for (i=0; i<num_vec; ++i) {
    const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& vec_i = sdva[i];
    OrdinalType vec_len = vec_i.length(); // allowed to vary
    ScalarType* sdm_i = sdm[i]; // ith column
    for (j=0; j<vec_len; ++j)
      sdm_i[j] = vec_i[j];
  }
}


///// copy Teuchos::SerialDenseMatrix<OT,ST> to
///// Array<Teuchos::SerialDenseVector<OT,ST> >
//template <typename OrdinalType, typename ScalarType> 
//void copy_data(
//  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm,
//  std::vector<Teuchos::SerialDenseVector<OrdinalType, ScalarType> >& sdva)
//{
//  OrdinalType i, j, num_vec = sdm.numRows(), vec_len = sdm.numCols();
//  sdva.resize(num_vec);
//  for (i=0; i<num_vec; ++i) {
//    Teuchos::SerialDenseVector<OrdinalType, ScalarType>& vec_i = sdva[i];
//    vec_i.sizeUninitialized(vec_len);
//    for (j=0; j<vec_len; ++j)
//      vec_i[j] = sdm(i,j);
//  }
//}
//
//
///// copy Teuchos::SerialDenseMatrix<OT,ST> to transposed
///// Array<Teuchos::SerialDenseVector<OT,ST> >
//template <typename OrdinalType, typename ScalarType> 
//void copy_data_transpose(
//  const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm,
//  std::vector<Teuchos::SerialDenseVector<OrdinalType, ScalarType> >& sdva)
//{
//  OrdinalType i, j, num_vec = sdm.numCols(), vec_len = sdm.numRows();
//  sdva.resize(num_vec);
//  for (i=0; i<num_vec; ++i) {
//    Teuchos::SerialDenseVector<OrdinalType, ScalarType>& vec_i = sdva[i];
//    const ScalarType* sdm_i = sdm[i]; // ith column
//    vec_i.sizeUninitialized(vec_len);
//    for (j=0; j<vec_len; ++j)
//      vec_i[j] = sdm_i[j];
//  }
//}


/// copy Teuchos::SerialDenseVector<OT,ST> to Teuchos::SerialDenseMatrix<OT,ST> - used by NestedModel::update_sub_iterator - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType> 
void copy_data(const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv,
               Teuchos::SerialDenseMatrix<OrdinalType1, ScalarType>& sdm,
               OrdinalType2 nr, OrdinalType2 nc)
{
  OrdinalType1 size_sdv = sdv.length();

  // This function is set up to do the transformation with either nr or nc or
  // both specified.  To omit nr or nc specification, a 0 is passed.
  if (nr && nc) { // both specified
    if (size_sdv != nr*nc) {
      Cerr << "Error: sdv length (" << size_sdv << ") does not equal nr*nc ("
	   << nr << '*' << nc << ") in copy_data(Teuchos_SerialDenseVector<>, "
	   << "Teuchos_SerialDenseMatrix<>)." << std::endl;
      abort_handler(-1);
    }
  }
  else if (nr) { // only nr is non-zero
    if (size_sdv%nr) {
      Cerr << "Error: sdv length (" << size_sdv << ") not evenly divisible by "
	   << "number of rows (" << nr << ") in copy_data(Teuchos_"
	   << "SerialDenseVector<>, Teuchos_SerialDenseMatrix<>)." << std::endl;
      abort_handler(-1);
    }
    nc = size_sdv/nr;
  }
  else if (nc) { // only nc is non-zero
    if (size_sdv%nc) {
      Cerr << "Error: sdv length (" << size_sdv << ") not evenly divisible by "
	   << "number of columns (" << nc << ") in copy_data(Teuchos_"
	   << "SerialDenseVector<>, Teuchos_SerialDenseMatrix<>)." << std::endl;
      abort_handler(-1);
    }
    nr = size_sdv/nc;
  }
  else { // neither specified
    Cerr << "Error: either nr or nc must be specified in copy_data(Teuchos_"
	 << "SerialDenseVector<>, Teuchos_SerialDenseMatrix<>)." << std::endl;
    abort_handler(-1);
  }

  if (sdm.numRows() != nr || sdm.numCols() != nc)
    sdm.shapeUninitialized(nr, nc);
  // sdv is head to tail by rows, which matches the visual matrix format that 
  // a user would employ in specifying a matrix as a <LISTof><REAL>
  OrdinalType1 counter = 0;
  for (OrdinalType1 i=0; i<nr; ++i)
    for (OrdinalType1 j=0; j<nc; ++j, ++counter)
      sdm(i,j) = sdv[counter];
}

///// copy std::list<T> to std::vector<T>
//template <typename T> 
//void copy_data(const std::list<T>& dl, std::vector<T>& da)
//{
//  size_t size_dl = dl.size();
//  if (size_dl != da.size())
//    da.resize(size_dl);
//  std::copy(dl.begin(), dl.end(), da.begin());
//}
//
///// copy std::list<T> to std::vector<std::vector<T> >
//template <typename T>
//void copy_data(const std::list<T>& dl, std::vector<std::vector<T> >& d2a,
//	       size_t num_a, size_t a_len)
//{
//  size_t i, j, size_dl = dl.entries();
//
//  // This function is set up to do the copy with either num_a or a_len or both
//  // specified.  To omit num_a or a_len specification, a 0 is passed.
//  if (num_a && a_len) { // both specified
//    if (size_dl != num_a*a_len) {
//      Cerr << "Error: dl length (" << size_dl <<") does not equal num_a*a_len ("
//	   << num_a << '*' << a_len << ") in copy_data(std::list<T>, "
//	   << "std::vector<vector<T> >)." << std::endl;
//      abort_handler(-1);
//    }
//  }
//  else if (num_a) { // only num_a is non-zero
//    if (size_dl%num_a) {
//      Cerr << "Error: dl length (" << size_dl << ") not evenly divisible by "
//	   << "number of arrays (" << num_a << ") in copy_data(std::list<T>"
//	   << ", std::vector<vector<T> >)." << std::endl;
//      abort_handler(-1);
//    }
//    a_len = size_dl/num_a;
//  }
//  else if (a_len) { // only a_len is non-zero
//    if (size_dl%a_len) {
//      Cerr << "Error: dl length (" << size_dl << ") not evenly divisible by "
//	   << "array length (" << a_len << ") in copy_data(std::list<T>, "
//	   << "std::vector<vector<T> >)." << std::endl;
//      abort_handler(-1);
//    }
//    num_a = size_dl/a_len;
//  }
//  else { // neither specified
//    Cerr << "Error: either num_a or a_len must be specified in "
//	 << "copy_data(std::list<T>,std::vector<vector<T> >)." << std::endl;
//    abort_handler(-1);
//  }
//
//  if (d2a.size() != num_a)
//    d2a.reshape(num_a);
//  typename std::list<T>::const_iterator dl_cit = dl.begin();
//  for (i=0; i<num_a; ++i) {
//    if (d2a[i].size() != a_len)
//      d2a[i].reshape(a_len);
//    for (j=0; j<a_len; ++j, ++dl_cit)
//      d2a[i][j] = *dl_cit;
//  }
//}

/// copy std::vector<vector<T> > to std::vector<T>(unroll vecOfvecs into vector) - used by ProcessApplicInterface::write_parameters_files - RWH
template <typename T> 
void copy_data(const std::vector<std::vector<T> >& d2a, std::vector<T>& da)
{
  typename std::vector<T>::size_type i, j, size_d2a = 0,
                                     num_arrays = d2a.size(), cntr = 0;
  for (i=0; i<num_arrays; ++i)
    size_d2a += d2a[i].size();
  if (size_d2a != da.size())
    da.resize(size_d2a);
  for (i=0; i<num_arrays; ++i) {
    typename std::vector<T>::size_type array_len = d2a[i].size();
    for (j=0; j<array_len; ++j)
      da[cntr++] = d2a[i][j];
  }
}

/// copy map<int, T> to std::vector<T> (discard integer keys) - used by SurrBasedGlobalMinimizer::core_run - RWH
template <typename T> 
void copy_data(const std::map<int, T>& im, std::vector<T>& da)
{
  size_t i, size_im = im.size();
  if (size_im != da.size())
    da.resize(size_im);
  typename std::map<int, T>::const_iterator im_cit = im.begin();
  for (i=0; i<size_im; ++i, ++im_cit)
    da[i] = im_cit->second;
}

/// copy Teuchos::SerialDenseVector<OrdinalType, ScalarType> to same
/// (used in place of operator= when a deep copy of a vector view is needed) - used by SensAnalysisGlobal::valid_sample_matrix - RWH
template <typename OrdinalType, typename ScalarType> 
void copy_data(const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& sdv1,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& sdv2)
{
  OrdinalType size_sdv1 = sdv1.length();
  if (size_sdv1 != sdv2.length())
    sdv2.sizeUninitialized(size_sdv1);
  for (OrdinalType i=0; i<size_sdv1; ++i)
    sdv2[i] = sdv1[i];
}

/// copy Teuchos::SerialDenseVector<OrdinalType, ScalarType> to
/// std::vector<ScalarType> - used by SurfpackApproximation constructor - RWH
template <typename OrdinalType, typename ScalarType, typename VectorType> 
void copy_data(const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& sdv,
	             VectorType& vec)
{
  OrdinalType size_sdv = sdv.length();
  if (size_sdv > vec.size())
    vec.resize(size_sdv);
  for (OrdinalType i=0; i<size_sdv; ++i)
    vec[i] = sdv[i];
}


/// copy Array<ScalarType> to
/// Teuchos::SerialDenseVector<OrdinalType, ScalarType> - used by NOWPACOptimizer - MSE
template <typename OrdinalType, typename ScalarType> 
void copy_data(const std::vector<ScalarType>& da,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& sdv)
{
 size_t size_da = da.size();
 if (sdv.length() != size_da)
   sdv.sizeUninitialized(size_da);
 for (OrdinalType i=0; i<size_da; ++i)
   sdv[i] = da[i];
}

/// copy ScalarType* to Teuchos::SerialDenseVector<OrdinalType, ScalarType> - used by ScalingModel::response_modify_n2s - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType> 
void copy_data(const ScalarType* ptr, const OrdinalType2 ptr_len,
	       Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv)
{
  if (sdv.length() != ptr_len)
    sdv.sizeUninitialized(ptr_len);
  for (OrdinalType2 i=0; i<ptr_len; ++i)
    sdv[i] = ptr[i];

  //sdv = RealVector(Copy, ptr, ptr_len);
  // not advisable since relying on default operator=.  Would be a good 
  // approach if a reference counting scheme was used for operator=.
}

/// copy ScalarType* to Teuchos::SerialDenseVector<OrdinalType, ScalarType> - used by NL2SOLLeastSq::core_run - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType> 
void copy_data(const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv,
	       ScalarType* ptr, const OrdinalType2 ptr_len)
{
  if (ptr_len != sdv.length()) { // could use <, but enforce exact match
    Cerr << "Error: bad ptr_len in copy_data(Teuchos::SerialDenseVector<>, "
	 << "T* ptr)." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType2 i=0; i<ptr_len; ++i)
    ptr[i] = sdv[i];
}


/// copy SerialDenseVector<> to Array<SerialDenseVector<> > - used by ConcurrentMetaIterator constructor - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void copy_data(const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv,
  std::vector<Teuchos::SerialDenseVector<OrdinalType1, ScalarType> >& sdva,
  OrdinalType2 num_vec, OrdinalType2 vec_len)
{
  // This function is set up to do the transformation with either num_vec or
  // vec_len or both specified.  To omit num_vec or vec_len specification, a 0
  // is passed.
  OrdinalType1 size_sdv = sdv.length();
  if (num_vec && vec_len) { // both specified
    if (size_sdv != num_vec*vec_len) {
      Cerr << "Error: sdv length (" << size_sdv << ") does not equal num_vec*"
	   << "vec_len (" << num_vec << '*' << vec_len << ") in copy_data("
	   << "Teuchos::SerialDenseVector<>, Dakota::Array<Teuchos::"
	   << "SerialDenseVector<> >)." << std::endl;
      abort_handler(-1);
    }
  }
  else if (num_vec) { // only num_vec is non-zero
    if (size_sdv%num_vec) {
      Cerr << "Error: sdv length (" << size_sdv << ") not evenly divisible by "
	   << "number of vectors (" << num_vec << ") in copy_data("
	   << "Teuchos::SerialDenseVector<>, Dakota::Array<Teuchos::"
	   << "SerialDenseVector<> >)." << std::endl;
      abort_handler(-1);
    }
    vec_len = size_sdv/num_vec;
  }
  else if (vec_len) { // only vec_len is non-zero
    if (size_sdv%vec_len) {
      Cerr << "Error: sdv length (" << size_sdv << ") not evenly divisible by "
	   << "vector length (" << vec_len << ") in copy_data(Teuchos::"
	   << "SerialDenseVector<>, Dakota::Array<Teuchos::"
	   << "SerialDenseVector<> >)." << std::endl;
      abort_handler(-1);
    }
    num_vec = size_sdv/vec_len;
  }
  else { // neither specified
    Cerr << "Error: either num_vec or vec_len must be specified in "
	 << "copy_data(Teuchos::SerialDenseVector<>, Dakota::Array<Teuchos::"
	 << "SerialDenseVector<> >)." << std::endl;
    abort_handler(-1);
  }

  if (sdva.size() != num_vec)
    sdva.resize(num_vec);
  // sdv is head to tail by rows, which matches the visual matrix format that 
  // a user would employ in specifying a matrix as a <LISTof><REAL>
  OrdinalType2 counter = 0;
  for (OrdinalType2 i=0; i<num_vec; ++i) {
    if (sdva[i].length() != vec_len)
      sdva[i].sizeUninitialized(vec_len);
    for (OrdinalType2 j=0; j<vec_len; ++j)
      sdva[i][j] = sdv[counter++];
  }
}


// Partial copy functions

/// copy a portion arbitrary vector to all of another arbitrary vector
template <typename vecType1, typename vecType2>
void copy_data_partial(
  const vecType1& source,
        vecType2& target,
	size_t start_idx,
        size_t len)
{
  // This requires that the target type supports tghe size() method, which RealVectors don't
  //if( len != target.size()) { // could use <, but enforce exact match
  //  Cerr << "Error: bad target vector length copy_data_partial." << std::endl;
  //  abort_handler(-1);
  //}
  for( size_t i=0; i<len; ++i)
    target[i] = source[i+start_idx];
}

/// copy portion of first SerialDenseVector to all of second SerialDenseVector - used by DataTransformModel::vars_mapping - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void copy_data_partial(
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv1,
  OrdinalType2 start_index1, OrdinalType2 num_items,
  Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv2)
{
  // sdv1 will be indexed from start_index1 to start_index1+num_items-1
  if (start_index1 + num_items > sdv1.length()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial("
	 << "Teuchos::SerialDenseVector<OrdinalType, ScalarType>, size_t, "
	 << "size_t, Teuchos::SerialDenseVector<OrdinalType, ScalarType>)."
	 << std::endl;
    abort_handler(-1);
  }
  if (num_items != sdv2.length())
    sdv2.sizeUninitialized(num_items);
  for (OrdinalType2 i=0; i<num_items; ++i)
    sdv2[i] = sdv1[start_index1+i];
}

/// copy all of first SerialDenseVector to portion of second SerialDenseVector - used by MixedVariables - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void copy_data_partial(
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv1,
  Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv2,
  OrdinalType2 start_index2)
{
  OrdinalType1 num_items = sdv1.length();
  // In this case, incoming sdv2 must already be sized and will be
  // indexed from start_index2 to start_index2+num_items-1
  if (start_index2 + num_items > sdv2.length()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial("
	 << "Teuchos::SerialDenseVector<OrdinalType, ScalarType>, "
	 << "Teuchos::SerialDenseVector<OrdinalType, ScalarType>, OrdinalType)."
	 << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType1 i=0; i<num_items; ++i)
    sdv2[start_index2+i] = sdv1[i];
}

/// copy portion of first SerialDenseVector to portion of second
/// SerialDenseVector - used by ScalingModel::secondary_resp_scaled2native - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void copy_data_partial(
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv1,
  OrdinalType2 start_index1, OrdinalType2 num_items,
  Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv2,
  OrdinalType2 start_index2)
{
  // In this case, incoming sdv2 must already be sized and will be
  // indexed from start_index2 to start_index2+num_items-1.  sdv1 will
  // be indexed from start_index1 to start_index1+num_items-1
  if (start_index1 + num_items > sdv1.length() ||
      start_index2 + num_items > sdv2.length()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial("
	 << "Teuchos::SerialDenseVector<OrdinalType, ScalarType>, OrdinalType, "
	 << "OrdinalType, Teuchos::SerialDenseVector<OrdinalType, ScalarType>, "
	 << "OrdinalType)." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType2 i=0; i<num_items; ++i)
    sdv2[start_index2+i] = sdv1[start_index1+i];
}

/// copy all of first SerialDenseVector to portion of second SerialDenseVector - used by SharedSurfpackApproxData::merge_variable_arrays - RWH
template <typename OrdinalType1, typename OrdinalType2, typename ScalarType>
void copy_data_partial(
  const Teuchos::SerialDenseVector<OrdinalType1, ScalarType>& sdv1,
  std::vector<ScalarType>& da2, OrdinalType2 start_index2)
{
  OrdinalType1 num_items = sdv1.length();
  // In this case, incoming da2 must already be sized and will be
  // indexed from start_index2 to start_index2+num_items-1
  if (start_index2 + num_items > da2.size()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial(Teuchos::"
	 << "SerialDenseVector<OrdinalType, ScalarType>, "
	 << "std::vector<ScalarType>, OrdinalType)." << std::endl;
    abort_handler(-1);
  }
  for (OrdinalType1 i=0; i<num_items; ++i)
    da2[start_index2+i] = sdv1[i];
}

/// copy portion of first Array<T> to all of second Array<T> - used by SharedResponseDataRep constructor - RWH
template <typename T>
void copy_data_partial(const std::vector<T>& da1, size_t start_index1,
		       size_t num_items, std::vector<T>& da2)
{
  // da1 will be indexed from start_index1 to start_index1+num_items-1
  if (start_index1 + num_items > da1.size()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial("
	 << "Dakota::Array<T>, size_t, size_t, Dakota::Array<T>)." << std::endl;
    abort_handler(-1);
  }
  if (num_items != da2.size())
    da2.resize(num_items);
  for (size_t i=0; i<num_items; ++i)
    da2[i] = da1[start_index1+i];
}

/// copy all of first Array<T> to portion of second Array<T> - used by ParamStudy::multidim_loop - RWH
template <typename T>
void copy_data_partial(const std::vector<T>& da1, std::vector<T>& da2,
                       size_t start_index2)
{
  size_t i, num_items = da1.size();
  // In this case, incoming da2 must already be sized and will be
  // indexed from start_index2 to start_index2+num_items-1
  if (start_index2 + num_items > da2.size()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial("
	 << "Dakota::Array<T>, Dakota::Array<T>, size_t)." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<num_items; ++i)
    da2[start_index2+i] = da1[i];
}

/// copy all of first Array<T> to portion of boost::multi_array<T, 1> - used by RelaxedVariables - RWH
template <typename T>
void copy_data_partial(const std::vector<T>& da, boost::multi_array<T, 1>& bma,
		       size_t start_index_bma)
{
  size_t i, num_items = da.size();
  // In this case, incoming bma must already be sized and will be
  // indexed from start_index_bma to start_index_bma+num_items-1
  if (start_index_bma + num_items > bma.size()) {
    Cerr << "Error: indexing out of bounds in copy_data_partial("
	 << "Dakota::Array<T>, boost::multi_array<T, 1>, size_t)." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<num_items; ++i)
    bma[start_index_bma+i] = da[i];
}

///// copy portion of first Array<T> to portion of second Array<T>
//template <typename T>
//void copy_data_partial(const std::vector<T>& da1, size_t start_index1,
//		       size_t num_items,std::vector<T>& da2,size_t start_index2)
//{
//  // In this case, incoming da2 must already be sized and will be
//  // indexed from start_index2 to start_index2+num_items-1.  da1 will
//  // be indexed from start_index1 to start_index1+num_items-1
//  if (start_index1 + num_items > da1.size() ||
//      start_index2 + num_items > da2.size()) {
//    Cerr << "Error: indexing out of bounds in copy_data_partial(Dakota::"
//	 << "Array<T>, size_t, size_t, Dakota::Array<T>, size_t)." << std::endl;
//    abort_handler(-1);
//  }
//  for (size_t i=0; i<num_items; ++i)
//    da2[start_index2+i] = da1[start_index1+i];
//}

/// Copies a row of a Teuchos_SerialDenseMatrix<int,Real> to std::vector<Real>
template<typename VectorType>
void copy_row_vector(const RealMatrix& m, RealMatrix::ordinalType i,
			   VectorType& row)
{
  RealMatrix::ordinalType j, num_items = m.numCols();
  if (row.size() != num_items)
    row.resize(num_items);
  for(j=0; j<num_items; ++j)
    row[j] = m(i,j);
}


/// Copies a row of a Teuchos_SerialDenseMatrix<int,Real> to std::vector<Real>
template<typename ScalarType>
void insert_row_vector(const std::vector<ScalarType>& row, 
                             RealMatrix::ordinalType i,
                             RealMatrix& m)
{
  if( (m.numRows() < i+1) || (m.numCols() != row.size()) )
    m.reshape(i+1, row.size());
  for( size_t j=0; j<row.size(); ++j)
    m(i,j) = row[j];
}


// ----------------------------------------------------
// Non-templated functions for creating relaxed vectors
// ----------------------------------------------------

/// merge a discrete integer vector into a single continuous vector
inline void merge_data_partial(const IntVector& d_vec,
			       RealVector& m_vec, size_t start_index_ma)
{
  size_t i, num_items = d_vec.length();
  // In this case, incoming m_vec must already be sized and will be
  // indexed from start_index_ma to start_index_ma+num_items-1
  size_t m_vec_len = m_vec.length();
  if (start_index_ma + num_items > m_vec_len) {
    Cerr << "Error: indexing out of bounds in merge_data_partial(IntVector, "
	 << "RealVector, size_t)." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<num_items; ++i)
    m_vec[start_index_ma+i] = (Real)d_vec[i];
}

/// merge a discrete integer vector into a single continuous array
inline void merge_data_partial(const IntVector& d_vec,
			       RealArray& m_array, size_t start_index_ma)
{
  size_t i, num_items = d_vec.length();
  // In this case, incoming m_array must already be sized and will be
  // indexed from start_index_ma to start_index_ma+num_items-1
  if (start_index_ma + num_items > m_array.size()) {
    Cerr << "Error: indexing out of bounds in merge_data_partial(IntVector, "
	 << "RealArray, size_t)." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<num_items; ++i)
    m_array[start_index_ma+i] = (Real)d_vec[i];
}


/// round entries of a RealVector yielding an IntVector
void iround(const RealVector& input_vec, IntVector& rounded_vec);


// -------------------------------
// templated set utility functions
// -------------------------------

/// retrieve the set value corresponding to the passed index
template <typename OrdinalType, typename ScalarType>
const ScalarType& set_index_to_value(OrdinalType index,
				     const std::set<ScalarType>& values)
{
  // TO DO: conditional activation for automatic bounds checking
  if (index < 0 || index >= values.size()) {
    Cerr << "\nError: index out of range in set_index_to_value()" << std::endl;
    abort_handler(-1);
  }
  typename std::set<ScalarType>::const_iterator cit = values.begin();
  std::advance(cit, index);
  return *cit;
}


/// calculate the set index corresponding to the passed value
template <typename ScalarType>
size_t set_value_to_index(const ScalarType& value,
			  const std::set<ScalarType>& values)
{
  typename std::set<ScalarType>::const_iterator cit = values.find(value);
  return (cit == values.end()) ? _NPOS : std::distance(values.begin(), cit);

  // linear search provides index in one pass, but find() plus distance()
  // should be faster for sorted associative containers
  //size_t index = 0;
  //typename std::set<ScalarType>::const_iterator cit;
  //for (cit=values.begin(); cit!=values.end(); ++cit, ++index)
  //  if (*cit == value)
  //    return index;
  //return _NPOS;
}


/// retrieve the set value corresponding to the passed index
template <typename OrdinalType, typename KeyType, typename ValueType>
const KeyType& map_index_to_key(OrdinalType index,
				const std::map<KeyType, ValueType>& pairs)
{
  // TO DO: conditional activation for automatic bounds checking
  if (index < 0 || index >= pairs.size()) {
    Cerr << "\nError: index out of range in map_index_to_key()" << std::endl;
    abort_handler(-1);
  }
  typename std::map<KeyType, ValueType>::const_iterator cit = pairs.begin();
  std::advance(cit, index);
  return cit->first;
}


/// retrieve the set value corresponding to the passed index
template <typename OrdinalType, typename KeyType, typename ValueType>
const ValueType& map_index_to_value(OrdinalType index,
				    const std::map<KeyType, ValueType>& pairs)
{
  // TO DO: conditional activation for automatic bounds checking
  if (index < 0 || index >= pairs.size()) {
    Cerr << "\nError: index out of range in map_index_to_value()" << std::endl;
    abort_handler(-1);
  }
  typename std::map<KeyType, ValueType>::const_iterator cit = pairs.begin();
  std::advance(cit, index);
  return cit->second;
}


/// calculate the map index corresponding to the passed key
template <typename KeyType, typename ValueType>
void map_keys_to_set(const std::map<KeyType, ValueType>& source_map,
		     std::set<KeyType>& target_set)
{
  target_set.clear();
  typename std::map<KeyType, ValueType>::const_iterator cit;
  for (cit=source_map.begin(); cit!=source_map.end(); ++cit)
    target_set.insert(cit->first);
}


/// calculate the map index corresponding to the passed key
template <typename KeyType, typename ValueType>
size_t map_key_to_index(const KeyType& key,
			const std::map<KeyType, ValueType>& pairs)
{
  typename std::map<KeyType, ValueType>::const_iterator cit = pairs.find(key);
  return (cit == pairs.end()) ? _NPOS : std::distance(pairs.begin(), cit);

  // linear search provides index in one pass, but find() plus distance()
  // should be faster for sorted associative containers
  //size_t index = 0;
  //typename std::set<ScalarType>::const_iterator cit;
  //for (cit=values.begin(); cit!=values.end(); ++cit, ++index)
  //  if (*cit == value)
  //    return index;
  //return _NPOS;
}


/// convert a SerialDenseVector of head-to-tail (x,y) pairs into a
/// std::set of (x), discarding the y values
template <typename OrdinalType, typename ScalarType>
void x_y_pairs_to_x_set(
  const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& xy_pairs,
  std::set<ScalarType>& x_set)
{
  x_set.clear();
  OrdinalType i, num_pairs = xy_pairs.length()/2;
  for (i=0; i<num_pairs; ++i)
    x_set.insert(xy_pairs[2*i]);
}


// ---------------------------------
// templated array utility functions
// ---------------------------------
#if defined(_MSC_VER)
// MSE: this may be too generic and could hide special cases:
//      can we rely on partial template specialization?

/// generic find_index (inactive)
template <typename ContainerType>
size_t find_index(const ContainerType& c,
		  const typename ContainerType::value_type& search_data)
{
  // should be more efficient than find() + distance()
  size_t cntr = 0;
  BOOST_FOREACH(const typename ContainerType::value_type& entry, c) {
    if (entry == search_data)
      return cntr;
    else
      ++cntr;
  }
  return _NPOS;
}


///// generic copy (inactive)
//template <typename MultiArrayType, typename DakArrayType>
//void copy_data(const MultiArrayType& ma, DakArrayType& da)
//{
//  size_t size_ma = ma.size();
//  if (size_ma != da.size())
//    da.resize(size_ma);
//  for (size_t i=0; i<size_ma; ++i)
//    da[i] = ma[i];
//}


#else
/// compute the index of an entry within a boost::multi_array
template <typename T> 
size_t find_index(const boost::multi_array<T, 1>& bma, const T& search_data)
{
  // should be more efficient than find() + distance()
  size_t i, len = bma.size();
  for (i=0; i<len; i++)
    if (bma[i] == search_data)
      return i;
  return _NPOS;
}


//template <typename T> 
//size_t find_index(boost::multi_array<T, 1>::const_array_view<1>::type bmav,
//	            const T& search_data)
// TO DO: difficulty with compilation of ::type --> work-around by enumeration


/// compute the index of an entry within a boost::multi_array view
inline size_t find_index(SizetMultiArrayConstView bmacv, size_t search_data)
{
  // should be more efficient than find() + distance()
  size_t len = bmacv.size();
  for (size_t i=0; i<len; i++)
    if (bmacv[i] == search_data)
      return i;
  return _NPOS;
}


/// compute the index of an entry within a boost::multi_array view
inline size_t find_index(StringMultiArrayConstView bmacv,
			 const String& search_data)
{
  // should be more efficient than find() + distance()
  size_t len = bmacv.size();
  for (size_t i=0; i<len; i++)
    if (bmacv[i] == search_data)
      return i;
  return _NPOS;
}


/// compute the index of an entry within a std::list
template <typename ListT>
size_t find_index(const ListT& l, const typename ListT::value_type& val)
{
  // should be more efficient than find() + distance()
  size_t cntr = 0;
  BOOST_FOREACH(const typename ListT::value_type& entry, l) {
    if (entry == val)
      return cntr;
    else
      ++cntr;
  }
  return _NPOS;
}


// copy MultiArrayView<T> to Array<T>
//template <typename T>
//void copy_data(boost::multi_array<T, 1>::const_array_view<1>::type ma,
//	         Array<T>& da)
// TO DO: difficulty with compilation of ::type --> work-around by enumeration


/// return an iterator to the first list element satisfying the
/// predicate test_fn w.r.t. the passed test_fn_data; end if not found
template <typename ListT>
typename ListT::const_iterator
find_if(const ListT& c,
        bool (*test_fn)(const typename ListT::value_type&, const std::string&),
        const std::string& test_fn_data)
{
  // a hand-coded list traversal is faster than the Functor approach
  for (typename ListT::const_iterator it = c.begin(); it != c.end(); ++it)
    if (test_fn(*it, test_fn_data))
      return it;
  return c.end();

  // while only a single list traversal is needed with find_if() in this case,
  // it is still a fair amount slower, presumably due to Functor overhead.
  //return find_if(begin(), end(), FunctionCompare<T>(test_fn, test_fn_data));
}

#endif

// copy std::vector<VecType> to Real*
// VectorType must support the length() method. 
template<typename VectorType, typename ScalarType>
void copy_data(const std::vector<VectorType>& va,
               ScalarType * ptr,
               int ptr_len)
{
  size_t total_len=0, cntr=0, num_vec = va.size();
  for( size_t i=0; i<num_vec; ++i)
    total_len += va[i].length();
  if (total_len != ptr_len) {
    Cerr << "copy_data Error: pointer allocation (" << ptr_len << ") does not equal "
	 << "total std::vector<VecType> length (" << total_len << ")." << std::endl;
    abort_handler(-1);
  }
  for( size_t i=0; i<num_vec; ++i)
  {
    int vec_len = va[i].length();
    for(int j=0; j<vec_len; ++j)
      ptr[cntr++] = va[i][j];
  }
}

/// copy boost::multi_array view to Array - used by ActiveSet::derivative_vector - RWH
inline void copy_data(SizetMultiArrayConstView ma, SizetArray& da)
{
  size_t size_ma = ma.size();
  if (size_ma != da.size())
    da.resize(size_ma);
  for (size_t i=0; i<size_ma; ++i)
    da[i] = ma[i];
}


/// copy boost::multi_array view to Array - used by Pecos::copy_data - RWH
inline void copy_data(StringMultiArrayConstView ma, StringArray& da)
{
  size_t size_ma = ma.size();
  if (size_ma != da.size())
    da.resize(size_ma);
  for (size_t i=0; i<size_ma; ++i)
    da[i] = ma[i];
}


/// return true if the item val appears in container v
template <typename DakContainerType>
inline bool contains(const DakContainerType& v,
                     const typename DakContainerType::value_type& val)
{
  return ( std::find(v.begin(), v.end(), val) != v.end() ) ? true : false;
}


} // namespace Dakota

/// return true if the string contains a floating point value
inline bool isfloat(const Dakota::String token) {
  static boost::regex float_regex("[\\+-]?[0-9]*\\.?[0-9]+\\.?[0-9]*[eEdD]?[\\+-]?[0-9]*|[Nn][Aa][Nn]|[\\+-]?[Ii][Nn][Ff](?:[Ii][Nn][Ii][Tt][Yy])?");
  return boost::regex_match(token, float_regex);
}


#endif // DATA_UTIL_H
