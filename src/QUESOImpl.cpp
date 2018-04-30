/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "QUESOImpl.hpp"
#include "NonDQUESOBayesCalibration.hpp"

namespace Dakota {


template<class V,class M>
QuesoJointPdf<V,M>::QuesoJointPdf(const char* prefix,
				  const QUESO::VectorSet<V,M>& domainSet,
				  NonDQUESOBayesCalibration*   nond_queso_ptr)
  : QUESO::BaseJointPdf<V,M>(((std::string)(prefix)+"generic").c_str(),
			     domainSet), nonDQUESOInstance(nond_queso_ptr)
{
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Entering QuesoJointPdf<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Leaving QuesoJointPdf<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }
}


template<class V,class M>
QuesoJointPdf<V,M>::~QuesoJointPdf()
{ }


template<class V,class M>
double QuesoJointPdf<V,M>::
actualValue(const V& domainVector, const V* domainDirection,
	    V* gradVector, M* hessianMatrix, V* hessianEffect) const
{ return nonDQUESOInstance->prior_density(domainVector); }


template<class V,class M>
double QuesoJointPdf<V,M>::
lnValue(const V& domainVector, const V* domainDirection,
	V* gradVector, M* hessianMatrix, V* hessianEffect) const
{ return nonDQUESOInstance->log_prior_density(domainVector); }


template<class V,class M>
double QuesoJointPdf<V,M>::
computeLogOfNormalizationFactor(unsigned int numSamples,
				bool m_logOfNormalizationFactor) const
{ }


/** Assumes meanVector is sized */
template<class V,class M>
void QuesoJointPdf<V,M>::
distributionMean(V & meanVector) const
{
  nonDQUESOInstance->prior_mean(meanVector);
}


/** Assumes covMatrix is sized */
template<class V,class M>
void QuesoJointPdf<V,M>::
distributionVariance(M & covMatrix) const
{
  nonDQUESOInstance->prior_variance(covMatrix);
}


template<class V, class M>
QuesoVectorRV<V,M>::QuesoVectorRV(const char* prefix,
				  const QUESO::VectorSet<V,M>& imageSet,
				  NonDQUESOBayesCalibration*   nond_queso_ptr)
  : QUESO::BaseVectorRV<V,M>(((std::string)(prefix)+"generic").c_str(),imageSet)
{
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54))
    *m_env.subDisplayFile() << "Entering QuesoVectorRV<V,M>::constructor()"
                            << ": prefix = " << m_prefix << std::endl;

  m_pdf = new QuesoJointPdf<V,M>(m_prefix.c_str(),
				 m_imageSet, nond_queso_ptr);
  // m_realizer   = NULL; // FIX ME: complete code
  // m_subCdf     = NULL; // FIX ME: complete code
  // m_unifiedCdf = NULL; // FIX ME: complete code
  // m_mdf        = NULL; // FIX ME: complete code

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54))
    *m_env.subDisplayFile() << "Leaving QuesoVectorRV<V,M>::constructor()"
                            << ": prefix = " << m_prefix << std::endl;
}


template<class V, class M>
QuesoVectorRV<V,M>::~QuesoVectorRV()
{
  // delete m_mdf;
  // delete m_unifiedCdf;
  // delete m_subCdf;
  // delete m_realizer;
  delete m_pdf;
}


template <class V, class M>
void
QuesoVectorRV<V,M>::print(std::ostream& os) const
{
  os << "QuesoVectorRV<V,M>::print() says, 'Please implement me.'" << std::endl;
  return;
}


template <class V, class M>
DerivInformedPropCovTK<V, M>::
DerivInformedPropCovTK(const char * prefix,
		       const QUESO::VectorSpace<V, M> & vectorSpace,
		       const std::vector<double> & scales,
		       const M & covMatrix,
		       NonDQUESOBayesCalibration* queso_instance):
  QUESO::ScaledCovMatrixTKGroup<V, M>(prefix, vectorSpace, scales, covMatrix),
  m_vectorSpace(vectorSpace), covIsDirty(false), chainIndex(0),
  nonDQUESOInstance(queso_instance)
{  /* empty ctor  */  }


template <class V, class M>
void DerivInformedPropCovTK<V, M>::updateTK()
{
  // QUESO stores the initial position in chain[0]. This updateTK()
  // function gets called _after_ chain[i] is populated if (i %
  // propCovUpdatePeriod) == 0, so this will be the index of the last
  // chain position accepted:
  chainIndex += nonDQUESOInstance->propCovUpdatePeriod;

  // compute update to proposal cov and tell AM it's dirty
  nonDQUESOInstance->precondition_proposal(chainIndex);
  this->updateLawCovMatrix(*nonDQUESOInstance->proposalCovMatrix);
  covIsDirty = true;

  if (nonDQUESOInstance->output_level() >= DEBUG_OUTPUT)
    Cout << "QUESO updateTK: New proposal covariance at sample "
	 << chainIndex + 1 << ":\n" << *nonDQUESOInstance->proposalCovMatrix;
}


template <class V, class M>
DerivInformedPropCovLogitTK<V, M>::
DerivInformedPropCovLogitTK(const char * prefix,
		       const QUESO::VectorSet<V, M> & vectorSet,
		       const std::vector<double> & scales,
		       const M & covMatrix,
		       NonDQUESOBayesCalibration* queso_instance):
  QUESO::TransformedScaledCovMatrixTKGroup<V, M>(prefix, vectorSet, scales, covMatrix),
  m_vectorSet(vectorSet), covIsDirty(false), chainIndex(0),
  nonDQUESOInstance(queso_instance)
{  /* empty ctor  */  }


template <class V, class M>
void DerivInformedPropCovLogitTK<V, M>::updateTK()
{
  chainIndex += nonDQUESOInstance->propCovUpdatePeriod;

  // compute update to proposal cov
  nonDQUESOInstance->precondition_proposal(chainIndex);
  this->updateLawCovMatrix(*nonDQUESOInstance->proposalCovMatrix);
  covIsDirty = true;

  if (nonDQUESOInstance->output_level() >= DEBUG_OUTPUT)
    Cout << "QUESO updateTK(): New proposal covariance at sample "
	 << chainIndex + 1 << ":\n" << *nonDQUESOInstance->proposalCovMatrix;
}


// Explicit instantiation of the templates
template class QuesoJointPdf<QUESO::GslVector, QUESO::GslMatrix>;
template class QuesoVectorRV<QUESO::GslVector, QUESO::GslMatrix>;
template class DerivInformedPropCovTK<QUESO::GslVector, QUESO::GslMatrix>;
template class DerivInformedPropCovLogitTK<QUESO::GslVector, QUESO::GslMatrix>;

}  // namespace Dakota
