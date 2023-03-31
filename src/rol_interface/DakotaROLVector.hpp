#pragma once
#ifndef DAKOTA_ROL_VECTOR_HPP
#define DAKOTA_ROL_VECTOR_HPP

namespace rol_interface {

class Vector;
/// Vector utility functions
inline ROL::Ptr<Vector> make_vector( int n ) noexcept;

inline Dakota::RealVector& get_vector( ROL::Vector<Dakota::Real>& x ) noexcept;

inline Dakota::RealVector& get_vector( ROL::Vector<Dakota::Real>& x ) noexcept;

inline Dakota::RealVector&  get_vector( ROL::Ptr<ROL::Vector<Dakota::Real>>& x ) noexcept;

inline const Dakota::RealVector& get_vector( const ROL::Vector<Dakota::Real>& x ) noexcept;

inline const Dakota::RealVector&  get_vector( const ROL::Ptr<const ROL::Vector<Dakota::Real>>& x ) noexcept;

inline Dakota::Real* get_vector_values( ROL::Vector<Dakota::Real>& x ) noexcept;

inline Dakota::Real* get_vector_values( ROL::Ptr<ROL::Vector<Dakota::Real>>& x ) noexcept;

inline const Dakota::Real* get_vector_values( const ROL::Vector<Dakota::Real>& x ) noexcept;

inline const Dakota::Real* get_vector_values( const ROL::Ptr<const ROL::Vector<Dakota::Real>>& x ) noexcept;


class Vector : public ROL::Vector<Dakota::Real> {

  using UnaryFunction  = ROL::Elementwise::UnaryFunction<Dakota::Real>;
  using BinaryFunction = ROL::Elementwise::BinaryFunction<Dakota::Real>;
  using ReductionOp    = ROL::Elementwise::ReductionOp<Dakota::Real>;

private:

  ROL::Ptr<Dakota::RealVector> vec_;

public:

  Vector( const ROL::Ptr<Dakota::RealVector>& vec ) : 
    vec_(vec) { }

  Vector( int length, bool zeroOut=true ) : 
    vec_( ROL::makePtr<Dakota::RealVector>(length,zeroOut) ) {} 

  Dakota::Real dot( const ROL::Vector<Dakota::Real>& x ) const override { 
    return vec_->dot(get_vector(x));
  }

  void set( const ROL::Vector<Dakota::Real>& x ) override { 
    *vec_ = get_vector(x);
  }

  void set( const Dakota::RealVector& x ) { 
    *vec_ = x;
  }


  void plus( const ROL::Vector<Dakota::Real>& x ) override { 
    *vec_ += get_vector(x);
  }

  void scale( const Dakota::Real alpha ) override {
    vec_->scale(alpha);
  }

  int dimension() const override { 
    return vec_->length();  
  }

  void applyUnary( const UnaryFunction &f ) override {
    for( int i=0; i<vec_->length(); ++i ) {
      (*vec_)(i) = f.apply(((*vec_)(i)));     
    }
  }

  void applyBinary( const BinaryFunction&            f, 
                    const ROL::Vector<Dakota::Real>& x ) override {
    auto& xv = get_vector(x);
    for( int i=0; i<vec_->length(); ++i ) {
      (*vec_)(i) = f.apply((*vec_)(i),xv(i));
    }    
  }
   
  Dakota::Real reduce( const ReductionOp &r ) const override {
    Dakota::Real result = r.initialValue();
    for( int i=0; i<vec_->length(); ++i ) {
      r.reduce((*vec_)(i),result);
    }
    return result;
  }

  void setScalar( const Dakota::Real C ) override {
    for( int i=0; i<vec_->length(); ++i ) {
      (*vec_)(i) = C;
    }    
  }

  void randomize( const Dakota::Real l=0.0, 
                  const Dakota::Real u=1.0 ) override {
    Dakota::Real a = (u-l);
    Dakota::Real b = l;
    Dakota::Real x(0);
    for( int i=0; i<vec_->length(); ++i ) {
      x = static_cast<Dakota::Real>(rand())/static_cast<Dakota::Real>(RAND_MAX);
      (*vec_)(i) = a*x + b;
    }    
  }

  ROL::Ptr<ROL::Vector<Dakota::Real>> clone() const override { 
    return make_vector(dimension());
  }

  ROL::Ptr<ROL::Vector<Dakota::Real>> basis( int i ) const override {
    auto b = ROL::makePtr<Vector>(dimension(),true);
    (*b)[i] = Dakota::Real{1.0};
    return b;
  }

  ROL::Ptr<Dakota::RealVector> getVector() {
    return vec_;
  }
  
  ROL::Ptr<const Dakota::RealVector> getVector() const { 
    return vec_;
  }
  
  void print( std::ostream& outStream ) const override {
    vec_->print(outStream);
  }

  // Add element access operators to circumvent needing to create and get Dakota::RealVectors
  Dakota::Real& operator() ( int i ) {
    return (*vec_)(i);
  }

  const Dakota::Real& operator() ( int i ) const  {
    return (*vec_)(i); 
  }

  Dakota::Real& operator[] ( int i ) {
    return (*vec_)[i];
  }
 
  const Dakota::Real& operator[] ( int i ) const {
    return (*vec_)[i]; 
  }

}; // class Vector



/// Vector utility functions
inline ROL::Ptr<Vector> make_vector( int n ) noexcept {
  return ROL::makePtr<Vector>(n);
}

inline Dakota::RealVector& get_vector( ROL::Vector<Dakota::Real>& x ) noexcept {
  return *(static_cast<Vector&>(x).getVector());
}

inline Dakota::RealVector& get_vector( ROL::Ptr<ROL::Vector<Dakota::Real>>& x ) noexcept {
  return get_vector(*x);
}

inline const Dakota::RealVector& get_vector( const ROL::Vector<Dakota::Real>& x ) noexcept {
  return *(static_cast<const Vector&>(x).getVector());
}

inline const Dakota::RealVector& get_vector( const ROL::Ptr<const ROL::Vector<Dakota::Real>>& x ) noexcept {
  return get_vector(*x);
}

inline Dakota::Real* get_vector_values( ROL::Vector<Dakota::Real>& x ) noexcept {
  return get_vector(x).values();
}

inline Dakota::Real* get_vector_values( ROL::Ptr<ROL::Vector<Dakota::Real>>& x ) noexcept {
  return get_vector_values(*x);
}

inline const Dakota::Real* get_vector_values( const ROL::Vector<Dakota::Real>& x ) noexcept {
  return get_vector(x).values();
}

inline const Dakota::Real* get_vector_values( const ROL::Ptr<const ROL::Vector<Dakota::Real>>& x ) noexcept {
  return get_vector_values(*x);
}

} // namespace rol_interface


#endif
