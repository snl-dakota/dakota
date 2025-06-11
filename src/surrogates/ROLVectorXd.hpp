#ifndef ROL_VECTORXD_HPP
#define ROL_VECTORXD_HPP

/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <Eigen/Core>
#include "ROL_Vector.hpp"

namespace dakota::surrogates {

inline Eigen::VectorXd& as_VectorXd( ROL::Vector<double>& x ) noexcept;
inline const Eigen::VectorXd& as_VectorXd( const ROL::Vector<double>& x ) noexcept;

class ROLVectorXd : public ROL::Vector<double> {
public:

  ROLVectorXd( int dim, bool zero_out=false ) : vec_{dim} {
    if(zero_out) {
      vec_.setZero();
    }
  }

  virtual ~ROLVectorXd() {}

  template<class T>
  inline ROLVectorXd& operator = ( T&& rhs ) {
    vec_ = std::forward<T>(rhs);
    return *this;
  }    


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
    vec_.setZero();
  }

  int dimension() const override {
    return vec_.size();
  }

  void set( const ROL::Vector<double>& x ) override {
    vec_ = as_VectorXd(x);
  }

  const ROL::Vector<double>& dual() const override {
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

  void applyBinary( const ROL::Elementwise::BinaryFunction<double>& bf,
                    const ROL::Vector<double>& x ) override {
    const auto& xd = as_VectorXd(x);
    for(int i=0; i<vec_.size(); ++i) {
      vec_(i) = bf.apply(vec_(i),xd(i));
    }
  }

  double reduce( const ROL::Elementwise::ReductionOp<double>& r ) const override {
    double result = r.initialValue();
    for(int i=0; i<vec_.size(); ++i) {
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
      vec_.array() += l;
    }
  }

  void setScalar( const double C ) override {
    vec_.setConstant(C);
  }

  // Mutable segment access
  inline auto segment(int start, int size) {
    return vec_.segment(start, size);
  }
  
  // Const segment access  
  inline auto segment(int start, int size) const {
    return vec_.segment(start, size);
  }
  
  // Template versions for compile-time size (more efficient)
  template<int Size>
  inline auto segment(int start) {
    return vec_.segment<Size>(start);
  }
  
  template<int Size>
  inline auto segment(int start) const {
    return vec_.segment<Size>(start);
  }
  
  // Head and tail convenience methods (commonly used)
  inline auto head(int size) {
    return vec_.head(size);
  }
  
  inline auto head(int size) const {
    return vec_.head(size);
  }
  
  inline auto tail(int size) {
    return vec_.tail(size);
  }
  
  inline auto tail(int size) const {
    return vec_.tail(size);
  }
  
  // Template versions for compile-time size
  template<int Size>
  inline auto head() {
    return vec_.head<Size>();
  }
  
  template<int Size>
  inline auto head() const {
    return vec_.head<Size>();
  }
  
  template<int Size>
  inline auto tail() {
    return vec_.tail<Size>();
  }
  
  template<int Size>
  inline auto tail() const {
    return vec_.tail<Size>();
  }


  // Element access

  inline double& operator() ( int i ) { 
    return vec_(i);
  }

  inline const double& operator() ( int i ) const { 
    return vec_(i);
  }

  inline Eigen::VectorXd& getVector() noexcept { 
    return vec_;
  }

  inline const Eigen::VectorXd& getVector() const noexcept { 
    return vec_;
  }


  inline static ROLVectorXd& downcast( ROL::Vector<double>& x ) noexcept {
    return static_cast<ROLVectorXd&>(x);
  }

  inline static const ROLVectorXd& downcast( const ROL::Vector<double>& x ) noexcept {
    return static_cast<const ROLVectorXd&>(x);
  }

private:

  Eigen::VectorXd vec_;

};

inline Eigen::VectorXd& as_VectorXd( ROL::Vector<double>& x ) noexcept {
  return ROLVectorXd::downcast(x).getVector();
}

inline const Eigen::VectorXd& as_VectorXd( const ROL::Vector<double>& x ) noexcept {
  return ROLVectorXd::downcast(x).getVector();
}

} // namespace dakota::surrogates

#endif // ROL_VECTORXD_HPP
