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
 * \file math_array.h
 *
 * Declarations for functions defined for arrays.
 */

#ifndef utilib_math_array_h
#define utilib_math_array_h

#include <utilib/math_basic.h>
#include <utilib/DUniform.h>
#include <utilib/Uniform.h>
#include <utilib/IntVector.h>
#include <utilib/DoubleVector.h>
#include <utilib/BasicArray.h>
#include <utilib/default_rng.h>
#include <utilib/BitArray.h>

namespace utilib {

/**
 * Sum up the values in an array.
 */
template <class T>
T sum(const BasicArray<T>& vec)
{
T ans=0;

for (size_type i=0; i<vec.size(); i++)
  ans += vec[i];
return(ans);
}


/**
 * Shuffle an array.
 * Shuffle performed is Algorithm P (Shuffling) from page 139 of
 * \if GeneratingLaTeX Knuth~\cite{Knu}. \endif
 * \if GeneratingHTML [Knu]. \endif
 * Only \a num values are shuffled into the first \a num positions on 
 * the array.
 */
template <class T, class RNGT>
void shuffle(BasicArray<T>& vec, RNGT* rng, size_type num)
{
Uniform urnd(rng);
T temp;

for (size_type i = 0; i<num; i++) {
  size_type j = Discretize(urnd(), i, vec.size()-1);
  temp = vec[j];
  vec[j] = vec[i];
  vec[i] = temp;
  }
}


/**
 * Shuffle a basic array using the given random number generator.
 */
template <class T, class RNGT>
inline void shuffle(BasicArray<T>& vec, RNGT* rng)
{shuffle(vec,rng,vec.size());}


/**
 * Shuffle a basic array using the default random number generator.
 */
template <class T>
inline void shuffle(BasicArray<T>& vec)
{shuffle(vec,&default_rng,vec.size());}


/**
 * Shuffle a bit array using the given random number generator.
 */
template <class RNGT>
void shuffle(BitArray& vec, RNGT* rng)
{
size_type j;
Uniform urnd(rng);
int tempj;

for (size_type i = 0; i<vec.size(); i++) {
  j = Discretize(urnd(), i, vec.size()-1);
  tempj = vec(j);
  vec.put(j,vec(i));
  vec.put(i,tempj);
  }
}


/**
 * Shuffle a bit array using the default randome number generator.
 */
inline void shuffle(BitArray& vec)
{ shuffle(vec,&default_rng); }


/**
 * Shuffle a subset of an array starting from \a start and ending at \a end - 1.
 * Within the range from \a start to \a end, the method operates like the
 * \c shuffle function.
 */
template <class T, class RNGT, typename Index1T, typename Index2T>
void subshuffle(BasicArray<T>& vec, RNGT* rng, Index1T start, Index2T end)
{
DUniform<size_type> urnd(rng);
T temp;

for (unsigned long int i = start; i<end; i++) {
  size_type j = urnd(i,end-1);
  temp = vec[j];
  vec[j] = vec[i];
  vec[i] = temp;
  }
}


/**
 * Compute the L_2 norm of the numerical array.
 * This is not a numerically stable implementation.
 */
template <class T>
double length(const BasicArray<T>& a)
{
double ans=0.0;

for (size_type i=0; i<a.size(); i++)
  ans += a[i]*a[i];

return sqrt(ans);
}


/**
 * Compute the distance between two numerical arrays.
 * This is not a numerically stable implementation.
 */
template <class T>
double dist(const BasicArray<T>& a, const BasicArray<T>& b)
{
double ans=0.0;

for (size_type i=0; i<a.size(); i++)
  ans += (a[i]-b[i])*(a[i]-b[i]);

return sqrt(ans);
}


/**
 * Return the index of the minimal element of the array.
 */
template <class T>
size_type argmin(const BasicArray<T>& vector)
{
size_type i, temp=0;

if (vector.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::argmin - empty array");
if (vector.size() == 1)
   return(0);

for (i=1; i<vector.size(); i++)
  if (vector[i] < vector[temp])  temp=i;

return(temp);
}


/**
 * Return the index of the minimal element of the array using a comparison 
 * class.
 */
template <class T, class CompareT>
size_type argmin(const BasicArray<T>& vector, CompareT compare_)
{
size_type i, temp=0;

if (vector.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::argmin - empty array");
if (vector.size() == 1)
   return(0);

for (i=1; i<vector.size(); i++)
  if (compare_(vector[i],vector[temp]))  temp=i;

return(temp);
}


/**
 * Return the index of the maximal element of the array.
 */
template <class T>
size_type argmax(const BasicArray<T>& vector)
{
size_type i, temp=0;

if (vector.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::argmax - empty array");
if (vector.size() == 1)
   return(0);

for (i=1; i<vector.size(); i++)
  if (vector[i] > vector[temp])  temp=i;

return(temp);
}


/**
 * Return the index of the maximal element of the array using a comparison class.
 */
template <class T, class CompareT>
size_type argmax(const BasicArray<T>& vector, CompareT compare_)
{
size_type i, temp=0;

if (vector.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::argmax - empty array");
if (vector.size() == 1)
   return(0);

for (i=1; i<vector.size(); i++)
  if (compare_(vector[temp],vector[i]))  temp=i;

return(temp);
}


/**
 * Randomly select a value
 */
size_type rand_select(double* x, size_type p, size_type r, size_type i, size_type* ws, DUniform<size_type>& drnd);


/**
 * Return the index of the median element of the array.
 */
template <class RNGT>
size_type argmedian(double* x, size_type n, size_type* ws, RNGT* rng)
{
bool delete_flag=false;
if (!ws) {
   ws = new size_type [n];
   delete_flag=true;
   }
for (size_type i=0; i<n; i++)
  ws[i] = i;

AnyRNG lrng(&default_rng);
if (rng)
   lrng = rng;
DUniform<size_type> drnd(&lrng);
size_type ans = rand_select(x,0,n-1,(n+1)/2,ws,drnd);

if (delete_flag)
   delete [] ws;

return ans;
}

          
/**
 * Return the index of the median element of the array
 */
inline size_type argmedian(double* x, size_type n, size_type* ws)
{
AnyRNG rng(&default_rng);
return argmedian(x,n,ws,&rng);
}

     
/**
 * Return the index of the median element of the array.
 */
template <class T, class RNGT>
size_type argmedian(const BasicArray<T>& vector, RNGT* rng)
{
if (vector.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::argmedian - empty array");
T* tmp_array = (T*)&(vector[0]);
return argmedian( tmp_array, vector.size(), 0, rng );
}


/**
 * Return the index of the median element of the array.
 */
template <class T>
size_type argmedian(const BasicArray<T>& vector)
{
AnyRNG rng = &default_rng;
return argmedian(vector,&rng);
}


/**
 * Return the index of the median element of the array.
 */
template <class T, class RNGT>
size_type argmedian(const BasicArray<T>& vec, BasicArray<int>& ws, 
			RNGT* rng)
{ return argmedian(vec.data(), vec.size(), ws.data(), rng); }


/**
 * Return the index of the median element of the array.
 */
template <class T>
size_type argmedian(const BasicArray<T>& vec, BasicArray<int>& ws)
{
AnyRNG rng = &default_rng;
return argmedian(vec,ws);
}


#ifdef min
#undef min
#endif

/**
 * Return the minimal element of an array.
 */
template <class T>
inline T min(const BasicArray<T>& vec) 
{
if (vec.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::min - empty array");
return vec[argmin(vec)];
}

/**
 * Return the minimal element of an array.
 */
template <class T, class CompareT>
inline T min(const BasicArray<T>& vec, CompareT compare) 
{
if (vec.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::min - empty array");
return vec[argmin(vec,compare)];
}

#ifdef max
#undef max
#endif


/**
 * Return the maximal element of an array.
 */
template <class T>
inline T max(const BasicArray<T>& vec) 
{
if (vec.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::max - empty array");
return vec[argmax(vec)];
}

/**
 * Return the maximal element of an array.
 */
template <class T, class CompareT>
inline T max(const BasicArray<T>& vec, CompareT compare) 
{
if (vec.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::max - empty array");
return vec[argmax(vec,compare)];
}


/**
 * Return the median element of an array.
 */
template <class T>
inline T median(const BasicArray<T>& vec) 
{
if (vec.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::median - empty array");
return vec[argmedian(vec)];
}


/**
 * Return the median element of an array.
 */
template <class T, class CompareT>
inline T median(const BasicArray<T>& vec, CompareT compare) 
{
if (vec.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::median - empty array");
return vec[argmedian(vec,compare)];
}


/**
 * Concatenate two arrays together
 */
template <class T>
BasicArray<T> concat(const BasicArray<T>& a, const BasicArray<T>& b)
{
BasicArray<T> ans(a.size() + b.size());

for (size_type i=0; i<a.size(); i++)
  ans[i] = a[i];
for (size_type j=a.size(); j<(a.size()+b.size()); j++)
  ans[j] = b[j-a.size()];

return(ans);
}


/**
 * Compute the mean of an array
 */
template <class T>
double mean(const BasicArray<T>& array)
{
if (array.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::mean - empty array");
double temp=0.0;

for (size_type i=0; i<array.size(); i++)
  temp += array[i];

return( temp/array.size() );
}


/**
 * Compute the mean of an array using an accessor object
 */
template <class T, class AccessorT>
double mean(const BasicArray<T>& array, AccessorT acc_)
{
if (array.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::mean - empty array");
double temp=0.0;

for (size_type i=0; i<array.size(); i++)
  temp += acc_(array[i]);

return( temp/array.size() );
}


/**
 * Compute the trimmed mean of an array
 */
template <class T>
double trimmed_mean(BasicArray<T>& array, const double percent)
{
if (array.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::trimmed_mean - empty array");
double temp=0.0;

sort(array);

size_type boundary = (size_type) floor(array.size() * percent);

for (size_type i=boundary; i<(array.size()-boundary); i++)
  temp += array[i];

return( temp/(array.size() - 2.0*boundary) );
}


/**
 * Compute the variance of an array
 */
template <class T, class AccessorT>
double var(const BasicArray<T>& array, AccessorT accessor,
						const bool sampleflag=true)
{
if (array.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::var - empty array");
double array_mean = mean(array,accessor);

double temp=0.0;
for (size_type i=0; i<array.size(); i++)
  temp += (accessor(array[i]) - array_mean) * (accessor(array[i]) - array_mean);

if ((sampleflag == false) || (array.size() == 1))
   return( temp / array.size() );
else
   return( temp / (array.size() - 1) );
}


/**
 * Compute the variance of an array
 */
template <class T>
double var(const BasicArray<T>& array, const bool sampleflag=true)
{
if (array.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::var - empty array");
double array_mean = mean(array);

double temp=0.0;
for (size_type i=0; i<array.size(); i++)
  temp += (array[i] - array_mean) * (array[i] - array_mean);

if ((sampleflag == false) || (array.size() == 1))
   return( temp / array.size() );
else
   return( temp / (array.size() - 1) );
}


/**
 * Compute the MAD (mean absolute deviation) of an array, given workspace
 */
template <class T>
double mad(BasicArray<T>& array, BasicArray<double>& work)
{
if (array.size() == 0)
   EXCEPTION_MNGR(runtime_error, "utilib::mad - empty array");
double meanval = mean(array);

for (size_type i=0; i<work.size(); i++)
  work[i] = fabs(array[i] - meanval);

return median(work);
}


/**
 * Compute the MAD (mean absolute deviation) of an array
 */
template <class T>
double mad(BasicArray<T>& array)
{
BasicArray<double> tmparray(array.size());
return mad(array,tmparray);
}


/**
 * Apply the function \c val to the elements of \c vec.
 */
template <class T>
void lapply(BasicArray<T>& vec, double (*func)(T val))
{
for (size_type i=0; i<vec.size(); i++)
  vec[i] = (*func)(vec[i]);
}


/**
 * Compute the inner product between two numerical arrays.
 */
template <class T>
double inner_product(const BasicArray<T>& v1,
                                const BasicArray<T>& v2)
{
double ans = 0.0;

for (size_type i=0; i<v1.size(); i++)
  ans += v1[i]*v2[i];

return ans;
}


/**
 * Round to a specified number of digits
 */
template <class VectorT>
void d_round(const VectorT& source, unsigned int num_digits, VectorT& dest)
{
dest.resize(source.size());
for (unsigned int i=0; i<source.size(); i++)
  dest[i] = ::d_round(source[i], num_digits);
}

} // namespace utilib
#endif
