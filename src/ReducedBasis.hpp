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

    class TruncationCondition;

    //
    //- Heading: Constructors, destructor, operators
    //

    /// default constructor
    ReducedBasis();

    void set_matrix(const RealMatrix &);

    const RealMatrix & get_matrix()
      { return matrix; }

    void center_matrix();

    void update_svd(bool center_matrix_by_col_means = true);

    bool is_valid() const
      { return is_valid_svd; }

    const Real & get_singular_values_sum() const
      { return singular_values_sum; }

    const Real & get_eigen_values_sum() const
      { return eigen_values_sum; }

    const RealVector & get_column_means() const
      { return column_means; }

    const RealVector & get_singular_values() const
      { return S_values; }

    RealVector get_singular_values(const TruncationCondition &) const;

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

    Real singular_values_sum;
    Real eigen_values_sum;

    TruncationCondition * truncation;


    // Support some varieties of truncation

  public:

    class TruncationCondition
    {
      public:
        /// default constructor
        TruncationCondition() { }

        virtual int get_num_components(const ReducedBasis &) const = 0;

        virtual void sanity_check(const ReducedBasis &) const;
    };

    class Untruncated : public TruncationCondition {
      public:
        Untruncated();
        int get_num_components(const ReducedBasis &) const;
    };

    class VarianceExplained : public TruncationCondition {
      public:
        VarianceExplained(Real var_exp);
        int get_num_components(const ReducedBasis &) const;

      private:
        Real variance_explained;
    };

    class HeuristicVarianceExplained : public TruncationCondition {
      public:
        HeuristicVarianceExplained(Real var_exp);
        int get_num_components(const ReducedBasis &) const;

      private:
        Real variance_explained;
    };

    class NumComponents : public TruncationCondition {
      public:
        NumComponents(int num_comp);

        int get_num_components(const ReducedBasis &) const;

      private:
        int num_components;
    };

};

} // namespace Dakota

#endif
