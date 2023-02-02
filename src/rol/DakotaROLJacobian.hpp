/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Jacobian
//- Description: Provides a unified Dakota/ROL interface for the Jacobian 
//               matrices of linear (in)equality constraints      
                                                 
//- Owner:       Greg von Winckel    
//- Version: 

#pragma once
#ifndef DAKOTA_ROL_JACOBIAN_H
#define DAKOTA_ROL_JACOBIAN_H

#include "ROL_TeuchosVector.hpp"
#include "ROL_LinearOperator.hpp"
#include "ROL_BLAS.hpp"

namespace Dakota {
namespace rol_interface {

class Jacobian : public ROL::Jacobian<Real> {
public:


  Jacobian() = delete;
  Jacobian( const ROL::Ptr<Cache>& cache, 
                  Dakota::CONSTRAINT_EQUALITY_TYPE type );

  virtual ~Jacobian() = default;

  void apply(       ROL::Vector<Real>& jv, 
              const ROL::Vector<Real>& v, 
                    Real&              tol ) const override;

  void applyAdjoint(       ROL::Vector<Real>& ajv, 
                     const ROL::Vector<Real>& v, 
                           Real&              tol ) const override;

  inline static ROL::Ptr<Jacobian> make_inequality( const ROL::Ptr<Cache>& cache ) {
    return ROL::makePtr<Jacobian>(cache,Dakota::CONSTRAINT_EQUALITY_TYPE::INEQUALITY);  
  } 

  inline static ROL::Ptr<Jacobian> make_equality( const ROL::Ptr<Cache>& cache ) { 
    return ROL::makePtr<Jacobian>(model,Dakota::CONSTRAINT_EQUALITY_TYPE::INEQUALITY);  
  } 

private:

  inline static ModelFunction<Dakota::RealMatrix> 
  matrix_getter( const Dakota::Model& model,
                       Dakota::CONSTRAINT_EQUALITY_TYPE type ) noexcept {
    return ( type == Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                    &Dakota::Model::linear_eq_constraint_coeffs   :
                    &Dakota::Model::linear_ineq_constraint_coeffs;
  }
 
  static constexpr Real zero(0), one(1);

  JacGetter               getMatrix;
  Teuchos::BLAS<int,Real> blas;    
  int                     nRows;      
  int                     nCols;

}; // class Jacobian

} // namespace rol_interface
} // namespace Dakota

#endif // DAKOTA_ROL_JACOBIAN_H
