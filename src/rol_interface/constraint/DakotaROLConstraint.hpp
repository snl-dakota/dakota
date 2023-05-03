#pragma once
#ifndef DAKOTA_ROL_CONSTRAINT_HPP
#define DAKOTA_ROL_CONSTRAINT_HPP

//- Class:        Constraint
//- Description:  Base class for providing a ROL::Constraint interface to Dakota's 
//                (non)linear (in)equality constraints
//- Owner:        Greg von Winckel
//- Version:

namespace rol_interface {


//--------------------------------------------------------------------------------

/// Base class for the various types of constraint subclasses

/** The Constraint class provides a default implementation of member functions
 *  taking the abstract ROL::Vector<Dakota::Real> arguments and automatically
 *  downcasts and permits direct access to the underlying Dakota::RealVector 
 *  container */   


class Constraint : public ROL::Constraint<Dakota::Real> {
public:

  // This Constraint type enum convention allows bit masking to extract properties

  enum class Type : std::uint8_t {
    LinearEquality     = 0u,
    LinearInequality   = 1u,
    NonlinearEquality  = 2u,
    NonlinearInequalty = 3u,
    Default
  };

  static constexpr std::uint8_t EQUALITY_MASK = 0x1u;
  static constexpr std::uint8_t LINEAR_MASK   = 0x2u;

  static inline bool is_equality( Type type ) {
    return (static_cast<std::uint8_t>(type) & EQUALITY_MASK) == 0;
  }

  static inline bool is_linear( Type type ) {
    return (static_cast<std::uint8_t>(type) & LINEAR_MASK) == 0;
  }

  struct TypeError : public std::runtime_error {
  };

  Constraint() = delete;
  Constraint( ModelInterface* model_interface, Type con_type, std::string con_name="Constraint");
  virtual ~Constraint() = default;


  inline Type get_type() const { return conType; }

  virtual std::string get_name() const { return conName; }

  void update( const ROL::Vector<Dakota::Real>& x,
                     ROL::UpdateType            type,
                     int                        iter = -1 ) override; 

  virtual void update( const Dakota::RealVector& x,
                             ROL::UpdateType     type,
                             int                 iter = -1 ); 

  void value(       ROL::Vector<Dakota::Real>& c,
              const ROL::Vector<Dakota::Real>& x,
                    Dakota::Real&              tol ) override final; 

  virtual void value(       Dakota::RealVector& c,
                      const Dakota::RealVector& x,
                            Dakota::Real&       tol );  

  void applyJacobian(       ROL::Vector<Dakota::Real>& jv,
                      const ROL::Vector<Dakota::Real>& v,
                      const ROL::Vector<Dakota::Real>& x,
                            Dakota::Real&              tol ) override final;  

  virtual void applyJacobian(       Dakota::RealVector& jv,
                              const Dakota::RealVector& v,
                              const Dakota::RealVector& x,
                                    Dakota::Real&       tol );  

  void applyAdjointJacobian(       ROL::Vector<Dakota::Real>& ajv,
                             const ROL::Vector<Dakota::Real>& v,
                             const ROL::Vector<Dakota::Real>& x,
                                   Dakota::Real&              tol ) override final;  

  virtual void applyAdjointJacobian(       Dakota::RealVector& ajv,
                                     const Dakota::RealVector& v,
                                     const Dakota::RealVector& x,
                                           Dakota::Real&       tol );  

  virtual void applyAdjointHessian(        ROL::Vector<Dakota::Real>& ahuv,
                                     const ROL::Vector<Dakota::Real>& u,
                                     const ROL::Vector<Dakota::Real>& v,
                                     const ROL::Vector<Dakota::Real>& x,
                                           Dakota::Real&              tol ) override;   

  virtual void applyAdjointHessian(        Dakota::RealVector& ahuv,
                                     const Dakota::RealVector& u,
                                     const Dakota::RealVector& v,
                                     const Dakota::RealVector& x,
                                           Dakota::Real&       tol );  

  ROL::Ptr<ROL::Vector<Dakota::Real>> make_lagrange_multiplier() const;

  void set_dimensions( int num_opt, int num_con );
  void set_value( const Dakota::Real* value );
  void set_jacobian( const Dakota::Real* jacobian );

  

protected:

  virtual void update_from_model( ModelInterface* ) = 0; 

  Dakota::Real* const targetData;    // pointer to equality constraint target data
  Dakota::Real* const valueData;     // pointer to constraint values
  Dakota::Real* const jacobianData;  // pointer to constraint jacobian 
  int numOpt, numCon;                // Number of optimization and constraint variables

private:

  ModelInterface* modelInterface;
  Type conType;
  std::string conName;

}; // class Constraint
 
} // namespace rol_interface


#endif // DAKOTA_ROL_CONSTRAINT_HPP
