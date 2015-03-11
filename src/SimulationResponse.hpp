/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SimulationResponse
//- Description:  Container class for response functions and their derivatives.
//-
//- Owner:        Mike Eldred
//- Version: $Id: SimulationResponse.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef SIMULATION_RESPONSE_H
#define SIMULATION_RESPONSE_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "DakotaResponse.hpp"

namespace Dakota {

class ProblemDescDB;


/// Container class for response functions and their derivatives.  
/// SimulationResponse provides the body class.

/** The SimulationResponse class is the "representation" of the
    response container class.  It is the "body" portion of the
    "handle-body idiom" (see Coplien "Advanced C++", p. 58).  The
    handle class (Response) provides for memory efficiency in
    management of multiple response objects through reference counting
    and representation sharing.  The body class (SimulationResponse)
    actually contains the response data (functionValues,
    functionGradients, functionHessians, etc.).  The representation is
    hidden in that an instance of SimulationResponse may only be
    created by Response.  Therefore, programmers create
    instances of the Response handle class, and only need to be
    aware of the handle/body mechanisms when it comes to managing
    shallow copies (shared representation) versus deep copies
    (separate representation used for history mechanisms). */

class SimulationResponse: public Response
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  SimulationResponse();
  /// standard constructor built from problem description database
  SimulationResponse(const Variables& vars, const ProblemDescDB& problem_db);
  /// alternate constructor that shares a SharedResponseData instance
  SimulationResponse(const SharedResponseData& srd, const ActiveSet& set);
  /// alternate constructor that shares a SharedResponseData instance
  SimulationResponse(const SharedResponseData& srd);
  /// alternate constructor using limited data
  SimulationResponse(const ActiveSet& set);
  /// destructor
  ~SimulationResponse();

protected:

  //
  //- Heading: member functions
  //

private:

  //
  //- Heading: Private data members
  //

};


inline SimulationResponse::SimulationResponse()
{ }


inline SimulationResponse::~SimulationResponse()
{ }

} // namespace Dakota


// Since we may serialize this class through a temporary, force
// serialization mode and no tracking
BOOST_CLASS_IMPLEMENTATION(Dakota::SimulationResponse, 
			   boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(Dakota::SimulationResponse, 
		     boost::serialization::track_never)

#endif // !SIMULATION_RESPONSE_H
