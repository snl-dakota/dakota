/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SurfpackApproximation 
//- Description:  Class for polynomial response surface approximation
//-               
//- Owner:        Brian Adams, Sandia National Laboratories
 

#ifndef SHARED_SURFPACK_APPROX_DATA_H 
#define SHARED_SURFPACK_APPROX_DATA_H 

#include "SharedApproxData.hpp"

class SurfData;
template< typename T > class SurfpackMatrix;

namespace Dakota {


/// Derived approximation class for Surfpack approximation classes.
/// Interface between Surfpack and Dakota.

/** The SharedSurfpackApproxData class is the interface between Dakota
    and Surfpack.  Based on the information in the ProblemDescDB that
    is passed in through the constructor, SharedSurfpackApproxData builds
    a Surfpack Surface object that corresponds to one of the following
    data-fitting techniques: polynomial regression, kriging,
    artificial neural networks, radial basis function network, or
    multivariate adaptaive regression splines (MARS). */

class SharedSurfpackApproxData: public SharedApproxData
{
  //
  //- Heading: Friends
  //

  friend class SurfpackApproximation;
    
    friend class VPSApproximation;  // Mohamed Ebeida

public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SharedSurfpackApproxData();
  /// alternate constructor
  SharedSurfpackApproxData(const String& approx_type,
			   const UShortArray& approx_order, size_t num_vars,
			   short data_order, short output_level);
  /// standard constructor: Surfpack surface of appropriate type will be created
  SharedSurfpackApproxData(ProblemDescDB& problem_db, size_t num_vars);
  /// destructor
  ~SharedSurfpackApproxData();

protected:

private:

  //
  //- Heading: Convenience functions
  //

  /// add Pecos::SurrogateData::SurrogateData{Vars,Resp} to SurfData,
  /// accounting for buildDataOrder available
  void add_sd_to_surfdata(const Pecos::SurrogateDataVars& sdv,
			  const Pecos::SurrogateDataResp& sdr, short fail_code,
			  SurfData& surf_data);

  /// copy RealSymMatrix to SurfpackMatrix (Real type only)
  void copy_matrix(const RealSymMatrix& rsm,
		   SurfpackMatrix<Real>& surfpack_matrix);

  /// merge cv, div, and drv vectors into a single ra array
  void merge_variable_arrays(const RealVector& cv,  const IntVector& div,
			     const RealVector& drv, RealArray& ra);
  /// aggregate {continuous,discrete int,discrete real} variables 
  /// from SurrogateDataVars into ra
  void sdv_to_realarray(const Pecos::SurrogateDataVars& sdv, RealArray& ra);
  /// aggregate {active,all} {continuous,discrete int,discrete real}
  /// variables into ra
  void vars_to_realarray(const Variables& vars, RealArray& ra);

  //
  //- Heading: Data
  //

  /// order of polynomial approximation
  unsigned short approxOrder;

  /// set of diagnostic metrics
  StringArray diagnosticSet;

  /// whether to perform cross validation
  bool crossValidateFlag;
  /// number of folds for CV
  unsigned numFolds;
  /// percentage of data for CV
  Real percentFold;
  /// whether to perform PRESS
  bool pressFlag;
};


inline SharedSurfpackApproxData::SharedSurfpackApproxData()
{ }


inline SharedSurfpackApproxData::~SharedSurfpackApproxData()
{ }

} // namespace Dakota
#endif
