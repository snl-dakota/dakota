/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Hessian
//- Description: Provides a unified Dakota/ROL interface for the Hessian of
//               the objective function 
                                                 
//- Owner:       Greg von Winckel    
//- Version: 


#pragma once
#ifndef DAKOTA_ROL_HESSIAN_H
#define DAKOTA_ROL_HESSIAN_H


namespace rol_interface {

class Hessian : public ROL::LinearOperator<Dakota::Real> {
public:
  Hessian() = delete;
  Hessian( const ROL::Ptr<ModelInterface>& model_interface, 
                 bool                      has_inverse = false );

  virtual ~Hessian() = default;

  void update( const ROL::Vector<Dakota::Real>& x, 
                     bool               flag = true, 
                     int                iter = -1 ) override;

  void apply(       ROL::Vector<Dakota::Real>& Hv, 
              const ROL::Vector<Dakota::Real>& v, 
                    Dakota::Real&              tol ) const override;

  void applyInverse(       ROL::Vector<Dakota::Real>& Hv, 
                     const ROL::Vector<Dakota::Real>& v, 
                           Dakota::Real&              tol ) const override;

  bool has_inverse() const { return hasInverse; }

private:

  static constexpr Dakota::Real zero = 0;
  static constexpr Dakota::Real one = 1;

  Teuchos::LAPACK<int,Dakota::Real>           lapack;         // For Hessian inversion
  Teuchos::BLAS<int,Dakota::Real>             blas;           // For Matrix-Vector multiplication
  ROL::Ptr<ModelInterface>            modelInterface; // Unified interface to Dakota::Model
  std::unique_ptr<Dakota::RealMatrix> workMat;        // LU Storage for Hessian inversion
  std::unique_ptr<int[]>              iPiv;           // LU Pivots for Hessian inversion
  int                                 nRows;      
  bool                                hasInverse; 

}; // class Hessian

} // namespace rol_interface

#endif // DAKOTA_ROL_LINEAROPERATOR_H
