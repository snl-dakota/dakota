/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ExperimentResponse
//- Description:  Container class for response functions and their derivatives.
//-
//- Owner:        Laura Swiler
//- Version: $Id: ExperimentResponse.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef EXPERIMENT_RESPONSE_H
#define EXPERIMENT_RESPONSE_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "DakotaResponse.hpp"
#include "ExperimentDataUtils.hpp"
#include "dakota_global_defs.hpp"

namespace Dakota {

class ProblemDescDB;


/// Container class for response functions and their derivatives.  
/// ExperimentResponse provides the body class.

/** The ExperimentResponse class is the "representation" of the
    response container class.  It is the "body" portion of the
    "handle-body idiom" (see Coplien "Advanced C++", p. 58).  The
    handle class (Response) provides for memory efficiency in
    management of multiple response objects through reference counting
    and representation sharing.  The body class (ExperimentResponse)
    actually contains the response data (functionValues,
    functionGradients, functionHessians, etc.).  The representation is
    hidden in that an instance of ExperimentResponse may only be
    created by Response.  Therefore, programmers create
    instances of the Response handle class, and only need to be
    aware of the handle/body mechanisms when it comes to managing
    shallow copies (shared representation) versus deep copies
    (separate representation used for history mechanisms). */

class ExperimentResponse: public Response
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  ExperimentResponse();
  /// standard constructor built from problem description database
  ExperimentResponse(const Variables& vars, const ProblemDescDB& problem_db);
  /// alternate constructor that shares a SharedResponseData instance
  ExperimentResponse(const SharedResponseData& srd, const ActiveSet& set);
  /// alternate constructor that shares a SharedResponseData instance
  ExperimentResponse(const SharedResponseData& srd);
  /// alternate constructor using limited data
  ExperimentResponse(const ActiveSet& set);
  /// destructor
  ~ExperimentResponse();

  void set_scalar_covariance(RealVector& scalars);

  const ExperimentCovariance& experiment_covariance() const;

  void set_full_covariance(std::vector<RealMatrix> &matrices,
                           std::vector<RealVector> &diagonals,
                           RealVector &scalars,
                           IntVector matrix_map_indices,
                           IntVector diagonal_map_indices,
                           IntVector scalar_map_indices );
  
  Real apply_covariance(const RealVector &residual) const;
  void apply_covariance_inv_sqrt(const RealVector& residuals, 
				 RealVector& weighted_residuals) const;
  void apply_covariance_inv_sqrt(const RealMatrix& gradients, 
				 RealMatrix& weighted_gradients) const;
  void apply_covariance_inv_sqrt(const RealSymMatrixArray& hessians,
				 RealSymMatrixArray& weighted_hessians) const;
  void get_covariance_diagonal( RealVector &diagonal ) const;

  /// covariance determinant for this experiment (default 1.0)
  Real covariance_determinant() const;

  /// log covariance determinant for this experiment (default 0.0)
  Real log_covariance_determinant() const;

protected:

  //
  //- Heading: virtual member functions redefinitions
  //

  /// Specialization of copy_rep; pulls base class data as well as
  /// derived specific data from the source rep into the this object.
  void copy_rep(Response* source_resp_rep);

private:

  //
  //- Heading: Private data members
  //

  /// sigma terms... 
 ExperimentCovariance expDataCovariance;
};


inline ExperimentResponse::ExperimentResponse()
{ }


inline ExperimentResponse::~ExperimentResponse()
{ }

} // namespace Dakota


// Since we may serialize this class through a temporary, force
// serialization mode and no tracking
BOOST_CLASS_IMPLEMENTATION(Dakota::ExperimentResponse, 
			   boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(Dakota::ExperimentResponse, 
		     boost::serialization::track_never)

#endif // !EXPERIMENT_RESPONSE_H
