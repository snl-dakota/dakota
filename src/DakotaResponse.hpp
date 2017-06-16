/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Response
//- Description:  Container class for response functions and their derivatives.
//-
//- Owner:        Mike Eldred
//- Version: $Id: DakotaResponse.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DAKOTA_RESPONSE_H
#define DAKOTA_RESPONSE_H

#include "DataResponses.hpp"
#include "DakotaActiveSet.hpp"
#include "SharedResponseData.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

namespace Dakota {

class ProblemDescDB;


/// Container class for response functions and their derivatives.  
/// Response provides the enveloper base class.

/** The Response class is a container class for an abstract set of
    functions (functionValues) and their first (functionGradients) and
    second (functionHessians) derivatives.  The functions may involve
    objective and constraint functions (optimization data set), least
    squares terms (parameter estimation data set), or generic response
    functions (uncertainty quantification data set).  For memory
    efficiency, it employs the "letter-envelope idiom" approach to
    reference counting and representation sharing (see Coplien
    "Advanced C++"), for which the base Response class serves as the
    envelope and one of its derived classes serves as the letter. */

class Response
{
  //
  //- Heading: Friends
  //

  /// equality operator
  friend bool operator==(const Response& resp1, const Response& resp2);
  /// inequality operator
  friend bool operator!=(const Response& resp1, const Response& resp2);

public:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Response();
  /// standard constructor built from problem description database
  Response(short type, const Variables& vars, const ProblemDescDB& problem_db);
  /// alternate constructor that shares response data
  Response(const SharedResponseData& srd, const ActiveSet& set);
  /// alternate constructor using limited data without sharing
  Response(short type, const ActiveSet& set);
  /// alternate constructor using limited data (explicit disallows implicit
  /// type conversion)
  explicit Response(const SharedResponseData& srd);
  /// copy constructor
  Response(const Response& response);
  /// destructor
  virtual ~Response();

  //
  //- Heading: operators
  //

  /// assignment operator
  Response operator=(const Response& response);
  
  //
  //- Heading: Member functions
  //

  /// return sharedRespData
  const SharedResponseData& shared_data() const;
  /// return sharedRespData
  SharedResponseData& shared_data();

  /// return the number of response functions
  size_t num_functions() const;

  /// return the active set
  const ActiveSet& active_set() const;
  /// set the active set
  void active_set(const ActiveSet& set);
  /// return the active set request vector
  const ShortArray& active_set_request_vector() const;
  /// set the active set request vector and verify consistent number
  /// of response functions
  void active_set_request_vector(const ShortArray& asrv);
  /// return the active set derivative vector
  const SizetArray& active_set_derivative_vector() const;
  /// set the active set derivative vector and reshape
  /// functionGradients/functionHessians if needed
  void active_set_derivative_vector(const SizetArray& asdv);

  /// return a function value
  const Real& function_value(size_t i) const;
  /// return a "view" of a function value for updating in place
  Real& function_value_view(size_t i);
  /// return all function values
  const RealVector& function_values() const;
  /// return all function values as a view for updating in place
  RealVector function_values_view();
  /// return all function values as a view for accessing the function
  /// values vector from a const response
  RealVector function_values_view() const;
  /// set a function value
  void function_value(const Real& function_val, size_t i);
  /// set all function values
  void function_values(const RealVector& function_vals);
  
  /// return the i-th function gradient as a const Real*
  const Real* function_gradient(int i) const;
  /// return the i-th function gradient as a SerialDenseVector view
  /// (shallow copy) for updating in place
  RealVector function_gradient_view(int i);
  /// return the i-th function gradient as a SerialDenseVector view
  /// (shallow copy) for accessing a column vector from a const matrix
  RealVector function_gradient_view(int i) const;
  /// return the i-th function gradient as a SerialDenseVector
  /// Teuchos::Copy (deep copy)
  RealVector function_gradient_copy(int i) const;
  /// return all function gradients
  const RealMatrix& function_gradients() const;
  /// return all function gradients as a view for updating in place
  RealMatrix function_gradients_view();
  /// return all function gradients as a view for updating in place
  RealMatrix function_gradients_view() const;
  /// set a function gradient
  void function_gradient(const RealVector& function_grad, int i);
  /// set all function gradients
  void function_gradients(const RealMatrix& function_grads);

  /// return the i-th function Hessian
  const RealSymMatrix& function_hessian(size_t i) const;
  /// return the i-th function Hessian as a Teuchos::View (shallow copy)
  /// for updating in place
  RealSymMatrix function_hessian_view(size_t i);
  /// return the i-th function Hessian as a Teuchos::View (shallow copy)
  /// for accessing the i-th matrix within a const matrix array
  RealSymMatrix function_hessian_view(size_t i) const;
  /// return all function Hessians
  const RealSymMatrixArray& function_hessians() const;
  /// return all function Hessians as Teuchos::Views (shallow copies)
  /// for updating in place
  RealSymMatrixArray function_hessians_view();
  /// return all function Hessians as Teuchos::Views (shallow copies)
  /// for updating in place
  RealSymMatrixArray function_hessians_view() const;
  /// set a function Hessian
  void function_hessian(const RealSymMatrix& function_hessian, size_t i);
  /// set all function Hessians
  void function_hessians(const RealSymMatrixArray& function_hessians);

  /// return const field values
  RealVector field_values_view(size_t i) const;
  /// return a "view" of a field value for updating in place
  RealVector field_values_view(size_t i);
  /// set a field value
  void field_values(const RealVector& field_val, size_t i);
  /// return a "view" of a field value's coordinates
  RealMatrix field_coords_view(size_t i);
  /// return a const "view" of a field value's coordinates
  const RealMatrix field_coords_view(size_t i) const;
  /// set a field value's coordinates
  void field_coords(const RealMatrix& field_coords, size_t i);

  /// return a view of the gradients of each field element
  RealMatrix field_gradients_view(size_t i) const;
  /// return a view of the hessians of each field element
  RealSymMatrixArray field_hessians_view(size_t i) const;

  /// return the field lengths from sharedRespData
  const IntVector& field_lengths() const;
  /// set the field lengths within sharedRespData
  void field_lengths(const IntVector& field_lens);
  /// return the num_coords_per_field from sharedRespData
  const IntVector& num_coords_per_field() const;
  /// set the coordinate values per field 
  void set_coord_values(const RealMatrix& coord_values, const size_t i);
  /// return the coordinate values per field 
  const RealMatrix& get_coord_values(const size_t i) const;

  /// return the fine-grained (unrolled) response function identifier
  /// strings from sharedRespData
  const StringArray& function_labels() const;
  /// set the fine-grained (unrolled) response function identifier
  /// strings within sharedRespData
  void function_labels(const StringArray& labels);

  /// return the user-provided field group labels instead of the
  /// unrolled labels available through function_labels()
  const StringArray& field_group_labels();

  /// read a response object of specified format from an std::istream 
  void read(std::istream& s, const unsigned short format = FLEXIBLE_RESULTS);
 
  /// write a response object to an std::ostream
  void write(std::ostream& s) const;

  /// read a response object in annotated format from an std::istream
  void read_annotated(std::istream& s);
  /// write a response object in annotated format to an std::ostream
  void write_annotated(std::ostream& s) const;

  /// read responseRep::functionValues in tabular format from an std::istream
  void read_tabular(std::istream& s);
  /// write responseRep::functionValues in tabular format to an std::ostream
  void write_tabular(std::ostream& s) const;

  /// write the labels to a tabular data stream
  void write_tabular_labels(std::ostream& s) const;

  /// read a response object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a response object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  /// return a deep response copy of the contained responseRep for use
  /// in history mechanisms (SharedResponseData uses a shallow copy by
  /// default)
  Response copy(bool deep_srd = false) const;

  /// return the number of doubles active in response.  Used for sizing 
  /// double* response_data arrays passed into read_data and write_data.
  int data_size();
  /// read from an incoming double* array
  void read_data(double* response_data);
  /// write to an incoming double* array
  void write_data(double* response_data);
  /// add incoming response to functionValues/Gradients/Hessians
  void overlay(const Response& response);
  /// Used in place of operator= when only results data updates are
  /// desired (functionValues/functionGradients/functionHessians are
  /// updated, ASV/labels/id's/etc. are not).  Care is taken to allow
  /// different derivative array sizing between the two response objects.
  void update(const Response& response);
  /// Overloaded form which allows update from components of a response
  /// object.  Care is taken to allow different derivative array sizing.
  void update(const RealVector& source_fn_vals,
	      const RealMatrix& source_fn_grads,
	      const RealSymMatrixArray& source_fn_hessians,
	      const ActiveSet& source_set);
  /// partial update of this response object from another response object.
  /// The response objects may have different numbers of response functions.
  void update_partial(size_t start_index_target, size_t num_items,
		      const Response& response, size_t start_index_source);
  /// Overloaded form which allows partial update from components of a 
  /// response object.  The response objects may have different numbers
  /// of response functions.
  void update_partial(size_t start_index_target, size_t num_items,
		      const RealVector& source_fn_vals,
		      const RealMatrix& source_fn_grads,
		      const RealSymMatrixArray& source_fn_hessians,
		      const ActiveSet& source_set, size_t start_index_source);

  /// rehapes response data arrays
  void reshape(size_t num_fns, size_t num_params, bool grad_flag,
	       bool hess_flag);
  /// resets all response data to zero
  void reset();
  /// resets all inactive response data to zero
  void reset_inactive();

  /// function to check responseRep (does this handle contain a body)
  bool is_null() const;
 
  /// method to set the covariance matrix defined for ExperimentResponse
  virtual void set_scalar_covariance(RealVector& scalars);
  /// retrieve the ExperimentCovariance structure
  virtual const ExperimentCovariance& experiment_covariance() const;
  /// method to set the full covariance matrices for ExperimentResponse
  virtual void set_full_covariance(std::vector<RealMatrix> &matrices, 
                           std::vector<RealVector> &diagonals,
                           RealVector &scalars,
                           IntVector matrix_map_indices,                      
                           IntVector diagonal_map_indices, 
                           IntVector scalar_map_indices ); 
  /// method to compute the triple product v'*inv(C)*v.
  virtual Real apply_covariance(const RealVector &residuals) const;
  /// method to compute (v'*inv(C)^1/2), to compute weighted residual
  virtual void apply_covariance_inv_sqrt(const RealVector &residuals, 
					 RealVector& weighted_residuals) const;
  virtual void apply_covariance_inv_sqrt(const RealMatrix& gradients, 
					 RealMatrix& weighted_gradients) const;
  virtual void apply_covariance_inv_sqrt(const RealSymMatrixArray& hessians,
					 RealSymMatrixArray& weighted_hessians) const;

  virtual void get_covariance_diagonal( RealVector &diagonal ) const;

  /// covariance determinant for one experiment (default 1.0)
  virtual Real covariance_determinant() const;

  /// log of covariance determinant for one experiment (default 0.0)
  virtual Real log_covariance_determinant() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Response(BaseConstructor, const Variables& vars,
	   const ProblemDescDB& problem_db);
  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Response(BaseConstructor, const SharedResponseData& srd,
	   const ActiveSet& set);
  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Response(BaseConstructor, const ActiveSet& set);
  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Response(BaseConstructor, const SharedResponseData& srd);


  //
  //- Heading: Virtual member functions
  //

  /// Implementation of data copy for Response letters (specialized by
  /// some derived letter types); pulls base class data from
  /// source_resp_rep into the this object.  
  virtual void copy_rep(Response* source_resp_rep);


  //
  //- Heading: Protected data members
  //

  /// reference-counted instance of shared response data: id's, labels
  SharedResponseData sharedRespData;

  // An abstract set of functions and their first and second derivatives.

  /// abstract set of response functions
  RealVector functionValues;
  /// first derivatives of the response functions
  /** the gradient vectors (plural) are column vectors in the matrix
      (singular) with (row, col) = (variable index, response fn index). */
  RealMatrix functionGradients;
  /// second derivatives of the response functions
  RealSymMatrixArray functionHessians;
  /// coordinates for the field values
  IntRealMatrixMap fieldCoords; // not all field have associated coords - RWH

  /// copy of the ActiveSet used by the Model to generate a Response instance
  ActiveSet responseActiveSet;

private:

  friend class boost::serialization::access;

  /// write a column of a SerialDenseMatrix
  template<class Archive, typename OrdinalType, typename ScalarType>
  void write_sdm_col
  (Archive& ar, int col,
   const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm) const;

  /// read a column of a SerialDenseMatrix
  template<class Archive, typename OrdinalType, typename ScalarType>
  void read_sdm_col(Archive& ar, int col, 
		    Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm);

  /// read a Response from an archive<class Archive>
  template<class Archive>
  void load(Archive& ar, const unsigned int version);

  /// read a Response letter object from an archive
  template<class Archive> 
  void load_rep(Archive& ar, const unsigned int version);

  /// write a Response to an archive
  template<class Archive>
  void save(Archive& ar, const unsigned int version) const;

  /// write a Response letter object to an archive
  template<class Archive> 
  void save_rep(Archive& ar, const unsigned int version) const;

  BOOST_SERIALIZATION_SPLIT_MEMBER()


  /// Used by standard envelope constructor to instantiate a new letter class
  Response* get_response(short type, const Variables& vars,
			 const ProblemDescDB& problem_db) const;
  /// Used by alternate envelope constructor to instantiate a new letter class
  Response* get_response(const SharedResponseData& srd,
			 const ActiveSet& set) const;
  /// Used by alternate envelope constructor to instantiate a new letter class
  Response* get_response(short type, const ActiveSet& set) const;
  /// Used by copy() to instantiate a new letter class
  Response* get_response(const SharedResponseData& srd) const;
  /// Used by read functions to instantiate a new letter class
  Response* get_response(short type) const;

  /// read a letter object in annotated format from an std::istream
  void read_annotated_rep(std::istream& s);
  /// write a letter object in annotated format to an std::ostream
  void write_annotated_rep(std::ostream& s) const;
  /// read a letter object from a packed MPI buffer
  void read_rep(MPIUnpackBuffer& s);
  /// write a letter object to a packed MPI buffer
  void write_rep(MPIPackBuffer& s) const;

  /// resizes the representation's containers
  void shape_rep(const ActiveSet& set, bool initialize = true);
  /// resizes the representation's containers
  void reshape_rep(size_t num_fns, size_t num_params, bool grad_flag,
		   bool hess_flag);

  /// Read gradients from a freeform stream. Insert error messages
  // into errors stream.
  void read_gradients(std::istream& s, const ShortArray &asv, 
      std::ostringstream &error);

  /// Read Hessians from a freeform stream. Insert error messages
  // into errors stream.
  void read_hessians(std::istream& s, const ShortArray &asv,
       std::ostringstream &error);

  /// Read function values from an annotated stream. Insert error messages
  // into errors stream. 
  void read_labeled_fn_vals(std::istream &s, const ShortArray &asv,
      std::ostringstream &errors);

  /// Read function values from a stream in a "flexible" way -- ignoring 
  /// any labels. Insert error messages into errors stream.
  void read_flexible_fn_vals(std::istream &s, const ShortArray &asv,
      std::ostringstream &errors);

/*  /// Read function values from a freeform stream. Insert error messages
  // into errors stream.
  void read_freeform_fn_vals(std::istream& s, const ShortArray &asv, 
      std::ostringstream &error);
*/

  /// Check for FAIL in stream
  bool failure_reported(std::istream &s);

  //
  //- Heading: Private data members
  //

  /// pointer to the body (handle-body idiom)
  Response* responseRep;
  /// number of handle objects sharing responseRep
  int referenceCount;
};


inline const SharedResponseData& Response::shared_data() const
{ return (responseRep) ? responseRep->sharedRespData : sharedRespData; }


inline SharedResponseData& Response::shared_data()
{ return (responseRep) ? responseRep->sharedRespData : sharedRespData; }


inline size_t Response::num_functions() const
{
  return (responseRep) ? responseRep->functionValues.length() :
    functionValues.length();
}


inline const Real& Response::function_value(size_t i) const
{
  if (responseRep) return responseRep->functionValues[i];
  else             return functionValues[i];
}


inline Real& Response::function_value_view(size_t i)
{
  if (responseRep) return responseRep->functionValues[i];
  else             return functionValues[i];
}


inline const RealVector& Response::function_values() const
{
  if (responseRep) return responseRep->functionValues;
  else             return functionValues;
}


inline RealVector Response::field_values_view(size_t i) const
{
  if (responseRep)
    return responseRep->field_values_view(i);
  else {
    size_t j, cntr = sharedRespData.num_scalar_responses();
    const IntVector& field_len = sharedRespData.field_lengths();
    for (j=0; j<i; j++)
      cntr += field_len[j];
    return RealVector(Teuchos::View, const_cast<Real*>(&functionValues[cntr]), field_len[i]);
  }
}

inline RealVector Response::field_values_view(size_t i)
{
  if (responseRep)
    return responseRep->field_values_view(i);
  else {
    size_t j, cntr = sharedRespData.num_scalar_responses();
    const IntVector& field_len = sharedRespData.field_lengths();
    for (j=0; j<i; j++)
      cntr += field_len[j];
    return RealVector(Teuchos::View, &functionValues[cntr], field_len[i]);
  }
}

inline RealMatrix Response::field_coords_view(size_t i)
{
  if (responseRep)
    return responseRep->field_coords_view(i);
  else {
    if( fieldCoords.find(i) == fieldCoords.end() )
      return RealMatrix(); // return empty matrix?  error out? - RWH
    else
      return RealMatrix(Teuchos::View, fieldCoords[i], fieldCoords[i].numRows(), fieldCoords[i].numCols());
  }
}


inline const RealMatrix Response::field_coords_view(size_t i) const
{
  const RealMatrix matrix = const_cast<Response*>(this)->field_coords_view(i);
  return matrix;
}


inline RealVector Response::function_values_view()
{
  return (responseRep) ?
    RealVector(Teuchos::View, responseRep->functionValues.values(),
	       responseRep->functionValues.length()) :
    RealVector(Teuchos::View, functionValues.values(), functionValues.length());
}


inline RealVector Response::function_values_view() const
{
  // Note: this const version is largely subsumed by const RealVector&
  // function_values().  We include it just for consistency with other
  // function_*_view().
  return (responseRep) ?
    RealVector(Teuchos::View, responseRep->functionValues.values(),
	       responseRep->functionValues.length()) :
    RealVector(Teuchos::View, const_cast<Real*>(functionValues.values()),
	       functionValues.length());
}


inline void Response::function_value(const Real& function_val, size_t i)
{
  if (responseRep) responseRep->functionValues[i] = function_val;
  else             functionValues[i] = function_val;
}


inline void Response::field_values(const RealVector& field_vals, size_t i)
{
  if (responseRep)
    responseRep->field_values(field_vals, i);
  else {
    const IntVector& field_len = sharedRespData.field_lengths();
    size_t j, cntr = sharedRespData.num_scalar_responses();
    for (j=0; j<i; j++)
      cntr += field_len[j];
    size_t len_i = field_len[i];
    for (j=0; j<len_i; ++j, ++cntr)
      functionValues[cntr] = field_vals[j];
  }
}


inline void Response::field_coords(const RealMatrix& coords, size_t i)
{
  if (responseRep)
    responseRep->field_coords(coords, i);
  else
    fieldCoords[i] = coords;
}


inline void Response::function_values(const RealVector& function_vals)
{
  if (responseRep) responseRep->functionValues = function_vals;
  else             functionValues = function_vals;
}


inline const Real* Response::function_gradient(int i) const
{
  if (responseRep) return responseRep->functionGradients[i];
  else             return functionGradients[i];
}


inline RealVector Response::function_gradient_view(int i)
{   
  if (responseRep)
    return Teuchos::getCol(Teuchos::View, responseRep->functionGradients, i);
  else
    return Teuchos::getCol(Teuchos::View, functionGradients, i);
}


inline RealVector Response::function_gradient_view(int i) const
{   
  if (responseRep)
    return Teuchos::getCol(Teuchos::View, responseRep->functionGradients, i);
  else
    return RealVector(Teuchos::View, const_cast<Real*>(functionGradients[i]),
		      functionGradients.numRows());
}


inline RealVector Response::function_gradient_copy(int i) const
{
  if (responseRep)
    return Teuchos::getCol(Teuchos::Copy, responseRep->functionGradients, i);
  else
    return RealVector(Teuchos::Copy, const_cast<Real*>(functionGradients[i]),
		      functionGradients.numRows());
}


inline const RealMatrix& Response::function_gradients() const
{ return (responseRep) ? responseRep->functionGradients : functionGradients; }


inline RealMatrix Response::function_gradients_view()
{
  if (responseRep)
    return RealMatrix(Teuchos::View, responseRep->functionGradients,
		      responseRep->functionGradients.numRows(),
		      responseRep->functionGradients.numCols());
  else
    return RealMatrix(Teuchos::View, functionGradients,
		      functionGradients.numRows(), functionGradients.numCols());
}

inline RealMatrix Response::function_gradients_view() const
{
  if (responseRep)
    return RealMatrix(Teuchos::View, responseRep->functionGradients,
		      responseRep->functionGradients.numRows(),
		      responseRep->functionGradients.numCols());
  else
    return RealMatrix(Teuchos::View, functionGradients,
		      functionGradients.numRows(), functionGradients.numCols());
}


inline void Response::
function_gradient(const RealVector& function_grad, int i)
{
  if (responseRep)
    Teuchos::setCol(function_grad, i, responseRep->functionGradients);
  else
    Teuchos::setCol(function_grad, i, functionGradients);
}


inline void Response::function_gradients(const RealMatrix& function_grads)
{
  if (responseRep) responseRep->functionGradients = function_grads;
  else             functionGradients = function_grads;
}


inline const RealSymMatrix& Response::function_hessian(size_t i) const
{
  if (responseRep) return responseRep->functionHessians[i];
  else             return functionHessians[i];
}


inline RealSymMatrix Response::function_hessian_view(size_t i)
{
  // Note: there is no implementation difference from the const version in
  // this case, such that maintaining just the stricter const version would
  // be sufficient.  Maintaining the distinction is just for consistency with
  // other function_*_view().
  if (responseRep)
    return RealSymMatrix(Teuchos::View, responseRep->functionHessians[i],
			 responseRep->functionHessians[i].numRows());
  else
    return RealSymMatrix(Teuchos::View, functionHessians[i],
			 functionHessians[i].numRows());
}


inline RealSymMatrix Response::function_hessian_view(size_t i) const
{
  if (responseRep)
    return RealSymMatrix(Teuchos::View, responseRep->functionHessians[i],
			 responseRep->functionHessians[i].numRows());
  else
    return RealSymMatrix(Teuchos::View, functionHessians[i],
			 functionHessians[i].numRows());
}


inline const RealSymMatrixArray& Response::function_hessians() const
{
  if (responseRep) return responseRep->functionHessians;
  else             return functionHessians;
}


inline RealSymMatrixArray Response::function_hessians_view()
{
  if (responseRep)
    return responseRep->function_hessians_view();
  else {
    size_t i, num_hess = functionHessians.size();
    RealSymMatrixArray fn_hessians_view(num_hess);
    for (i=0; i<num_hess; ++i)
      fn_hessians_view[i] = RealSymMatrix(Teuchos::View, functionHessians[i],
					  functionHessians[i].numRows());
    return fn_hessians_view;
  }
}

inline RealSymMatrixArray Response::function_hessians_view() const
{
  if (responseRep)
    return responseRep->function_hessians_view();
  else {
    size_t i, num_hess = functionHessians.size();
    RealSymMatrixArray fn_hessians_view(num_hess);
    for (i=0; i<num_hess; ++i)
      fn_hessians_view[i] = RealSymMatrix(Teuchos::View, functionHessians[i],
					  functionHessians[i].numRows());
    return fn_hessians_view;
  }
}


inline void Response::
function_hessian(const RealSymMatrix& function_hessian, size_t i)
{
  if (responseRep) responseRep->functionHessians[i] = function_hessian;
  else             functionHessians[i] = function_hessian;
}


inline void Response::
function_hessians(const RealSymMatrixArray& function_hessians)
{
  if (responseRep) responseRep->functionHessians = function_hessians;
  else             functionHessians = function_hessians;
}


inline const IntVector& Response::field_lengths() const
{
  if (responseRep) return responseRep->sharedRespData.field_lengths();
  else             return sharedRespData.field_lengths();
}


inline const IntVector& Response::num_coords_per_field() const
{
  if (responseRep) return responseRep->sharedRespData.num_coords_per_field();
  else             return sharedRespData.num_coords_per_field();
}


inline const StringArray& Response::function_labels() const
{
  if (responseRep) return responseRep->sharedRespData.function_labels();
  else             return sharedRespData.function_labels();
}


inline void Response::function_labels(const StringArray& fn_labels)
{
  if (responseRep) responseRep->sharedRespData.function_labels(fn_labels);
  else             sharedRespData.function_labels(fn_labels);
}

inline const StringArray& Response::field_group_labels()
{
  if (responseRep) 
    return responseRep->sharedRespData.field_group_labels();
  else             
    return sharedRespData.field_group_labels();
}

inline const ActiveSet& Response::active_set() const
{ return (responseRep) ? responseRep->responseActiveSet : responseActiveSet; }


inline void Response::active_set(const ActiveSet& set)
{
  if (responseRep) {
    responseRep->active_set_request_vector(set.request_vector());
    responseRep->active_set_derivative_vector(set.derivative_vector());
  }
  else {
    active_set_request_vector(set.request_vector());
    active_set_derivative_vector(set.derivative_vector());
  }
}


inline const ShortArray& Response::active_set_request_vector() const
{
  if (responseRep) return responseRep->responseActiveSet.request_vector();
  else             return responseActiveSet.request_vector();
}


inline const SizetArray& Response::active_set_derivative_vector() const
{
  if (responseRep) return responseRep->responseActiveSet.derivative_vector();
  else             return responseActiveSet.derivative_vector();
}


inline void Response::update(const Response& response)
{
  // rep forward handled downstream
  update(response.function_values(), response.function_gradients(),
	 response.function_hessians(), response.active_set());
}


inline void Response::
update_partial(size_t start_index_target, size_t num_items,
	       const Response& response, size_t start_index_source)
{
  // rep forward handled downstream
  update_partial(start_index_target, num_items, response.function_values(),
		 response.function_gradients(), response.function_hessians(),
		 response.active_set(), start_index_source);
}


inline bool Response::is_null() const
{ return (responseRep == NULL); }


inline RealMatrix Response::field_gradients_view(size_t i) const
{
  if (responseRep)
    return responseRep->field_gradients_view(i);
  else {
    size_t j, cntr = sharedRespData.num_scalar_responses();
    const IntVector& field_len = sharedRespData.field_lengths();
    for (j=0; j<i; j++)
      cntr += field_len[j];
    return RealMatrix(Teuchos::View, functionGradients, 
		      functionGradients.numRows(), field_len[i],
		      0, cntr);
  }
}


inline RealSymMatrixArray Response::field_hessians_view(size_t i) const
{
  if (responseRep)
    return responseRep->field_hessians_view(i);
  else {
    const IntVector& field_len = sharedRespData.field_lengths();
    size_t j, num_field_hess = field_len[i], 
      cntr = sharedRespData.num_scalar_responses();;
    for (j=0; j<i; j++)
      cntr += field_len[j];
    RealSymMatrixArray fn_hessians_view(num_field_hess);
    for (j=0; j<num_field_hess; ++j)
      fn_hessians_view[j] = RealSymMatrix(Teuchos::View,functionHessians[cntr+j],
					  functionHessians[j].numRows());
    return fn_hessians_view;
  }
}


/// global comparison function for Response
inline bool responses_id_compare(const Response& resp, const void* id)
{ return ( *(const String*)id == resp.shared_data().responses_id() ); }


/// std::istream extraction operator for Response.  Calls read(std::istream&).
inline std::istream& operator>>(std::istream& s, Response& response)
{ response.read(s); return s; }


/// std::ostream insertion operator for Response.  Calls write(std::ostream&).
inline std::ostream& operator<<(std::ostream& s, const Response& response)
{ response.write(s); return s; }


/// MPIUnpackBuffer extraction operator for Response.  Calls
/// read(MPIUnpackBuffer&).
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, Response& response)
{ response.read(s); return s; }


/// MPIPackBuffer insertion operator for Response.  Calls write(MPIPackBuffer&).
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const Response& response)
{ response.write(s); return s; }


/// inequality operator for Response
inline bool operator!=(const Response& resp1, const Response& resp2)
{ return !(resp1 == resp2); }

} // namespace Dakota


// Since we may serialize this class through a temporary, force
// serialization mode and no tracking
BOOST_CLASS_IMPLEMENTATION(Dakota::Response, 
			   boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(Dakota::Response, 
		     boost::serialization::track_never)


#endif // !DAKOTA_RESPONSE_H
