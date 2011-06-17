/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Response
//- Description:  Container class for response functions and their derivatives.
//-
//- Owner:        Mike Eldred
//- Version: $Id: DakotaResponse.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DAKOTA_RESPONSE_H
#define DAKOTA_RESPONSE_H

#include "system_defs.h"
#include "data_types.h"
#include "DakotaActiveSet.H"
#include "Teuchos_SerialDenseHelpers.hpp"
#ifdef DAKOTA_HAVE_BOOST_REGEX
#include <boost/regex.hpp>
#else
#include "CtelRegExp.H"
#endif // DAKOTA_HAVE_BOOST_REGEX

namespace Dakota {

class ProblemDescDB;


/// Container class for response functions and their derivatives.  
/// ResponseRep provides the body class.

/** The ResponseRep class is the "representation" of the
    response container class.  It is the "body" portion of the
    "handle-body idiom" (see Coplien "Advanced C++", p. 58).  The
    handle class (Response) provides for memory efficiency in
    management of multiple response objects through reference counting
    and representation sharing.  The body class (ResponseRep)
    actually contains the response data (functionValues,
    functionGradients, functionHessians, etc.).  The representation is
    hidden in that an instance of ResponseRep may only be
    created by Response.  Therefore, programmers create
    instances of the Response handle class, and only need to be
    aware of the handle/body mechanisms when it comes to managing
    shallow copies (shared representation) versus deep copies
    (separate representation used for history mechanisms). */

class ResponseRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class Response;

  /// equality operator
  friend bool operator==(const ResponseRep& rep1, const ResponseRep& rep2);

private:

  //
  //- Heading: Private member functions
  //

  /// default constructor
  ResponseRep();
  /// standard constructor built from problem description database
  ResponseRep(const Variables& vars, const ProblemDescDB& problem_db);
  /// alternate constructor using limited data
  ResponseRep(const ActiveSet& set);
  /// destructor
  ~ResponseRep();

  /// read a responseRep object from an std::istream
  void read(std::istream& s);
  /// write a responseRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a responseRep object from an std::istream (annotated format)
  void read_annotated(std::istream& s);
  /// write a responseRep object to an std::ostream (annotated format)
  void write_annotated(std::ostream& s) const;

  /// read functionValues from an std::istream (tabular format)
  void read_tabular(std::istream& s);
  /// write functionValues to an std::ostream (tabular format)
  void write_tabular(std::ostream& s) const;

  /// read a responseRep object from a binary stream
  void read(BiStream& s);
  /// write a responseRep object to a binary stream
  void write(BoStream& s) const;

  /// read a responseRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a responseRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  /// return the number of doubles active in response.  Used for sizing 
  /// double* response_data arrays passed into read_data and write_data.
  int data_size();

  /// read from an incoming double* array
  void  read_data(double* response_data);
  /// write to an incoming double* array
  void write_data(double* response_data);

  /// add incoming response to functionValues/Gradients/Hessians
  void overlay(const Response& response);

  /// update this response object from components of another response object
  void update(const RealVector& source_fn_vals,
	      const RealMatrix& source_fn_grads,
	      const RealSymMatrixArray& source_fn_hessians,
	      const ActiveSet& source_set);
  /// partially update this response object partial components of
  /// another response object
  void update_partial(size_t start_index_target, size_t num_items,
		      const RealVector& source_fn_vals,
		      const RealMatrix& source_fn_grads,
		      const RealSymMatrixArray& source_fn_hessians,
		      const ActiveSet& source_set, size_t start_index_source);

  /// rehapes response data arrays
  void reshape(const size_t& num_fns, const size_t& num_params, bool grad_flag,
	       bool hess_flag);

  /// resets all response data to zero
  void reset();
  /// resets all inactive response data to zero
  void reset_inactive();

  /// set the active set request vector and verify consistent number
  /// of response functions
  void active_set_request_vector(const ShortArray& asrv);
  /// set the active set derivative vector and reshape
  /// functionGradients/functionHessians if needed
  void active_set_derivative_vector(const SizetArray& asdv);

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing responseRep
  int referenceCount;

  // An abstract set of functions and their first and second derivatives.

  /// abstract set of response functions
  RealVector functionValues;
  /// first derivatives of the response functions
  /** the gradient vectors (plural) are column vectors in the matrix
      (singular) with (row, col) = (variable index, response fn index). */
  RealMatrix functionGradients;
  /// second derivatives of the response functions
  RealSymMatrixArray functionHessians;

  /// copy of the ActiveSet used by the Model to generate a Response instance
  ActiveSet responseActiveSet;

  /// response function identifiers used to improve output readability
  StringArray functionLabels;

  /// response identifier string from the input file
  String responsesId;
};


inline ResponseRep::ResponseRep(): referenceCount(1)
{ }


inline ResponseRep::~ResponseRep()
{ }


/// Container class for response functions and their derivatives.  
/// Response provides the handle class.

/** The Response class is a container class for an abstract set of
    functions (functionValues) and their first (functionGradients) and
    second (functionHessians) derivatives.  The functions may involve
    objective and constraint functions (optimization data set), least
    squares terms (parameter estimation data set), or generic response
    functions (uncertainty quantification data set).  It is not
    currently part of a class hierarchy, since the abstraction has
    been sufficiently general and has not required specialization.
    For memory efficiency, it employs the "handle-body idiom" approach
    to reference counting and representation sharing (see Coplien
    "Advanced C++", p. 58), for which Response serves as the handle
    and ResponseRep serves as the body. */

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
  Response(const Variables& vars, const ProblemDescDB& problem_db);
  /// alternate constructor using limited data
  Response(const ActiveSet& set);
  /// copy constructor
  Response(const Response& response);
  /// destructor
  ~Response();

  //
  //- Heading: operators
  //

  /// assignment operator
  Response operator=(const Response& response);

  //
  //- Heading: Member functions
  //

  /// return the number of response functions
  size_t num_functions() const;

  /// return the active set
  const ActiveSet& active_set() const;
  /// set the active set
  void active_set(const ActiveSet& set);
  /// return the active set request vector
  const ShortArray& active_set_request_vector() const;
  /// set the active set request vector
  void active_set_request_vector(const ShortArray& asrv);
  /// return the active set derivative vector
  const SizetArray& active_set_derivative_vector() const;
  /// set the active set derivative vector
  void active_set_derivative_vector(const SizetArray& asdv);

  /// return the response identifier
  const String& responses_id() const;

  /// return a response function identifier string
  const String& function_label(const size_t& i) const;
  /// return the response function identifier strings
  const StringArray& function_labels() const;
  /// set a response function identifier string
  void function_label(const String& label, const size_t& i);
  /// set the response function identifier strings
  void function_labels(const StringArray& labels);

  /// return a function value
  const Real& function_value(const size_t& i) const;
  /// return all function values
  const RealVector& function_values() const;
  /// set a function value
  void function_value(const Real& function_val, const size_t& i);
  /// set all function values
  void function_values(const RealVector& function_vals);
  
  /// return a function gradient as a Teuchos_SerialDenseVector VIEW
  RealVector function_gradient(const int& i) const;
  /// return a function gradient as a Teuchos::Copy vector (deep copy)
  RealVector function_gradient_copy(const int& i) const;
  /// return all function gradients
  const RealMatrix& function_gradients() const;
  /// set a function gradient
  void function_gradient(const RealVector& function_grad, const int& i);
  /// set all function gradients
  void function_gradients(const RealMatrix& function_grads);

  /// return a function Hessian
  const RealSymMatrix& function_hessian(const size_t& i) const;
  /// return all function Hessians
  const RealSymMatrixArray& function_hessians() const;
  /// set a function Hessian
  void function_hessian(const RealSymMatrix& function_hessian, const size_t& i);
  /// set all function Hessians
  void function_hessians(const RealSymMatrixArray& function_hessians);

  /// read a response object from an std::istream
  void read(std::istream& s);
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

  /// read a response object from the binary restart stream
  void read(BiStream& s);
  /// write a response object to the binary restart stream
  void write(BoStream& s) const;

  /// read a response object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a response object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  Response copy() const; ///< a deep copy for use in history mechanisms

  /// handle class forward to corresponding body class member function
  int  data_size();
  /// handle class forward to corresponding body class member function
  void read_data(double*  response_data);
  /// handle class forward to corresponding body class member function
  void write_data(double* response_data);
  /// handle class forward to corresponding body class member function
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
  void reshape(const size_t& num_fns, const size_t& num_params, bool grad_flag,
	       bool hess_flag);
  /// handle class forward to corresponding body class member function
  void reset();
  /// handle class forward to corresponding body class member function
  void reset_inactive();

  /// function to check responseRep (does this handle contain a body)
  bool is_null() const;

private:

  //
  //- Heading: Private data members
  //
 
  /// pointer to the body (handle-body idiom)
  ResponseRep* responseRep;
};


inline size_t Response::num_functions() const
{ return responseRep->functionValues.length(); }


inline const Real& Response::function_value(const size_t& i) const
{ return responseRep->functionValues[i]; }


inline const RealVector& Response::function_values() const
{ return responseRep->functionValues; }


inline void Response::function_value(const Real& function_val, const size_t& i)
{ responseRep->functionValues[i] = function_val; }


inline void Response::function_values(const RealVector& function_vals)
{ responseRep->functionValues = function_vals; }


inline RealVector Response::function_gradient(const int& i) const
{ return Teuchos::getCol(Teuchos::View, responseRep->functionGradients, i); }


inline RealVector Response::function_gradient_copy(const int& i) const
{ return Teuchos::getCol(Teuchos::Copy, responseRep->functionGradients, i); }


inline const RealMatrix& Response::function_gradients() const
{ return responseRep->functionGradients; }


inline void Response::
function_gradient(const RealVector& function_grad, const int& i)
{ Teuchos::setCol(function_grad, i, responseRep->functionGradients); }


inline void Response::function_gradients(const RealMatrix& function_grads)
{ responseRep->functionGradients = function_grads; }


inline const RealSymMatrix& Response::function_hessian(const size_t& i) const
{ return responseRep->functionHessians[i]; }


inline const RealSymMatrixArray& Response::function_hessians() const
{ return responseRep->functionHessians; }


inline void Response::
function_hessian(const RealSymMatrix& function_hessian, const size_t& i)
{ responseRep->functionHessians[i] = function_hessian; }


inline void Response::
function_hessians(const RealSymMatrixArray& function_hessians)
{ responseRep->functionHessians = function_hessians; }


inline const ActiveSet& Response::active_set() const
{ return responseRep->responseActiveSet; }


inline void Response::active_set(const ActiveSet& set)
{
  responseRep->active_set_request_vector(set.request_vector());
  responseRep->active_set_derivative_vector(set.derivative_vector());
}


inline const ShortArray& Response::active_set_request_vector() const
{ return responseRep->responseActiveSet.request_vector(); }


inline void Response::active_set_request_vector(const ShortArray& asrv)
{ responseRep->active_set_request_vector(asrv); }


inline const SizetArray& Response::active_set_derivative_vector() const
{ return responseRep->responseActiveSet.derivative_vector(); }


inline void Response::active_set_derivative_vector(const SizetArray& asdv)
{ responseRep->active_set_derivative_vector(asdv); }


inline const String& Response::responses_id() const
{ return responseRep->responsesId; }


inline const String& Response::function_label(const size_t& i) const
{ return responseRep->functionLabels[i]; }


inline const StringArray& Response::function_labels() const
{ return responseRep->functionLabels; }


inline void Response::function_label(const String& label, const size_t& i)
{ responseRep->functionLabels[i] = label; }


inline void Response::function_labels(const StringArray& labels)
{ responseRep->functionLabels = labels; }


inline int Response::data_size()
{ return responseRep->data_size(); }


inline void Response::read_data(double* response_data)
{ responseRep->read_data(response_data); }


inline void Response::write_data(double* response_data)
{ responseRep->write_data(response_data); }


inline void Response::overlay(const Response& response)
{ responseRep->overlay(response); }


inline void Response::update(const Response& response)
{
  responseRep->update(response.function_values(), response.function_gradients(),
		      response.function_hessians(), response.active_set());
}


inline void Response::
update(const RealVector& source_fn_vals,
	     const RealMatrix& source_fn_grads,
	     const RealSymMatrixArray& source_fn_hessians,
	     const ActiveSet& source_set)
{
  responseRep->update(source_fn_vals, source_fn_grads, source_fn_hessians,
		      source_set);
}


inline void Response::
update_partial(size_t start_index_target, size_t num_items,
		     const Response& response, size_t start_index_source)
{
  responseRep->update_partial(start_index_target, num_items,
			      response.function_values(),
			      response.function_gradients(),
			      response.function_hessians(),
			      response.active_set(), start_index_source);
}


inline void Response::
update_partial(size_t start_index_target, size_t num_items,
		     const RealVector& source_fn_vals,
		     const RealMatrix& source_fn_grads,
		     const RealSymMatrixArray& source_fn_hessians,
		     const ActiveSet& source_set, size_t start_index_source)
{
  responseRep->update_partial(start_index_target, num_items, source_fn_vals,
			      source_fn_grads, source_fn_hessians, source_set,
			      start_index_source);
}


inline void Response::
reshape(const size_t& num_fns, const size_t& num_params, bool grad_flag,
	bool hess_flag)
{ responseRep->reshape(num_fns, num_params, grad_flag, hess_flag); }


inline void Response::reset()
{ responseRep->reset(); }


inline void Response::reset_inactive()
{ responseRep->reset_inactive(); }


inline bool Response::is_null() const
{ return (responseRep) ? false : true; }


inline void Response::read(std::istream& s)
{ if (responseRep) responseRep->read(s); }


inline void Response::write(std::ostream& s) const
{ if (responseRep) responseRep->write(s); }


inline void Response::read_annotated(std::istream& s)
{
  if (responseRep) // should not occur in current usage
    responseRep->read_annotated(s); // fwd to existing rep
  else { // read from neutral file: responseRep must be instantiated
    responseRep = new ResponseRep(); // default constructor is sufficient
    responseRep->read_annotated(s); // fwd to new rep
  }
}


inline void Response::write_annotated(std::ostream& s) const
{ if (responseRep) responseRep->write_annotated(s); }


inline void Response::read_tabular(std::istream& s)
{ if (responseRep) responseRep->read_tabular(s); }


inline void Response::write_tabular(std::ostream& s) const
{ if (responseRep) responseRep->write_tabular(s); }


inline void Response::read(BiStream& s)
{
  if (responseRep) // should not occur in current usage
    responseRep->read(s); // fwd to existing rep
  else { // read from restart: responseRep must be instantiated
    responseRep = new ResponseRep(); // default constructor is sufficient
    responseRep->read(s); // fwd to new rep
  }
}


inline void Response::write(BoStream& s) const
{ if (responseRep) responseRep->write(s); }


inline void Response::read(MPIUnpackBuffer& s)
{
  bool body;
  s >> body;
  if (body) { // response is not an empty handle
    if (!responseRep) // responseRep should not be defined in current usage
      responseRep = new ResponseRep(); // default constructor is sufficient
    responseRep->read(s); // fwd to rep
  }
  else if (responseRep) {
    if (--responseRep->referenceCount == 0)
      delete responseRep;
    responseRep = NULL;
  }
}


inline void Response::write(MPIPackBuffer& s) const
{
  s << !is_null();
  if (responseRep)
    responseRep->write(s);
}


/// global comparison function for Response
inline bool responses_id_compare(const Response& resp, const void* id)
{ return ( *(const String*)id == resp.responses_id() ); }


/// std::istream extraction operator for Response.  Calls read(std::istream&).
inline std::istream& operator>>(std::istream& s, Response& response)
{ response.read(s); return s; }


/// std::ostream insertion operator for Response.  Calls write(std::ostream&).
inline std::ostream& operator<<(std::ostream& s, const Response& response)
{ response.write(s); return s; }


/// BiStream extraction operator for Response.  Calls read(BiStream&).
inline BiStream& operator>>(BiStream& s, Response& response)
{ response.read(s); return s; }


/// BoStream insertion operator for Response.  Calls write(BoStream&).
inline BoStream& operator<<(BoStream& s, const Response& response)
{ response.write(s); return s; }


/// MPIUnpackBuffer extraction operator for Response.  Calls
/// read(MPIUnpackBuffer&).
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, Response& response)
{ response.read(s); return s; }


/// MPIPackBuffer insertion operator for Response.  Calls write(MPIPackBuffer&).
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const Response& response)
{ response.write(s); return s; }


/// equality operator for Response
inline bool operator==(const Response& resp1, const Response& resp2)
{ return ( *(resp1.responseRep) == *(resp2.responseRep) ); }


/// inequality operator for Response
inline bool operator!=(const Response& resp1, const Response& resp2)
{ return !(resp1 == resp2); }


/// Global utility function to ease migration from CtelRegExp to Boost.Regex
#ifdef DAKOTA_HAVE_BOOST_REGEX
inline std::string re_match(const std::string& token, const boost::regex& re)
{
  std::string str_match;
  boost::smatch found_substr;
  if( boost::regex_search(token, found_substr, re) ) {
    str_match = std::string(found_substr[0].first, found_substr[0].second);
  }
  return str_match;
}
#else
inline std::string re_match(const std::string& token, CtelRegexp& re)
{ return re.match(token); }
#endif // DAKOTA_HAVE_BOOST_REGEX

} // namespace Dakota

#endif // !DAKOTA_RESPONSE_H
