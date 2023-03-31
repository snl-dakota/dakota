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

namespace rol_interface {

class Jacobian : public ROL::LinearOperator<Dakota::Real> {
public:

  Jacobian() = delete;
  Jacobian( const Dakota::Real* const data, int num_rows, int num_cols );

  virtual ~Jacobian() = default;

  void apply(       ROL::Vector<Dakota::Real>& jv, 
              const ROL::Vector<Dakota::Real>& v, 
                    Dakota::Real&              tol ) const override;

  void applyAdjoint(       ROL::Vector<Dakota::Real>& ajv, 
                     const ROL::Vector<Dakota::Real>& v, 
                           Dakota::Real&              tol ) const override;
private:
  Teuchos::BLAS<int,Dakota::Real>  blas;    
  int nRows, nCols;
  const Dakota::Real* const jacValues;
}; // class Jacobian

} // namespace rol_interface

#endif // DAKOTA_ROL_JACOBIAN_H
