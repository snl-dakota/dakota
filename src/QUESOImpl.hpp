/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/** File: QUESO evaluation and utility implementations */

#ifndef DAKOTA_QUESO_IMPL_H
#define DAKOTA_QUESO_IMPL_H

// for generic log prior eval
#include "queso/JointPdf.h"
#include "queso/VectorSet.h"
#include "queso/VectorRV.h"

// to specialize the transition kernel with covariance update
#include "queso/ScaledCovMatrixTKGroup.h"
#include "queso/TransformedScaledCovMatrixTKGroup.h"
#include "queso/TransitionKernelFactory.h"

namespace Dakota {

class NonDQUESOBayesCalibration;

/// Dakota specialization of QUESO generic joint PDF
template <class V, class M>
class QuesoJointPdf: public QUESO::BaseJointPdf<V, M>
{
public:
  //! Default constructor.
  /*! Instantiates an object of the class, i.e. a scalar function,
      given a prefix and its domain.*/
  QuesoJointPdf(const char*                  prefix,
		const QUESO::VectorSet<V,M>& domainSet,
		NonDQUESOBayesCalibration*   nond_queso_ptr);
  //! Destructor
  virtual ~QuesoJointPdf();

  //! Actual value of the PDF (scalar function).
  double actualValue(const V& domainVector, const V* domainDirection, 
		     V* gradVector, M* hessianMatrix, V* hessianEffect) const;
  
  //! Logarithm of the value of the function.
  double lnValue(const V& domainVector, const V* domainDirection, 
		 V* gradVector, M* hessianMatrix, V* hessianEffect) const;

  // NOTE: likely don't need for MCMC:
  //! Computes the logarithm of the normalization factor.
  double computeLogOfNormalizationFactor(unsigned int numSamples,
					 bool m_logOfNormalizationFactor) const;

  //! Mean value of underlying random variable
  void distributionMean(V & meanVector) const;

  void distributionVariance(M & covMatrix) const;

private:
  using QUESO::BaseScalarFunction<V,M>::m_env;
  using QUESO::BaseScalarFunction<V,M>::m_prefix;
  // using QUESO::BaseScalarFunction<V,M>::m_domainSet;
  // using QUESO::BaseJointPdf<V,M>::m_normalizationStyle;
  // using QUESO::BaseJointPdf<V,M>::m_logOfNormalizationFactor;
  NonDQUESOBayesCalibration* nonDQUESOInstance;
};


/// Dakota specialization of QUESO vector-valued random variable
template <class V, class M>
class QuesoVectorRV: public QUESO::BaseVectorRV<V,M>
{

public:
  //! Default constructor
  /*! Constructs a generic queso vector RV, given a prefix and the
      image set of the vector RV.*/
  QuesoVectorRV(const char*                  prefix,
		const QUESO::VectorSet<V,M>& imageSet,
		NonDQUESOBayesCalibration*   nond_queso_ptr);
  //! Virtual destructor
  virtual ~QuesoVectorRV();
 
  //! TODO: Prints the vector RV (required pure virtual).
  void print(std::ostream& os) const;

private:
  using QUESO::BaseVectorRV<V,M>::m_env;
  using QUESO::BaseVectorRV<V,M>::m_prefix;
  using QUESO::BaseVectorRV<V,M>::m_imageSet;
  using QUESO::BaseVectorRV<V,M>::m_pdf;
  // using QUESO::BaseVectorRV<V,M>::m_realizer;  // only needed to make draws
  // using QUESO::BaseVectorRV<V,M>::m_subCdf;
  // using QUESO::BaseVectorRV<V,M>::m_unifiedCdf;
  // using QUESO::BaseVectorRV<V,M>::m_mdf;
};



/// Dakota transition kernel that updates proposal covariance based on
/// derivatives (for random walk case)
template <class V, class M>
class DerivInformedPropCovTK: public QUESO::ScaledCovMatrixTKGroup<V, M>
{
public:
  DerivInformedPropCovTK(const char * prefix,
			 const QUESO::VectorSpace<V, M> & vectorSpace,
			 const std::vector<double> & scales,
			 const M & covMatrix,
			 NonDQUESOBayesCalibration* queso_instance);

  virtual ~DerivInformedPropCovTK() {  /* empty dtor */ }
  /// update the transition kernel with new covariance information
  virtual void updateTK();
  virtual bool covMatrixIsDirty() { return covIsDirty; }
  virtual void cleanCovMatrix()   { covIsDirty = false; }

private:
  /// calibration parameter vector space (note: hides base class member)
  const QUESO::VectorSpace<V, M> & m_vectorSpace;
  //using QUESO::ScaledCovMatrixTKGroup<V, M>::m_vectorSpace;
  /// whether we've updated the proposal covariance
  bool covIsDirty;
  /// index into current chain position
  unsigned int chainIndex;

  /// Dakota QUESO instance for callbacks
  NonDQUESOBayesCalibration* nonDQUESOInstance;
};





/// Dakota transition kernel that updates proposal covariance based on
/// derivatives (for logit random walk case)
template <class V, class M>
class DerivInformedPropCovLogitTK: public QUESO::TransformedScaledCovMatrixTKGroup<V, M>
{
public:
  DerivInformedPropCovLogitTK(const char * prefix,
			 const QUESO::VectorSet<V, M> & vectorSet,
			 const std::vector<double> & scales,
			 const M & covMatrix,
			 NonDQUESOBayesCalibration* queso_instance);

  virtual ~DerivInformedPropCovLogitTK() {  /* empty dtor */ }
  // update the transition kernel with new covariance information
  virtual void updateTK();
  virtual bool covMatrixIsDirty() { return covIsDirty; }
  virtual void cleanCovMatrix()   { covIsDirty = false; }

private:
  /// calibration parameter vector set (note: hides base class member)
  const QUESO::VectorSet<V, M> & m_vectorSet;
  //using QUESO::TransformedScaledCovMatrixTKGroup<V, M>::m_vectorSpace;
  /// whether we've updated the proposal covariance
  bool covIsDirty;
  /// index into current chain position
  unsigned int chainIndex;

  /// Dakota QUESO instance for callbacks
  NonDQUESOBayesCalibration* nonDQUESOInstance;
};





/// Custom RW TKfactory: passes Dakota QUESO instance pointer to the TK at build
/** Can't share this factory between random walk and logit as their
    constructor arguments differ */
// BMA TODO: Remove this needless template parameter
template <class DerivedTK>
class TKFactoryDIPC : public QUESO::TransitionKernelFactory
{
public:
  TKFactoryDIPC(const std::string & name)
    : QUESO::TransitionKernelFactory(name), nonDQUESOInstance(NULL)
  {  /* empty ctor */  }

  virtual ~TKFactoryDIPC() {  /* empty dtor */  }

  void set_callback(NonDQUESOBayesCalibration* queso_instance)
  { nonDQUESOInstance = queso_instance; }

protected:
  virtual QUESO::SharedPtr<QUESO::BaseTKGroup<QUESO::GslVector, QUESO::GslMatrix> >::Type build_tk()
  {
    QUESO::SharedPtr<QUESO::BaseTKGroup<QUESO::GslVector, QUESO::GslMatrix> >::Type new_tk;

    new_tk.reset( new DerivedTK(this->m_options->m_prefix.c_str(),
				*(this->m_vectorSpace),
				*(this->m_dr_scales),
				*(this->m_initial_cov_matrix),
				this->nonDQUESOInstance) );

    return new_tk;
  }

private:
  NonDQUESOBayesCalibration* nonDQUESOInstance;
};




/// Custom Logit RW TKfactory: passed Dakota QUESO instance pointer to the TK at build
/** Can't share this factory between random walk and logit as their
    constructor arguments differ */
// BMA TODO: Remove the template parameter
template <class DerivedTK>
class TKFactoryDIPCLogit: public QUESO::TransitionKernelFactory
{
public:
  TKFactoryDIPCLogit(const std::string & name)
    : QUESO::TransitionKernelFactory(name), nonDQUESOInstance(NULL)
  {  /* empty ctor */  }

  virtual ~TKFactoryDIPCLogit() {  /* empty dtor */  }

  void set_callback(NonDQUESOBayesCalibration* queso_instance)
  { nonDQUESOInstance = queso_instance; }

protected:
  virtual QUESO::SharedPtr<QUESO::BaseTKGroup<QUESO::GslVector, QUESO::GslMatrix> >::Type build_tk()
  {
    QUESO::SharedPtr<QUESO::BaseTKGroup<QUESO::GslVector, QUESO::GslMatrix> >::Type new_tk;

    new_tk.reset( new DerivedTK(this->m_options->m_prefix.c_str(),
				(this->m_target_pdf->domainSet()),
				*(this->m_dr_scales),
				*(this->m_initial_cov_matrix),
				this->nonDQUESOInstance) );

    return new_tk;
  }

private:
  NonDQUESOBayesCalibration* nonDQUESOInstance;
};


}

#endif
