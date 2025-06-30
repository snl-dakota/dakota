#include "DakotaROLInterface.hpp"

namespace rol_interface {

Dakota::Real* Vector::data() {
  return vec_->values();
}

const Dakota::Real* Vector::data() const {
  return vec_->values();
}

Dakota::RealVector& Vector::getVector() {
  return *vec_;
}

const Dakota::RealVector& Vector::getVector() const {
  return *vec_;
}

Vector::Vector( const ROL::Ptr<Dakota::RealVector>& vec ) 
: vec_{vec} {}
    
Vector::Vector( int length, bool zeroOut ) 
: vec_{ ROL::makePtr<Dakota::RealVector>(length,zeroOut) } {
} 
  
Dakota::Real Vector::dot( const ROL::Vector<Dakota::Real>& x ) const { 
  return vec_->dot(as_dakota_vector(x));
}

Dakota::Real Vector::norm() const {
  return std::sqrt(vec_->dot(*vec_));
}

void Vector::set( const ROL::Vector<Dakota::Real>& x ) { 
  *vec_ = as_dakota_vector(x);
}


void Vector::plus( const ROL::Vector<Dakota::Real>& x ) { 
  *vec_ += as_dakota_vector(x);
}

void Vector::scale( const Dakota::Real alpha ) {
  *vec_ *= alpha;
}

void Vector::setScalar( const Dakota::Real alpha ) {
  *vec_ = alpha;
}

void Vector::zero() {
  *vec_ = 0;
}

int Vector::dimension() const { 
  return vec_->length();  
}

void Vector::applyUnary( const ROL::Elementwise::UnaryFunction<Dakota::Real>& f ) {
  for( int i=0; i<vec_->length(); ++i ) {
    (*vec_)(i) = f.apply(((*vec_)(i)));     
  }
}

void Vector::applyBinary( const ROL::Elementwise::BinaryFunction<Dakota::Real>& f, 
                          const ROL::Vector<Dakota::Real>& x ) {
  const auto& xv = as_dakota_vector(x);
  for( int i=0; i<vec_->length(); ++i ) {
    (*vec_)(i) = f.apply((*vec_)(i),xv(i));
  }    
}
 
Dakota::Real Vector::reduce( const ROL::Elementwise::ReductionOp<Real>& r ) const {
  Dakota::Real result = r.initialValue();
  for( int i=0; i<vec_->length(); ++i ) {
    r.reduce((*vec_)(i),result);
  }
  return result;
}

void Vector::randomize( const Dakota::Real l, const Dakota::Real u ) {
  Dakota::Real a = (u-l);
  Dakota::Real b = l;
  Dakota::Real x(0);
  for( int i=0; i<vec_->length(); ++i ) {
    x = static_cast<Dakota::Real>(rand())/static_cast<Dakota::Real>(RAND_MAX);
    (*vec_)(i) = a*x + b;
  }    
}

ROL::Ptr<ROL::Vector<Dakota::Real>> Vector::clone() const { 
  return make_vector(dimension());
}

ROL::Ptr<ROL::Vector<Dakota::Real>> Vector::basis( int i ) const {
  auto b = ROL::makePtr<Dakota::RealVector>(vec_->length(),true);
  (*b)[i] = Dakota::Real{1.0};
  return ROL::makePtr<Vector>(b);
}

void Vector::print( std::ostream& outStream ) const {
  vec_->print(outStream);
}

// Helper functions

ROL::Ptr<ROL::Vector<Dakota::Real>> make_vector( int length, bool zeroOut ) {
  return ROL::makePtr<Vector>(length,zeroOut);
}

Dakota::RealVector& as_dakota_vector( ROL::Vector<Dakota::Real>& x ) {
  return static_cast<Vector&>(x).getVector();
}

const Dakota::RealVector& as_dakota_vector( const ROL::Vector<Dakota::Real>& x ) {
  return static_cast<const Vector&>(x).getVector();
}

Dakota::Real* get_dakota_vector_data_pointer( ROL::Vector<Dakota::Real>& x ) {
  return static_cast<Vector&>(x).data();
}

const Dakota::Real* get_dakota_vector_data_pointer( const ROL::Vector<Dakota::Real>& x ) {
  return static_cast<const Vector&>(x).data();
}


} // namespace rol_interface

