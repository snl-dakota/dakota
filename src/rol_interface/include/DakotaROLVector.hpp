#ifndef DAKOTA_ROL_VECTOR_HPP
#define DAKOTA_ROL_VECTOR_HPP

namespace rol_interface {

class Vector : public ROL::Vector<Dakota::Real> {
public:

  Vector( const ROL::Ptr<Dakota::RealVector>& vec );

  Vector( int length, bool zeroOut=true );

  void applyBinary( const ROL::Elementwise::BinaryFunction<Dakota::Real>& f, 
                    const ROL::Vector<Dakota::Real>&                      x ) override;
 
  void applyUnary( const ROL::Elementwise::UnaryFunction<Dakota::Real>& f ) override;

  int dimension() const override;

  Dakota::Real dot( const ROL::Vector<Dakota::Real>& x ) const override;

  Dakota::Real norm() const override;

  void plus( const ROL::Vector<Dakota::Real>& x ) override;

  void randomize( const Dakota::Real l=0.0, 
                  const Dakota::Real u=1.0 ) override;

  Dakota::Real reduce( const ROL::Elementwise::ReductionOp<Dakota::Real>& r ) const override;

  void scale( const Dakota::Real alpha ) override;

  void set( const ROL::Vector<Dakota::Real>& x ) override;

  void setScalar( const Dakota::Real alpha ) override;

  void zero() override;

  ROL::Ptr<ROL::Vector<Dakota::Real>> clone() const override;

  ROL::Ptr<ROL::Vector<Dakota::Real>> basis( int i ) const override;

  void print( std::ostream& outStream ) const override;

  Dakota::RealVector& getVector();
  const Dakota::RealVector& getVector() const;

  Dakota::Real* data();
  const Dakota::Real* data() const;

private:

  ROL::Ptr<Dakota::RealVector> vec_;


}; // class Vector
  
ROL::Ptr<ROL::Vector<Dakota::Real>> make_vector( int length, bool zeroOut=false );

Dakota::RealVector& as_dakota_vector( ROL::Vector<Dakota::Real>& x );

const Dakota::RealVector& as_dakota_vector( const ROL::Vector<Dakota::Real>& x );

Dakota::Real* get_dakota_vector_data_pointer( ROL::Vector<Dakota::Real>& x );

const Dakota::Real* get_dakota_vector_data_pointer( const ROL::Vector<Dakota::Real>& x );

} // namespace rol_interface


#endif
