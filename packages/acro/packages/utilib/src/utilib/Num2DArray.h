/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file Num2DArray.h
 *
 * Defines the utilib::Num2DArray class, which is not actively used.
 */

#ifndef utilib_Num2DArray_h
#define utilib_Num2DArray_h

#include <utilib_config.h>
#include <utilib/NumArray.h>
#include <utilib/Simple2DArray.h>

namespace utilib {

template <class T>
class Num2DArray;


/// Perform matrix-matrix multiplication
template <class T>
Num2DArray<T> operator%    (const Num2DArray<T> & m1, const Num2DArray<T> & m2)
{
static Num2DArray<T> temp_mat(m1.nrows(), m2.ncols());
matmult(temp_mat, m1, m2);
return temp_mat;
}
 

/// Perform matrix-vector multiplication
template <class T>
NumArray<T> operator%    (const Num2DArray<T> & m, const NumArray<T> & v)
{
static NumArray<T> temp_vec(m.nrows());
matmult(temp_vec, m, v);
return temp_vec;
}
 

/// Perform vector-matrix multiplication
template <class T>
NumArray<T> operator%    (const NumArray<T> & v, const Num2DArray<T> & m)
{
static NumArray<T> temp_vec(m.ncols());
matmult(temp_vec,v,m);
return temp_vec;
}
 
 
/// Perform matrix-matrix multiplication
template <class T>
void matmult(Num2DArray<T>& res, const Num2DArray<T> & m1, 
					const Num2DArray<T> & m2)
{
if (m1.ncols() != m2.nrows())
   EXCEPTION_MNGR(runtime_error, "matmult: ncols(M1) != nrows(M2) " << m1.ncols() << " " << m2.nrows());
if ((res.nrows() != m1.nrows()) || (res.ncols() != m2.ncols()))
   EXCEPTION_MNGR(runtime_error, "matmult: bad dimensions for result.");
 
T temp;
size_type M1Ncols = m1.ncols();
size_type M1Nrows = m1.nrows();
size_type M2Ncols = m2.ncols();
for (size_type i=0; i<M1Nrows; i++){
  for (size_type j=0; j<M2Ncols; j++){
    temp = m1[i][0] * m2[0][j];
    for (size_type k=1; k<M1Ncols; k++){
      temp += m1[i][k] * m2[k][j];
      }
    res[i][j] = temp;
    }
  }
}
 

/// Perform matrix-vector multiplication
template <class T>
void matmult(NumArray<T>& res, const Num2DArray<T>& m, const NumArray<T>& v)
{
if (m.ncols() != v.size())
   EXCEPTION_MNGR(runtime_error, "matmult: ncols(M) != size(V)" );
if (res.size() != m.nrows())
   EXCEPTION_MNGR(runtime_error, "matmult: size(RES) != nrows(M)" );
 
T temp;
size_type NRows = m.nrows();
size_type NCols = v.size();
for (size_type i=0; i<NRows; i++) {
  temp = m[i][0] * v[0];
  for (size_type j=1; j<NCols; j++)
    temp += m[i][j] * v[j];
  res[i] = temp;
  }
}
 
 
/// Perform vector-matrix multiplication
template <class T>
void matmult(NumArray<T>& res, const NumArray<T> & v, const Num2DArray<T> & m)
{
if (m.nrows() != v.size())
   EXCEPTION_MNGR(runtime_error, "matmult: size(V) != nrows(M)" );
if (res.size() != m.ncols())
   EXCEPTION_MNGR(runtime_error, "matmult: size(RES) != ncols(M)" );
 
T temp;
size_type NRows = v.size();
size_type NCols = m.ncols();
for (size_type i=0; i<NCols; i++) {
  temp = m[0][i] * v[0];
  for (size_type j=1; j<NRows; j++)
    temp += m[j][i] * v[j];
  res[i] = temp;
  }
}



/**
 * A 2D array data type for numerical data.  Num2DArray extends the
 * definition of Simple2DArray to include numerical array operations.
 */
template <class T>
class Num2DArray: public Simple2DArray<T>
{
public:

  #if !defined(DOXYGEN)
  typedef Simple2DArray<T> base_t;
  #if !defined(UTILIB_SOLARIS_CC)
  using base_t::a;
  #endif
  #endif

	/// Empty constructor.
  Num2DArray()  : Simple2DArray<T>() {}
	/**
	 * Constructor that initializes the 2D array using a given array.
	 * The 2D array only constructs the array of pointers, which point
	 * into the data owned by the \c array object.
	 */
  Num2DArray(const BasicArray<T>& array, const int nrows=1,
                const EnumDataOwned own=DataNotOwned)
		: Simple2DArray<T>(array,nrows,own) {}
	/**
	 * Constructor that initializes the 2D array using a given array.
	 * The 2D array only constructs the array of pointers, which point
	 * into the data owned by the \c d object.
	 */
  Num2DArray(const int nrows, const int ncols, T *d=((T*)0),
                const EnumDataOwned own=DataNotOwned)
		: Simple2DArray<T>(nrows,ncols,d,own) {}
	/**
	 * Constructor that initializes the 2D array using a given array.
	 * The 2D array only constructs the array of pointers, which point
	 * into the data owned by the \c array object.
	 */
  Num2DArray(const int nrows, const int ncols, const BasicArray<T>& array,
                const EnumDataOwned own=DataNotOwned)
		: Simple2DArray<T>(nrows,ncols,array,own) {}
	/// Copy constructor.
  Num2DArray(const Num2DArray& array)
		: Simple2DArray<T>(array) {}

	/// Copies the \b array object by constructing a new \b Num2DArray
  Num2DArray<T>& operator=(const Num2DArray<T>& array)
                {Simple2DArray<T>::operator=(array); return *this;}

  virtual ~Num2DArray<T>() {}

	/// Sets the array equal to the pair-wise value \a x + \a y
  void plus  (const Num2DArray<T>& x, const Num2DArray<T>& y);
	/// Sets the array equal to the pair-wise value \a x - \a y
  void minus (const Num2DArray<T>& x, const Num2DArray<T>& y);
	/// Sets the array equal to the pair-wise value \a x * \a y
  void times (const Num2DArray<T>& x, const Num2DArray<T>& y);
	/// Sets the array equal to the pair-wise value \a x / \a y
  void divide(const Num2DArray<T>& x, const Num2DArray<T>& y);

	/// Row-wise operations to set i-th row equal to \a x[i] + \a y
  void plus  (const Num2DArray<T>& x, const NumArray<T>& y);
	/// Row-wise operations to set i-th row equal to \a x[i] - \a y
  void minus (const Num2DArray<T>& x, const NumArray<T>& y);
	/// Row-wise operations to set i-th row equal to \a x[i] * \a y
  void times (const Num2DArray<T>& x, const NumArray<T>& y);
	/// Row-wise operations to set i-th row equal to \a x[i] / \a y
  void divide(const Num2DArray<T>& x, const NumArray<T>& y);

	/// Element-wise operations to add the value z
  void plus  (const Num2DArray<T>& x, const T z);
	/// Element-wise operations to subtract the value z
  void minus (const Num2DArray<T>& x, const T z);
	/// Element-wise operations to multiply the value z
  void times (const Num2DArray<T>& x, const T z);
	/// Element-wise operations to divide the value z
  void divide(const Num2DArray<T>& x, const T z);

	/// Changes the sign of elements in the array.
  Num2DArray<T> operator-    ();
 
	/// Adds the value of \a x element-wise to the current array.
  Num2DArray<T>& operator+= (const Num2DArray<T>& x);
	/// Subtracts the value of \a x element-wise to the current array.
  Num2DArray<T>& operator-= (const Num2DArray<T>& x);
	/// Multiplies the value of \a x element-wise to the current array.
  Num2DArray<T>& operator*= (const Num2DArray<T>& x);
	/// Divides the value of \a x element-wise to the current array.
  Num2DArray<T>& operator/= (const Num2DArray<T>& x);

	/// Adds \a element-wise to the rows of the current-array
  Num2DArray<T>& operator+=  (const NumArray<T>& y);
	/// Subtracts \a element-wise to the rows of the current-array
  Num2DArray<T>& operator-=  (const NumArray<T>& y);
	/// Multiplies \a element-wise to the rows of the current-array
  Num2DArray<T>& operator*=  (const NumArray<T>& y);
	/// Divides \a element-wise to the rows of the current-array
  Num2DArray<T>& operator/=  (const NumArray<T>& y);

	/// Adds \a z to the elements of the current array
  Num2DArray<T>& operator+=  (const T z);
	/// Subtracts \a z to the elements of the current array
  Num2DArray<T>& operator-=  (const T z);
	/// Multiplies \a z to the elements of the current array
  Num2DArray<T>& operator*=  (const T z);
	/// Divides \a z to the elements of the current array
  Num2DArray<T>& operator/=  (const T z);

  #if !defined(DOXYGEN)
  friend void matmult<>(Num2DArray<T>& res, const Num2DArray<T>&, 
				const Num2DArray<T>&);
  friend void matmult<>(NumArray<T>& res, const Num2DArray<T>&, 
				const NumArray<T>&);
  friend void matmult<>(NumArray<T>& res, const NumArray<T>&, 
				const Num2DArray<T>&);
  friend Num2DArray<T> operator%<>(const Num2DArray<T> &, const Num2DArray<T> &);
  friend NumArray<T> operator%<>(const Num2DArray<T> &, const NumArray<T> &);
  friend NumArray<T> operator%<>(const NumArray<T> &, const Num2DArray<T> &);
  #endif

};
 

#if !defined(DOXYGEN)

#define BINARYOP(opname,pseudonym)\
template <class T>\
inline Num2DArray<T> opname (const Num2DArray<T>& a1, const Num2DArray<T>& a2)\
{\
Num2DArray<T> res;\
res.pseudonym(a1,a2);\
return res;\
}\
\
template <class T>\
inline Num2DArray<T> opname (const Num2DArray<T>& a1, const NumArray<T>& val)\
{\
Num2DArray<T> res;\
res.pseudonym(a1,val);\
return res;\
}\
\
template <class T>\
inline Num2DArray<T> opname (const NumArray<T>& val, const Num2DArray<T>& a1)\
{\
Num2DArray<T> res;\
res.pseudonym(a1,val);\
return res;\
}\
\
template <class T>\
inline Num2DArray<T> opname (const Num2DArray<T>& a1, const T val)\
{\
Num2DArray<T> res;\
res.pseudonym(a1,val);\
return res;\
}\
\
template <class T>\
inline Num2DArray<T> opname (const T& val, const Num2DArray<T>& a1)\
{\
Num2DArray<T> res;\
res.pseudonym(a1,val);\
return res;\
}

BINARYOP( operator+ , plus )
BINARYOP( operator- , minus )
BINARYOP( operator* , times )
BINARYOP( operator/ , divide )

#undef BINARYOP


#define BINARYOP(opname,op1,pseudonym, op)\
template <class T>\
void Num2DArray<T>::pseudonym(const Num2DArray<T>& a1, const Num2DArray<T>& a2)\
{\
if ((a1.nrows() != a2.nrows()) || (a1.ncols() != a2.ncols()))\
   EXCEPTION_MNGR(runtime_error, "Num2DArray<T>::pseudonym : Unequal 2D array sizes " << a1.nrows() << "x" << a1.ncols() << " and " << a2.nrows() << "x" << a2.ncols());\
resize(a1.nrows(),a2.ncols());\
for (size_type i=0; i<a->Nrows; i++)\
  for (size_type j=0; j<a->Ncols; j++)\
    a->Data[i][j] = a1[i][j] op a2[i][j];\
}\
\
template <class T>\
void Num2DArray<T>::pseudonym(const Num2DArray<T>& a1, const NumArray<T>& array)\
{\
if (a1.ncols() != array.size())\
   EXCEPTION_MNGR(runtime_error, "Num2DArray<T>::pseudonym : Number of columns " << a1.ncols() << " does not equal array length " << array.size());\
resize(a1.nrows(),a1.ncols());\
for (size_type i=0; i<a->Nrows; i++)\
  for (size_type j=0; j<a->Ncols; j++)\
    a->Data[i][j] = a1[i][j] op array[j];\
}\
\
template <class T>\
void Num2DArray<T>::pseudonym(const Num2DArray<T>& a1, const T val)\
{\
resize(a1.nrows(),a1.ncols());\
for (size_type i=0; i<a->Nrows; i++)\
  for (size_type j=0; j<a->Ncols; j++)\
    a->Data[i][j] = a1[i][j] op val;\
}\
\
template <class T>\
Num2DArray<T>& Num2DArray<T>::opname(const Num2DArray<T>& a1)\
{\
if ((base_t::nrows() != a1.nrows()) || (base_t::ncols() != a1.ncols()))\
   EXCEPTION_MNGR(runtime_error, "Num2DArray<T>::opname : Unequal 2D array sizes " << base_t::nrows() << "x" << base_t::ncols() << " and " << a1.nrows() << "x" << a1.ncols());\
for (size_type i=0; i<base_t::nrows(); i++)\
  for (size_type j=0; j<base_t::ncols(); j++)\
    a->Data[i][j] op1 a1.a->Data[i][j];\
return *this;\
}\
\
template <class T>\
Num2DArray<T>& Num2DArray<T>::opname(const NumArray<T>& a1)\
{\
if (base_t::ncols() != a1.size())\
   EXCEPTION_MNGR(runtime_error, "Num2DArray<T>::opname : Number of columns " << base_t::ncols() << " does not equal array length " << a1.size());\
for (size_type i=0; i<base_t::nrows(); i++)\
  for (size_type j=0; j<base_t::ncols(); j++)\
    a->Data[i][j] op1 a1.data()[j];\
return *this;\
}\
\
template <class T>\
Num2DArray<T>& Num2DArray<T>::opname(const T val)\
{\
for (size_type i=0; i<base_t::nrows(); i++)\
  for (size_type j=0; j<base_t::ncols(); j++)\
    a->Data[i][j] op1 val;\
return *this;\
}

BINARYOP(operator+=,+=,plus, + )
BINARYOP(operator-=,-=,minus, - )
BINARYOP(operator*=,*=,times, * )
BINARYOP(operator/=,/=,divide, / )

#undef BINARYOP

#endif

template <class T>
Num2DArray<T> Num2DArray<T>::operator-    ()
{
Num2DArray<T> res;
for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    res.a->Data[i][j] = - a->Data[i][j];
return res;
}

} // namespace utilib

#endif
