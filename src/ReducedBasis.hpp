/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ReducedBasis
//- Description:
//-
//-
//- Owner:        Russell Hooper
//- Created:      Jun 10, 2015
//- Version: $Id$

#ifndef REDUCED_BASIS_H 
#define REDUCED_BASIS_H 

#include "ExperimentDataUtils.hpp"

namespace Dakota {

/** The ReducedBasis class is used to ... (TODO - RWH)
    */

class ReducedBasis
{

  public:

    //
    //- Heading: Constructors, destructor, operators
    //

    /// default constructor
    ReducedBasis();

    void set_matrix(const RealMatrix &);

    void center_matrix();

    void update_svd(bool center_matrix_by_col_means = true);

    const RealVector & get_singular_values() const
      { return S_values; }

    const RealMatrix & get_left_singular_vector() const
      { return U_matrix; }

    const RealMatrix & get_right_singular_vector_transpose() const
      { return VT_matrix; }

  private:

    RealMatrix matrix;
    RealMatrix workingMatrix;

    RealMatrix U_matrix;
    RealVector S_values;
    RealMatrix VT_matrix;

    RealVector column_means;

    bool is_valid_svd;

};

} // namespace Dakota

#endif
