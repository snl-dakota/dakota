#pragma once

#include <Eigen/Core>
#include "ROL_Vector.hpp"

namespace dakota::surrogates {

class ROLVectorXd : public ROL::Vector<double> {
public:

  ROLVectorXd( int dim, bool zero_out=false ) : vec_{dim} {
    if(zero_out) {
      vec_->setZero();
    }
  }

  virtual ~ROLVectorXd() {}

  void plus( const ROL::Vector<double>& x ) override {
    vec_ += as_VectorXd(x);
  }

  void scale( const double alpha ) override {
    vec_ *= alpha;
  }

  double dot( const ROL::Vector<double>& x ) const override {
    return vec_.dot(as_VectorXd(x));
  }

  double norm() const override {
    return vec_.norm();
  }

  ROL::Ptr<ROL::Vector<double>> clone() const override {
    return ROL::makePtr<ROLVectorXd>(vec_.size());
  }

  void axpy( const double alpha, const ROL::Vector<double>& x ) override {
    vec_ += (alpha * as_VectorXd(x));
  }

  void zero() override {
    vec_->setZero();
  }

  int dimension() const override {
    return vec_.size();
  }

  void set( const ROL::Vector<double>& x ) override {
    vec_ = as_VectorXd(x);
  }

  const Vector<double>& dual() const override {
    return *this;
  }

  double apply( const ROL::Vector<double>& x ) const override {
    return dot(x);
  }

  ROL::Ptr<ROL::Vector<double>> basis( int i ) const override {
    auto b = ROL::makePtr<ROLVectorXd>(vec_.size(),true);
    (*b)(i) = 1.0;
    return b;
  } 

  void applyUnary( const ROL::Elementwise::UnaryFunction<double>& uf ) override {
    for(int i=0; i<vec_.size(); ++i) {
      vec_(i) = uf.apply(vec_(i));
    }
  }

  void applyBinary( const ROL::Elementwise::UnaryFunction<double>& bf,
                    const ROL::Vector<double>& x ) override {
    const auto& xd = as_VectorXd(x);
    for(int i=0; i<vec_.size(); ++i) {
      vec_(i) = bf.apply(vec_(i),xd(i));
    }
  }

  double reduce( const ROL::Elementwise::ReductionOp<double>& r ) const override {
    double result = r.initialValue();
    for(int i=0; i<vec_.dim(); ++i) {
      r.reduce(vec_(i),result);
    }
    return result;
  }

  void print( std::ostream& os ) const override {
    os << vec_;
  }
  
  void randomize( const double l=0.0, const double u=1.0 ) override {
    vec_ = Eigen::VectorXd::Random();
    if(u-l != 1.0) {
      vec_ *= (u-l);
    }
    if(l != 0.0) {
      vec_ += l;
    }
  }

  void setScalar( const double C ) override {
    vec_ = C;
  }

  // Element access

  inline double& operator() ( int i ) { 
    return vec_(i);
  }

  inline const double& operator() ( int i ) const { 
    return vec_(i);
  }

private:

  inline static VectorXd& as_VectorXd( ROL::Vector<double>& x ) {
    return static_cast<ROLVectorXd&>(x).vec_;
  }

  inline static const VectorXd& as_VectorXd( const ROL::Vector<double>& x ) {
    return static_cast<const ROLVectorXd&>(x).vec_;
  }

  Eigen::VectorXd vec_;

};

} // namespace dakota::surrogates
