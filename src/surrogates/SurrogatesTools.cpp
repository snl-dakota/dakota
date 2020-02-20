/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesTools.hpp"
#include "../util/MathTools.hpp"
#include <Eigen/Dense>

using Eigen::VectorXi;
using Eigen::MatrixXi;

namespace dakota {
namespace surrogates {

void
compute_next_combination( int num_dims, int level, VectorXi &index,
                          bool &extend, int &h, int &t )
{
  if ( !extend )
  {
    t = level;
    h = 0;
    index[0] = level;
    for ( int d = 1; d < num_dims; d++ )
      index[d] = 0;
  }
  else
  {
    if ( 1 < t )
      h = 0;

    t = index[h];
    index[h] = 0;
    index[0] = t - 1;
    index[h+1] = index[h+1] + 1;
    h += 1;
  }
  extend = ( index[num_dims-1] != level );
}

// ------------------------------------------------------------

void
size_level_index_vector( int num_dims, int level, MatrixXi &indices )
{
  int num_indices;
  if ( level > 0 )
  {
    num_indices = util::n_choose_k( num_dims + level, num_dims ) -
      util::n_choose_k( num_dims + level-1, num_dims );
    indices.resize( num_indices, num_dims );
    bool extend = false;
    int h = 0, t = 0;
    VectorXi index( num_dims ); // important this is initialized to zero
    index *= 0;
    int i = 0;
    while ( true )
    {
      compute_next_combination( num_dims, level, index, extend, h, t );
      for ( int d = 0; d < num_dims; d++ )
        indices( i, d ) = index[d];
      i++;

      if ( !extend ) break;
    }
  }
  else
  {
    indices.resize( 1, num_dims );
  }
}

// ------------------------------------------------------------

void
compute_hyperbolic_subdim_level_indices(
                int num_dims, int level,
                int num_active_dims, double p,
                MatrixXi & indices )
{
  double eps = 100 * std::numeric_limits<double>::epsilon();

  MatrixXi working_indices( num_active_dims, 10 );
  int l = num_active_dims;
  int num_indices = 0;
  while ( true )
  {
    MatrixXi level_data;
    size_level_index_vector( num_active_dims, l, level_data );
    level_data.transposeInPlace();
    for ( int i = 0; i < level_data.cols(); ++i )
    {
      Eigen::Map<VectorXi> index( level_data.col(i).data(), num_active_dims );
      if ( util::num_nonzeros(index) == num_active_dims )
      {
        double pnorm = util::p_norm( index, p );
        if ( ( pnorm > level-1 + eps ) &&( pnorm < level + eps ) )
        {
          if ( num_indices >= working_indices.cols() )
            working_indices.resize( working_indices.rows(), num_indices+1000 );
          Eigen::Map<VectorXi> col( working_indices.col(num_indices).data(), num_active_dims );
          col = index;
          num_indices++;
        }
      }
    }
    l++;
    if ( l > level ) break;
  }
  // Remove unwanted memory
  indices.resize( num_indices, num_active_dims );
  for( int row=0; row<indices.rows(); ++row )
    for( int col=0; col<indices.cols(); ++col )
      indices(row, col) = working_indices(col, row);
}

// ------------------------------------------------------------

void
compute_hyperbolic_level_indices( int num_dims, int level, double p, MatrixXi & indices )
{
  if ( level == 0 )
    indices = MatrixXi::Zero(3, 1);
  else
  {
    indices.resize( num_dims, num_dims );
    for ( int  i = 0; i < num_dims; ++i )
    {
      VectorXi index; // need to initialize values to zero
      index.setZero( num_dims );
      index(i) = level;
      for ( int d = 0; d < num_dims; ++d )
        indices(d,i) = index(d);
    }

    for ( int d = 2; d < std::min( level+1, num_dims+1 ); d++ )
    {
      MatrixXi level_comb;
      compute_hyperbolic_subdim_level_indices( num_dims, level, d, p, level_comb );

      if ( level_comb.rows() == 0  )  break;

      level_comb.transposeInPlace();

      MatrixXi dims_comb;
      size_level_index_vector( num_dims, d, dims_comb );
      MatrixXi dim_indices( dims_comb.cols(), dims_comb.rows() );
      dims_comb.transposeInPlace();
      int num_dim_indices = 0;
      for ( int i = 0; i < dims_comb.cols(); i++ )
      {
        Eigen::Map<VectorXi> index( dims_comb.col(i).data(), num_dims );
        if ( util::num_nonzeros( index ) == d )
        {
          Eigen::Map<VectorXi> col( dim_indices.col(num_dim_indices).data(), num_dims );
          col = index;
          num_dim_indices++;
        }
      }
      // Chop off unused memory;
      dim_indices.conservativeResize( num_dims, num_dim_indices );

      MatrixXi new_indices( num_dims, num_dim_indices*level_comb.cols() );
      int num_new_indices = 0;
      for ( int i = 0; i < dim_indices.cols(); i++ )
      {
        Eigen::Map<VectorXi> dim_index( dim_indices.col(i).data(), num_dims );
        VectorXi I;
        util::nonzero( dim_index, I );
        for ( int j = 0; j < level_comb.cols(); j++ )
        {
          auto index = new_indices.col(num_new_indices);
          index.setZero();
          for ( int k = 0; k < I.size(); ++k )
          {
            index[I[k]] = level_comb(k,j);
          }
          num_new_indices++;
        }
      }

      util::append_columns( new_indices, indices );
    }
  }
}


}  // namespace surrogates
}  // namespace dakota
