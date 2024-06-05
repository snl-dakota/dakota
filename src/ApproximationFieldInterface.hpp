/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef APPROXIMATION_FIELD_INTERFACE_H
#define APPROXIMATION_FIELD_INTERFACE_H

#include "ApproximationInterface.hpp"


namespace Dakota {

/// Derived class within the interface class hierarchy for supporting
/// field-based approximations to simulation-based results.

class ApproximationFieldInterface: public ApproximationInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// primary constructor
  ApproximationFieldInterface(ProblemDescDB& problem_db, const Variables& am_vars,
			 bool am_cache, const String& am_interface_id,
			 const Response & resp);
  /// alternate constructor for instantiations on the fly
  //ApproximationFieldInterface(const String& approx_type,
  //      		 const UShortArray& approx_order,
  //      		 const Variables& am_vars, bool am_cache,
  //      		 const String& am_interface_id, size_t num_fns,
  //                     int num_fields,
  //      		 short data_order, short output_level);
  /// destructor
  ~ApproximationFieldInterface();

protected:

  int numFields;
  int numScalars;
  std::vector<std::vector<int>> fieldFnIndices;
};


inline ApproximationFieldInterface::~ApproximationFieldInterface() { }

} // namespace Dakota

#endif
