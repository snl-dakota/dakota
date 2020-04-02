/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 * MathTools.hpp
 * author
 */

/////////////
// Defines //
/////////////

#ifndef DAKOTA_UTIL_MATH_TOOLS_HPP
#define DAKOTA_UTIL_MATH_TOOLS_HPP

/////////////
// Imports //
/////////////

#include "util_data_types.hpp"

///////////////////////
// Using / Namespace //
///////////////////////

namespace dakota {
namespace util {

/////////////
// Headers //
/////////////

/**
 *  \brief Caclulate cominatorial N choose K
*/
int n_choose_k(int n, int k);

/**
 *  \brief Caclulate and return number of nonzero entries 
*/
template< typename T>
int num_nonzeros(const T mat)
{
  return (mat.array() != 0).count();
}


/**
 *  \brief Create a vector of indices for nonzero entries in input vector
*/
template< typename T1, typename T2>
void nonzero( const T1 & v, T2 & result )
{
  int num_nonzeros = 0;
  T2 working_vec = v;
  for( int i = 0; i < v.size(); ++i )
  {
    if ( v[i] != 0 )
    {
      working_vec[num_nonzeros] = i;
      num_nonzeros++;
    }
  }

  result.resize( num_nonzeros );
  for( int i = 0; i < num_nonzeros; ++i )
    result(i) = working_vec(i);
}


/**
 *  \brief Create a vector of indices for nonzero entries in input vector
*/
template< typename T1, typename T2>
void append_columns( const T1 & new_cols, T2 & target )
{
  // How do we want to handle errors? 
  if( (target.rows() != new_cols.rows()) && (new_cols.cols() > 0) )
    throw std::runtime_error("Incompatible row dimensions for append_columns.");

  int num_orig_cols = target.cols();
  target.conservativeResize(Eigen::NoChange, num_orig_cols + new_cols.cols() );
  for( int i=0; i<new_cols.cols(); ++i )
    target.col(num_orig_cols+i) = new_cols.col(i);
}


/**
 *  \brief Caclulate and return p-norm of a vector
*/
template< typename T >
double p_norm( const T & v, double p )
{
  double sum = 0.;
  for( int i=0; i<v.size(); ++i )
    sum += std::pow( std::abs( (double)v(i) ), p );

  return std::pow( sum, 1./p );
}

} // namespace util
} // namespace dakota

#endif // include guard
