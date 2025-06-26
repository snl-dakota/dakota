/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaFieldApproximation.hpp"
#ifdef HAVE_DAKOTA_PYTHON_SURROGATES
#include "DakotaSurrogatesPython.hpp"
#endif  // HAVE_DAKOTA_PYTHON_SURROGATES

namespace Dakota {

/** constructor initializes the base class part of letter classes
    (BaseConstructor overloading avoids infinite recursion in the
    derived class constructors - Coplien, p. 139) */
FieldApproximation::FieldApproximation(BaseConstructor,
                                       const ProblemDescDB& problem_db,
                                       const SharedApproxData& shared_data,
                                       const StringArray& approx_labels)
    : Approximation(BaseConstructor(), problem_db, shared_data,
                    approx_labels.at(0)),
      numComponents(approx_labels.size()) { /* empty ctor */ }

FieldApproximation::FieldApproximation(NoDBBaseConstructor ndbbc,
                                       const SharedApproxData& shared_data)
    : Approximation(ndbbc, shared_data), numComponents(0) { /* empty ctor */ }

/** The default constructor is used in Array<Approximation> instantiations
    and by the alternate envelope constructor.  approxRep is NULL in this
    case (problem_db is needed to build a meaningful Approximation object). */
FieldApproximation::FieldApproximation()
    : Approximation(), numComponents(0) { /* empty ctor */ }

/** Envelope constructor only needs to extract enough data to properly
    execute get_approx, since Approximation(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
FieldApproximation::FieldApproximation(ProblemDescDB& problem_db,
                                       const SharedApproxData& shared_data,
                                       const StringArray& approx_labels)
    : FieldApproximation(BaseConstructor(), problem_db, shared_data,
                         approx_labels) {
  // assert( !approxRep ); // Needs to be set by us
  if (numComponents == 1)
    approxRep = get_approx(problem_db, shared_data, approx_labels[0]);
  else
    approxRep = get_field_approx(problem_db, shared_data, approx_labels);
}

/** Used only by the envelope constructor to initialize approxRep to the
    appropriate derived type. */
std::shared_ptr<FieldApproximation> FieldApproximation::get_field_approx(
    ProblemDescDB& problem_db, const SharedApproxData& shared_data,
    const StringArray& approx_labels) {
  const String& approx_type = shared_data.data_rep()->approxType;

  Cout << "FieldApproximation::get_field_approx: creating \"" << approx_type
       << "\" for each of these responses:\n";
  for (auto const& label : approx_labels)
    Cout << "\t\"" << label << "\"" << std::endl;

  // The only dervived Field approximation at present.
  std::shared_ptr<FieldApproximation> approx;
#ifdef HAVE_DAKOTA_PYTHON_SURROGATES
  approx = std::make_shared<SurrogatesPythonApprox>(problem_db, shared_data,
                                                    approx_labels);
#endif

  return approx;
}

FieldApproximation::~FieldApproximation() { /* empty dtor */ }

/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void FieldApproximation::build(int num_resp) {
  if (approxRep)
    approxRep->build(num_resp);
  else {  // default is only a data check; augmented/replaced by derived class
    check_points(approxData.points());
  }
}

}  // namespace Dakota
