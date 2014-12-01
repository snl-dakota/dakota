/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Response
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

static const char rcsId[]="@(#) $Id: DakotaResponse.cpp 7029 2010-10-22 00:17:02Z mseldre $";

BOOST_CLASS_EXPORT(Dakota::ResponseRep)
BOOST_CLASS_EXPORT(Dakota::Response)

namespace Dakota {

/** Need a populated problem description database to build a meaningful
    Response object, so set the responseRep=NULL in default constructor
    for efficiency.  This then requires a check on NULL in the copy
    constructor, assignment operator, and destructor. */
Response::Response(): responseRep(NULL)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Response::Response(), responseRep = NULL" << std::endl;
#endif
}


Response::Response(const Variables& vars, const ProblemDescDB& problem_db):
  responseRep(new ResponseRep(vars, problem_db))
{
#ifdef REFCOUNT_DEBUG
  Cout << "Response::Response(vars, problem_db), responseRep referenceCount = "
       << responseRep->referenceCount << std::endl;
#endif
}


Response::Response(const ActiveSet& set): responseRep(new ResponseRep(set))
{
#ifdef REFCOUNT_DEBUG
  Cout << "Response::Response(set), responseRep referenceCount = "
       << responseRep->referenceCount << std::endl;
#endif
}


Response::Response(const Response& response)
{
  // Increment new (no old to decrement)
  responseRep = response.responseRep;
  if (responseRep) // Check for an assignment of NULL
    responseRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  Cout << "Response::Response(Response&)" << std::endl;
  if (responseRep)
    Cout << "responseRep referenceCount = " << responseRep->referenceCount
	 << std::endl;
#endif
}


Response Response::operator=(const Response& response)
{
  if (responseRep != response.responseRep) { // normal case: old != new
    // Decrement old
    if (responseRep) // Check for NULL
      if ( --responseRep->referenceCount == 0 ) 
	delete responseRep;
    // Assign and increment new
    responseRep = response.responseRep;
    if (responseRep) // Check for NULL
      responseRep->referenceCount++;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "Response::operator=(Response&)" << std::endl;
  if (responseRep)
    Cout << "responseRep referenceCount = " << responseRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


Response::~Response()
{
  if (responseRep) { // Check for NULL
    --responseRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "responseRep referenceCount decremented to " 
         << responseRep->referenceCount << std::endl;
#endif
    if (responseRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting responseRep" << std::endl;
#endif
      delete responseRep;
    }
  }
}


Response Response::copy() const
{
  Response response; // new handle: responseRep=NULL

  if (responseRep) {
    // allocate a responseRep body and copy data attributes
    response.responseRep = new ResponseRep();
    response.responseRep->functionValues    = responseRep->functionValues;
    response.responseRep->functionGradients = responseRep->functionGradients;
    response.responseRep->functionHessians  = responseRep->functionHessians;
    response.responseRep->responseActiveSet = responseRep->responseActiveSet;
    response.responseRep->functionLabels    = responseRep->functionLabels;
    response.responseRep->responsesId       = responseRep->responsesId;
  }

  return response;
}


/** The standard constructor used by Dakota::ModelRep. */
ResponseRep::
ResponseRep(const Variables& vars, const ProblemDescDB& problem_db):
  referenceCount(1), functionLabels(problem_db.get_sa("responses.labels")),
  responsesId(problem_db.get_string("responses.id"))
{
  size_t sum_field_lengths= 0;
  size_t numFieldResponses = 
    problem_db.get_sizet("responses.num_field_responses")+
    problem_db.get_sizet("responses.num_field_objectives")+
    problem_db.get_sizet("responses.num_field_calibration_terms");
  if (numFieldResponses > 0) {
    numScalarResponses =  problem_db.get_sizet("responses.num_scalar_responses")+ 
        problem_db.get_sizet("responses.num_scalar_objectives")+
        problem_db.get_sizet("responses.num_scalar_calibration_terms");
    if ((numFieldResponses + numScalarResponses) !=
         problem_db.get_sizet("responses.num_response_functions")) 
          Cerr << "Warning: number of scalar and field response functions must sum " 
	       << "to total number of response functions." << std::endl;
    numTotalResponses = numScalarResponses + numFieldResponses;
  }      
  if (numFieldResponses > 0) {
    fieldLengths.resize(numFieldResponses);
    fieldLengths = problem_db.get_iv("responses.lengths");
    for (size_t i=0; i<numFieldResponses; i++) 
      sum_field_lengths += fieldLengths(i);
  }
  size_t num_fns;
  if (numFieldResponses == 0) {
    size_t num_resp_fns
      = problem_db.get_sizet("responses.num_response_functions");
    num_fns = (num_resp_fns) ? num_resp_fns :
      problem_db.get_sizet("responses.num_nonlinear_inequality_constraints") +
      problem_db.get_sizet("responses.num_nonlinear_equality_constraints")   +
      std::max(problem_db.get_sizet("responses.num_objective_functions"),
	       problem_db.get_sizet("responses.num_least_squares_terms"));
  } 
  else {
    size_t num_scalar_resp_fns
      = problem_db.get_sizet("responses.num_scalar_responses");
    size_t num_scalar_fns = (num_scalar_resp_fns) ? num_scalar_resp_fns :
      problem_db.get_sizet("responses.num_nonlinear_inequality_constraints") +
      problem_db.get_sizet("responses.num_nonlinear_equality_constraints")   +
      std::max(problem_db.get_sizet("responses.num_scalar_objectives"),
	       problem_db.get_sizet("responses.num_scalar_calibration_terms"));
      num_fns = num_scalar_fns + sum_field_lengths;
  }
   
  if (num_fns == 0)
    Cerr << "Warning: total number of response functions is zero.  This is "
	 << "admissible in rare cases (e.g., nested overlays)." << std::endl;

  // the derivative arrays must accomodate either active or inactive variables,
  // but the default is active variables.  Derivative arrays are resized if a
  // DVV of different length is set within responseActiveSet.
  size_t num_params = vars.cv();

  // Resize & initialize response data
  // Conserve memory by checking DB info prior to sizing grad/hessian arrays
  bool grad_flag = (problem_db.get_string("responses.gradient_type") == "none")
                 ? false : true;
  bool hess_flag = (problem_db.get_string("responses.hessian_type")  == "none")
                 ? false : true;
  functionValues.resize(num_fns);
  functionValues = 0.;
  short asv_value = 1;
  if (grad_flag) {
    asv_value |= 2;
    functionGradients.reshape(num_params, num_fns);
    functionGradients = 0.;
  }
  if (hess_flag) {
    asv_value |= 4;
    functionHessians.resize(num_fns);
    for (size_t i=0; i<num_fns; i++) {
      functionHessians[i].reshape(num_params);
      functionHessians[i] = 0.;
    }
  }

  // set up the response ActiveSet.  This object is copied by Iterator for its
  // initial activeSet and used by Model for MPI buffer size estimation.
  //responseActiveSet.reshape(num_fns, num_params);
  ShortArray asv(num_fns, asv_value);
  responseActiveSet.request_vector(asv);
  responseActiveSet.derivative_vector(vars.continuous_variable_ids());
  
  // need to append per field id
  if (numFieldResponses > 0) {
    size_t num_resp_fns
      = problem_db.get_sizet("responses.num_response_functions");
    StringArray origLabels(num_resp_fns);
    origLabels=functionLabels;
    size_t cntr = 0;
    functionLabels.resize(num_fns);
    for (size_t i=0; i<numFieldResponses; i++){
      for (size_t j=0; j<fieldLengths(i); j++){
        build_label(functionLabels[numScalarResponses+cntr+j],origLabels[numScalarResponses+i],j+1);
      }
      cntr+=fieldLengths(i);
    }
  }
}


/** Used for building a response object of the correct size on the fly
    (e.g., by slave analysis servers performing execute() on a
    local_response).  functionLabels is not needed for this purpose
    since it's not passed in the MPI send/recv buffers. However,
    NPSOLOptimizer's user-defined functions option uses this
    constructor to build bestResponseArray.front() and
    bestResponseArray.front() needs functionLabels for I/O, so
    construction of functionLabels has been added. */
ResponseRep::ResponseRep(const ActiveSet& set): referenceCount(1),
  functionValues(set.request_vector().size()), responseActiveSet(set)
{
  // Set flags according to asv content.
  const ShortArray& asv = set.request_vector();
  size_t i, num_fns = asv.size(),
    num_params = set.derivative_vector().size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_fns; i++) {
    if (asv[i] & 2)
      grad_flag = true;
    if (asv[i] & 4)
      hess_flag = true;
  }

  // Reshape functionGradients and functionHessians according to content of asv
  if (grad_flag) {
    functionGradients.shapeUninitialized(num_params, num_fns);
    functionGradients = 0.;
  }
  if (hess_flag) {
    functionHessians.resize(num_fns);
    for (i=0; i<num_fns; i++) {
      functionHessians[i].reshape(num_params);
      functionHessians[i] = 0.;
    }
  }

  // Build a default functionLabels array (currently only used for
  // bestResponse by NPSOLOptimizer's user-defined functions option).
  functionLabels.resize(num_fns);
  build_labels(functionLabels, "f");
}


// I/O Notes:

// Bitwise AND used in ASV checks in read/write fns: the '&' operator does a bit
// by bit AND operation.  For example, 7 & 4 does a bitwise AND on 111 and 100
// and returns 100 or 4.  It is not necessary to verify that, for example,
// (asv[i] & 4) == 4 since (asv[i] & 4) will be nonzero/true only if the 4 bit
// is present in asv[i] (the "== value" can be omitted so long as only the
// presence of a single bit is of interest).

// Matrix sizing: since fn gradients and hessians are of size 0 if their type is
// "none", all grad loops are run from 0 to num rows and all Hessian loops are
// run from 0 to num hessians, rather than from 0 to num_fns.  For the number
// of derivative variables, the length of responseActiveSet.derivative_vector()
// is used rather than the array sizes since the matrices are originally sized
// according to the maximal case and all entries may not be used.

// When creating functionGradients/functionHessians on the fly in read() input
// functions, grad_flag/hess_flag are used rather than inferring sizes from
// responseActiveSet since it is safer to match the sizing of the response
// object that was written (originally sized by the Response constructor) than
// to base sizing on the particular constent of an asv (which can vary from
// eval. to eval.).  For example, assigning empty gradient/Hessian matrices to
// Model::currentResponse in a restart operation would cause problems.


/** ASCII version of read needs capabilities for capturing data omissions or
    formatting errors (resulting from user error or asynch race condition) and
    analysis failures (resulting from nonconvergence, instability, etc.). */
void ResponseRep::read(std::istream& s)
{
  // Failure capturing:
  // The read operator detects analysis failure through reading a failure 
  // string returned by the user's output_filter/analysis_driver.  This string
  // must occur at the beginning of the file, even if some portion of the 
  // results were successfully computed prior to failure.  Other mechanisms for
  // denoting analysis failure that were considered included (1) use of an 
  // empty file (disadvantage: ambiguity with asynch race condition) and (2) 
  // use of a success/fail string (disadvantage: would require addition of 
  // "success" field to non-failure output).  The current approach avoids these
  // problems by doing a read of 4 characters.  If these characters are not 
  // "fail" or "FAIL" then it resets the stream pointer to the beginning.  This
  // works for ifstreams, but may be problematic for other istreams.  Once 
  // failure is detected, an exception of type int is thrown (to distinguish 
  // from std::string exceptions) that will be caught after try{ execute() }.
  // NOTE: s.peek() triggering on 'f' or 'F' would be another possibility.
  // NOTE: reading the first token using "s >> fail_string;" should work for a
  //       file having less than four characters.
  size_t i, j, k;
  int fail_code = 1;
  char fail_chars[4] = {0,0,0,0};
  std::string fail_string("fail");
  // Old version failed for fewer than 4 characters in results file:
  //s >> fail_chars[0] >> fail_chars[1] >> fail_chars[2] >> fail_chars[3];
  for (i=0; i<4; i++) {
    s >> fail_chars[i];
    //Cout << "fail_char[" << i << "] = " << fail_chars[i] << std::endl;
    if (tolower(fail_chars[i]) != fail_string[i]) { // FAIL, Fail, fail, etc.
      fail_code = 0; // No failure communicated from results file 
      s.seekg(0); // Reset stream to beginning
      break; // exit for loop
    }
  }
  if (fail_code) {
    //Cerr << "Failure captured with string = " << fail_chars << std::endl;
    throw fail_code;
  }

  // Destroy old values and set to zero (so that else assignments are not needed
  // below). The arrays have been properly sized by the ResponseRep constructor.
  reset();

  // Get fn. values as governed by ASV requests
  std::string token;
  boost::regex reg_exp("[\\+-]?[0-9]*\\.?[0-9]+\\.?[0-9]*[eEdD]?[\\+-]?[0-9]*|-?[Nn][Aa][Nn]|[\\+-]?[Ii][Nn][Ff]([Ii][Nn][Ii][Tt][Yy])?");
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t nf = asv.size();
  for (i=0; i<nf; i++) {
    if (asv[i] & 1) { // & 1 masks off 2nd and 3rd bit
      if (s) { // get value
	s >> token;
	// On RHEL 4, strtod preserves Inf/NaN, but atof doesn't
	//Cout << "Debug read: token = " << token << '\n';
	//Cout << "Debug read: atof of token = " << atof(token) << '\n';
	//Cout << "Debug read: strtod of token = " << strtod(token, NULL)<<'\n';
	// On error, atof returns 0.0. Must verify token is a number.
	if(token == re_match(token, reg_exp))
	  functionValues[i] = std::atof(token.c_str()); // handles NaN and +/-Inf
	else
	  throw std::string( "Response format error with functionValue "
			      + boost::lexical_cast<std::string>(i+1) );
      }
      else
        throw std::string( "At EOF: insufficient data for functionValue "
                           + boost::lexical_cast<std::string>(i+1) );

      if (s) { // get optional tag
	//s.ignore(256, '\n'); // simple soln., but requires consistent '\n'
        int pos = s.tellg(); // save stream pos prior to token extraction
        s >> token; // get next field (may be a tag or a number)
        // Check to see if token matches the pattern (see CtelRegExp class docs)
	// of a numerical value (including +/-Inf and NaN) or of the beginning
	// of a gradient/Hessian block.  If it does, then rewind the stream.
        if ( !token.empty() &&
	     ( token[(size_t)0]=='[' || token == re_match(token, reg_exp) ) )
          s.seekg(pos); // token is not a tag, rewind
        // else field was properly extracted as a tag
      }
    }
  }

  // Get function gradients as governed by ASV requests
  // For brackets, chars are used rather than token strings to allow optional
  // white space between brackets and values.
  char l_bracket, r_bracket; // eat white space and grab 1 character
  size_t ng = functionGradients.numCols(), nv = functionGradients.numRows();
  for (i=0; i<ng; ++i) { // prevent loop if functionGradients not sized
    if (asv[i] & 2) { // & 2 masks off 1st and 3rd bit
      if (s)
        s >> l_bracket;
      else
        throw std::string( "At EOF: insufficient data for functionGradient "
                           + boost::lexical_cast<std::string>(i+1) );

      read_col_vector_trans(s, (int)i, functionGradients); // fault tolerant

      if (s)
        s >> r_bracket;
      else {
        throw std::string( "At EOF: insufficient data for functionGradient "
                           + boost::lexical_cast<std::string>(i+1) );
      }
      if (l_bracket != '[' || r_bracket != ']') {
        throw std::string( "Response format error with functionGradient "
                           + boost::lexical_cast<std::string>(i+1) );
      }
    }
  }

  // Get function Hessians as governed by ASV requests
  char l_brackets[2], r_brackets[2]; // eat white space and grab 2 characters
  size_t nh = functionHessians.size();
  for (i=0; i<nh; i++) { // prevent loop if functionHessians not sized
    if (asv[i] & 4) { // & 4 masks off 1st and 2nd bit
      if (s)
        s >> l_brackets[0] >> l_brackets[1];
      else
        throw std::string( "At EOF: insufficient data for functionHessian "
                           + boost::lexical_cast<std::string>(i+1) );

      Dakota::read_data(s, functionHessians[i]); // fault tolerant

      if (s)
        s >> r_brackets[0] >> r_brackets[1];
      else {
        throw std::string( "At EOF: insufficient data for functionHessian "
                           + boost::lexical_cast<std::string>(i+1) );
      }
      if ((l_brackets[0] != '[' || l_brackets[1] != '[')  ||
	  (r_brackets[0] != ']' || r_brackets[1] != ']')) {
        throw std::string( "Response format error with functionHessian "
                           + boost::lexical_cast<std::string>(i+1) );
      }
    }
  }
}


/** ASCII version of write. */
void ResponseRep::write(std::ostream& s) const
{
  const ShortArray& asv = responseActiveSet.request_vector();
  const SizetArray& dvv = responseActiveSet.derivative_vector();
  size_t i, nf = asv.size();
  bool deriv_flag = false;
  for (i=0; i<nf; ++i)
    if (asv[i] & 6)
      { deriv_flag = true; break; }

  // Write ASV/DVV information
  s << "Active set vector = { ";
  array_write_annotated(s, asv, false);
  if (deriv_flag) { // dvv != vars.continuous_variable_ids() ?,
                    // outputLevel > NORMAL_OUTPUT ?
    s << "} Deriv vars vector = { ";
    array_write_annotated(s, dvv, false);
    s << "}\n";
  }
  else
    s << "}\n";

  // Make sure a valid set of functionLabels exists. This has been a problem
  // since there is no way to build these labels in the default Response
  // constructor (used by lists & vectors of Response objects).
  if (functionLabels.size() != nf) {
    Cerr << "Error with functionLabels in ResponseRep::write." << std::endl;
    abort_handler(-1);
  }

  // Write the function values if present
  for (i=0; i<nf; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      s << "                     " << std::setw(write_precision+7) 
        << functionValues[i] << ' ' << functionLabels[i] << '\n';

  // Write the function gradients if present
  size_t ng = functionGradients.numCols();
  for (i=0; i<ng; ++i) {
    if (asv[i] & 2) { // & 2 masks off 1st and 3rd bit
      // NOTE: col_vec is WRITTEN-out like a row_vec for historical consistency
      write_col_vector_trans(s, (int)i, true, true, false, functionGradients);
      s << functionLabels[i] << " gradient\n";
    }
  }

  // Write the function Hessians if present
  size_t nh = functionHessians.size();
  for (i=0; i<nh; ++i) {
    if (asv[i] & 4) { // & 4 masks off 1st and 2nd bit
      Dakota::write_data(s, functionHessians[i], true, true, false);
      s << functionLabels[i] << " Hessian\n";
    }
  }
  s << std::endl;
}


/** read_annotated() is used for neutral file translation of restart files.
    Since objects are built solely from this data, annotations are used.
    This version closely mirrors the BiStream version. */
void ResponseRep::read_annotated(std::istream& s)
{
  // Read sizing data
  size_t i, num_fns, num_params;
  bool grad_flag, hess_flag;
  s >> num_fns >> num_params >> grad_flag >> hess_flag;

  // Reshape and read responseActiveSet and functionLabels.
  responseActiveSet.reshape(num_fns, num_params);
  functionLabels.resize(num_fns);
  s >> responseActiveSet >> functionLabels;

  // reshape response arrays and reset all data to zero
  reshape(num_fns, num_params, grad_flag, hess_flag);
  reset();

  // Get fn. values as governed by ASV requests
  const ShortArray& asv = responseActiveSet.request_vector();
  std::string token; // used with atof() to handle +/-inf & nan
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      { s >> token; functionValues[i] = std::atof(token.c_str()); }

  // Get function gradients as governed by ASV requests
  size_t nv = functionGradients.numRows();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 2) // & 2 masks off 1st and 3rd bit
      read_col_vector_trans(s, (int)i, functionGradients); // fault tolerant

  // Get function Hessians as governed by ASV requests
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) // & 4 masks off 1st and 2nd bit
      read_lower_triangle(s, functionHessians[i]); // fault tolerant
}


/** write_annotated() is used for neutral file translation of restart files.
    Since objects need to be build solely from this data, annotations are used.
    This version closely mirrors the BoStream version, with the exception of
    the use of white space between fields. */
void ResponseRep::write_annotated(std::ostream& s) const
{
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, num_fns = asv.size(),
    num_params = responseActiveSet.derivative_vector().size();
  bool grad_flag = !functionGradients.empty(),
       hess_flag = !functionHessians.empty();

  // Write ResponseRep sizing data
  s << num_fns   << ' ' << num_params << ' '
    << grad_flag << ' ' << hess_flag  << ' ';

  // Write responseActiveSet and functionLabels.  Don't separately annotate
  // arrays with sizing data since ResponseRep handles this all at once.
  responseActiveSet.write_annotated(s);
  array_write_annotated(s, functionLabels, false);

  // Write the function values if present
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      s << functionValues[i] << ' ';

  // Write the function gradients if present
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 2) // & 2 masks off 1st and 3rd bit
      write_col_vector_trans(s, (int)i, false, false, false, functionGradients);

  // Write the function Hessians if present
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) // & 4 masks off 1st and 2nd bit
      write_lower_triangle(s, functionHessians[i], false);
}


/** read_tabular is used to read functionValues in tabular format.  It
    is currently only used by ApproximationInterfaces in reading samples
    from a file.  There is insufficient data in a tabular file to build
    complete response objects; rather, the response object must be
    constructed a priori and then its functionValues can be set. */
void ResponseRep::read_tabular(std::istream& s)
{
  // Read the function values, regardless of ASV.  Since this is a table
  // format, there must be a field read even when data is inactive.
  size_t i, num_fns = functionValues.length();
  std::string token;
  for (i=0; i<num_fns; ++i) {
    if (s)
      { s >> token; functionValues[i] = std::atof(token.c_str()); }
    else {
      throw std::string( "At EOF: insufficient data for RealVector["
                         + boost::lexical_cast<std::string>(i) + "]" );
    }
  }
}


/** write_tabular is used for output of functionValues in a tabular
    format for convenience in post-processing/plotting of DAKOTA results. */
void ResponseRep::write_tabular(std::ostream& s) const
{
  // Set stream format
  s << std::setprecision(write_precision) 
    << std::resetiosflags(std::ios::floatfield);

  // Print a field for each of the function values, even if inactive (since
  // this is a table and the header associations must be preserved).  Dropouts
  // (inactive data) could be printed as 0's (the inactive value), "N/A", -9999,
  // a blank field, etc.  Using either "N/A" or a blank field gives the correct
  // meaning visually, but both cause problems with data import.
  size_t i, num_fns = functionValues.length();
  const ShortArray& asv = responseActiveSet.request_vector();
  for (i=0; i<num_fns; ++i) {
    if (asv[i] & 1)
      s << std::setw(write_precision+4) << functionValues[i] << ' ';
    else
      s << "               "; // blank field for inactive data
    // BMA TODO: write something that can be read back in for tabular...
    //s << std::numeric_limits<double>::quiet_NaN(); // inactive data
    //s << "EMPTY"; // inactive data
  }
  s << std::endl; // table row completed
}


void ResponseRep::write_tabular_labels(std::ostream& s) const
{
  size_t num_fns = functionLabels.size();
  for (size_t j=0; j<num_fns; ++j)
    s << std::setw(14) << functionLabels[j] << ' ';
  s << std::endl; // table row completed
}


/** UnpackBuffer version differs from BiStream version in the omission
    of functionLabels.  Master processor retains labels and interface ids
    and communicates asv and response data only with slaves. */
void ResponseRep::read(MPIUnpackBuffer& s)
{
  size_t i, num_fns, num_params;
  bool grad_flag, hess_flag;

  // Read sizing data and responseActiveSet (reshape not needed)
  s >> num_fns >> num_params >> grad_flag >> hess_flag
    >> responseActiveSet; // >> functionLabels;

  // reshape response arrays and reset all data to zero
  reshape(num_fns, num_params, grad_flag, hess_flag);
  reset();

  // Get fn. values as governed by ASV requests
  const ShortArray& asv = responseActiveSet.request_vector();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      s >> functionValues[i];

  // Get function gradients as governed by ASV requests
  for (int i=0; i<num_fns; ++i)
    if (asv[i] & 2) // & 2 masks off 1st and 3rd bit
      read_col_vector_trans(s, i, functionGradients);

  // Get function Hessians as governed by ASV requests
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) // & 4 masks off 1st and 2nd bit
      read_lower_triangle(s, functionHessians[i]);
}


/** MPIPackBuffer version differs from BoStream version only in the
    omission of functionLabels.  The master processor retains labels
    and ids and communicates asv and response data only with slaves. */
void ResponseRep::write(MPIPackBuffer& s) const
{
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, num_fns = asv.size(),
    num_params = responseActiveSet.derivative_vector().size();
  bool grad_flag = !functionGradients.empty(),
       hess_flag = !functionHessians.empty();

  // Write sizing data and responseActiveSet
  s << num_fns << num_params << grad_flag << hess_flag
    << responseActiveSet; // << functionLabels;

  // Write the function values if present
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      s << functionValues[i];

  // Write the function gradients if present
  for (int i=0; i<num_fns; ++i)
    if (asv[i] & 2) // & 2 masks off 1st and 3rd bit
      write_col_vector_trans(s, i, functionGradients);

  // Write the function Hessians if present
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) // & 4 masks off 1st and 2nd bit
      write_lower_triangle(s, functionHessians[i]);
}


/// convenience fnction to write a serial dense matrix column to an Archive
template<class Archive, typename OrdinalType, typename ScalarType>
void ResponseRep::write_sdm_col
(Archive& ar, int col,
 const Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm) const
{
  OrdinalType nr = sdm.numRows(); 
  const ScalarType* sdm_c = sdm[col]; // column vector 
  for (OrdinalType row=0; row<nr; ++row) 
    ar & sdm_c[row];
}

/// convenience fnction to read a serial dense matrix column from an Archive
template<class Archive, typename OrdinalType, typename ScalarType>
void ResponseRep::read_sdm_col
(Archive& ar, int col, Teuchos::SerialDenseMatrix<OrdinalType, ScalarType>& sdm)
{
  OrdinalType nr = sdm.numRows(); 
  ScalarType* sdm_c = sdm[col]; // column vector
  for (OrdinalType row=0; row<nr; ++row) 
    ar & sdm_c[row];
}


/** Binary version differs from ASCII version in 2 primary ways:
    (1) it lacks formatting.
    (2) the Response has not been sized a priori.  In reading data from
        the binary restart file, a ParamResponsePair was constructed with
        its default constructor which called the Response default 
        constructor.  Therefore, we must first read sizing data and resize 
        all of the arrays. */
template<class Archive> 
void ResponseRep::load(Archive& ar, const unsigned int version)
{
  size_t i, num_fns, num_params;
  bool grad_flag, hess_flag;
    
  // Read sizing data, responseActiveSet, and functionLabels
  ar & num_fns;
  ar & num_params;
  ar & grad_flag;
  ar & hess_flag;
  ar & responseActiveSet;
  ar & functionLabels;


  // reshape response arrays and reset all data to zero
  reshape(num_fns, num_params, grad_flag, hess_flag);
  reset();

  const ShortArray& asv = responseActiveSet.request_vector();

  // Get fn. values as governed by ASV requests
  for (size_t i=0; i<num_fns; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      ar & functionValues[i];

  // Get function gradients as governed by ASV requests
  for (int i=0; i<num_fns; ++i)
    if (asv[i] & 2) // & 2 masks off 1st and 3rd bit
      read_sdm_col(ar, i, functionGradients);
    
  // Get function Hessians as governed by ASV requests
  for (size_t i=0; i<num_fns; ++i)
    if (asv[i] & 4) // & 4 masks off 1st and 2nd bit
      ar & functionHessians[i];
}


/** Binary version differs from ASCII version in 2 primary ways:
    (1) It lacks formatting.
    (2) In reading data from the binary restart file, ParamResponsePairs are
        constructed with their default constructor which calls the Response
        default constructor.  Therefore, we must first write sizing data so
        that ResponseRep::read(BoStream& s) can resize the arrays. */
template<class Archive> 
void ResponseRep::save(Archive& ar, const unsigned int version) const
{
    
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t num_fns = asv.size(),
    num_params = responseActiveSet.derivative_vector().size();
  bool grad_flag = !functionGradients.empty(),
    hess_flag = !functionHessians.empty();
    
  // Write sizing data, responseActiveSet, and functionLabels
  ar & num_fns;
  ar & num_params;
  ar & grad_flag;
  ar & hess_flag;
  ar & responseActiveSet;
  ar & functionLabels;

  // Write the function values if present
  for (size_t i=0; i<num_fns; ++i)
    if (asv[i] & 1) // & 1 masks off 2nd and 3rd bit
      ar & functionValues[i];

  // Write the function gradients if present
  for (int i=0; i<num_fns; ++i)
    if (asv[i] & 2) // & 2 masks off 1st and 3rd bit
      write_sdm_col(ar, i, functionGradients);

  // Write the function Hessians if present
  for (size_t i=0; i<num_fns; ++i)
    if (asv[i] & 4) // & 4 masks off 1st and 2nd bit
      ar & functionHessians[i];
}

// These shouldn't be necessary, but using to avoid static linking
// issues until can find the right Boost macro ordering
template void ResponseRep:: 
load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive& ar, 
				      const unsigned int version); 
template void ResponseRep:: 
save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive& ar, 
				      const unsigned int version) const; 

int ResponseRep::data_size()
{
  // return the number of doubles active in response for sizing double* 
  // response_data arrays passed into read_data and write_data
  int size = 0;
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, num_fns = functionValues.length(),
    grad_size = responseActiveSet.derivative_vector().size(),
    hess_size = grad_size*(grad_size+1)/2; // (n^2+n)/2
  for (i=0; i<num_fns; ++i) {
    if (asv[i] & 1) ++size;
    if (asv[i] & 2) size += grad_size;
    if (asv[i] & 4) size += hess_size;
  }
  return size;
}


void ResponseRep::read_data(double* response_data)
{
  // read from an incoming double* array
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, j, k, cntr = 0, num_fns = functionValues.length(),
    num_params = responseActiveSet.derivative_vector().size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1)
      functionValues[i] = response_data[cntr++];
  num_fns = functionGradients.numCols();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 2) {
      Real* fn_grad_i = functionGradients[i];
      for (j=0; j<num_params; ++j, ++cntr)
        fn_grad_i[j] = response_data[cntr];
    }
  num_fns = functionHessians.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) {
      RealSymMatrix& fn_hess_i = functionHessians[i];
      for (j=0; j<num_params; ++j)
        for (k=0; k<=j; ++k, ++cntr)
          fn_hess_i(j,k) = response_data[cntr];
    }
}


void ResponseRep::write_data(double* response_data)
{
  // write to an incoming double* array
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, j, k, cntr = 0, num_fns = functionValues.length(),
    num_params = responseActiveSet.derivative_vector().size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1)
      response_data[cntr++] = functionValues[i];
  num_fns = functionGradients.numCols();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 2) {
      const Real* fn_grad_i = functionGradients[i];
      for (j=0; j<num_params; j++, ++cntr)
        response_data[cntr] = fn_grad_i[j];
    }
  num_fns = functionHessians.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) {
      const RealSymMatrix& fn_hess_i = functionHessians[i];
      for (j=0; j<num_params; ++j)
        for (k=0; k<=j; ++k, ++cntr)
          response_data[cntr] = fn_hess_i(j,k);
    }
}


void ResponseRep::overlay(const Response& response)
{
  // add incoming response to functionValues/Gradients/Hessians
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, j, k, num_fns = asv.size(),
    num_params = responseActiveSet.derivative_vector().size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1)
      functionValues[i] += response.function_value(i);
  num_fns = functionGradients.numCols();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 2) {
      const Real* partial_fn_grad = response.function_gradient(i);
      Real* fn_grad = functionGradients[i];
      for (j=0; j<num_params; ++j)
        fn_grad[j] += partial_fn_grad[j];
    }
  num_fns = functionHessians.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 4) {
      const RealSymMatrix& partial_fn_hess = response.function_hessian(i);
      RealSymMatrix& fn_hess = functionHessians[i];
      for (j=0; j<num_params; ++j)
        for (k=0; k<=j; ++k)
          fn_hess(j,k) += partial_fn_hess(j,k);
    }
}


/** Copy function values/gradients/Hessians data _only_.  Prevents unwanted
    overwriting of responseActiveSet, functionLabels, etc.  Also, care is
    taken to account for differences in derivative variable matrix sizing. */
void ResponseRep::
update(const RealVector& source_fn_vals, const RealMatrix& source_fn_grads,
       const RealSymMatrixArray& source_fn_hessians,
       const ActiveSet& source_set)
{
  // current response data
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, j, k, num_fns = asv.size(), 
    num_params = responseActiveSet.derivative_vector().size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_fns; i++) {
    if (asv[i] & 2)
      grad_flag = true;
    if (asv[i] & 4)
      hess_flag = true;
  }

  // verify that incoming data is of sufficient size
  if (source_set.request_vector().size() < num_fns) {
    Cerr << "Error: insufficient number of response functions to copy "
	 << "response results in ResponseRep::update()." << std::endl;
    abort_handler(-1);
  }
  if ( (grad_flag || hess_flag) && 
       source_set.derivative_vector().size() < num_params) {
    Cerr << "Error: insufficient number of derivative variables to copy "
	 << "response results in ResponseRep::update()." << std::endl;
    abort_handler(-1);
  }

  // for functionValues, using operator= does not allow vector size to differ.
  // for functionGradients/functionHessians, using operator= does not allow
  // num_fns or derivative variable matrix sizing to differ.  This includes the
  // extreme case in which the incoming derivative arrays may have zero size
  // (e.g., this can happen in a multilevel strategy where a captured duplicate
  // came from the evals of a previous nongradient-based algorithm).
  //functionValues    = source_fn_vals;
  //functionGradients = source_fn_grads;
  //functionHessians  = source_fn_hessians;

  for (i=0; i<num_fns; i++)
    if (asv[i] & 1) // assign each entry since size may differ
      functionValues[i] = source_fn_vals[i];

  // copy source_fn_grads and source_fn_hessians.  For now, a mapping from
  // responseActiveSet.derivative_vector() to source_set.derivative_vector()
  // is _not_ used, although this may be needed in the future.
  if (grad_flag) {
    if (source_fn_grads.numCols() < num_fns) {
      Cerr << "Error: insufficient incoming gradient size to copy response "
	   << "results required in ResponseRep::update()." << std::endl;
      abort_handler(-1);
    }
    for (i=0; i<num_fns; i++)
      if (asv[i] & 2) // assign each entry since size may differ
	for (j=0; j<num_params; j++)
	  functionGradients(j,i) = source_fn_grads(j,i);
  }
  if (hess_flag) {
    if (source_fn_hessians.size() < num_fns) {
      Cerr << "Error: insufficient incoming Hessian size to copy response "
	   << "results required in ResponseRep::update()." << std::endl;
      abort_handler(-1);
    }
    for (i=0; i<num_fns; i++)
      if (asv[i] & 4) // assign each entry since size may differ
	for (j=0; j<num_params; j++)
	  for (k=0; k<=j; k++)
	    functionHessians[i](j,k) = source_fn_hessians[i](j,k);
  }

  // Remove any data retrieved from response but not reqd by responseActiveSet
  // (id_vars_set_compare will detect duplication when the current asv is a
  // subset of the database asv and more can be retrieved than requested).
  if (responseActiveSet != source_set) // only reset if needed
    reset_inactive();
}


/** Copy function values/gradients/Hessians data _only_.  Prevents unwanted
    overwriting of responseActiveSet, functionLabels, etc.  Also, care is
    taken to account for differences in derivative variable matrix sizing. */
void ResponseRep::
update_partial(size_t start_index_target, size_t num_items,
	       const RealVector& source_fn_vals,
	       const RealMatrix& source_fn_grads,
	       const RealSymMatrixArray& source_fn_hessians,
	       const ActiveSet& source_set, size_t start_index_source)
{
  if (!num_items)
    return;

  // current response data
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, j, k, num_fns = asv.size(), 
    num_params = responseActiveSet.derivative_vector().size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_fns; i++) {
    if (asv[i] & 2)
      grad_flag = true;
    if (asv[i] & 4)
      hess_flag = true;
  }

  // verify that incoming data is of sufficient size
  if (start_index_target + num_items > num_fns ||
      start_index_source + num_items > source_set.request_vector().size() ) {
    Cerr << "Error: insufficient number of response functions to update partial"
	 << " response results in ResponseRep::update_partial()." << std::endl;
    abort_handler(-1);
  }
  if ( (grad_flag || hess_flag) && 
       source_set.derivative_vector().size() < num_params) {
    Cerr << "Error: insufficient number of derivative variables to update "
	 << "partial response derivative results in ResponseRep::"
	 << "update_partial()." << std::endl;
    abort_handler(-1);
  }

  for (i=0; i<num_items; i++)
    if (asv[start_index_target+i] & 1)
      functionValues[start_index_target+i]
	= source_fn_vals[start_index_source+i];

  // copy source_fn_grads and source_fn_hessians.  For now, a mapping from
  // responseActiveSet.derivative_vector() to source_set.derivative_vector()
  // is _not_ used, although this may be needed in the future.
  if (grad_flag) {
    if (source_fn_grads.numCols() < start_index_source + num_items) {
      Cerr << "Error: insufficient incoming gradient size to update partial "
	   << "response gradient results required in ResponseRep::"
	   << "update_partial()." << std::endl;
      abort_handler(-1);
    }
    for (i=0; i<num_items; i++)
      if (asv[start_index_target+i] & 2)
	for (j=0; j<num_params; j++)
	  functionGradients(j,start_index_target+i)
	    = source_fn_grads(j,start_index_source+i);
  }
  if (hess_flag) {
    if (source_fn_hessians.size() < start_index_source + num_items) {
      Cerr << "Error: insufficient incoming Hessian size to update partial "
	   << "response Hessian results required in ResponseRep::"
	   << "update_partial()." << std::endl;
      abort_handler(-1);
    }
    for (i=0; i<num_items; i++)
      if (asv[start_index_target+i] & 4)
	for (j=0; j<num_params; j++)
	  for (k=0; k<=j; k++)
	    functionHessians[start_index_target+i](j,k)
	      = source_fn_hessians[start_index_source+i](j,k);
  }

  // Remove any data retrieved from response but not reqd by responseActiveSet
  // (id_vars_set_compare will detect duplication when the current asv is a
  // subset of the database asv and more can be retrieved than requested).
  if (responseActiveSet != source_set) // only reset if needed
    reset_inactive();// active data not included in partial update are not reset
}


/** Reshape functionValues, functionGradients, and functionHessians
    according to num_fns, num_params, grad_flag, and hess_flag. */
void ResponseRep::reshape(size_t num_fns, size_t num_params,
			  bool grad_flag, bool hess_flag)
{
  if (responseActiveSet.request_vector().size()    != num_fns || 
      responseActiveSet.derivative_vector().size() != num_params)
    responseActiveSet.reshape(num_fns, num_params);
  if (functionLabels.size() != num_fns) {
    functionLabels.resize(num_fns);
    build_labels(functionLabels, "f");
  }
  if (functionValues.length() != num_fns)
    functionValues.resize(num_fns);
  if (grad_flag) {
    if (functionGradients.numRows() != num_params || 
        functionGradients.numCols() != num_fns)
      functionGradients.reshape(num_params, num_fns);
  }
  else if (!functionGradients.empty())
    functionGradients.shape(0,0);
  if (hess_flag) {
    if (functionHessians.size() != num_fns)
      functionHessians.resize(num_fns);
    for (size_t i=0; i<num_fns; ++i) {
      if (functionHessians[i].numRows() != num_params)
        functionHessians[i].reshape(num_params);
    }
  }
  else if (!functionHessians.empty())
    functionHessians.clear();
}


/** Reset all numerical response data (not labels, ids, or active set)
    to zero. */
void ResponseRep::reset()
{
  functionValues = 0.;
  functionGradients = 0.;
  size_t nh = functionHessians.size();
  for (size_t i=0; i<nh; i++)
    functionHessians[i] = 0.;
}


/** Used to clear out any inactive data left over from previous evaluations. */
void ResponseRep::reset_inactive()
{
  const ShortArray& asv = responseActiveSet.request_vector();
  size_t i, asv_len = asv.size(), ng = functionGradients.numCols(),
    nh = functionHessians.size();
  for (i=0; i<asv_len; ++i)
    if ( !(asv[i] & 1) ) // value bit is omitted
      functionValues[i] = 0.;
  for (int i=0; i<ng; ++i)
    if ( !(asv[i] & 2) ) { // gradient bit is omitted
      std::fill_n(functionGradients[i], functionGradients.numRows(), 0.);
    }
  for (i=0; i<nh; ++i)
    if ( !(asv[i] & 4) ) // Hessian bit is omitted
      functionHessians[i] = 0.;  // the i_th Hessian (all rows & columns)
}


/// equality operator for ResponseRep
bool operator==(const ResponseRep& rep1, const ResponseRep& rep2)
{
  return (rep1.functionValues    == rep2.functionValues    &&
	  rep1.functionGradients == rep2.functionGradients &&
	  rep1.functionHessians  == rep2.functionHessians  &&
	  rep1.responseActiveSet == rep2.responseActiveSet &&
	  rep1.functionLabels    == rep2.functionLabels);//&&
        //rep1.responsesId       == rep2.responsesId);
}


void ResponseRep::active_set_request_vector(const ShortArray& asrv)
{
  // a change in ASV length is not currently allowed
  if (responseActiveSet.request_vector().size() != asrv.size()) {
    Cerr << "Error: total number of response functions may not be changed in "
	 << "ResponseRep::active_set_request_vector(ShortArray&)." << std::endl;
    abort_handler(-1);
  }
  // assign the new request vector
  responseActiveSet.request_vector(asrv);
}


void ResponseRep::active_set_derivative_vector(const SizetArray& asdv)
{
  // resize functionGradients/functionHessians if needed to accomodate
  // a change in DVV size
  size_t new_deriv_vars = asdv.size();
  if (responseActiveSet.derivative_vector().size() != new_deriv_vars) {
    size_t num_fns = responseActiveSet.request_vector().size();
    //reshape(num_fns, new_deriv_vars, !functionGradients.empty(),
    //	      !functionHessians.empty());
    if (!functionGradients.empty())
      functionGradients.reshape(new_deriv_vars, num_fns);
    if (!functionHessians.empty())
      for (size_t i=0; i<num_fns; i++)
	functionHessians[i].reshape(new_deriv_vars);
  }
  // assign the new derivative vector
  responseActiveSet.derivative_vector(asdv);
}

/** Implementation of serialization load for the Response handle */
template<class Archive>
void Response::load(Archive& ar, const unsigned int version)
{
  if (responseRep) // should not occur in current usage
    responseRep->load(ar, version); // fwd to existing rep
  else { // read from restart: responseRep must be instantiated
    responseRep = new ResponseRep(); // default constructor is sufficient
    responseRep->load(ar, version); // fwd to new rep
  }
}


/** Implementation of serialization save for the Response handle */
template<class Archive>
void Response::save(Archive& ar, const unsigned int version) const
{
  if (responseRep) responseRep->save(ar, version);
}


// These shouldn't be necessary, but using to avoid static linking
// issues until can find the right Boost macro ordering
template void Response::
load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive& ar, 
				      const unsigned int version);

template void Response::
save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive& ar, 
				      const unsigned int version) const;


} // namespace Dakota
