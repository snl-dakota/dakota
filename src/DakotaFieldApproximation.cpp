/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaCompositeApproximation.hpp"


namespace Dakota {

/** constructor initializes the base class part of letter classes
    (BaseConstructor overloading avoids infinite recursion in the
    derived class constructors - Coplien, p. 139) */
FieldApproximation::
FieldApproximation(BaseConstructor, const ProblemDescDB& problem_db,
                   const SharedApproxData& shared_data, 
                   const StringArray& approx_labels):
  Approximation(BaseConstructor(), problem_db, shared_data, String("field"))
{ /* empty ctor */ }

/** The default constructor is used in Array<Approximation> instantiations
    and by the alternate envelope constructor.  approxRep is NULL in this
    case (problem_db is needed to build a meaningful Approximation object). */
FieldApproximation::FieldApproximation():
  Approximation()
{ /* empty ctor */ }


/** Envelope constructor only needs to extract enough data to properly
    execute get_approx, since Approximation(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
FieldApproximation::
FieldApproximation(ProblemDescDB& problem_db,
                   const SharedApproxData& shared_data,
                   const StringArray& approx_labels):
  FieldApproximation(BaseConstructor(), problem_db, shared_data, approx_labels)
{
  //assert( !approxRep ); // Needs to be set by us
  approxRep = get_field_approx(problem_db, shared_data, approx_labels);
}


/** Used only by the envelope constructor to initialize approxRep to the 
    appropriate derived type. */
std::shared_ptr<FieldApproximation> FieldApproximation::
get_field_approx(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
           const StringArray& approx_labels)
{
    const String& approx_type = shared_data.data_rep()->approxType;

    Cout << "FieldApproximation::get_field_approx: creating \""
         << approx_type << "\" for each of these responses:\n";
    for (auto const & label : approx_labels)
      Cout << "\t\"" << label << "\"" << std::endl;

    // TODO: create a CompositeApproximation class which allows registration of
    //       scalar Approximations which then get created and registered here.
    //       Then start delegating or customizing API calls using ApproximationFieldInterface
    //       and needed implementation in FieldApproximation classes.
    //std::shared_ptr<FieldApproximation>(
    //    new CompositeApproximation(problem_db, shared_data, approx_labels));
    auto approx = std::make_shared<CompositeApproximation>(problem_db, shared_data, approx_labels);

    for (size_t i=0; i<approx_labels.size(); ++i)
      approx->add_approximation(std::make_shared<Approximation>(problem_db, shared_data, approx_labels[i]));

    return approx;


//#ifdef HAVE_SURFPACK
//    else if (approx_type == "global_polynomial"     ||
//	     approx_type == "global_kriging"        ||
//	     approx_type == "global_neural_network" || // TO DO: Two ANN's ?
//	     approx_type == "global_radial_basis"   ||
//	     approx_type == "global_mars"           ||
//	     approx_type == "global_moving_least_squares")
//      return std::make_shared<SurfpackApproximation>
//	(problem_db, shared_data, approx_label);
//#endif // HAVE_SURFPACK
//#ifdef HAVE_DAKOTA_SURROGATES
//    else if (approx_type == "global_exp_gauss_proc")
//      return std::make_shared<SurrogatesGPApprox>
//	(problem_db, shared_data, approx_label);
//    else if (approx_type == "global_exp_poly")
//      return std::make_shared<SurrogatesPolyApprox>
//	(problem_db, shared_data, approx_label);
//#ifdef HAVE_DAKOTA_PYTHON_SURROGATES
//    else if (approx_type == "global_exp_python")
//      return std::make_shared<SurrogatesPythonApprox>
//	(problem_db, shared_data, approx_label);
//#endif // HAVE_DAKOTA_PYTHON_SURROGATES
//#endif // HAVE_DAKOTA_SURROGATES
//    else {
//      Cerr << "Error: FieldApproximation type " << approx_type << " not available."
//	   << std::endl;
//    }
//  }
  return std::shared_ptr<FieldApproximation>();
}


FieldApproximation::~FieldApproximation()
{ /* empty dtor */ }


/** This is the common base class portion of the virtual fn and is
    insufficient on its own; derived implementations should explicitly
    invoke (or reimplement) this base class contribution. */
void FieldApproximation::build()
{
  if (approxRep)
    approxRep->build();
  else { // default is only a data check; augmented/replaced by derived class
    check_points(approxData.points());
  }
}

} // namespace Dakota
