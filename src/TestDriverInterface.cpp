/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TestDriverInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred, Brian Adams

#include "TestDriverInterface.hpp"
#include "ParallelLibrary.hpp"
#include "DataMethod.hpp"  // for output levels
//#include <unistd.h> // for sleep(int)
#ifdef DAKOTA_MODELCENTER
#include "PHXCppApi.h"
#endif
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/assign.hpp>
#include <vector>
#include "Teuchos_SerialDenseHelpers.hpp"
#include "NonDLHSSampling.hpp"
#include "spectral_diffusion.hpp"
#include "predator_prey.hpp"

namespace Dakota {

/// offset used text_book exponent: 1.0 is nominal, 1.4 used for B&B testing
const double POW_VAL = 1.0 ;
/// levenshtein_distance computes the distance between its argument and this
// reference string
const String LEV_REF = "Dakota";

StringRealMap TestDriverInterface::levenshteinDistanceCache;

#ifdef DAKOTA_SALINAS
/// subroutine interface to SALINAS simulation code
int salinas_main(int argc, char *argv[], MPI_Comm* comm);
#endif // DAKOTA_SALINAS


TestDriverInterface::TestDriverInterface(const ProblemDescDB& problem_db)
  : DirectApplicInterface(problem_db)
{
  // register this class' analysis driver types with the string to enum map
  // at the base class
  driverTypeMap["cantilever"]             = CANTILEVER_BEAM;
  driverTypeMap["mod_cantilever"]         = MOD_CANTILEVER_BEAM;
  driverTypeMap["cyl_head"]               = CYLINDER_HEAD;
  driverTypeMap["extended_rosenbrock"]    = EXTENDED_ROSENBROCK;
  driverTypeMap["generalized_rosenbrock"] = GENERALIZED_ROSENBROCK;
  driverTypeMap["lf_rosenbrock"]          = LF_ROSENBROCK;
  driverTypeMap["extra_lf_rosenbrock"]    = EXTRA_LF_ROSENBROCK;
  driverTypeMap["mf_rosenbrock"]          = MF_ROSENBROCK;
  driverTypeMap["rosenbrock"]             = ROSENBROCK;
  driverTypeMap["modified_rosenbrock"]    = MODIFIED_ROSENBROCK;
  driverTypeMap["lf_poly_prod"]           = LF_POLY_PROD;
  driverTypeMap["poly_prod"]              = POLY_PROD;
  driverTypeMap["gerstner"]               = GERSTNER;
  driverTypeMap["scalable_gerstner"]      = SCALABLE_GERSTNER;
  driverTypeMap["log_ratio"]              = LOGNORMAL_RATIO;
  driverTypeMap["multimodal"]             = MULTIMODAL;
  driverTypeMap["lf_short_column"]        = LF_SHORT_COLUMN;
  driverTypeMap["mf_short_column"]        = MF_SHORT_COLUMN;
  driverTypeMap["short_column"]           = SHORT_COLUMN;
  driverTypeMap["side_impact_cost"]       = SIDE_IMPACT_COST;
  driverTypeMap["side_impact_perf"]       = SIDE_IMPACT_PERFORMANCE;
  driverTypeMap["sobol_rational"]         = SOBOL_RATIONAL;
  driverTypeMap["sobol_g_function"]       = SOBOL_G_FUNCTION;
  driverTypeMap["sobol_ishigami"]         = SOBOL_ISHIGAMI;
  driverTypeMap["steel_column_cost"]      = STEEL_COLUMN_COST;
  driverTypeMap["steel_column_perf"]      = STEEL_COLUMN_PERFORMANCE;
  driverTypeMap["text_book"]              = TEXT_BOOK;
  driverTypeMap["text_book1"]             = TEXT_BOOK1;
  driverTypeMap["text_book2"]             = TEXT_BOOK2;
  driverTypeMap["text_book3"]             = TEXT_BOOK3;
  driverTypeMap["text_book_ouu"]          = TEXT_BOOK_OUU;
  driverTypeMap["scalable_text_book"]     = SCALABLE_TEXT_BOOK;
  driverTypeMap["scalable_monomials"]     = SCALABLE_MONOMIALS;
  driverTypeMap["mogatest1"]              = MOGATEST1;
  driverTypeMap["mogatest2"]              = MOGATEST2;
  driverTypeMap["mogatest3"]              = MOGATEST3;
  driverTypeMap["illumination"]           = ILLUMINATION;
  driverTypeMap["barnes"]                 = BARNES;
  driverTypeMap["barnes_lf"]              = BARNES_LF;
  driverTypeMap["herbie"]                 = HERBIE;
  driverTypeMap["smooth_herbie"]          = SMOOTH_HERBIE;
  driverTypeMap["shubert"]                = SHUBERT;
  driverTypeMap["salinas"]                = SALINAS;
  driverTypeMap["mc_api_run"]             = MODELCENTER;
  driverTypeMap["modelcenter"]            = MODELCENTER;
  driverTypeMap["genz"]                   = GENZ;
  driverTypeMap["damped_oscillator"]      = DAMPED_OSCILLATOR;
  driverTypeMap["steady_state_diffusion_1d"] = STEADY_STATE_DIFFUSION_1D;
  driverTypeMap["transient_diffusion_1d"] = TRANSIENT_DIFFUSION_1D;
  driverTypeMap["predator_prey"]          = PREDATOR_PREY;
  driverTypeMap["aniso_quad_form"]        = ANISOTROPIC_QUADRATIC_FORM;
  driverTypeMap["bayes_linear"]           = BAYES_LINEAR;

  // convert strings to enums for analysisDriverTypes, iFilterType, oFilterType
  analysisDriverTypes.resize(numAnalysisDrivers);
  std::map<String, driver_t>::iterator sd_iter;
  for (size_t i=0; i<numAnalysisDrivers; ++i) {
    sd_iter = driverTypeMap.find(analysisDrivers[i]);//toLower(Drivers[i]));
    if (sd_iter == driverTypeMap.end()) {
      if (outputLevel > NORMAL_OUTPUT)
	Cerr << "Warning: analysis_driver \"" << analysisDrivers[i] << "\" not "
	     << "available at construct time in TestDriverInterface.\n       "
	     << "  Subsequent interface plug-in may resolve." << std::endl;
      analysisDriverTypes[i] = NO_DRIVER;
    }
    else
      analysisDriverTypes[i] = sd_iter->second;
  }

  sd_iter = driverTypeMap.find(iFilterName);  //toLower(iFilterName));
  if (sd_iter == driverTypeMap.end()) {
    if (outputLevel > NORMAL_OUTPUT)
      Cerr << "Warning: input filter \"" << iFilterName << "\" not available at"
	   << " construct time in TestDriverInterface.\n         Subsequent "
	   << "interface plug-in may resolve." << std::endl;
    iFilterType = NO_DRIVER;
  }
  else
    iFilterType = sd_iter->second;

  sd_iter = driverTypeMap.find(oFilterName);  //toLower(oFilterName));
  if (sd_iter == driverTypeMap.end()) {
    if (outputLevel > NORMAL_OUTPUT)
      Cerr << "Warning: output filter \"" << oFilterName << "\" not available "
	   << "at construct time in TestDriverInterface.\n         Subsequent"
	   << " interface plug-in may resolve." << std::endl;
    oFilterType = NO_DRIVER;
  }
  else
    oFilterType = sd_iter->second;

  // define localDataView from analysisDriverTypes,
  // overriding any base class constructor setting
  localDataView = 0;
  for (size_t i=0; i<numAnalysisDrivers; ++i)
    switch (analysisDriverTypes[i]) {
    case CANTILEVER_BEAM: case MOD_CANTILEVER_BEAM:
    case ROSENBROCK:   case LF_ROSENBROCK:    case EXTRA_LF_ROSENBROCK:
    case MF_ROSENBROCK:    case MODIFIED_ROSENBROCK:
    case SHORT_COLUMN: case LF_SHORT_COLUMN: case MF_SHORT_COLUMN:
    case SOBOL_ISHIGAMI: case STEEL_COLUMN_COST: case STEEL_COLUMN_PERFORMANCE:
      localDataView |= VARIABLES_MAP;    break;
    case NO_DRIVER: // assume VARIABLES_VECTOR approach for plug-ins for now
    case CYLINDER_HEAD:       case LOGNORMAL_RATIO:     case MULTIMODAL:
    case GERSTNER:            case SCALABLE_GERSTNER:
    case EXTENDED_ROSENBROCK: case GENERALIZED_ROSENBROCK:
    case SIDE_IMPACT_COST:    case SIDE_IMPACT_PERFORMANCE:
    case SOBOL_G_FUNCTION:    case SOBOL_RATIONAL:
    case TEXT_BOOK:     case TEXT_BOOK1: case TEXT_BOOK2: case TEXT_BOOK3:
    case TEXT_BOOK_OUU: case SCALABLE_TEXT_BOOK: case SCALABLE_MONOMIALS:
    case MOGATEST1:     case MOGATEST2:          case MOGATEST3:
    case ILLUMINATION:  case LF_POLY_PROD:       case POLY_PROD:
    case BARNES:        case BARNES_LF:
    case HERBIE:        case SMOOTH_HERBIE:      case SHUBERT:
    case SALINAS:       case MODELCENTER:
    case GENZ: case DAMPED_OSCILLATOR:
    case STEADY_STATE_DIFFUSION_1D: case TRANSIENT_DIFFUSION_1D:
    case PREDATOR_PREY: case ANISOTROPIC_QUADRATIC_FORM: case BAYES_LINEAR:
      localDataView |= VARIABLES_VECTOR; break;
    }

  // define varTypeMap for analysis drivers based on xCM/XDM maps
  if (localDataView & VARIABLES_MAP) {
    // define the string to enumeration map
    //switch (ac_name) {
    //case ROSENBROCK: case LF_ROSENBROCK: case MF_ROSENBROCK:
    //case SOBOL_ISHIGAMI:
      varTypeMap["x1"] = VAR_x1; varTypeMap["x2"] = VAR_x2;
      varTypeMap["x3"] = VAR_x3; //varTypeMap["x4"]  = VAR_x4;
      //varTypeMap["x5"] = VAR_x5; varTypeMap["x6"]  = VAR_x6;
      //varTypeMap["x7"] = VAR_x7; varTypeMap["x8"]  = VAR_x8;
      //varTypeMap["x9"] = VAR_x9; varTypeMap["x10"] = VAR_x10; break;
    //case SHORT_COLUMN: case LF_SHORT_COLUMN: case MF_SHORT_COLUMN:
      varTypeMap["b"] = VAR_b; varTypeMap["h"] = VAR_h;
      varTypeMap["P"] = VAR_P; varTypeMap["M"] = VAR_M; varTypeMap["Y"] = VAR_Y;
      //break;
    //case MF_ROSENBROCK: case MF_SHORT_COLUMN: (add to previous)
      varTypeMap["ModelForm"] = VAR_MForm;
    //case CANTILEVER_BEAM: case MOD_CANTILEVER_BEAM:
      varTypeMap["w"] = VAR_w; varTypeMap["t"] = VAR_t; varTypeMap["R"] = VAR_R;
      varTypeMap["E"] = VAR_E; varTypeMap["X"] = VAR_X;
      //varTypeMap["Y"] = VAR_Y; break;
    //case STEEL_COLUMN:
      varTypeMap["Fs"] = VAR_Fs; varTypeMap["P1"] = VAR_P1;
      varTypeMap["P2"] = VAR_P2; varTypeMap["P3"] = VAR_P3;
      varTypeMap["B"]  = VAR_B;  varTypeMap["D"]  = VAR_D;
      varTypeMap["H"]  = VAR_H;  //varTypeMap["b"] = VAR_b;
      varTypeMap["d"]  = VAR_d;  //varTypeMap["h"] = VAR_h;
      varTypeMap["F0"] = VAR_F0; //varTypeMap["E"] = VAR_E; break;
    //}
  }
}


TestDriverInterface::~TestDriverInterface()
{
  // No tear-down required for now
}


/** Derived map to evaluate a particular built-in test analysis function */
int TestDriverInterface::derived_map_ac(const String& ac_name)
{
  // NOTE: a Factory pattern might be appropriate in the future to manage the
  // conditional presence of linked subroutines in TestDriverInterface.

#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within TestDriverInterface." << std::endl;
#endif // MPI_DEBUG
  int fail_code = 0;
  std::map<String, driver_t>::iterator sd_iter = driverTypeMap.find(ac_name);
  driver_t ac_type
    = (sd_iter!=driverTypeMap.end()) ? sd_iter->second : NO_DRIVER;
  switch (ac_type) {
  case CANTILEVER_BEAM:
    fail_code = cantilever(); break;
  case MOD_CANTILEVER_BEAM:
    fail_code = mod_cantilever(); break;
  case CYLINDER_HEAD:
    fail_code = cyl_head(); break;
  case ROSENBROCK:
    fail_code = rosenbrock(); break;
  case MODIFIED_ROSENBROCK:
    fail_code = modified_rosenbrock(); break;
  case GENERALIZED_ROSENBROCK:
    fail_code = generalized_rosenbrock(); break;
  case EXTENDED_ROSENBROCK:
    fail_code = extended_rosenbrock(); break;
  case LF_ROSENBROCK:
    fail_code = lf_rosenbrock(); break;
  case EXTRA_LF_ROSENBROCK:
    fail_code = extra_lf_rosenbrock(); break;
  case MF_ROSENBROCK:
    fail_code = mf_rosenbrock(); break;
  case LF_POLY_PROD:
    fail_code = lf_poly_prod(); break;
  case POLY_PROD:
    fail_code = poly_prod(); break;
  case GERSTNER:
    fail_code = gerstner(); break;
  case SCALABLE_GERSTNER:
    fail_code = scalable_gerstner(); break;
  case LOGNORMAL_RATIO:
    fail_code = log_ratio(); break;
  case MULTIMODAL:
    fail_code = multimodal(); break;
  case SHORT_COLUMN:
    fail_code = short_column(); break;
  case LF_SHORT_COLUMN:
    fail_code = lf_short_column(); break;
  case MF_SHORT_COLUMN:
    fail_code = mf_short_column(); break;
  case SIDE_IMPACT_COST:
    fail_code = side_impact_cost(); break;
  case SIDE_IMPACT_PERFORMANCE:
    fail_code = side_impact_perf(); break;
  case SOBOL_RATIONAL:
    fail_code = sobol_rational(); break;
  case SOBOL_G_FUNCTION:
    fail_code = sobol_g_function(); break;
  case SOBOL_ISHIGAMI:
    fail_code = sobol_ishigami(); break;
  case STEEL_COLUMN_COST:
    fail_code = steel_column_cost(); break;
  case STEEL_COLUMN_PERFORMANCE:
    fail_code = steel_column_perf(); break;
  case TEXT_BOOK:
    fail_code = text_book(); break;
  case TEXT_BOOK1: // for testing concurrent analyses
    fail_code = text_book1(); break;
  case TEXT_BOOK2: // for testing concurrent analyses
    fail_code = text_book2(); break;
  case TEXT_BOOK3: // for testing concurrent analyses
    fail_code = text_book3(); break;
  case TEXT_BOOK_OUU:
    fail_code = text_book_ouu(); break;
  case SCALABLE_TEXT_BOOK:
    fail_code = scalable_text_book(); break;
  case SCALABLE_MONOMIALS:
    fail_code = scalable_monomials(); break;
  case MOGATEST1:
    fail_code = mogatest1(); break;
  case MOGATEST2:
    fail_code = mogatest2(); break;
  case MOGATEST3:
    fail_code = mogatest3(); break;
  case ILLUMINATION:
    fail_code = illumination(); break;
  case BARNES:
    fail_code = barnes(); break;
  case BARNES_LF:
    fail_code = barnes_lf(); break;
  case HERBIE:
    fail_code = herbie(); break;
  case SMOOTH_HERBIE:
    fail_code = smooth_herbie(); break;
  case SHUBERT:
    fail_code = shubert(); break;
#ifdef DAKOTA_SALINAS
  case SALINAS:
    fail_code = salinas(); break;
#endif // DAKOTA_SALINAS
#ifdef DAKOTA_MODELCENTER
  case MODELCENTER: //case MC_API_RUN:
    fail_code = mc_api_run(); break;
#endif // DAKOTA_MODELCENTER
  case GENZ:
    fail_code = genz(); break;
  case DAMPED_OSCILLATOR:
    fail_code = damped_oscillator(); break;
  case STEADY_STATE_DIFFUSION_1D:
    fail_code = steady_state_diffusion_1d(); break;
  case TRANSIENT_DIFFUSION_1D:
    fail_code = transient_diffusion_1d(); break;
  case PREDATOR_PREY:
    fail_code = predator_prey(); break;
  case ANISOTROPIC_QUADRATIC_FORM:
    fail_code = aniso_quad_form(); break;
  case BAYES_LINEAR: 
    fail_code = bayes_linear(); break;
  default: {
    Cerr << "Error: analysis_driver '" << ac_name << "' is not available in "
	 << "the direct interface." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  }

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating direct analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }

  return 0;
}


// -----------------------------------------
// Begin direct interfaces to test functions
// -----------------------------------------
int TestDriverInterface::cantilever()
{
  using std::pow;

  if (multiProcAnalysisFlag) {
    Cerr << "Error: cantilever direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  // cantilever normally has 6 variables: 2 design + 4 uncertain
  // If, however, design variables are _inserted_ into the uncertain variable
  // distribution parameters (e.g., dakota_rbdo_cantilever_mapvars.in) instead
  // of augmenting the uncertain variables, then the number of variables is 4.
  // Design gradients are not supported for the case of design var insertion.
  if ( (numVars != 4 && numVars != 6) || numADIV || numADRV ||//var count, no dv
       (gradFlag && numVars == 4 && numDerivVars != 4) ) { // design insertion
    Cerr << "Error: Bad number of variables in cantilever direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
   if (numFns < 2 || numFns > 3) {
    Cerr << "Error: Bad number of functions in cantilever direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Compute the cross-sectional area, stress, and displacement of the
  // cantilever beam.  This simulator is unusual in that it must support both
  // the case of design variable insertion and the case of design variable
  // augmentation.  It does not support mixed insertion/augmentation.  In
  // the 6 variable case, w,t,R,E,X,Y are all passed in; in the 4 variable
  // case, w,t assume local values.
  std::map<var_t, Real>::iterator m_iter = xCM.find(VAR_w);
  Real w = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam width
  m_iter = xCM.find(VAR_t);
  Real t = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam thickness
  Real R = xCM[VAR_R], // yield strength
       E = xCM[VAR_E], // Young's modulus
       X = xCM[VAR_X], // horizontal load
       Y = xCM[VAR_Y]; // vertical load

  // allow f,c1,c2 (optimization) or just c1,c2 (calibration)
  bool objective; size_t c1i, c2i;
  if (numFns == 2) { objective = false; c1i = 0; c2i = 1; }
  else             { objective = true;  c1i = 1; c2i = 2; }

  // optimization inequality constraint: <= 0 and scaled O(1)
  //Real g_stress = stress/R - 1.0;
  //Real g_disp   = disp/D0  - 1.0;

  Real D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
       R_sq = R*R, X_sq = X*X, Y_sq = Y*Y;
  Real stress = 600.*Y/w/t_sq + 600.*X/w_sq/t;
  Real D1 = 4.*pow(L,3)/E/area,  D2 = pow(Y/t_sq, 2)+pow(X/w_sq, 2),
       D3 = D1/std::sqrt(D2)/D0, D4 = D1*std::sqrt(D2)/D0;

  // **** f:
  if (objective && (directFnASV[0] & 1))
    fnVals[0] = area;

  // **** c1:
  if (directFnASV[c1i] & 1)
    fnVals[c1i] = stress/R - 1.;

  // **** c2:
  if (directFnASV[c2i] & 1)
    fnVals[c2i] = D4 - 1.;

  // **** df/dx:
  if (objective && (directFnASV[0] & 2))
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w:  fnGrads[0][i] = t;  break; // design var derivative
      case VAR_t:  fnGrads[0][i] = w;  break; // design var derivative
      default: fnGrads[0][i] = 0.; break; // uncertain var derivative
      }

  // **** dc1/dx:
  if (directFnASV[c1i] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[c1i][i] = -600.*(Y/t + 2.*X/w)/w_sq/t/R; break;// dv
      case VAR_t: fnGrads[c1i][i] = -600.*(2.*Y/t + X/w)/w/t_sq/R; break;// dv
      case VAR_R: fnGrads[c1i][i] = -stress/R_sq;  break; // uncertain var deriv
      case VAR_E: fnGrads[c1i][i] = 0.;            break; // uncertain var deriv
      case VAR_X: fnGrads[c1i][i] = 600./w_sq/t/R; break; // uncertain var deriv
      case VAR_Y: fnGrads[c1i][i] = 600./w/t_sq/R; break; // uncertain var deriv
      }

  // **** dc2/dx:
  if (directFnASV[c2i] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[c2i][i] = -D3*2.*X_sq/w_sq/w_sq/w - D4/w; break;// dv
      case VAR_t: fnGrads[c2i][i] = -D3*2.*Y_sq/t_sq/t_sq/t - D4/t; break;// dv
      case VAR_R: fnGrads[c2i][i] = 0.;             break; // unc var deriv
      case VAR_E: fnGrads[c2i][i] = -D4/E;          break; // unc var deriv
      case VAR_X: fnGrads[c2i][i] = D3*X/w_sq/w_sq; break; // unc var deriv
      case VAR_Y: fnGrads[c2i][i] = D3*Y/t_sq/t_sq; break; // unc var deriv
      }

  // **** d^2f/dx^2:
  if (objective && (directFnASV[0] & 4))
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	fnHessians[0](i,j)
	  = ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_t) ||
	      (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_w) ) ? 1. : 0.;

  // **** d^2c1/dx^2:
  if (directFnASV[c1i] & 4) {
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_w)
	  fnHessians[c1i](i,j) = 1200.*(Y/t + 3.*X/w)/w_sq/area/R;
	else if (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_t)
	  fnHessians[c1i](i,j) = 1200.*(3.*Y/t + X/w)/t_sq/area/R;
	else if (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_R)
	  fnHessians[c1i](i,j) = 2.*stress/pow(R, 3);
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_t) ||
		  (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_w) )
	  fnHessians[c1i](i,j) = 1200.*(Y/t + X/w)/w_sq/t_sq/R;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_R) ||
		  (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_w) )
	  fnHessians[c1i](i,j) = 600.*(Y/t + 2.*X/w)/w_sq/t/R_sq;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_w) )
	  fnHessians[c1i](i,j) = -1200./w_sq/w/t/R;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_w) )
	  fnHessians[c1i](i,j) = -600./w_sq/t_sq/R;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_R) ||
		  (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_t) )
	  fnHessians[c1i](i,j) = 600.*(2.*Y/t + X/w)/w/t_sq/R_sq;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_t) )
	  fnHessians[c1i](i,j) = -600./w_sq/t_sq/R;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_t) )
	  fnHessians[c1i](i,j) = -1200./w/t_sq/t/R;
	else if ( (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_R) )
	  fnHessians[c1i](i,j) = -600./w_sq/t/R_sq;
	else if ( (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_R) )
	  fnHessians[c1i](i,j) = -600./w/t_sq/R_sq;
	else
	  fnHessians[c1i](i,j) = 0.;
  }

  // **** d^2c2/dx^2:
  if (directFnASV[c2i] & 4) {
    Real D5 = 1./std::sqrt(D2)/D0, D6 = -D1/2./D0/pow(D2,1.5);
    Real D7 = std::sqrt(D2)/D0,    D8 =  D1/2./D0/std::sqrt(D2);
    Real dD2_dX = 2.*X/w_sq/w_sq, dD3_dX = D6*dD2_dX, dD4_dX = D8*dD2_dX;
    Real dD2_dY = 2.*Y/t_sq/t_sq, dD3_dY = D6*dD2_dY, dD4_dY = D8*dD2_dY;
    Real dD1_dw = -D1/w, dD2_dw = -4.*X_sq/w_sq/w_sq/w,
      dD3_dw = D5*dD1_dw + D6*dD2_dw, dD4_dw = D7*dD1_dw + D8*dD2_dw;
    Real dD1_dt = -D1/t, dD2_dt = -4.*Y_sq/t_sq/t_sq/t,
      dD3_dt = D5*dD1_dt + D6*dD2_dt, dD4_dt = D7*dD1_dt + D8*dD2_dt;
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_w)
	  fnHessians[c2i](i,j) = D3*10.*X_sq/pow(w_sq,3)
	    - 2.*X_sq/w_sq/w_sq/w*dD3_dw + D4/w_sq - dD4_dw/w;
	else if (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_t)
	  fnHessians[c2i](i,j) = D3*10.*Y_sq/pow(t_sq,3)
	    - 2.*Y_sq/t_sq/t_sq/t*dD3_dt + D4/t_sq - dD4_dt/t;
	else if (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_E) {
	  Real dD1_dE = -D1/E, dD4_dE = D7*dD1_dE;
	  fnHessians[c2i](i,j) = D4/E/E - dD4_dE/E;
	}
	else if (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_X)
	  fnHessians[c2i](i,j) = D3/w_sq/w_sq + X/w_sq/w_sq*dD3_dX;
	else if (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_Y)
	  fnHessians[c2i](i,j) = D3/t_sq/t_sq + Y/t_sq/t_sq*dD3_dY;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_t) ||
		  (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_w) )
	  fnHessians[c2i](i,j) = -2.*X_sq/w_sq/w_sq/w*dD3_dt - dD4_dt/w;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_w) )
	  fnHessians[c2i](i,j) = -dD4_dw/E;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_w) )
	  fnHessians[c2i](i,j) = -4.*X*D3/w_sq/w_sq/w + X/w_sq/w_sq*dD3_dw;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_w) )
	  fnHessians[c2i](i,j) = Y/t_sq/t_sq*dD3_dw;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_t) )
	  fnHessians[c2i](i,j) = -dD4_dt/E;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_t) )
	  fnHessians[c2i](i,j) = X/w_sq/w_sq*dD3_dt;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_t) )
	  fnHessians[c2i](i,j) = -4.*Y*D3/t_sq/t_sq/t + Y/t_sq/t_sq*dD3_dt;
	else if ( (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_E) )
	  fnHessians[c2i](i,j) = -dD4_dX/E;
	else if ( (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_E) )
	  fnHessians[c2i](i,j) = -dD4_dY/E;
	else if ( (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_X) )
	  fnHessians[c2i](i,j) = X/w_sq/w_sq*dD3_dY;
	else
	  fnHessians[c2i](i,j) = 0.;
  }

  return 0; // no failure
}


int TestDriverInterface::mod_cantilever()
{
  using std::pow;

  if (multiProcAnalysisFlag) {
    Cerr << "Error: cantilever direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  // cantilever normally has 6 variables: 2 design + 4 uncertain
  // If, however, design variables are _inserted_ into the uncertain variable
  // distribution parameters (e.g., dakota_rbdo_cantilever_mapvars.in) instead
  // of augmenting the uncertain variables, then the number of variables is 4.
  // Design gradients are not supported for the case of design var insertion.
  if ( (numVars != 4 && numVars != 6) || numADIV || numADRV ||//var count, no dv
       (gradFlag && numVars == 4 && numDerivVars != 4) ) { // design insertion
    Cerr << "Error: Bad number of variables in cantilever direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns < 2 || numFns > 3) {
    Cerr << "Error: Bad number of functions in mod_cantilever direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Compute the cross-sectional area, stress, and displacement of the
  // cantilever beam.  This simulator is unusual in that it must support both
  // the case of design variable insertion and the case of design variable
  // augmentation.  It does not support mixed insertion/augmentation.  In
  // the 6 variable case, w,t,R,E,X,Y are all passed in; in the 4 variable
  // case, w,t assume local values.
  std::map<var_t, Real>::iterator m_iter = xCM.find(VAR_w);
  Real w = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam width
  m_iter = xCM.find(VAR_t);
  Real t = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam thickness
  Real R = xCM[VAR_R], // yield strength
       E = xCM[VAR_E], // Young's modulus
       X = xCM[VAR_X], // horizontal load
       Y = xCM[VAR_Y]; // vertical load

  // allow f,c1,c2 (optimization) or just c1,c2 (calibration)
  bool objective; size_t c1i, c2i;
  if (numFns == 2) { objective = false; c1i = 0; c2i = 1; }
  else             { objective = true;  c1i = 1; c2i = 2; }

  // UQ limit state <= 0: don't scale stress by random variable r
  //double g_stress = stress - r;
  //double g_disp   = displ  - D0;

  Real D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
       R_sq = R*R, X_sq = X*X, Y_sq = Y*Y;
  Real stress = 600.*Y/w/t_sq + 600.*X/w_sq/t;
  Real D1 = 4.*pow(L,3)/E/area, D2 = pow(Y/t_sq, 2)+pow(X/w_sq, 2),
       D3 = D1/std::sqrt(D2),   displ = D1*std::sqrt(D2);

  // **** f:
  if (objective && (directFnASV[0] & 1))
    fnVals[0] = area;

  // **** c1:
  if (directFnASV[c1i] & 1)
    fnVals[c1i] = stress - R;

  // **** c2:
  if (directFnASV[c2i] & 1)
    fnVals[c2i] = displ - D0;

  // **** df/dx:
  if (objective && (directFnASV[0] & 2))
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w:  fnGrads[0][i] = t;  break; // design var derivative
      case VAR_t:  fnGrads[0][i] = w;  break; // design var derivative
      default: fnGrads[0][i] = 0.; break; // uncertain var derivative
      }

  // **** dc1/dx:
  if (directFnASV[c1i] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[c1i][i] = -600.*(Y/t + 2.*X/w)/w_sq/t; break;//des var
      case VAR_t: fnGrads[c1i][i] = -600.*(2.*Y/t + X/w)/w/t_sq; break;//des var
      case VAR_R: fnGrads[c1i][i] = -1.;          break; // uncertain var deriv
      case VAR_E: fnGrads[c1i][i] =  0.;          break; // uncertain var deriv
      case VAR_X: fnGrads[c1i][i] =  600./w_sq/t; break; // uncertain var deriv
      case VAR_Y: fnGrads[c1i][i] =  600./w/t_sq; break; // uncertain var deriv
      }

  // **** dc2/dx:
  if (directFnASV[c2i] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[c2i][i] = -D3*2.*X_sq/w_sq/w_sq/w - displ/w; break;
      case VAR_t: fnGrads[c2i][i] = -D3*2.*Y_sq/t_sq/t_sq/t - displ/t; break;
      case VAR_R: fnGrads[c2i][i] =  0.;             break; // unc var deriv
      case VAR_E: fnGrads[c2i][i] = -displ/E;        break; // unc var deriv
      case VAR_X: fnGrads[c2i][i] =  D3*X/w_sq/w_sq; break; // unc var deriv
      case VAR_Y: fnGrads[c2i][i] =  D3*Y/t_sq/t_sq; break; // unc var deriv
      }

  /* Alternative modification: take E out of displ denominator to remove
     singularity in tail (at 20 std deviations).  In PCE/SC testing, this
     had minimal impact and did not justify the nonstandard form.

  Real D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
       R_sq = R*R, X_sq = X*X, Y_sq = Y*Y;
  Real stress = 600.*Y/w/t_sq + 600.*X/w_sq/t;
  Real D1 = 4.*pow(L,3)/area, D2 = pow(Y/t_sq, 2)+pow(X/w_sq, 2),
       D3 = D1/std::sqrt(D2), D4 = D1*std::sqrt(D2);

  // **** c2:
  if (directFnASV[c2i] & 1)
    fnVals[c2i] = D4 - D0*E;

  // **** dc2/dx:
  if (directFnASV[c2i] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[c2i][i] = -D3*2.*X_sq/w_sq/w_sq/w - D4/w; break;// des
      case VAR_t: fnGrads[c2i][i] = -D3*2.*Y_sq/t_sq/t_sq/t - D4/t; break;// des
      case VAR_R: fnGrads[c2i][i] =  0.;             break; // unc var deriv
      case VAR_E: fnGrads[c2i][i] = -D0;             break; // unc var deriv
      case VAR_X: fnGrads[c2i][i] =  D3*X/w_sq/w_sq; break; // unc var deriv
      case VAR_Y: fnGrads[c2i][i] =  D3*Y/t_sq/t_sq; break; // unc var deriv
      }
  */

  return 0; // no failure
}


int TestDriverInterface::cyl_head()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: cyl_head direct fn does not yet support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numADIV || numADRV || (gradFlag && numDerivVars != 2)) {
    Cerr << "Error: Bad number of variables in cyl_head direct fn." <<std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 4) {
    Cerr << "Error: Bad number of functions in cyl_head direct fn." <<std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in cyl_head direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  Real exhaust_offset = 1.34;
  Real exhaust_dia    = 1.556;
  Real intake_offset  = 3.25;
  // Use nondimensional xC[1]:
  // (0. <= nondimensional <= 4.), (0. in <= dimensional <= 0.004 in)
  Real warranty       = 100000. + 15000. * (4. - xC[1]);
  Real cycle_time     = 45. + 4.5*std::pow(4. - xC[1], 1.5);
  Real wall_thickness = intake_offset - exhaust_offset - (xC[0]+exhaust_dia)/2.;
  Real horse_power    = 250.+200.*(xC[0]/1.833-1.);
  Real max_stress     = 750. + std::pow(std::fabs(wall_thickness),-2.5);

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] =  -1.*(horse_power/250.+warranty/100000.);

  // **** c1:
  if (directFnASV[1] & 1)
    fnVals[1] = max_stress/1500.-1.;

  // **** c2:
  if (directFnASV[2] & 1)
    fnVals[2] = 1.-warranty/100000.;

  // **** c3:
  if (directFnASV[3] & 1)
    fnVals[3] = cycle_time/60. - 1.;

  // **** c4: (Unnecessary if intake_dia upper bound reduced to 2.164)
  //if (directFnASV[4] & 1)
  //  fnVals[4] = 1.-20.*wall_thickness;

  // **** df/dx:
  if (directFnASV[0] & 2) {
    fnGrads[0][0] = -.8/1.833;
    fnGrads[0][1] = 0.15;
  }

  // **** dc1/dx:
  if (directFnASV[1] & 2) {
    fnGrads[1][0] = 1.25/1500*std::pow(wall_thickness, -3.5);
    fnGrads[1][1] = 0.;
  }

  // **** dc2/dx:
  if (directFnASV[2] & 2) {
    fnGrads[2][0] = 0.;
    fnGrads[2][1] = 0.15;
  }

  // **** dc3/dx:
  if (directFnASV[3] & 2) {
    fnGrads[3][0] = 0.;
    fnGrads[3][1] = -0.1125*std::sqrt(4. - xC[1]);
  }

  return 0; // no failure
}


int TestDriverInterface::multimodal()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: multimodal direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if ( numVars != 2 || numADIV || numADRV ||
       ( ( gradFlag || hessFlag ) && numDerivVars != 2 ) ) {
    Cerr << "Error: Bad number of variables in multimodal direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in multimodal direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = (xC[0]*xC[0]+4)*(xC[1]-1)/20 - std::sin(5*xC[0]/2) - 2;

  // **** df/dx:
  if (directFnASV[0] & 2) {
    fnGrads[0][0] = xC[0]*(xC[1]-1)/10 - (5/2)*std::cos(5*xC[0]/2);
    fnGrads[0][1] = (xC[0]*xC[0]+4)/20;
  }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4) {
    fnHessians[0](0,0) = (xC[1]-1)/10 + (25/4)*std::sin(5*xC[0]/2);
    fnHessians[0](0,1) = fnHessians[0](1,0) = xC[0]/10;
    fnHessians[0](1,1) = 0.0;
  }

  return 0; // no failure
}

int TestDriverInterface::rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: rosenbrock direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 2 || numADIV > 1 || numADRV) { // allow ModelForm discrete int
    Cerr << "Error: Bad number of variables in rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 2) { // 1 fn -> opt, 2 fns -> least sq
    Cerr << "Error: Bad number of functions in rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  bool least_sq_flag = (numFns > 1);
  Real x1 = xCM[VAR_x1], x2 = xCM[VAR_x2], f1 = x2-x1*x1, f2 = 1.-x1;

  if (least_sq_flag) {
    // **** Residual R1:
    if (directFnASV[0] & 1)
      fnVals[0] = 10.*f1;
    // **** Residual R2:
    if (directFnASV[1] & 1)
      fnVals[1] = f2;

    // **** dR1/dx:
    if (directFnASV[0] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[0][i] = -20.*x1; break;
	case VAR_x2: fnGrads[0][i] =  10.;    break;
	}
    // **** dR2/dx:
    if (directFnASV[1] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[1][i] = -1.; break;
	case VAR_x2: fnGrads[1][i] =  0.; break;
	}

    // **** d^2R1/dx^2:
    if (directFnASV[0] & 4)
      for (size_t i=0; i<numDerivVars; ++i)
	for (size_t j=0; j<=i; ++j)
	  if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	    fnHessians[0](i,j) = -20.;
	  else
	    fnHessians[0](i,j) =   0.;
    // **** d^2R2/dx^2:
    if (directFnASV[1] & 4)
      fnHessians[1] = 0.;
  }
  else {
    // **** f:
    if (directFnASV[0] & 1)
      fnVals[0] = 100.*f1*f1+f2*f2;

    // **** df/dx:
    if (directFnASV[0] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[0][i] = -400.*f1*x1 - 2.*f2; break;
	case VAR_x2: fnGrads[0][i] =  200.*f1;            break;
	}

    // **** d^2f/dx^2:
    if (directFnASV[0] & 4)
      for (size_t i=0; i<numDerivVars; ++i)
	for (size_t j=0; j<=i; ++j)
	  if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	    fnHessians[0](i,j) = -400.*(x2 - 3.*x1*x1) + 2.;
	  else if ( (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x2) ||
		    (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x1) )
	    fnHessians[0](i,j) = -400.*x1;
	  else if (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x2)
	    fnHessians[0](i,j) =  200.;
  }

  return 0; // no failure
}

int TestDriverInterface::modified_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: modified rosenbrock direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 2 || numADIV > 1 || numADRV) { // allow ModelForm discrete int
    Cerr << "Error: Bad number of variables in modified rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 3) { // 1 fn -> opt, 2 fns -> least sq
    Cerr << "Error: Bad number of functions in modified rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  bool least_sq_flag = (numFns > 1);
  Real x1 = xCM[VAR_x1], x2 = xCM[VAR_x2], f1 = x2-x1*x1, f2 = 1.-x1,
    f3 = std::sin(2.*PI*x1+x2);

  if (least_sq_flag) {
    // **** Residual R1:
    if (directFnASV[0] & 1)
      fnVals[0] = 10.*f1;
    // **** Residual R2:
    if (directFnASV[1] & 1)
      fnVals[1] = f2;
    // **** Residual R3:
    if (directFnASV[2] & 1)
      fnVals[2] = f3;

    // **** dR1/dx:
    if (directFnASV[0] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[0][i] = -20.*x1; break;
	case VAR_x2: fnGrads[0][i] =  10.;    break;
	}
    // **** dR2/dx:
    if (directFnASV[1] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[1][i] = -1.; break;
	case VAR_x2: fnGrads[1][i] =  0.; break;
	}
    // **** dR3/dx:
    if (directFnASV[2] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[2][i] = 2.*PI*std::cos(2.*PI*x1+x2);break;
	case VAR_x2: fnGrads[2][i] = std::cos(2.*PI*x1+x2); break;
	}

    // **** d^2R1/dx^2:
    if (directFnASV[0] & 4)
      for (size_t i=0; i<numDerivVars; ++i)
	for (size_t j=0; j<=i; ++j)
	  if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	    fnHessians[0](i,j) = -20.;
	  else
	    fnHessians[0](i,j) =   0.;
    // **** d^2R2/dx^2:
    if (directFnASV[1] & 4)
      fnHessians[1] = 0.;
  }
  else {
    // **** f:
    if (directFnASV[0] & 1)
      fnVals[0] = 100.*f1*f1+f2*f2+f3*f3;

    // **** df/dx:
    if (directFnASV[0] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[0][i] = -400.*f1*x1 - 2.*f2+
	    2.*PI*std::sin(2.*(2.*PI*x1+x2));
	  break;
	case VAR_x2: fnGrads[0][i] =  200.*f1+std::sin(2.*(2.*PI*x1+x2));
	  break;
	}
    
    // **** d^2f/dx^2:
    if (directFnASV[0] & 4)
      for (size_t i=0; i<numDerivVars; ++i)
	for (size_t j=0; j<=i; ++j)
	  if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	    fnHessians[0](i,j) = -400.*(x2 - 3.*x1*x1) + 2.+
	      8.*PI*PI*std::cos(2.*(2.*PI*x1+x2));
	  else if ( (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x2) ||
		    (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x1) )
	    fnHessians[0](i,j) = -400.*x1+4.*PI*std::cos(2.*(2.*PI*x1+x2));
	  else if (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x2)
	    fnHessians[0](i,j) =  200.+2.*std::cos(2.*(2.*PI*x1+x2));
  }

  return 0; // no failure
}  

int TestDriverInterface::generalized_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: generalized_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: discrete variables not supported in generalized_rosenbrock "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if ( (directFnASV[0] & 6) && numVars != numDerivVars ) {
    Cerr << "Error: DVV subsets not supported in generalized_rosenbrock direct "
	 << "fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1 && numFns != 2*(numVars-1)) {
    Cerr << "Error: Bad number of functions in extended_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  bool least_sq_flag = (numFns > 1);

  // This multidimensional extension results from the overlay of numVars-1
  // coupled 2D Rosenbrock problems.  When defining as a NLS problem,
  // residuals are paired and # residuals == 2 * (# vars - 1)

  for (size_t i=1; i<numVars; ++i) {
    size_t index_ip1 = i, index_i = i-1; // offset by 1
    const Real& x_ip1 = xC[index_ip1];
    const Real& x_i   = xC[index_i];
    Real f1 = x_ip1 - x_i*x_i, f2 = 1. - x_i;

    if (least_sq_flag) {
      size_t rindex_2i = 2*i-1, rindex_2im1 = 2*i-2;

      // **** R_2im1:
      if (directFnASV[rindex_2im1] & 1)
	fnVals[rindex_2im1] = 10.*f1;
      // **** R_2i:
      if (directFnASV[rindex_2i] & 1)
	fnVals[rindex_2i] = f2;

      // *** dR_2im1/dx:
      if (directFnASV[rindex_2im1] & 2) { // define non-zeros (set_local_data)
	Real* grad = fnGrads[rindex_2im1];
	grad[index_i]   = -20.*x_i;
	grad[index_ip1] =  10.;
      }
      // **** dR_2i/dx:
      if (directFnASV[rindex_2i] & 2) { // define non-zeros (set_local_data)
	Real* grad = fnGrads[rindex_2i];
	grad[index_i]     = -1.;
	//grad[index_ip1] =  0.;
      }

      // **** d^2R_2im1/dx^2:
      if (directFnASV[rindex_2im1] & 4) // define non-zeros (set_local_data)
	fnHessians[rindex_2im1](index_i,index_i) = -20.;
      // **** d^2R_2i/dx^2:
      if (directFnASV[rindex_2i] & 4)
	fnHessians[rindex_2i] = 0.;

    }
    else {

      // **** f:
      if (directFnASV[0] & 1)
	fnVals[0] += 100.*f1*f1 + f2*f2;

      // **** df/dx:
      if (directFnASV[0] & 2) {
	fnGrads[0][index_i]   += -400.*f1*x_i - 2.*f2;
	fnGrads[0][index_ip1] +=  200.*f1;
      }

      // **** d^2f/dx^2:
      if (directFnASV[0] & 4) {
	Real fx = x_ip1 - 3.*x_i*x_i;
	fnHessians[0](index_i,index_i)     += -400.*fx + 2.0;
	fnHessians[0](index_i,index_ip1)   += -400.*x_i;
	fnHessians[0](index_ip1,index_i)   += -400.*x_i;
	fnHessians[0](index_ip1,index_ip1) +=  200.;
      }
    }
  }

  return 0; // no failure
}


int TestDriverInterface::extended_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: extended_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: discrete variables not supported in extended_rosenbrock "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if ( (directFnASV[0] & 6) && numVars != numDerivVars ) {
    Cerr << "Error: DVV subsets not supported in extended_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numVars % 2) {
    Cerr << "Error: Bad number of variables in extended_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1 && numFns != numVars) {
    Cerr << "Error: Bad number of functions in extended_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // This multidimensional extension results from the sum of numVars/2
  // uncoupled 2D Rosenbrock problems.  When defining as a NLS problem,
  // residuals are paired and # residuals == # vars

  bool least_sq_flag = (numFns > 1);
  Real alpha = 100., sqrt_alpha = 10.;
  for (size_t i=1; i<=numVars/2; ++i) {
    size_t index_2i = 2*i-1, index_2im1 = 2*i-2; // offset by 1
    const Real& x_2i   = xC[index_2i];
    const Real& x_2im1 = xC[index_2im1];
    Real f1 = x_2i - x_2im1*x_2im1, f2 = 1. - x_2im1;

    if (least_sq_flag) {

      // **** Residual R_2im1:
      if (directFnASV[index_2im1] & 1)
	fnVals[index_2im1] = sqrt_alpha*f1;
      // **** Residual R_2i:
      if (directFnASV[index_2i] & 1)
	fnVals[index_2i] = f2;

      // *** dR_2im1/dx:
      if (directFnASV[index_2im1] & 2) { // define non-zeros (set_local_data)
	Real* grad = fnGrads[index_2im1];
	grad[index_2im1] = -2.*sqrt_alpha*x_2im1;
	grad[index_2i]   =     sqrt_alpha;
      }
      // **** dR_2i/dx:
      if (directFnASV[index_2i] & 2) { // define non-zeros (set_local_data)
	Real* grad = fnGrads[index_2i];
	grad[index_2im1] = -1.;
	//grad[index_2i] =  0.;
      }

      // **** d^2R_2im1/dx^2:
      if (directFnASV[index_2im1] & 4) // define non-zeros (set_local_data)
	fnHessians[index_2im1](index_2im1,index_2im1) = -2.*sqrt_alpha;
      // **** d^2R_2i/dx^2:
      if (directFnASV[index_2i] & 4)
	fnHessians[index_2i] = 0.;

    }
    else {
      // **** f:
      if (directFnASV[0] & 1)
	fnVals[0] += alpha*f1*f1 + f2*f2;

      // **** df/dx:
      if (directFnASV[0] & 2) {
	fnGrads[0][index_2im1] += -4.*alpha*f1*x_2im1 - 2.*f2;
	fnGrads[0][index_2i]   +=  2.*alpha*f1;
      }

      // **** d^2f/dx^2:
      if (directFnASV[0] & 4) {
	Real fx = x_2i - 3.*x_2im1*x_2im1;
	fnHessians[0](index_2im1,index_2im1) += -4.*alpha*fx + 2.0;
	fnHessians[0](index_2im1,index_2i)   += -4.*alpha*x_2im1;
	fnHessians[0](index_2i,index_2im1)   += -4.*alpha*x_2im1;
	fnHessians[0](index_2i,index_2i)     +=  2.*alpha;
      }
    }
  }

  return 0; // no failure
}


int TestDriverInterface::lf_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: lf_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 2 || numADIV > 1 || numADRV) { // allow ModelForm discrete int
    Cerr << "Error: Bad number of variables in lf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 1) {
    Cerr << "Error: Bad number of functions in lf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  Real x1 = xCM[VAR_x1],     x2 = xCM[VAR_x2],
       f1 = x2 - x1*x1 + .2, f2 = 0.8 - x1; // terms offset by +/- .2

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 100.*f1*f1+f2*f2;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_x1: fnGrads[0][i] = -400.*f1*x1 - 2.*f2; break;
      case VAR_x2: fnGrads[0][i] =  200.*f1;            break;
      }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	  fnHessians[0](i,j) = -400.*(x2 - 3.*x1*x1 + .2) + 2.;
	else if ( (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x2) ||
		  (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x1) )
	  fnHessians[0](i,j) = -400.*x1;
	else if (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x2)
	  fnHessians[0](i,j) =  200.;

  return 0; // no failure
}

// extra low fidelity rosenbrock
int TestDriverInterface::extra_lf_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: extra_lf_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 2 || numADIV > 1 || numADRV) { // allow ModelForm discrete int
    Cerr << "Error: Bad number of variables in lf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 1) {
    Cerr << "Error: Bad number of functions in extra_lf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  Real a = 0.92;
  Real b = -1.0;
  Real c = 1.1;
  Real d = 1.1;

  Real x1 = xCM[VAR_x1],     x2 = xCM[VAR_x2],
       f1 = x2 - a*x1*x1 + b, f2 = c - d*x1;

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 100.*f1*f1+f2*f2;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_x1: fnGrads[0][i] = -400.*a*f1*x1 - 2.*d*f2; break;
      case VAR_x2: fnGrads[0][i] =  200.*f1;            break;
      }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	  fnHessians[0](i,j) = -400.*a*f1 + 400.*a*a*x1*x1 + 2.*d*d;
	else if ( (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x2) ||
		  (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x1) )
	  fnHessians[0](i,j) = -400.*a*x1;
	else if (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x2)
	  fnHessians[0](i,j) =  200.;

  return 0; // no failure
}


int TestDriverInterface::mf_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: mf_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 3 || numADRV) {
    Cerr << "Error: Bad number of variables in mf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 1) {
    Cerr << "Error: Bad number of functions in mf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  switch (xDIM[VAR_MForm]) {
  case 1:    rosenbrock(); break;
  case 2: lf_rosenbrock(); break;
  default:       return 1; break;
  }

  return 0; // no failure
}

/// modified lo-fi Rosenbrock to test SBO with hierarchical approximations
int TestDriverInterface::lf_poly_prod()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: lf_poly_prod direct fn does not yet support multiprocessor "
      << "analyses." << std::endl;
    abort_handler(-1);
  }

  if ( (gradFlag || hessFlag) && (numADIV || numADRV) ) {
    Cerr << "Error: lf_poly_prod direct fn assumes no discrete variables in "
	 << "derivative or hessian mode." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numACV != 2) {
    Cerr << "Error: Bad number of variables in lf_poly_prod direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numFns != 1 ) {
    Cerr << "Error: Bad number of functions in lf_poly_prod direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if (directFnASV[0] & 1)
    fnVals[0] = xC[0]*xC[0] - 0.5*xC[1];

  if (directFnASV[0] & 2) {
    fnGrads[0][0] = 2.0*xC[0];
    fnGrads[0][1] = -0.5;
  }

  if (directFnASV[0] & 4)
    fnHessians[0](0,0) = 2.0;

  return 0;
}

/// modified lo-fi Rosenbrock to test SBO with hierarchical approximations
int TestDriverInterface::poly_prod()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: poly_prod direct fn does not yet support multiprocessor "
      << "analyses." << std::endl;
    abort_handler(-1);
  }

  if ( (gradFlag || hessFlag) && (numADIV || numADRV) ) {
    Cerr << "Error: poly_prod direct fn assumes no discrete variables in "
	 << "derivative or hessian mode." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numACV != 2) {
    Cerr << "Error: Bad number of variables in poly_prod direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numFns != 1 ) {
    Cerr << "Error: Bad number of functions in poly_prod direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Compute and output responses
  double x0_sq = xC[0]*xC[0], x1_sq = xC[1]*xC[1];
  double t2 = x0_sq - 0.5*xC[1];
  double t1 = xC[0] + x1_sq/2.;

  if (directFnASV[0] & 1)
    fnVals[0] = t1*t2;

  if (directFnASV[0] & 2) {
    fnGrads[0][0] = 2.0*xC[0]*t1 + t2;
    fnGrads[0][1] = t2*xC[1] - t1/2.0;
  }

  if (directFnASV[0] & 4) {
    fnHessians[0](0,0) = 4.0*xC[0] + 2.*t1;
    fnHessians[0](1,1) = t2 - xC[1];
    fnHessians[0](1,0) = 2.*xC[0]*xC[1] - 0.5;
  }
  return 0;
}


int TestDriverInterface::gerstner()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: gerstner direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numADIV || numADRV || (gradFlag && numDerivVars != 2)) {
    Cerr << "Error: Bad number of variables in gerstner direct fn."<< std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in gerstner direct fn."<< std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in gerstner direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  const Real& x = xC[0]; const Real& y = xC[1];
  String an_comp = (!analysisComponents.empty() &&
		    !analysisComponents[analysisDriverIndex].empty()) ?
    analysisComponents[analysisDriverIndex][0] : "iso1";
  short test_fn; Real x_coeff, y_coeff, xy_coeff;
  if (an_comp        == "iso1")
    { test_fn = 1; x_coeff = y_coeff = 10.; }
  else if (an_comp   == "iso2")
    { test_fn = 2; x_coeff = y_coeff = xy_coeff = 1.; }
  else if (an_comp   == "iso3")
    { test_fn = 3; x_coeff = y_coeff = 10.; }
  else if (an_comp == "aniso1")
    { test_fn = 1; x_coeff =  1.; y_coeff = 10.; }
  else if (an_comp == "aniso2")
    { test_fn = 2; x_coeff =  1.; y_coeff = xy_coeff = 10.; }
  else if (an_comp == "aniso3")
    { test_fn = 3; x_coeff = 10.; y_coeff = 5.; }
  else {
    Cerr << "Error: analysis component specification required in gerstner "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // **** f:
  if (directFnASV[0] & 1) {
    switch (test_fn) {
    case 1:
      fnVals[0] = x_coeff*std::exp(-x*x) + y_coeff*std::exp(-y*y); break;
    case 2:
      fnVals[0]	=  x_coeff*std::exp(x) + y_coeff*std::exp(y)
	        + xy_coeff*std::exp(x*y);                          break;
    case 3:
      fnVals[0] = std::exp(-x_coeff*x*x - y_coeff*y*y);            break;
    }
  }

  // **** df/dx:
  if (directFnASV[0] & 2) {
    Real val;
    switch (test_fn) {
    case 1:
      fnGrads[0][0] = -2.*x*x_coeff*std::exp(-x*x);
      fnGrads[0][1] = -2.*y*y_coeff*std::exp(-y*y); break;
    case 2:
      val = xy_coeff*std::exp(x*y);
      fnGrads[0][0] = x_coeff*std::exp(x) + val*y;
      fnGrads[0][1] = y_coeff*std::exp(y) + val*x;  break;
    case 3:
      val = std::exp(-x_coeff*x*x - y_coeff*y*y);
      fnGrads[0][0] = -2.*x*x_coeff*val;
      fnGrads[0][1] = -2.*y*y_coeff*val;            break;
    }
  }

  return 0; // no failure
}

int TestDriverInterface::scalable_gerstner()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: scalable_gerstner direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: Bad variable types in scalable_gerstner direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in scalable_gerstner direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in scalable_gerstner direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  String an_comp = (!analysisComponents.empty() &&
		    !analysisComponents[analysisDriverIndex].empty()) ?
    analysisComponents[analysisDriverIndex][0] : "iso1";
  short test_fn; Real even_coeff, odd_coeff, inter_coeff;
  if (an_comp        == "iso1")
    { test_fn = 1; even_coeff = odd_coeff = 10.; }
  else if (an_comp   == "iso2")
    { test_fn = 2; even_coeff = odd_coeff = inter_coeff = 1.; }
  else if (an_comp   == "iso3")
    { test_fn = 3; even_coeff = odd_coeff = 10.; }
  else if (an_comp == "aniso1")
    { test_fn = 1; even_coeff =  1.; odd_coeff = 10.; }
  else if (an_comp == "aniso2")
    { test_fn = 2; even_coeff =  1.; odd_coeff = inter_coeff = 10.; }
  else if (an_comp == "aniso3")
    { test_fn = 3; even_coeff = 10.; odd_coeff = 5.; }
  else {
    Cerr << "Error: analysis component specification required in gerstner "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // **** f:
  if (directFnASV[0] & 1) {
    switch (test_fn) {
    case 1:
      fnVals[0] = 0.;
      for (size_t i=0; i<numVars; ++i)
	fnVals[0] += (i%2) ? odd_coeff*std::exp(-xC[i]*xC[i]) :
	                    even_coeff*std::exp(-xC[i]*xC[i]); break;
    case 2:
      fnVals[0] = 0.;
      for (size_t i=0; i<numVars; ++i)
	if (i%2)
	  fnVals[0] +=  odd_coeff*std::exp(xC[i])
	    + inter_coeff*std::exp(xC[i]*xC[i-1]);
	else
	  fnVals[0] += even_coeff*std::exp(xC[i]);
      break;
    case 3: {
      Real sum = 0;
      for (size_t i=0; i<numVars; ++i)
	sum -= (i%2) ? odd_coeff*xC[i]*xC[i] : even_coeff*xC[i]*xC[i];
      fnVals[0] = std::exp(sum); break;
    }
    }
  }

  // **** df/dx:
  if (directFnASV[0] & 2) {
    Real val;
    switch (test_fn) {
    case 1:
      for (size_t i=0; i<numVars; ++i)
	fnGrads[0][i] = (i%2) ? -2.*xC[i]* odd_coeff*std::exp(-xC[i]*xC[i])
	                      : -2.*xC[i]*even_coeff*std::exp(-xC[i]*xC[i]);
      break;
    case 2:
      for (size_t i=0; i<numVars; ++i)
	{
	  if (i%2)
	    fnGrads[0][i] = odd_coeff*std::exp(xC[i])
	      + inter_coeff*xC[i-1]*std::exp(xC[i]*xC[i-1]);
	  else
	    {
	      fnGrads[0][i] = even_coeff*std::exp(xC[i]);
	      if ( i+1 < numVars )
		fnGrads[0][i] += inter_coeff*xC[i+1]*std::exp(xC[i]*xC[i+1]);
	    }
	}
      break;
    case 3:
      if (directFnASV[0] & 1)
	val = fnVals[0];
      else {
	Real sum = 0;
	for (size_t i=0; i<numVars; ++i)
	  sum -= (i%2) ? odd_coeff*xC[i]*xC[i] : even_coeff*xC[i]*xC[i];
	val = std::exp(sum);
      }
      for (size_t i=0; i<numVars; ++i)
	fnGrads[0][i] = (i%2) ? -2.*xC[i]* odd_coeff*val
	                      : -2.*xC[i]*even_coeff*val;
      break;
    }
  }

  return 0; // no failure
}


void TestDriverInterface::
get_genz_coefficients( int num_dims, Real factor, int c_type,
		       RealVector &c, RealVector &w )
{
  c.resize( num_dims );
  w.resize( num_dims );
  switch ( c_type )
    {
    case 0:
      {
	Real csum = 0.0;
	for ( int d = 0; d < num_dims; d++ )
	  {
	    w[d] = 0.0;
	    c[d] = ( (Real)d + 0.5 ) / (Real)num_dims;
	    csum += c[d];
	  }
	for ( int d = 0; d < num_dims; d++ )
	  {
	    c[d] *= ( factor / csum );
	  }
	break;
      }
    case 1:
      {
	Real csum = 0.0;
	for ( int d = 0; d < num_dims; d++ )
	  {
	    w[d] = 0.0;
	    c[d] = 1.0 / (Real)( ( d + 1 ) * ( d + 1 ) );
	    csum += c[d];
	  }
	for ( int d = 0; d < num_dims; d++ )
	  {
	    c[d] *= ( factor / csum );
	  }
	break;
      }
    case 2:
      {
	Real csum = 0.0;
	for ( int d = 0; d < num_dims; d++ )
	  {
	    w[d] = 0.;
	    c[d] = std::exp( (Real)(d+1)*std::log( 1.e-8 ) / (Real)num_dims );
	    csum += c[d];
	  }
	for ( int d = 0; d < num_dims; d++ )
	  {
	    c[d] *= ( factor / csum );
	  }
	break;
      }
    default:
      throw(std::runtime_error("GetCoefficients() ensure type in [0,1]"));
    }
}


/** \brief Solve the 1D diffusion equation with an uncertain variable 
 * coefficient using the spectral Chebyshev collocation method.
 *
 * del(k del(u) ) = f on [0,1] subject to u(0) = 0 u(1) = 0
 * 
 * Here we set f = -1 and 
 * k = 1+4.*sum_d [cos(2*pi*x)/(pi*d)^2*z[d]] d=1,...,num_dims
 * where z_d are random variables, typically i.i.d uniform[-1,1]
 */
int TestDriverInterface::steady_state_diffusion_1d()
{
  // ------------------------------------------------------------- //
  // Pre-processing 
  // ------------------------------------------------------------- //

  if (multiProcAnalysisFlag) {
    Cerr << "Error: steady_state_diffusion_1d direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if ( ( numVars < 1 )  || ( numADIV > 1 ) ) {
    Cerr << "Error: Bad variable types in steady_state_diffusion_1d direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns < 1) {
    Cerr << "Error: Bad number of functions in steady_state_diffusion_1d "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag||gradFlag) {
    Cerr << "Error: Gradients and Hessians are not supported in " 
	 << "steady_state_diffusion_1d direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // ------------------------------------------------------------- //
  // Read parameters from discrete state variables 
  // ------------------------------------------------------------- //

  // Get the mesh resolution from the first discrete integer variable
  size_t mesh_size_index = find_index(xDILabels, "mesh_size");
  int order = ( mesh_size_index == _NPOS ) ? 20 : xDI[mesh_size_index];

  if (order % 2 != 0) {
    Cerr << "Error: Mesh size must be even." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Get the kernel specification from the discrete string variables
  size_t kernel_index = find_index(xDSLabels, "kernel_type");
  String kernel = ( kernel_index == _NPOS ) ? "default" : xDS[kernel_index];

  if (order + 1 < xC.length() && kernel == "exponential") {
    Cerr << "Error: Mesh size must be greater than or equal "
         << "to the number of random variables + 1 when using "
         << "the exponential kernel." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Get the positivity flag from the discrete string variables
  size_t pos_index = find_index(xDSLabels, "positivity");
  String pos_string = ( pos_index == _NPOS ) ? "on" : xDS[pos_index];
  bool positivity = ( pos_string == "on" );

  // Get the field mean from the discrete real variables
  size_t mean_index = find_index(xDRLabels, "field_mean");
  Real field_mean = ( mean_index == _NPOS ) ? 1.0 : xDR[mean_index];

  // Get the field mean from the discrete real variables
  size_t std_dev_index = find_index(xDRLabels, "field_std_dev");
  Real field_std_dev = ( std_dev_index == _NPOS ) ? 1.0 : xDR[std_dev_index];

  // Get the kernel order from the discrete real variables
  size_t kern_ord_index = find_index(xDRLabels, "kernel_order");
  Real kernel_order = ( kern_ord_index == _NPOS ) ? 1.0 : xDR[kern_ord_index];

  // Get the kernel length from the discrete real variables
  size_t kern_len_index = find_index(xDRLabels, "kernel_length");
  Real kernel_length = ( kern_len_index == _NPOS ) ? 1.0 : xDR[kern_len_index];

  // Initialize domain and boundary conditions:
  RealVector bndry_conds(2), domain_limits(2); // initialize to zero
  domain_limits[1] = 1.;

  // Compute default QoI coordinates:
  RealVector qoi_coords( numFns, false );
  if (numFns > 1) {
    Real range = domain_limits[1]-domain_limits[0];
    Real h = (range*0.9) / (Real)(numFns-1);
    for (int i=0; i<numFns; i++)
      qoi_coords[i] = domain_limits[0]+range*0.05+(Real)i*h;
  }
  else
    qoi_coords[0] = (domain_limits[1]+domain_limits[0])/2.;

  // If QoI coordinates provided through discrete real variables, overwrite
  // defaults:
  for (int i=0; i<numFns; i++) {
    size_t coord_index = find_index(xDRLabels, "coord_" + boost::lexical_cast<String>(i));
    if ( coord_index != _NPOS )
      qoi_coords[i] = xDR[coord_index];
  }

  // ------------------------------------------------------------- //
  // Initialize and evaluate model 
  // ------------------------------------------------------------- //

  SpectralDiffusionModel model;
  model.set_num_qoi( numFns );
  model.set_qoi_coords( qoi_coords );
  model.set_field_mean( field_mean );
  model.set_field_std_dev( field_std_dev );

  // These are ignored if kernel is not exponential
  model.set_positivity( positivity );
  model.set_kernel_order( kernel_order );
  model.set_kernel_length( kernel_length );

  model.initialize( order, kernel, bndry_conds, domain_limits );
  
  model.evaluate( xC, fnVals ); 

  return  0;
}


int TestDriverInterface::transient_diffusion_1d()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: transient_diffusion_1d direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 7 || numADIV > 2) {
    Cerr << "Error: unsupported variable counts in transient_diffusion_1d "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 1) {
    Cerr << "Error: unsupported function counts in transient_diffusion_1d "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag || gradFlag) {
    Cerr << "Error: gradients and Hessians are not supported in " 
	 << "transient_diffusion_1d direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  // Get the number of spatial discretization points and the number of
  // Fourier solution modes from the discrete integer variables
  size_t nx_index = find_index(xDILabels, "N_x"),
         nm_index = find_index(xDILabels, "N_mod");
  int N_x   = ( nx_index == _NPOS ) ? 200 : xDI[nx_index];
  int N_mod = ( nm_index == _NPOS ) ?  21 : xDI[nm_index];

  RealVector x(N_x+1, false), u_n(N_x+1, false), f_tilde(N_x+1, false),
             A(N_x, false);
  Real Pi = 4. * std::atan(1.);
  
  // scaling from [-1,1] to [xi_min, xi_max]
  // [-1,1] -> [l,u]: xi = l + (u-l) * (xC+1) / 2
  RealVector xi(numVars);
  xi[0] = Pi*xC[0]; xi[1] = Pi*xC[1]; xi[2] = Pi*xC[2]; // [-pi,pi]
  xi[3] = 0.005 + 0.004*xC[3]; // [.001, .009]
  xi[4] = (xC[4]+1.)/2.; xi[5] = (xC[5]+1.)/2.; xi[6] = (xC[6]+1.)/2.; // [0,1]

  // spatial discretization
  size_t i, n;
  Real dx = 1. / N_x;
  x[0] = 0.;
  for (i=1; i<=N_x; ++i)
    x[i] = x[i-1] + dx;

  Real gamma = 1., T_f = 0.5, sum_A, int_u_n = 0.,
    f_t_1, f_t_2, x_i, xpi, npi_g, u_exp, xi2_sq = xi[2]*xi[2],
    sin_xi0 = std::sin(xi[0]), sin_xi1 = std::sin(xi[1]),
    i_fn = 3.5*(sin_xi0 + 7.*sin_xi1*sin_xi1 + xi2_sq*xi2_sq*sin_xi0/10.);
  Real g_fn = 50., a_i = -0.5;
  for (i=4; i<=6; ++i)
    g_fn *= (std::abs(4.*xi[i] - 2.) + a_i) / (1.+a_i);

  f_tilde[0] = 0.;
  for (n=0; n<N_mod; ++n) {
    sum_A = 0.; u_n[0] = 0.; npi_g = n * Pi / gamma;
    u_exp = std::exp(-xi[3] * npi_g * npi_g * T_f);
    for (i=1; i<=N_x; ++i) {
      x_i = x[i]; xpi = Pi*x_i; f_t_1 = std::sin(xpi);
      f_t_2 = std::sin(2.*xpi) + std::sin(3.*xpi)
	    + 50.* ( std::sin(9.*xpi) + std::sin(21.*xpi) );
      // This term is the product of (a realization of) the initial solution
      // and the n_th modal term
      f_tilde[i] = std::sin(npi_g * x_i) * (g_fn * f_t_1 + i_fn * f_t_2);

      // n_th coeff of the modal expansion (integrated over a single interval)
      A[i-1] = (f_tilde[i] + f_tilde[i-1]) * dx / 2.; // area (trapezoidal rule)
      sum_A += A[i-1];

      u_n[i] = std::sin(npi_g * x_i) * u_exp;
    }

    // n_th coefficient of the modal expansion (...sum of the integrals)
    u_n.scale(2. * sum_A);
    for (i=1; i<=N_x; ++i)
      int_u_n += u_n[i-1] + u_n[i]; // ends counted once, interior counted twice
  }

  // QoI is the integral of the solution over the physical space
  fnVals[0] = int_u_n * dx / 2.; // trapezoidal rule

  return 0;
}

int TestDriverInterface::predator_prey()
{
  // ------------------------------------------------------------- //
  // Pre-processing 
  // ------------------------------------------------------------- //

  if (multiProcAnalysisFlag) {
    Cerr << "Error: predator_prey direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if ( numVars < 1 || numADIV > 1 || numADRV > 1 ) {
    Cerr << "Error: Bad variable types in predator_prey direct fn."<< std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 3) {
    Cerr << "Error: Bad number of functions in predator_prey direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag || gradFlag) {
    Cerr << "Error: Gradients and Hessians are not supported in " 
	 << "predator_prey direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // ------------------------------------------------------------- //
  // Read parameters from discrete state variables 
  // ------------------------------------------------------------- //

  // Get the mesh resolution from the first discrete integer variable
  size_t step_index = find_index(xDILabels, "time_steps");
  int num_tsteps = ( step_index == _NPOS ) ? 101 : xDI[step_index];
  if (num_tsteps % 2 != 1) {
    Cerr << "Error: Number of time steps must be odd" << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  size_t tf_index = find_index(xDRLabels, "final_time");
  Real final_time = ( tf_index == _NPOS ) ? 10. : xDR[tf_index];

  RealVector init_conditions(3);
  init_conditions[0] = 0.7; init_conditions[1] = 0.5;
  init_conditions[2] = 0.2; // these could be made random variables

  // ------------------------------------------------------------- //
  // Initialize and evaluate model 
  // ------------------------------------------------------------- //

  PredatorPreyModel model;
  model.set_initial_conditions( init_conditions );
  model.set_time(final_time, final_time/((double)num_tsteps-1.));
  
  model.evaluate( xC, fnVals ); 

  return  0;
}

int TestDriverInterface::genz()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: genz direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: Bad variable types in genz direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in genz direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in genz direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  String test;//, resp;
  if (analysisComponents.empty() ||
      analysisComponents[analysisDriverIndex].empty())
    { test = "os1"; /*resp = "generic";*/ }
  else {
    StringArray& an_comps = analysisComponents[analysisDriverIndex];
    test = an_comps[0];
    //if (an_comps.size() > 1) resp = an_comps[1];
  }

  int coeff_type, fn_type;
  Real factor;
  if (test == "os1")
    { coeff_type = 0; fn_type = 0; factor = 4.5; }
  else if (test == "os2")
    { coeff_type = 1; fn_type = 0; factor = 4.5; }
  else if (test == "os3")
    { coeff_type = 2; fn_type = 0; factor = 4.5; }
  else if (test == "cp1")
    { coeff_type = 0; fn_type = 1; factor = .25; }
  else if (test == "cp2")
    { coeff_type = 1; fn_type = 1; factor = .25; }
  else if (test == "cp3")
    { coeff_type = 2; fn_type = 1; factor = .25; }
  else {
    Cerr << "Error: analysis component specification required in genz "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  RealVector c, w;
  get_genz_coefficients( numVars, factor, coeff_type, c, w );
  Real pi = 4.0 * std::atan( 1.0 );

  // **** f:
  if (directFnASV[0] & 1) {
    switch (fn_type) {
    case 0:
      fnVals[0] = 2.0 * pi * w[0];
      for ( int d = 0; d < numVars; d++ )
	fnVals[0] += c[d] * xC[d];
      fnVals[0] = std::cos( fnVals[0] );
      break;
    case 1:
      fnVals[0] = 1.0;
      for ( int d = 0; d < numVars; d++ )
	fnVals[0] += c[d]* xC[d];
      Real expo = -(Real)(numVars+1);
      //if (resp == "residual") expo /= 2.;// reproduce Genz after sum res^2
      fnVals[0] = std::pow( fnVals[0], expo );
      break;
    }
  }

  return 0; // no failure
}


int TestDriverInterface::damped_oscillator()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: damped oscillator direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars < 1 || numVars > 6 || numADIV || numADRV) {
    Cerr << "Error: Bad variable types in damped oscillator direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns < 1) {
    Cerr << "Error: Bad number of functions in damped oscillator direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag || gradFlag) {
    Cerr << "Error: Gradients and Hessians not supported in damped oscillator "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  Real initial_time = 0., final_time = 20., //delta_t = 0.3;
    delta_t = (final_time - initial_time) / numFns;
  //int num_time_steps = numFns;

  Real pi = 4.0 * std::atan( 1.0 );

  Real b = xC[0], k = 0.035, F = 0.1, w = 1.0, x0 = 0.5, v0 = 0.;
  if ( numVars >= 2 ) k  = xC[1];
  if ( numVars >= 3 ) F  = xC[2];
  if ( numVars >= 4 ) w  = xC[3];
  if ( numVars >= 5 ) x0 = xC[4];
  if ( numVars >= 6 ) v0 = xC[5];

  Real kw = ( k-w*w ), bw = b*w, g = b / 2.;
  Real zeta2 = bw*bw + kw*kw, zeta = std::sqrt(zeta2);
  Real phi = std::atan( -bw / kw );
  Real sqrtk = std::sqrt( k );
  Real wd = sqrtk*std::sqrt( 1.-g*g / k );
  if ( kw / zeta2 < 0 ) phi += pi;
  Real B1 = -F*( bw ) / zeta2, B2 = F*kw / zeta2,
       A1 = x0-B1,             A2 = ( v0+g*A1-w*B2 ) / wd;

  if ( sqrtk <= g ) {
    Cerr << "Error: damped_oscillator parameters do not result in under-damped "
	 << "solution." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // always include response at final_time plus additional time steps;
  // response at initial time isn't included as it isn't a fn of some params.
  Real time = initial_time, y_stead, y_trans;
  for (size_t i=0; i<numFns; ++i) {
    time += delta_t;
    if (directFnASV[i] & 1) {
      // Steady state solution (y_stead) for rhs = 0
      y_stead = F * std::sin( w*time + phi ) / zeta;
      // Compute transient (y_trans) component of solution
      y_trans = std::exp( -g*time )*( A1 * std::cos( wd*time ) +
				      A2 * std::sin( wd*time ) );
      fnVals[i] = y_stead + y_trans;
    }
  }

  return 0; // no failure
}


int TestDriverInterface::log_ratio()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: log_ratio direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if ( numVars != 2 || numADIV || numADRV ||
       ( ( gradFlag || hessFlag ) && numDerivVars != 2 ) ) {
    Cerr << "Error: Bad number of variables in log_ratio direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in log_ratio direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  const Real& x1 = xC[0]; const Real& x2 = xC[1];

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = x1/x2;

  // **** df/dx:
  if (directFnASV[0] & 2) {
    fnGrads[0][0] = 1./x2;
    fnGrads[0][1] = -x1/(x2*x2);
  }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4) {
    fnHessians[0](0,0) = 0.0;
    fnHessians[0](0,1) = fnHessians[0](1,0) = -1./(x2*x2);
    fnHessians[0](1,1) = 2.*x1/std::pow(x2,3);
  }

  return 0; // no failure
}


int TestDriverInterface::short_column()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: short_column direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 5 || numADIV > 1 || numADRV) { // allow ModelForm discrete int
    Cerr << "Error: Bad number of variables in short_column direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  size_t ai, lsi;
  if (numFns == 1)      // option for limit state only
    lsi = 0;
  else if (numFns == 2) // option for area + limit state
    { ai = 0; lsi = 1; }
  else {
    Cerr << "Error: Bad number of functions in short_column direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // b = xC[0] = column base   (design var.)
  // h = xC[1] = column height (design var.)
  // P = xC[2] (normal uncertain var.)
  // M = xC[3] (normal uncertain var.)
  // Y = xC[4] (lognormal uncertain var.)
  Real b = xCM[VAR_b], h = xCM[VAR_h], P = xCM[VAR_P], M = xCM[VAR_M],
       Y = xCM[VAR_Y], b_sq = b*b, h_sq = h*h, P_sq = P*P, Y_sq = Y*Y;

  // **** f (objective = bh = cross sectional area):
  if (numFns > 1 && (directFnASV[ai] & 1))
    fnVals[ai] = b*h;

  // **** g (limit state = short column response):
  if (directFnASV[lsi] & 1)
    fnVals[lsi] = 1. - 4.*M/(b*h_sq*Y) - P_sq/(b_sq*h_sq*Y_sq);

  // **** df/dx (w.r.t. active variables):
  if (numFns > 1 && (directFnASV[ai] & 2))
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_b: // design variable derivative
	fnGrads[ai][i] = h;  break;
      case VAR_h: // design variable derivative
	fnGrads[ai][i] = b;  break;
      default: // uncertain variable derivative
	fnGrads[ai][i] = 0.; break;
      }

  // **** dg/dx (w.r.t. active variables):
  if (directFnASV[lsi] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_b: // design variable derivative
	fnGrads[lsi][i] = 4.*M/(b_sq*h_sq*Y) + 2.*P_sq/(b_sq*b*h_sq*Y_sq);break;
      case VAR_h: // design variable derivative
	fnGrads[lsi][i] = 8.*M/(b*h_sq*h*Y)  + 2.*P_sq/(b_sq*h_sq*h*Y_sq);break;
      case VAR_P: // uncertain variable derivative
	fnGrads[lsi][i] = -2.*P/(b_sq*h_sq*Y_sq);                         break;
      case VAR_M: // uncertain variable derivative
	fnGrads[lsi][i] = -4./(b*h_sq*Y);                                 break;
      case VAR_Y: // uncertain variable derivative
	fnGrads[lsi][i] = 4.*M/(b*h_sq*Y_sq) + 2.*P_sq/(b_sq*h_sq*Y_sq*Y);break;
      }

  // **** d^2f/dx^2: (SORM)
  if (numFns > 1 && (directFnASV[ai] & 4))
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	fnHessians[ai](i,j)
	  = ( (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_h) ||
	      (varTypeDVV[i] == VAR_h && varTypeDVV[j] == VAR_b) ) ? 1. : 0.;

  // **** d^2g/dx^2: (SORM)
  if (directFnASV[lsi] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_b)
	  fnHessians[lsi](i,j) = -8.*M/(b_sq*b*h_sq*Y)
	    - 6.*P_sq/(b_sq*b_sq*h_sq*Y_sq);
	else if ( (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_h) ||
		  (varTypeDVV[i] == VAR_h && varTypeDVV[j] == VAR_b) )
	  fnHessians[lsi](i,j) = -8.*M/(b_sq*h_sq*h*Y)
	    - 4.*P_sq/(b_sq*b*h_sq*h*Y_sq);
	else if (varTypeDVV[i] == VAR_h && varTypeDVV[j] == VAR_h)
	  fnHessians[lsi](i,j) = -24.*M/(b*h_sq*h_sq*Y)
	    - 6.*P_sq/(b_sq*h_sq*h_sq*Y_sq);
	else if (varTypeDVV[i] == VAR_P && varTypeDVV[j] == VAR_P)
	  fnHessians[lsi](i,j) = -2./(b_sq*h_sq*Y_sq);
	else if ( (varTypeDVV[i] == VAR_P && varTypeDVV[j] == VAR_M) ||
		  (varTypeDVV[i] == VAR_M && varTypeDVV[j] == VAR_P) )
	  fnHessians[lsi](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_P && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_P) )
	  fnHessians[lsi](i,j) = 4.*P/(b_sq*h_sq*Y_sq*Y);
	else if (varTypeDVV[i] == VAR_M && varTypeDVV[j] == VAR_M)
	  fnHessians[lsi](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_M && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_M) )
	  fnHessians[lsi](i,j) = 4./(b*h_sq*Y_sq);
	else if (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_Y)
	  fnHessians[lsi](i,j) = -8.*M/(b*h_sq*Y_sq*Y)
	    - 6.*P_sq/(b_sq*h_sq*Y_sq*Y_sq);
	else { // unsupported cross-derivative
	  Cerr << "Error: unsupported Hessian cross term in short_column."
	       << std::endl;
	  abort_handler(INTERFACE_ERROR);
	}

  return 0; // no failure
}


int TestDriverInterface::lf_short_column()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: lf_short_column direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 5 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in lf_short_column direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  short form = 2; // high fidelity case is form 1
  if (!analysisComponents.empty() &&
      !analysisComponents[analysisDriverIndex].empty()) {
    const String& an_comp = analysisComponents[analysisDriverIndex][0];
    if (an_comp      == "lf1") form = 2;
    else if (an_comp == "lf2") form = 3;
    else if (an_comp == "lf3") form = 4;
  }
  return alternate_short_column_forms(form);
}


int TestDriverInterface::mf_short_column()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: mf_short_column direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 5 || numADIV > 1 || numADRV) {
    Cerr << "Error: Bad number of variables in mf_short_column direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 2) {
    Cerr << "Error: Bad number of functions in mf_short_column direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  int form = xDIM[VAR_MForm];
  switch (form) {
  case 1:  return short_column();                     break;
  default: return alternate_short_column_forms(form); break;
  // monotonic mean: 3 1 2 4
  //case 1: return alternate_short_column_forms(3); break;
  //case 2: return short_column();                  break;
  //case 3: return alternate_short_column_forms(2); break;
  //case 4: return alternate_short_column_forms(4); break;
  // monotonic std dev: 2 1 4 3
  //case 1: return alternate_short_column_forms(2); break;
  //case 2: return short_column();                  break;
  //case 3: return alternate_short_column_forms(4); break;
  //case 4: return alternate_short_column_forms(3); break;
  }
}


int TestDriverInterface::alternate_short_column_forms(int form)
{
  size_t ai, lsi;
  if (numFns == 1)      // option for limit state only
    lsi = 0;
  else if (numFns == 2) // option for area + limit state
    { ai = 0; lsi = 1; }
  else {
    Cerr << "Error: Bad number of functions in alternate_short_column_forms "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // b = xC[0] = column base   (design var.)
  // h = xC[1] = column height (design var.)
  // P = xC[2] (normal uncertain var.)
  // M = xC[3] (normal uncertain var.)
  // Y = xC[4] (lognormal uncertain var.)
  Real b = xCM[VAR_b], h = xCM[VAR_h], P = xCM[VAR_P], M = xCM[VAR_M],
       Y = xCM[VAR_Y], b_sq = b*b, h_sq = h*h, P_sq = P*P, M_sq = M*M,
       Y_sq = Y*Y;

  // **** f (objective = bh = cross sectional area):
  if (numFns > 1 && (directFnASV[ai] & 1))
    fnVals[ai] = b*h;

  // **** g (limit state = short column response):
  if (directFnASV[lsi] & 1) {
    switch (form) {
    //case 1: short_column(); // original high fidelity case
    case 2:
      fnVals[lsi] = 1. - 4.*P/(b*h_sq*Y) - P_sq/(b_sq*h_sq*Y_sq); break;
    case 3:
      fnVals[lsi] = 1. - 4.*M/(b*h_sq*Y) - M_sq/(b_sq*h_sq*Y_sq); break;
    case 4:
      fnVals[lsi] = 1. - 4.*M/(b*h_sq*Y) - P_sq/(b_sq*h_sq*Y_sq)
	               - 4.*(P - M)/(b*h*Y);                      break;
    default: return 1;                                            break;
    }
  }

  return 0; // no failure
}


int TestDriverInterface::side_impact_cost()
{
  if (numVars != 7 || numFns != 1) {
    Cerr << "Error: wrong number of inputs/outputs in side_impact_cost."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 1.98 + 4.9*xC[0] + 6.67*xC[1] + 6.98*xC[2] + 4.01*xC[3]
              + 1.78*xC[4] + 2.73*xC[6];

  // **** df/dx:
  if (directFnASV[0] & 2) {
    Real* fn_grad = fnGrads[0];
    fn_grad[0] = 4.9;  fn_grad[1] = 6.67; fn_grad[2] = 6.98;
    fn_grad[3] = 4.01; fn_grad[4] = 1.78; fn_grad[5] = 0.;
    fn_grad[6] = 2.73;
  }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    fnHessians[0] = 0.;

  return 0; // no failure
}


int TestDriverInterface::side_impact_perf()
{
  if (numVars != 11 || numFns != 10) {
    Cerr << "Error: wrong number of inputs/outputs in side_impact_perf."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 1.16 - 0.3717*xC[1]*xC[3] - 0.00931*xC[1]*xC[9]
              - 0.484*xC[2]*xC[8] + 0.01343*xC[5]*xC[9];
  if (directFnASV[1] & 1)
    fnVals[1] = 28.98 + 3.818*xC[2] - 4.2*xC[0]*xC[1] + 0.0207*xC[4]*xC[9]
              + 6.63*xC[5]*xC[8] - 7.7*xC[6]*xC[7] + 0.32*xC[8]*xC[9];
  if (directFnASV[2] & 1)
    fnVals[2] = 33.86 + 2.95*xC[2] + 0.1792*xC[9] - 5.057*xC[0]*xC[1]
              - 11*xC[1]*xC[7] - 0.0215*xC[4]*xC[9] - 9.98*xC[6]*xC[7]
              + 22*xC[7]*xC[8];
  if (directFnASV[3] & 1)
    fnVals[3] = 46.36 - 9.9*xC[1] - 12.9*xC[0]*xC[7] + 0.1107*xC[2]*xC[9];
  if (directFnASV[4] & 1)
    fnVals[4] = 0.261 - 0.0159*xC[0]*xC[1] - 0.188*xC[0]*xC[7]
              - 0.019*xC[1]*xC[6] + 0.0144*xC[2]*xC[4] + 0.0008757*xC[4]*xC[9]
              + 0.08045*xC[5]*xC[8] + 0.00139*xC[7]*xC[10]
              + 0.00001575*xC[9]*xC[10];
  if (directFnASV[5] & 1)
    fnVals[5] = 0.214 + 0.00817*xC[4] - 0.131*xC[0]*xC[7] - 0.0704*xC[0]*xC[8]
              + 0.03099*xC[1]*xC[5] - 0.018*xC[1]*xC[6] + 0.0208*xC[2]*xC[7]
              + 0.121*xC[2]*xC[8] - 0.00364*xC[4]*xC[5] + 0.0007715*xC[4]*xC[9]
              - 0.0005354*xC[5]*xC[9] + 0.00121*xC[7]*xC[10];
  if (directFnASV[6] & 1)
    fnVals[6] = 0.74 - 0.61*xC[1] - 0.163*xC[2]*xC[7] + 0.001232*xC[2]*xC[9]
              - 0.166*xC[6]*xC[8] + 0.227*xC[1]*xC[1];
  if (directFnASV[7] & 1)
    fnVals[7] = 4.72 - 0.5*xC[3] - 0.19*xC[1]*xC[2] - 0.0122*xC[3]*xC[9]
              + 0.009325*xC[5]*xC[9] + 0.000191*xC[10]*xC[10];
  if (directFnASV[8] & 1)
    fnVals[8] = 10.58 - 0.674*xC[0]*xC[1] - 1.95*xC[1]*xC[7]
              + 0.02054*xC[2]*xC[9] - 0.0198*xC[3]*xC[9] + 0.028*xC[5]*xC[9];
  if (directFnASV[9] & 1)
    fnVals[9] = 16.45 - 0.489*xC[2]*xC[6] - 0.843*xC[4]*xC[5]
              + 0.0432*xC[8]*xC[9] - 0.0556*xC[8]*xC[10]
              - 0.000786*xC[10]*xC[10];

  // **** df/dx and d^2f/dx^2:
  bool grad_flag = false, hess_flag = false;
  for (size_t i=0; i<numFns; ++i) {
    if (directFnASV[i] & 2) grad_flag = true;
    if (directFnASV[i] & 4) hess_flag = true;
  }
  if (grad_flag)
    Cerr << "Error: gradients not currently supported in side_impact_perf()."
	 << std::endl;
  if (hess_flag)
    Cerr << "Error: Hessians not currently supported in side_impact_perf()."
	 << std::endl;
  if (grad_flag || hess_flag)
    abort_handler(INTERFACE_ERROR);

  return 0; // no failure
}


int TestDriverInterface::steel_column_cost()
{
  if (numVars != 3 || numFns != 1) {
    Cerr << "Error: wrong number of inputs/outputs in steel_column_cost."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // In the steel column description in Kuschel & Rackwitz, Cost is _not_
  // defined as a random variable.  That is Cost is not a fn(B, D, H), but
  // is rather defined as a fn(b, d, h).  Since dCost/dX|_{X=mean} is not the
  // same as dCost/dmean for non-normal X (jacobian_dX_dS is not 1), dCost/dX
  // may not be used and an optional interface must be defined for Cost.

  // b  = xC[0] = flange breadth   (lognormal unc. var., mean is design var.)
  // d  = xC[1] = flange thickness (lognormal unc. var., mean is design var.)
  // h  = xC[2] = profile height   (lognormal unc. var., mean is design var.)

  Real b = xCM[VAR_b], d = xCM[VAR_d], h = xCM[VAR_h];

  // **** f (objective = bd + 5h = cost of column):
  if (directFnASV[0] & 1)
    fnVals[0] = b*d + 5.*h;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_b:
	fnGrads[0][i] = d;  break;
      case VAR_d:
	fnGrads[0][i] = b;  break;
      case VAR_h:
	fnGrads[0][i] = 5.; break;
      }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	fnHessians[0](i,j)
	  = ( (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_d) ||
	      (varTypeDVV[i] == VAR_d && varTypeDVV[j] == VAR_b) ) ? 1. : 0.;

  return 0; // no failure
}


int TestDriverInterface::steel_column_perf()
{
  if (numVars != 9 || numFns != 1) {
    Cerr << "Error: wrong number of inputs/outputs in steel_column_perf."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // In the steel column description in Kuschel & Rackwitz, Cost is _not_
  // defined as a random variable.  That is Cost is not a fn(B, D, H), but
  // is rather defined as a fn(b, d, h).  Since dCost/dX|_{X=mean} is not the
  // same as dCost/dmean for non-normal X (jacobian_dX_dS is not 1), dCost/dX
  // may not be used and an optional interface must be defined for Cost.

  // set effective length s based on assumed boundary conditions
  // actual length of the column is 7500 mm
  Real s = 7500;

  // Fs = yield stress     (lognormal unc. var.)
  // P1 = dead weight load (normal    unc. var.)
  // P2 = variable load    (gumbel    unc. var.)
  // P3 = variable load    (gumbel    unc. var.)
  // B  = flange breadth   (lognormal unc. var., mean is design var.)
  // D  = flange thickness (lognormal unc. var., mean is design var.)
  // H  = profile height   (lognormal unc. var., mean is design var.)
  // F0 = init. deflection (normal    unc. var.)
  // E  = elastic modulus  (weibull   unc. var.)

  Real F0 = xCM[VAR_F0], B = xCM[VAR_B], D = xCM[VAR_D], H = xCM[VAR_H],
    Fs = xCM[VAR_Fs], E = xCM[VAR_E], P = xCM[VAR_P1]+xCM[VAR_P2]+xCM[VAR_P3],
    Pi = 3.1415926535897932385, Pi2 = Pi*Pi, Pi4 = Pi2*Pi2, Pi6 = Pi2*Pi4,
    B2 = B*B, D2 = D*D, H2 = H*H, H3 = H*H2, H5 = H2*H3, E2 = E*E, E3 = E*E2,
    s2 = s*s, X = Pi2*E*B*D*H2 - 2.*s2*P, X2 = X*X, X3 = X*X2;

  // **** g (limit state):
  if (directFnASV[0] & 1)
    fnVals[0] = Fs - P*(1./2./B/D + Pi2*F0*E*H/X);

  // **** dg/dx (w.r.t. active/uncertain variables):
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_F0: // df/dF0
	fnGrads[0][i] = -E*H*P*Pi2/X;                       break;
      case VAR_P1: case VAR_P2: case VAR_P3: // df/dP1, df/dP2, df/dP3
	fnGrads[0][i] = -1./2./B/D - B*D*E2*F0*H3*Pi4/X2;   break;
      case VAR_B: // df/dB
	fnGrads[0][i] = P*(1./2./B2/D + D*E2*F0*H3*Pi4/X2); break;
      case VAR_D: // df/dD
	fnGrads[0][i] = P*(1./2./B/D2 + B*E2*F0*H3*Pi4/X2); break;
      case VAR_H: // df/dH
	fnGrads[0][i] = E*F0*P*Pi2*(X + 4.*P*s2)/X2;        break;
      case VAR_Fs: // df/dFs
	fnGrads[0][i] = 1.;	                            break;
      case VAR_E: // df/dE
	fnGrads[0][i] = 2.*F0*H*P*P*Pi2*s2/X2;	            break;
      }

  // **** d^2g/dx^2: (SORM)
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_Fs || varTypeDVV[j] == VAR_Fs)
	  fnHessians[0](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_P1) ||
                  (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_P2) ||
                  (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_P3) ||
		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_P3) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = -4.*B*D*E2*F0*H3*Pi4*s2/X3;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_B) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_B) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_B) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j)
	    = 1./2./B2/D + D*E2*F0*H3*Pi4/X2*(2.*B*D*E*H2*Pi2/X - 1.);
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_D) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j)
	    = 1./2./B/D2 + B*E2*F0*H3*Pi4/X2*(2.*B*D*E*H2*Pi2/X - 1.);
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_H) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = B*D*E2*F0*H2*Pi4*(X+8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_F0) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_F0) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_F0) ||
		  (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = -B*D*E2*H3*Pi4/X2;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_E) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = 4.*B*D*E*F0*H3*P*Pi4*s2/X3;
	else if (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_B)
	  fnHessians[0](i,j) = -P*(1./B2/B/D + 2.*D2*E3*F0*H5*Pi6/X3);
	else if ( (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_B) )
	  fnHessians[0](i,j)
	    = -P*(1./2./B2/D2 + E2*F0*H3*Pi4/X2*(2.*B*D*E*H2*Pi2/X - 1.));
	else if ( (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_B) )
	  fnHessians[0](i,j) = -D*E2*F0*H2*P*Pi4*(X + 8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_B) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = D*E2*H3*P*Pi4/X2;
	else if ( (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_B) )
	  fnHessians[0](i,j) = -4.*D*E*F0*H3*P*P*Pi4*s2/X3;
	else if (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_D)
	  fnHessians[0](i,j) = -P*(1./B/D2/D + 2.*B2*E3*F0*H5*Pi6/X3);
	else if ( (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_D) )
	  fnHessians[0](i,j) = -B*E2*F0*H2*P*Pi4*(X + 8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = B*E2*H3*P*Pi4/X2;
	else if ( (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_D) )
	  fnHessians[0](i,j) = -4.*B*E*F0*H3*P*P*Pi4*s2/X3;
	else if (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_H)
	  fnHessians[0](i,j) = -2.*B*D*E2*F0*H*P*Pi4*(X + 8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = E*P*Pi2*(X + 4.*P*s2)/X2;
	else if ( (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_H) )
	  fnHessians[0](i,j) = -2.*F0*P*P*Pi2*s2*(3.*X + 8.*P*s2)/X3;
	else if (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_F0)
	  fnHessians[0](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = 2.*H*P*P*Pi2*s2/X2;
	else if (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_E)
	  fnHessians[0](i,j) = -4.*B*D*F0*H3*P*P*Pi4*s2/X3;
	else { // unsupported derivative
	  Cerr << "Error: unsupported Hessian cross term in steel_column."
	       << std::endl;
	  abort_handler(INTERFACE_ERROR);
	}

  return 0; // no failure
}


int TestDriverInterface::sobol_rational()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: sobol_rational direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numFns != 1) {
    Cerr << "Error: Bad number of inputs/outputs in sobol_rational direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // f = (x2 + 0.5)^4 / (x1 + 0.5)^2
  // See Storlie et al. SAND2008-6570

  const Real& x1 = xC[0]; const Real& x2 = xC[1];

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = std::pow((x2 + 0.5), 4.) / std::pow((x1 + 0.5), 2.);

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 1: // x1
	fnGrads[0][i] = -2.*std::pow(x2 + 0.5, 4.)/std::pow(x1 + 0.5, 3.);
	break;
      case 2: // x2
	fnGrads[0][i] =  4.*std::pow(x2 + 0.5, 3.)/std::pow(x1 + 0.5, 2.);
	break;
      }

  return 0; // no failure
}


int TestDriverInterface::sobol_g_function()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: sobol_g_function direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars < 1 || numVars > 10 || numFns != 1) {
    Cerr << "Error: Bad number of inputs/outputs in sobol_g_function direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Sobol g-Function: see Storlie et al. SAND2008-6570
  int a[] = {0,1,2,4,8,99,99,99,99,99};

  // **** f:
  if (directFnASV[0] & 1) {
    fnVals[0] = 2.;
    for (int i=0; i<numVars; ++i)
      fnVals[0] *= ( std::abs(4.*xC[i] - 2.) + a[i] ) / ( 1. + a[i] );
  }

  // **** df/dx:
  if (directFnASV[0] & 2) {
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      Real& fn_grad_0i = fnGrads[0][i];
      if (4.*xC[var_index] == 2.) // zero gradient assumed at discontinuity
	fn_grad_0i = 0.;
      else {
	fn_grad_0i = (4.*xC[var_index] > 2.) ?
	  8. / ( 1. + a[var_index] ) : -8. / ( 1. + a[var_index] );
	for (int j=0; j<numVars; ++j)
	  if (j != var_index)
	    fn_grad_0i *= ( std::abs(4.*xC[j] - 2.) + a[j] ) / ( 1. + a[j] );
      }
    }
  }

  return 0; // no failure
}


int TestDriverInterface::sobol_ishigami()
{
  using std::pow;
  using std::sin;
  using std::cos;

  if (multiProcAnalysisFlag) {
    Cerr << "Error: sobol_ishigami direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 3 || numFns != 1) {
    Cerr << "Error: Bad number of inputs/outputs in sobol_ishigami direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Ishigami Function: see Storlie et al. SAND2008-6570
  const Real pi = 3.14159265358979324;
  Real x1 = xCM[VAR_x1], x2 = xCM[VAR_x2], x3 = xCM[VAR_x3];

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = ( 1. + 0.1 * pow(2.*pi*x3 - pi, 4.0) ) *
      sin(2.*pi*x1 - pi) + 7. * pow(sin(2*pi*x2 - pi), 2.0);

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_x1:
	fnGrads[0][i]
	  = 2.*pi * ( 1. + 0.1*pow(2.*pi*x3 - pi, 4.) ) * cos(2.*pi*x1 - pi);
	break;
      case VAR_x2:
	fnGrads[0][i] = 28.*pi * sin(2.*pi*x2 - pi) * cos(2.*pi*x2 - pi);
	break;
      case VAR_x3:
	fnGrads[0][i] = 0.8 * pow(2.*pi*x3 - pi, 3.) * sin(2.*pi*x1 - pi);
	break;
      }

  return 0; // no failure
}


int TestDriverInterface::text_book()
{
  // This version is used when multiple analysis drivers are not employed.
  // In this case, execute text_book1/2/3 sequentially.

  if (numFns > 3) {
    Cerr << "Error: Bad number of functions in text_book direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  // The presence of discrete variables can cause offsets in directFnDVV which
  // the text_book derivative logic does not currently account for.
  if ( (gradFlag || hessFlag) && (numADIV || numADRV || numADSV) ) {
    Cerr << "Error: text_book direct fn assumes no discrete variables in "
	 << "derivative mode." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  text_book1(); // objective fn val/grad/Hessian
  if (numFns > 1)
    text_book2(); // constraint 1 val/grad/Hessian
  if (numFns > 2)
    text_book3(); // constraint 2 val/grad/Hessian

  // Test failure capturing for Direct case
  //int r = rand();
  //if (r < 10000) // RAND_MAX = 32767
  //  return 1; // failure

  //sleep(5); // for faking a more expensive evaluation
  return 0; // no failure
}


// text_book1/2/3 are used when evalComm is split into multiple analysis
// servers.  In this case, the 3 portions are executed in parallel.
int TestDriverInterface::text_book1()
{
  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  size_t i;
  if (directFnASV[0] & 1) {
    Real local_val = 0.0;
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is
      // fine in the direct case so long as everything is self-consistent.
      Real x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (Real)xDI[i-numACV];
      else if (i<numACV+numADIV+numADRV)
	x_i = xDR[i-numACV-numADIV];
      else
        x_i = levenshtein_distance(xDS[i-numACV-numADIV-numADRV]);
      local_val += std::pow(x_i-POW_VAL, 4);
#ifdef TB_EXPENSIVE
      // MDO98/WCSMO99 TFLOP/NOW timings: j<=15000 used for fnVals[0] only
      //   (text_book1 only)
      // StrOpt_2002 TFLOP timings: j<=5000 used for all fnVals, fnGrads, and
      //   fnHessians that are present (text_book1/2/3)
      for (size_t j=1; j<=5000; ++j)
        local_val += 1./(std::pow(x_i-POW_VAL,4)+j/100.)
	               /(std::pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[0] = global_val;
    }
    else
      fnVals[0] = local_val;
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    //for (i=0; i<numDerivVars; ++i)
    //  fnGrads[0][i] = 4.*pow(xC[i]-POW_VAL,3);
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      Real x_i = xC[var_index]; // assumes no discrete vars
      fnGrads[0][i] = 4.*std::pow(x_i-POW_VAL,3);
#ifdef TB_EXPENSIVE
      for (size_t j=1; j<=5000; ++j)
        fnGrads[0][i] += 1./(std::pow(x_i-POW_VAL,3)+j/100.)
                           /(std::pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [numDerivVars];
      parallelLib.reduce_sum_a((Real*)fnGrads[0], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	RealVector fn_grad_col_vec = Teuchos::getCol(Teuchos::View, fnGrads, 0);
	copy_data(sum_fns, (int)numDerivVars, fn_grad_col_vec);
	delete [] sum_fns;
      }
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    //for (i=0; i<numDerivVars; ++i)
    //  fnHessians[0][i][i] = 12.*pow(xC[i]-POW_VAL,2);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      Real x_i = xC[var_index]; // assumes no discrete vars
      fnHessians[0](i,i) = 12.*std::pow(x_i-POW_VAL,2);
#ifdef TB_EXPENSIVE
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=1; k<=5000; ++k)
	  fnHessians[0](i,j) += 1./(std::pow(x_i-POW_VAL,2)+k/100.)
                                   /(std::pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      int num_reals = numDerivVars * numDerivVars;
      Real* local_fns = new Real [num_reals];
      std::copy(fnHessians[0].values(), fnHessians[0].values() + num_reals,
                local_fns);
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [num_reals];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_reals);
      delete [] local_fns;
      if (analysisCommRank == 0) {
	std::copy(sum_fns, sum_fns + num_reals, fnHessians[0].values());
	delete [] sum_fns;
      }
    }
  }

  //sleep(1);
  return 0;
}


int TestDriverInterface::text_book2()
{
  // **********************************
  // **** c1: x[0]*x[0] - 0.5*x[1] ****
  // **********************************
  size_t i;
  if (directFnASV[1] & 1) {
    Real local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is
      // fine in the direct case so long as everything is self-consistent.
      Real x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (Real)xDI[i-numACV];
      else if (i<numACV+numADIV+numADRV)
	x_i = xDR[i-numACV-numADIV];
      else
        x_i = levenshtein_distance(xDS[i-numACV-numADIV-numADRV]);
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val += x_i*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val -= 0.5*x_i;
#ifdef TB_EXPENSIVE
      for (size_t j=1; j<=5000; ++j)
        local_val += 1./(std::pow(x_i-POW_VAL,4)+j/100.)
	               /(std::pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[1] = global_val;
    }
    else
      fnVals[1] = local_val;
  }

  // *****************
  // **** dc1/dx: ****
  // *****************
  if (directFnASV[1] & 2) {
    std::fill_n(fnGrads[1], fnGrads.numRows(), 0.);

    //fnGrads[1][0] = 2.*xC[0];
    //fnGrads[1][1] = -0.5;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fnGrads[1][i] = 2.*xC[0];
      else if (var_index == 1)
        fnGrads[1][i] = -0.5;
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=1; j<=5000; ++j)
        fnGrads[1][i] += 1./(std::pow(x_i-POW_VAL,3)+j/100.)
                           /(std::pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [numDerivVars];
      parallelLib.reduce_sum_a((Real*)fnGrads[1], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	RealVector fn_grad_col_vec = Teuchos::getCol(Teuchos::View, fnGrads, 1);
	copy_data(sum_fns, (int)numDerivVars, fn_grad_col_vec);
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c1/dx^2: ****
  // *********************
  if (directFnASV[1] & 4) {
    fnHessians[1] = 0.;
    //fnHessians[1][0][0] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
	fnHessians[1](i,i) = 2.;
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=1; k<=5000; ++k)
	  fnHessians[1](i,j) += 1./(std::pow(x_i-POW_VAL,2)+k/100.)
                                   /(std::pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      int num_reals = numDerivVars * numDerivVars;
      Real* local_fns = new Real [num_reals];
      std::copy(fnHessians[1].values(), fnHessians[1].values() + num_reals,
                local_fns);
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [num_reals];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_reals);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy( sum_fns, sum_fns + num_reals, fnHessians[1].values() );
	delete [] sum_fns;
      }
    }
  }

  //sleep(1);
  return 0;
}


int TestDriverInterface::text_book3()
{
  // **********************************
  // **** c2: x[1]*x[1] - 0.5*x[0] ****
  // **********************************
  size_t i;
  if (directFnASV[2] & 1) {
    Real local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is
      // fine in the direct case so long as everything is self-consistent.
      Real x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (Real)xDI[i-numACV];
      else if (i<numACV+numADIV+numADRV)
	x_i = xDR[i-numACV-numADIV];
      else
        x_i = levenshtein_distance(xDS[i-numACV-numADIV-numADRV]);
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val -= 0.5*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val += x_i*x_i;
#ifdef TB_EXPENSIVE
      for (size_t j=1; j<=5000; ++j)
        local_val += 1./(std::pow(x_i-POW_VAL,4)+j/100.)
	               /(std::pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[2] = global_val;
    }
    else
      fnVals[2] = local_val;
  }

  // *****************
  // **** dc2/dx: ****
  // *****************
  if (directFnASV[2] & 2) {
    std::fill_n(fnGrads[2], fnGrads.numRows(), 0.);

    //fnGrads[2][0] = -0.5;
    //fnGrads[2][1] = 2.*xC[1];
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fnGrads[2][i] = -0.5;
      else if (var_index == 1)
        fnGrads[2][i] = 2.*xC[1];
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=1; j<=5000; ++j)
        fnGrads[2][i] += 1./(std::pow(x_i-POW_VAL,3)+j/100.)
                           /(std::pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [numDerivVars];
      parallelLib.reduce_sum_a((Real*)fnGrads[2], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	RealVector fn_grad_col_vec = Teuchos::getCol(Teuchos::View, fnGrads, 2);
	copy_data(sum_fns, (int)numDerivVars, fn_grad_col_vec);
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c2/dx^2: ****
  // *********************
  if (directFnASV[2] & 4) {
    fnHessians[2] = 0.;
    //fnHessians[2][1][1] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 1)
	fnHessians[2](i,i) = 2.;
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=1; k<=5000; ++k)
	  fnHessians[2](i,j) += 1./(std::pow(x_i-POW_VAL,2)+k/100.)
                                   /(std::pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      int num_reals = numDerivVars * numDerivVars;
      Real* local_fns = new Real [num_reals];
      std::copy(fnHessians[2].values(), fnHessians[2].values() + num_reals,
                local_fns);
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [num_reals];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_reals);
      delete [] local_fns;
      if (analysisCommRank == 0) {
	std::copy(sum_fns, sum_fns + num_reals, fnHessians[2].values());
	delete [] sum_fns;
      }
    }
  }

  //sleep(1);
  return 0;
}


int TestDriverInterface::text_book_ouu()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: text_book_ouu direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  // typical usage is 2 design vars + 6 uncertain variables, although the
  // number of uncertain variables can be any factor of two.
  if (numVars < 4 || numVars % 2 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in text_book_ouu direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns > 3) {
    Cerr << "Error: Bad number of functions in text_book_ouu direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in text_book_ouu direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  size_t i, split = 2 + (numVars - 2)/2; // split the uncertain vars among d1,d2
  // xC[0], xC[1]     = design
  // xC[2], xC[3],... = uncertain

  // **** f:
  if (directFnASV[0] & 1) {
    Real f = 0.;
    for(i=2; i<split; ++i)
      f += std::pow(xC[i]-10.*xC[0], 4.0);
    for(i=split; i<numVars; ++i)
      f += std::pow(xC[i]-10.*xC[1], 4.0);
    fnVals[0] = f;
  }

  // **** c1:
  if (numFns>1 && (directFnASV[1] & 1))
    fnVals[1] = xC[0]*(xC[2]*xC[2] - 0.5*xC[3]);

  // **** c2:
  if (numFns>2 && (directFnASV[2] & 1))
    fnVals[2] = xC[1]*(xC[3]*xC[3] - 0.5*xC[2]);

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 1: { // design variable derivative
	Real f0 = 0., xC0 = xC[0];
	for (size_t j=2; j<split; ++j)
	  f0 += -40.*std::pow(xC[j]-10.*xC0, 3.0);
	fnGrads[0][i] = f0;
	break;
      }
      case 2: { // design variable derivative
	Real f1 = 0., xC1 = xC[1];
	for (size_t j=split; j<numVars; ++j)
	  f1 += -40.*std::pow(xC[j]-10.*xC1, 3.0);
	fnGrads[0][i] = f1;
	break;
      }
      default: { // uncertain variable derivative
	size_t var_index = directFnDVV[i] - 1;
	Real xCvi = xC[var_index];
	fnGrads[0][i] = (var_index<split) ? 4.*std::pow(xCvi-10.*xC[0], 3)
                                          : 4.*std::pow(xCvi-10.*xC[1], 3);
	break;
      }
      }

  // **** dc1/dx:
  if (numFns>1 && (directFnASV[1] & 2))
    for (i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 1: // design variable derivative
	fnGrads[1][i] = xC[2]*xC[2] - 0.5*xC[3]; break;
      case 3: // uncertain variable derivative
	fnGrads[1][i] = 2*xC[0]*xC[2];           break;
      case 4: // uncertain variable derivative
	fnGrads[1][i] = -0.5*xC[0];              break;
      default: // all other derivatives
	fnGrads[1][i] = 0.;                      break;
      }

  // **** dc2/dx:
  if (numFns>2 && (directFnASV[2] & 2))
    for (i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 2: // design variable derivative
	fnGrads[2][i] = xC[3]*xC[3] - 0.5*xC[2]; break;
      case 3: // uncertain variable derivative
	fnGrads[2][i] = -0.5*xC[1];              break;
      case 4: // uncertain variable derivative
	fnGrads[2][i] = 2*xC[1]*xC[3];           break;
      default: // all other derivative
	fnGrads[2][i] = 0.;                      break;
      }

  //sleep(5); // for faking a more expensive evaluation
  return 0; // no failure
}


int TestDriverInterface::scalable_text_book()
{
  if (numADIV || numADRV) {
    Cerr << "Error: scalable_text_book direct fn does not support discrete "
	 << "variables." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  size_t i, j;
  if (directFnASV[0] & 1) {
    fnVals[0] = 0.;
    for (i=0; i<numACV; ++i)
      fnVals[0] += std::pow(xC[i]-POW_VAL, 4);
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      fnGrads[0][i] = 4.*std::pow(xC[var_index]-POW_VAL,3);
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    for (i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      fnHessians[0](i,i) = 12.*std::pow(xC[var_index]-POW_VAL,2);
    }
  }

  // *********************
  // **** constraints ****
  // *********************
  // "symmetric" constraint pairs are defined from pairs of variables
  // (although odd constraint or variable counts are also allowable):
  // for i=1:num_fns-1, c[i] = x[i-1]^2 - x[i]/2    for  odd i
  //                    c[i] = x[i-1]^2 - x[i-2]/2  for even i
  for (i=1; i<numFns; ++i) {
    // ************
    // **** c: ****
    // ************
    if (directFnASV[i] & 1) {
      fnVals[i] = (i-1 < numACV) ? xC[i-1]*xC[i-1] : 0.;
      if (i%2) //  odd constraint
	{ if (i   < numACV) fnVals[i] -= xC[i]/2.; }
      else     // even constraint
	{ if (i-2 < numACV) fnVals[i] -= xC[i-2]/2.; }
    }
    // ****************
    // **** dc/dx: ****
    // ****************
    if (directFnASV[i] & 2) {
      std::fill_n(fnGrads[i], fnGrads.numRows(), 0.);
      for (j=0; j<numDerivVars; ++j) {
	size_t var_index = directFnDVV[j] - 1;
	if (i-1 < numACV && var_index == i-1) // both constraints
	  fnGrads[i][j] = 2.*xC[i-1];
	else if (i%2)         //  odd constraint
	  { if (i   < numACV && var_index == i)   fnGrads[i][j] = -0.5; }
	else                  // even constraint
	  { if (i-2 < numACV && var_index == i-2) fnGrads[i][j] = -0.5; }
      }
    }
    // ********************
    // **** d^2c/dx^2: ****
    // ********************
    if (directFnASV[i] & 4) {
      fnHessians[i] = 0.;
      if (i-1 < numACV)
	for (j=0; j<numDerivVars; ++j)
	  if (directFnDVV[j] == i) // both constraints
	    fnHessians[i](j,j) = 2.;
    }
  }

  //sleep(5); // for faking a more expensive evaluation
  return 0; // no failure
}


int TestDriverInterface::scalable_monomials()
{
  if (numADIV || numADRV) {
    Cerr << "Error: scalable_monomials direct fn does not support discrete "
	 << "variables." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in scalable_monomials direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // get power of monomial from analysis components, if available (default to 1)
  int power = 1;
  if (!analysisComponents.empty() &&
      !analysisComponents[analysisDriverIndex].empty())
    power = std::atoi(analysisComponents[analysisDriverIndex][0].c_str());

  // ***************************
  // **** f: sum x[i]^power ****
  // ***************************
  if (directFnASV[0] & 1) {
    fnVals[0] = 0.;
    for (size_t i=0; i<numACV; ++i)
      fnVals[0] += std::pow(xC[i], power);
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      switch (power) {
      case 0:  fnGrads[0][i] = 0;                                      break;
      default: fnGrads[0][i] = power*std::pow(xC[var_index], power-1); break;
      }
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      switch (power) {
      case 0: case 1:
	fnHessians[0](i,i) = 0; break;
      default:
	fnHessians[0](i,i) = power*(power-1)*std::pow(xC[var_index], power-2);
	break;
      }
    }
  }

  return 0; // no failure
}





int TestDriverInterface::mogatest1()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: mogatest1 direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if ( (numACV + numADIV + numADRV) != 3) {
    Cerr << "Error: Bad number of variables in mogatest1 direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 2) {
    Cerr << "Error: Bad number of functions in mogatest1 direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // kernel shared with standlone driver in test/mogatest1.cpp
  double f0 = 0.0;
  double f1 = 0.0;
  for (size_t i=0; i<numVars; i++) {
    // orders all continuous vars followed by all discrete vars.  This is
    // fine in the direct case so long as everything is self-consistent.
    Real x_i;
    if (i<numACV)
      x_i = xC[i];
    else if (i<numACV+numADIV)
      x_i = (Real)xDI[i-numACV];
    else
      x_i = xDR[i-numACV-numADIV];

    f0 = pow(x_i-(1/sqrt(3.0)),2) + f0;
    f1 = pow(x_i+(1/sqrt(3.0)),2) + f1;
  }
  f0 = 1-exp(-1*f0);
  f1 = 1-exp(-1*f1);

  if (directFnASV[0] & 1)
    fnVals[0] = f0;
  if (directFnASV[1] & 1)
    fnVals[1] = f1;
  if (directFnASV[0] & 2 || directFnASV[1] & 2) {
    Cerr << "Error: Analytic gradients not supported in mogatest1."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (directFnASV[0] & 4 || directFnASV[1] & 4) {
    Cerr << "Error: Analytic Hessians not supported in mogatest1."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  return 0; // no failure
}


int TestDriverInterface::mogatest2()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: mogatest2 direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 2 || numADIV > 0 || numADRV > 0) {
    // TODO: allow discrete variables
    Cerr << "Error: Bad number of variables in mogatest2 direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 2) {
    Cerr << "Error: Bad number of functions in mogatest2 direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // kernel shared with standlone driver in test/mogatest2.cpp
  double f0=0;
  double f1=0;
  const double PI = 3.14159265358979;
  f0 = xC[0];
  f1 = sin(2*PI*4*xC[0]);
  f1 = f1*(-1*xC[0]/(1+10*xC[1]));
  f1 = f1+1-pow((xC[0]/(1+10*xC[1])),2);
  f1 = f1*(1+10*xC[1]);

  if (directFnASV[0] & 1)
    fnVals[0] = f0;
  if (directFnASV[1] & 1)
    fnVals[1] = f1;
  if (directFnASV[0] & 2 || directFnASV[1] & 2) {
    Cerr << "Error: Analytic gradients not supported in mogatest2."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (directFnASV[0] & 4 || directFnASV[1] & 4) {
    Cerr << "Error: Analytic Hessians not supported in mogatest2."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  return 0; // no failure
}

int TestDriverInterface::mogatest3()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: mogatest3 direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numACV != 2 || numADIV > 0 || numADRV > 0) {
    // TODO: allow discrete variables
    Cerr << "Error: Bad number of variables in mogatest3 direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 4) {
    Cerr << "Error: Bad number of functions in mogatest3 direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // kernel shared with standlone driver in test/mogatest3.cpp
  double f0=0;
  double f1=0;
  double g0=0;
  double g1=0;
  f0 = pow(xC[0]-2,2)+pow(xC[1]-1,2)+2;
  f1 = 9*xC[0]-pow(xC[1]-1,2);
  g0 = (xC[0]*xC[0])+(xC[1]*xC[1])-225;
  g1 = xC[0]-3*xC[1]+10;

  if (directFnASV[0] & 1)
    fnVals[0] = f0;
  if (directFnASV[1] & 1)
    fnVals[1] = f1;
  if (directFnASV[2] & 1)
    fnVals[2] = g0;
  if (directFnASV[3] & 1)
    fnVals[3] = g1;
  if (directFnASV[0] & 2 || directFnASV[1] & 2 ||
      directFnASV[2] & 2 || directFnASV[3] & 2) {
    Cerr << "Error: Analytic gradients not supported in mogatest3."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (directFnASV[0] & 4 || directFnASV[1] & 4||
      directFnASV[2] & 4 || directFnASV[3] & 4) {
    Cerr << "Error: Analytic Hessians not supported in mogatest3."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  return 0; // no failure
}

int TestDriverInterface::illumination()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: illumination direct fn does not yet support multiprocessor "
      << "analyses." << std::endl;
    abort_handler(-1);
  }

  if ( (gradFlag || hessFlag) && (numADIV || numADRV) ) {
    Cerr << "Error: illumination direct fn assumes no discrete variables in "
	 << "derivative or hessian mode." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  size_t const num_vars = numACV; // keep everything consistent with standalone

  if ( num_vars != 7) {
    Cerr << "Error: Bad number of variables in illumination direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numFns != 1 ) {
    Cerr << "Error: Bad number of functions in illumination direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // compute function and gradient values
  double A[11][7] ={
  { 0.347392, 0.205329, 0.191987, 0.077192, 0.004561, 0.024003, 0.000000},
  { 0.486058, 0.289069, 0.379202, 0.117711, 0.006667, 0.032256, 0.000000},
  { 0.752511, 0.611283, 2.417907, 0.701700, 0.473047, 0.285597, 0.319187},
  { 0.303582, 0.364620, 1.898185, 0.693173, 0.607718, 0.328582, 0.437394},
  { 0.540946, 0.411549, 1.696545, 0.391735, 0.177832, 0.110119, 0.083817},
  { 0.651840, 0.540687, 3.208793, 0.639020, 0.293811, 0.156842, 0.128499},
  { 0.098008, 0.245771, 0.742564, 0.807976, 0.929739, 0.435144, 0.669797},
  { 0.000000, 0.026963, 0.000000, 0.246606, 0.414657, 0.231777, 0.372202},
  { 0.285597, 0.320457, 0.851227, 0.584677, 0.616436, 0.341447, 0.477329},
  { 0.324622, 0.306394, 0.991904, 0.477744, 0.376266, 0.158288, 0.198745},
  { 0.000000, 0.050361, 0.000000, 0.212042, 0.434397, 0.286455, 0.462731} };

  // KRD found bugs in previous computation; this Hessian no longer used:
  double harray[7][7] ={
  { 1.929437, 1.572662, 6.294004, 1.852205, 1.222324, 0.692036, 0.768564},
  { 1.572662, 1.354287, 5.511537, 1.787932, 1.320048, 0.724301, 0.870382},
  { 6.294004, 5.511537, 25.064512, 7.358494, 5.133563, 2.791970, 3.257364},
  { 1.852205, 1.787932, 7.358494, 2.883178, 2.497491, 1.321922, 1.747230},
  { 1.222324, 1.320048, 5.133563, 2.497491, 2.457733, 1.295927, 1.816568},
  { 0.692036, 0.724301, 2.791970, 1.321922, 1.295927, 0.694642, 0.968982},
  { 0.768564, 0.870382, 3.257364, 1.747230, 1.816568, 0.968982, 1.385357} };


  // Calculation of grad(f) = df/dx_I and hess(f) = d^2f/(dx_I dx_J):
  //
  // U = sum_{i=0:10}{ ( 1 - sum_{j=0:6}{ A[i][j]*x[j] } )^2 }
  // dU/dx_I = sum_{i=0:10}{ 2.0*( 1 - sum_{j=0:6}{ A[i][j]*x[j] } )*-A[i][I] }
  // d^2U/(dx_I dx_J) = sum_{i=0:10){ 2.0 * A[i][I] * A[i][J] }
  //
  // f = sqrt(U)
  // df/dx_I = 0.5*(dU/dx_I)/sqrt(U)
  //         = 0.5*(dU/dx_I)/f
  // d^2f/(dx_I dx_J)
  //   = -0.25 * U^(-1.5) * dU/dx_I * dU/dx_J + 0.5/sqrt(U) * d2U/(dx_I dx_J)
  //   = ( -df/dx_I * df/dx_J  + 0.5 * d2U/(dx_I dx_J) ) / f
  //
  // so to (efficiently) compute grad(f) we precompute f
  // and to (efficiently) compute hess(f) we precompute f and grad(f)

  size_t Ider, Jder;
  double grad[7];
  for(Ider=0; Ider<num_vars; ++Ider)
    grad[Ider] = 0.0;

  // **** f: (f is required to calculate any derivative of f; perform always)
  double U = 0.0;
  for (size_t i=0; i<11; i++) {
    double dtmp = 0.0;
    for (size_t j =0; j<num_vars; j++)
      dtmp += A[i][j] * xC[j];
    dtmp = 1.0 - dtmp;
    U += dtmp*dtmp;

    // precompute grad(U) unconditionally as it might be needed (cheap)
    for(Ider=0; Ider<num_vars; ++Ider)
      grad[Ider] -= dtmp*2.0*A[i][Ider]; //this is grad(U)
  }
  double fx = sqrt(U);
  if (directFnASV[0] & 1)
    fnVals[0] = fx;

  // **** df/dx:
  if (directFnASV[0] & 6) { // gradient required for itself and to calcuate the Hessian
    for(Ider=0; Ider<num_vars; ++Ider)
      grad[Ider] *= (0.5/fx); // this is now updated to be grad(f)

    if (directFnASV[0] & 2) { // if ASV demands gradient, print it
      for (int Ider=0; Ider<num_vars; ++Ider)
	fnGrads[0][Ider] = grad[Ider];
    }
  }

  // **** the hessian ddf/dxIdxJ
  if (directFnASV[0] & 4) {
    for(Ider=0; Ider<num_vars; ++Ider) {
      for(Jder=Ider; Jder<num_vars; ++Jder) {
	// define triangular part of hess(U)
	for(size_t i=0; i<11; ++i)
	  fnHessians[0](Ider,Jder) += A[i][Ider]*A[i][Jder]; // this is 0.5*hess(U)

	fnHessians[0](Jder,Ider) = fnHessians[0](Ider,Jder) // this is hess(f)
	  = (fnHessians[0](Ider,Jder)- grad[Ider]*grad[Jder])/fx;
      }
    }
  }
  return 0;
}

/// barnes test for SBO perforamnce from Rodriguez, Perez, Renaud, et al.
int TestDriverInterface::barnes()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: barnes direct fn does not yet support multiprocessor "
      << "analyses." << std::endl;
    abort_handler(-1);
  }

  if ( hessFlag ) {
    Cerr << "Error: barnes direct fn does not yet support analytic Hessians."
      << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( gradFlag && (numADIV || numADRV) ) {
    Cerr << "Error: barnes direct fn assumes no discrete variables in "
	 << "derivative mode." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numACV != 2) {
    Cerr << "Error: Bad number of variables in barnes direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numFns != 4 ) {
    Cerr << "Error: Bad number of functions in barnes direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }


  // Verification test for SBO performance.
  // Taken from Rodriguez, Perez, Renaud, et al.
  // Constraints g >= 0.

  double a[] = { 75.196,   -3.8112,    0.12694,    -2.0567e-3,  1.0345e-5,
		 -6.8306,   0.030234, -1.28134e-3,  3.5256e-5, -2.266e-7,
		  0.25645, -3.4604e-3, 1.3514e-5, -28.106,     -5.2375e-6,
		 -6.3e-8,   7.0e-10,   3.4054e-4,  -1.6638e-6, -2.8673,
		  0.0005};
  double x1 = xC[0], x2 = xC[1], x1x2 = x1*x2, x2_sq = x2*x2, x1_sq = x1*x1;

  // **** f
  if (directFnASV[0] & 1) {
    double f = a[0] + a[1]*x1 + a[2]*x1_sq + a[3]*x1_sq*x1 + a[4]*x1_sq*x1_sq
      + a[5]*x2 + a[6]*x1x2 + a[7]*x1*x1x2 + a[8]*x1x2*x1_sq
      + a[9]*x2*x1_sq*x1_sq + a[10]*x2_sq + a[11]*x2*x2_sq + a[12]*x2_sq*x2_sq
      + a[13]/(x2+1.) + a[14]*x2_sq*x1_sq + a[15]*x1*x1_sq*x2_sq
      + a[16]*x1x2*x2_sq*x1_sq + a[17]*x1*x2_sq + a[18]*x1x2*x2_sq
      + a[19]*exp(a[20]*x1x2);
    fnVals[0] = f;
  }

  // **** g1
  if (directFnASV[1] & 1)
    fnVals[1] = x1x2/700. - 1.;

  // **** g2
  if (directFnASV[2] & 1)
    fnVals[2] = x2/5. - x1_sq/625.;

  // **** g3
  if (directFnASV[3] & 1)
    fnVals[3] = pow(x2/50. - 1., 2.) - x1/500. + 0.11;

  // **** df/dx
  if (directFnASV[0] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[0][i] = a[1] + 2.*a[2]*x1 + 3.*a[3]*x1_sq + 4.*a[4]*x1_sq*x1
	  + a[6]*x2 + 2.*a[7]*x1x2 + 3.*a[8]*x2*x1_sq + 4.*a[9]*x1x2*x1_sq
	  + 2.*a[14]*x2_sq*x1 + 3.*a[15]*x1_sq*x2_sq + 3.*a[16]*x2*x2_sq*x1_sq
	  + a[17]*x2_sq + a[18]*x2*x2_sq + a[19]*a[20]*x2*exp(a[20]*x1x2);
	break;
      case 1:
	fnGrads[0][i] = a[5] + a[6]*x1 + a[7]*x1_sq + a[8]*x1*x1_sq
	  + a[9]*x1_sq*x1_sq + 2.*a[10]*x2 + 3.*a[11]*x2_sq + 4.*a[12]*x2*x2_sq
	  - a[13]/pow(x2+1., 2.) + 2.*a[14]*x2*x1_sq + 2.*a[15]*x1*x1_sq*x2
	  + 3.*a[16]*x1*x2_sq*x1_sq + 2.*a[17]*x1x2 + 3.*a[18]*x1*x2_sq
	  + a[19]*a[20]*x1*exp(a[20]*x1x2);
	break;
      }
    }
  }

  // **** dg1/dx
  if (directFnASV[1] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[1][i] = x2/700.;
	break;
      case 1:
	fnGrads[1][i] = x1/700.;
	break;
      }
    }
  }

  // **** dg2/dx
  if (directFnASV[2] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[2][i] = -2.*x1/625.;
	break;
      case 1:
	fnGrads[2][i] = 0.2;
	break;
      }
    }
  }

  // **** dg3/dx
  if (directFnASV[3] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[3][i] = -1./500.;
	break;
      case 1:
	fnGrads[3][i] = 2.*(x2/50. - 1.)/50.;
	break;
      }
    }
  }
  return 0;
}

/// lo-fi barnes test for SBO perforamnce from Rodriguez, Perez, Renaud, et al.
int TestDriverInterface::barnes_lf()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: barnes_lf direct fn does not yet support multiprocessor "
      << "analyses." << std::endl;
    abort_handler(-1);
  }

  if ( hessFlag ) {
    Cerr << "Error: barnes_lf direct fn does not yet support analytic Hessians."
      << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( gradFlag && (numADIV || numADRV) ) {
    Cerr << "Error: barnes_lf direct fn assumes no discrete variables in "
	 << "derivative mode." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numACV != 2) {
    Cerr << "Error: Bad number of variables in barnes_lf direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  if ( numFns != 4 ) {
    Cerr << "Error: Bad number of functions in barnes_lf direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Verification test for SBO performance.
  // Taken from Rodriguez, Perez, Renaud, et al.
  // Constraints g >= 0.

  double a[] = { 75.196,   -3.8112,    0.12694,    -2.0567e-3,  1.0345e-5,
		 -6.8306,   0.030234, -1.28134e-3,  3.5256e-5, -2.266e-7,
		  0.25645, -3.4604e-3, 1.3514e-5, -28.106,     -5.2375e-6,
		 -6.3e-8,   7.0e-10,   3.4054e-4,  -1.6638e-6, -2.8673,
		  0.0005};
  // Taylor series of Barnes function about the point (p1,p2)
  double p1  = 30.0;
  double p2  = 40.0;
  double x1  = xC[0]-p1;
  double x12 = x1*x1;
  double x13 = x12*x1;
  double x2  = xC[1]-p2;
  double x22 = x2*x2;
  double x23 = x22*x2;

  // **** f
  if (directFnASV[0] & 1) {
    fnVals[0] =
      - 2.74465943148169
      + 0.01213957527281*x1
      + 0.00995748775273*x12
      - 5.557060816484793e-04*x13
      + (1.15084419109172+0.00947331101091*x1+2.994070392732408e-05*x12)*x2
      + (-0.02997939337414-1.676054720545071e-04*x1)*x22
      - 0.00132216646850*x23;
  }

  // **** g1
  if (directFnASV[1] & 1)
    fnVals[1] = (xC[0]+xC[1]-50.)/10.;

  // **** g2
  if (directFnASV[2] & 1)
    fnVals[2] = (-0.64*xC[0]+xC[1])/6.;

  // **** g3
  if (directFnASV[3] & 1) {
    if (xC[1] > 50)
      fnVals[3] =
	-0.00599508167546*xC[0]
	+ 0.0134054101569*xC[1]
	- 0.34054101569933;
    else
      fnVals[3] =
	-0.00599508167546*xC[0]
	- 0.01340541015699*xC[1]
	+ 1.;
  }

  // **** df/dx
  if (directFnASV[0] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[0][i] =
	  - 0.58530968989099+0.01991497550546*xC[0]-0.00166711824495*x12
	  + (0.00767686877527+ 5.988140785464816e-05*xC[0])*x2
	  - 1.676054720545071e-04*x22;
	break;
      case 1:
	fnGrads[0][i] =
	    0.86664486076442+0.00947331101091*xC[0]+2.994070392732408e-05*x12
	  + 2*(-0.02495122921250-1.676054720545071e-04*xC[0])*x2
	  - 0.00396649940550*x22;
	break;
      }
    }
  }

  // **** dg1/dx
  if (directFnASV[1] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[1][i] = 1./10.;
	break;
      case 1:
	fnGrads[1][i] = 1./10.;
	break;
      }
    }
  }

  // **** dg2/dx
  if (directFnASV[2] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[2][i] = -0.64/6.;
	break;
      case 1:
	fnGrads[2][i] = 1./6.;
	break;
      }
    }
  }

  // **** dg3/dx
  if (directFnASV[3] & 2) {
    for (size_t i=0; i<numDerivVars; i++) {
      int var_index = directFnDVV[i] - 1;
      switch (var_index) {
      case 0:
	fnGrads[3][i] = -0.00599508167546;
	break;
      case 1:
	if (xC[1] > 50)
	  fnGrads[3][i] = 0.01340541015692;
	else
	  fnGrads[3][i] = -0.01340541015692;
	break;
      }
    }
  }
  return 0;
}


/// 1D Herbie function and its derivatives (apart from a multiplicative factor)
void TestDriverInterface::
herbie1D(size_t der_mode, Real xc_loc, std::vector<Real>& w_and_ders)
{
  w_and_ders[0]=w_and_ders[1]=w_and_ders[2]=0.0;

  Real rtemp1=xc_loc-1.0;
  Real rtemp1_sq=rtemp1*rtemp1;
  Real rtemp2=xc_loc+1.0;
  Real rtemp2_sq=rtemp2*rtemp2;
  Real rtemp3=8.0*(xc_loc+0.1);

  if(der_mode & 1) //1=2^0: the 0th derivative of the response (the response itself)
    w_and_ders[0]=
      std::exp(-rtemp1_sq)
      +std::exp(-0.8*rtemp2_sq)
      -0.05*std::sin(rtemp3);
  if(der_mode & 2) //2=2^1: the 1st derivative of the response
    w_and_ders[1]=
      -2.0*rtemp1*std::exp(-rtemp1_sq)
      -1.6*rtemp2*std::exp(-0.8*rtemp2_sq)
      -0.4*std::cos(rtemp3);
  if(der_mode & 4) //4=2^2: the 2nd derivative of the response
    w_and_ders[2]=
      (-2.0+4.0*rtemp1_sq)*std::exp(-rtemp1_sq)
      +(-1.6+2.56*rtemp2_sq)*std::exp(-0.8*rtemp2_sq)
      +3.2*std::sin(rtemp3);
  if(der_mode > 7) {
    Cerr << "only 0th through 2nd derivatives are implemented for herbie1D()\n";
    assert(false); //should throw an exception get brian to help
  }
}


/// 1D Smoothed Herbie= 1DHerbie minus the high frequency sine term, and its derivatives (apart from a multiplicative factor)
void TestDriverInterface::
smooth_herbie1D(size_t der_mode, Real xc_loc, std::vector<Real>& w_and_ders)
{
  w_and_ders[0]=w_and_ders[1]=w_and_ders[2]=0.0;

  Real rtemp1=xc_loc-1.0;
  Real rtemp1_sq=rtemp1*rtemp1;
  Real rtemp2=xc_loc+1.0;
  Real rtemp2_sq=rtemp2*rtemp2;

  if(der_mode & 1) //1=2^0: the 0th derivative of the response (the response itself)
    w_and_ders[0]=
      std::exp(-rtemp1_sq)
      +std::exp(-0.8*rtemp2_sq);
  if(der_mode & 2) //2=2^1: the 1st derivative of the response
    w_and_ders[1]=
      -2.0*rtemp1*std::exp(-rtemp1_sq)
      -1.6*rtemp2*std::exp(-0.8*rtemp2_sq);
  if(der_mode & 4) //4=2^2: the 2nd derivative of the response
    w_and_ders[2]=
      (-2.0+4.0*rtemp1_sq)*std::exp(-rtemp1_sq)
      +(-1.6+2.56*rtemp2_sq)*std::exp(-0.8*rtemp2_sq);
  if(der_mode > 7) {
    Cerr << "only 0th through 2nd derivatives are implemented for smooth_herbie1D()\n";
    assert(false); //should throw an exception get brian to help
  }
}


/// 1D Shubert function and its derivatives (apart from a multiplicative factor)
void TestDriverInterface::
shubert1D(size_t der_mode, Real xc_loc, std::vector<Real>& w_and_ders)
{
  w_and_ders[0]=w_and_ders[1]=w_and_ders[2]=0.0;

  size_t k;
  Real k_real;

  if(der_mode & 1) {
    for (k=1; k<=5; ++k) {
      k_real=static_cast<Real>(k);
      w_and_ders[0]+=k_real*std::cos(xc_loc*(k_real+1.0)+k_real);
    }
  }
  if(der_mode & 2) {
    for (k=1; k<=5; ++k) {
      k_real=static_cast<Real>(k);
      w_and_ders[1]+=k_real*(k_real+1.0)*-std::sin(xc_loc*(k_real+1.0)+k_real);
    }
  }
  if(der_mode & 4) {
    for (k=1; k<=5; ++k) {
      k_real=static_cast<Real>(k);
      w_and_ders[2]+=k_real*(k_real+1.0)*(k_real+1.0)*-std::cos(xc_loc*(k_real+1.0)+k_real);
    }
  }
  if(der_mode > 7) {
    Cerr << "only 0th through 2nd derivatives are implemented for shubert1D()\n";
    assert(false); //should throw an exception get brian to help
  }
}


/// N-D Herbie function and its derivatives
int TestDriverInterface::herbie()
{
  size_t i;
  std::vector<size_t> der_mode(numVars);
  for (i=0; i<numVars; ++i)
    der_mode[i]=1;
  if(directFnASV[0] >= 2)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=2;
  if(directFnASV[0] >= 4)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=4;
  std::vector<Real> w(numVars);
  std::vector<Real> d1w(numVars);
  std::vector<Real> d2w(numVars);
  std::vector<Real> w_and_ders(3);

  for(i=0; i<numVars; ++i) {
    herbie1D(der_mode[i],xC[i],w_and_ders);
    w[i]  =w_and_ders[0];
    d1w[i]=w_and_ders[1];
    d2w[i]=w_and_ders[2];
  }

  separable_combine(-1.0,w,d1w,d2w);
  return 0;
}

/// N-D Smoothed Herbie function and its derivatives
int TestDriverInterface::smooth_herbie()
{
  size_t i;
  std::vector<size_t> der_mode(numVars);
  for (i=0; i<numVars; ++i)
    der_mode[i]=1;
  if(directFnASV[0] >= 2)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=2;
  if(directFnASV[0] >= 4)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=4;
  std::vector<Real> w(numVars);
  std::vector<Real> d1w(numVars);
  std::vector<Real> d2w(numVars);
  std::vector<Real> w_and_ders(3);

  for(i=0; i<numVars; ++i) {
    smooth_herbie1D(der_mode[i], xC[i], w_and_ders);
    w[i]  =w_and_ders[0];
    d1w[i]=w_and_ders[1];
    d2w[i]=w_and_ders[2];
  }

  separable_combine(-1.0,w,d1w,d2w);
  return 0;
}

int TestDriverInterface::shubert()
{
  size_t i;
  std::vector<size_t> der_mode(numVars);
  for (i=0; i<numVars; ++i)
    der_mode[i]=1;
  if(directFnASV[0] >= 2)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=2;
  if(directFnASV[0] >= 4)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=4;
  std::vector<Real> w(numVars);
  std::vector<Real> d1w(numVars);
  std::vector<Real> d2w(numVars);
  std::vector<Real> w_and_ders(3);

  for(i=0; i<numVars; ++i) {
    shubert1D(der_mode[i],xC[i],w_and_ders);
    w[i]  =w_and_ders[0];
    d1w[i]=w_and_ders[1];
    d2w[i]=w_and_ders[2];
  }

  separable_combine(1.0,w,d1w,d2w);
  return 0;
}

int TestDriverInterface::bayes_linear()
{
  // This test driver implements the linear verification example in the 
  // document "User Guidelines and Best Practices for CASL VUQ Analysis 
  // using Dakota", CASL-U-2014-0038-000.  The example is in Appendix A 
  // and Section 6.2.6.  It is of the form: 
  // y = g(x)*Beta + epsilon, where epsilon ~ N(0,1/lamda * R)
  // Beta is a set of unknown regression coefficients we are trying to infer, 
  // epsilon is the vector of observational errors having variance 
  // 1/lambda = sigma^2 
  // and R is a fixed correlation function.

  if (multiProcAnalysisFlag) {
    Cerr << "Error: bayes_linear direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars < 1 || numVars > 500 || numADIV || numADRV) {
    Cerr << "Error: Bad variable types in Bayes linear fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns < 1) {
    Cerr << "Error: Bad number of functions in Bayes linear direct fn."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag || gradFlag) {
    Cerr << "Error: Gradients and Hessians not supported in Bayes linear "
	 << "direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  
  /*const Real pi = 3.14159265358979324;
 
  Real mean_pred = 0.4; int i;
  for (i=1; i<numVars; i++) {
     mean_pred += 0.4 + 0.5*std::sin(2*pi*i/numVars);
  }
  RealVector n_means, n_std_devs, n_l_bnds, n_u_bnds;
  n_means.resize(numFns); n_std_devs.resize(numFns); 
  n_l_bnds.resize(numFns); n_u_bnds.resize(numFns);
  for (i=0; i<numFns; i++) {
    n_means(i)=mean_pred;
    n_std_devs(i)=0.0316; // initial lambda = 1000.
    n_l_bnds(i)=-DBL_MAX;
    n_u_bnds(i)=DBL_MAX;
  }

  NonDLHSSampling* normal_sampler;
  String rng("mt19937");
  unsigned short sample_type = SUBMETHOD_LHS;
  int seed = 1; 
  seed += (int)clock();     
  Cout << "seed " << seed; 
  RealSymMatrix correl_matrix(numFns);
  correl_matrix = 0.8;
  for (i=0; i<numFns; i++) 
    correl_matrix(i,i)=1.0;
 
  Cout << "correl matrix " << correl_matrix << '\n';

  normal_sampler = new NonDLHSSampling(sample_type, 1, seed,
                                       rng, n_means, n_std_devs, 
                                       n_l_bnds, n_u_bnds, correl_matrix);  
  
  Cout << "normal samples " << normal_sampler->all_samples() << '\n'; 
  const RealMatrix& lhs_samples = normal_sampler->all_samples();
  RealVector temp_cvars = Teuchos::getCol(Teuchos::View, const_cast<RealMatrix&>(lhs_samples), 0);

  for (i=0; i<numFns; i++) {
    fnVals[i]=temp_cvars[i];
  } 
  */
  
  Real pred=0.0;
  for (int i=0; i<numVars; i++) 
    pred += xC[i];
  fnVals[0]= pred;
   
  return 0;
}  


/// this function combines N 1D functions and their derivatives to compute a N-D separable function and its derivatives, logic is general enough to support different 1D functions in different dimensions (can mix and match)
void TestDriverInterface::separable_combine(Real mult_scale_factor, std::vector<Real>& w, std::vector<Real>& d1w, std::vector<Real>& d2w)
{
  // *************************************************************
  // **** now that w(x_i), dw(x_i)/dx_i, and d^2w(x_i)/dx_i^2 ****
  // **** are defined we can calculate the response, gradient ****
  // **** of the response, and Hessian of the response in an  ****
  // **** identical fashion                                   ****
  // *************************************************************

  Real local_val;
  size_t i, j, k, i_var_index, j_var_index;

  // ****************************************
  // **** response                       ****
  // **** f=\prod_{i=1}^{numVars} w(x_i) ****
  // ****************************************
  if (directFnASV[0] & 1) {
    local_val=mult_scale_factor;
    for (i=0; i<numVars; ++i)
      local_val*=w[i];
    fnVals[0]=local_val;
  }

  // **************************************************
  // **** gradient of response                     ****
  // **** df/dx_i=(\prod_{j=1}^{i-1} w(x_j)) ...   ****
  // ****        *(dw(x_i)/dx_i) ...               ****
  // ****        *(\prod_{j=i+1}^{numVars} w(x_j)) ****
  // **************************************************
  if (directFnASV[0] & 2) {
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (i=0; i<numDerivVars; ++i) {
      i_var_index = directFnDVV[i] - 1;
      local_val=mult_scale_factor*d1w[i_var_index];
      for (j=0; j<i_var_index; ++j)
	local_val*=w[j];
      for (j=i_var_index+1; j<numVars; ++j)
	local_val*=w[j];
      fnGrads[0][i]=local_val;
    }
  }

  // ***********************************************************
  // **** Hessian of response                               ****
  // **** if(i==j)                                          ****
  // **** d^2f/dx_i^2=(\prod_{k=1}^{i-1} w(x_k)) ...        ****
  // ****            *(d^2w(x_i)/dx_i^2) ...                ****
  // ****            *(\prod_{k=i+1}^{numVars} w(x_k))      ****
  // ****                                                   ****
  // **** if(i<j)                                           ****
  // **** d^2f/(dx_i*dx_j)=(\prod_{k=1}^{i-1} w(x_k)) ...   ****
  // ****                 *(dw(x_i)/dx_i) ...               ****
  // ****                 *(\prod_{k=i+1}^{j-1} w(x_k)) ... ****
  // ****                 *(dw(x_j)/dx_j) ...               ****
  // ****                 *(\prod_{k=j+1}^{numVars} w(x_j)) ****
  // ***********************************************************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.; //what does this do? I think it's vestigal
    for (i=0; i<numDerivVars; ++i) {
      i_var_index = directFnDVV[i] - 1;
      for (j=0; j<numDerivVars; ++j) {
	j_var_index = directFnDVV[j] - 1;
	if (i_var_index==j_var_index )
	  local_val = mult_scale_factor*d2w[i_var_index];
	else
	  local_val = mult_scale_factor*d1w[i_var_index]*d1w[j_var_index];
	for (k=0; k<numVars; ++k)
	  if( (k!=i_var_index) && (k!=j_var_index) )
	    local_val*=w[k];
	fnHessians[0](i,j) =local_val;
      }
    }
  }
}

Real TestDriverInterface::levenshtein_distance(const String &v) {
  /// Levenshtein distance is the number of changes (single character
  // deletions, additions, or changes) to convert one string (v) to another
  // (levenshteinReference). This nice implementation shamelessly stolen/adapted
  // from Wikipedia, which attributes it to Wager and Fischer, 
  // doi:10.1145/321796.321811.
  // Results are stored in levenshteinDistanceCache to avoid needless repeated 
  // calcuations
  SRMCIter d_match;
  d_match = levenshteinDistanceCache.find(v);
  if (d_match != levenshteinDistanceCache.end())
    return d_match->second;
  const String &w = LEV_REF;
  size_t v_len = v.size(), w_len = w.size();
  IntMatrix d(v_len+1, w_len+1);
  size_t i, j;
  for(i=0; i<=v_len; ++i)
    d(i,0) = i;
  for(j=0; j<=w_len; ++j)
    d(0,j) = j;
  for(j=1; j<=w_len; ++j) {
    for(i=1; i<=v_len; ++i) {
      if(v[i-1] == w[j-1])
        d(i,j) = d(i-1,j-1);
      else
        d(i,j) = std::min( d(i-1,j)+1, std::min(d(i  ,j-1) + 1,
                                                d(i-1,j-1) + 1));
    }
  }
  levenshteinDistanceCache[v] = d(v_len,w_len);
  return levenshteinDistanceCache[v];
}

#ifdef DAKOTA_SALINAS
int TestDriverInterface::salinas()
{
  if (numFns < 1 || numFns > 3) {
    Cerr << "Error: Bad number of functions in salinas direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numVars < 1) {
    Cerr << "Error: Bad number of variables in salinas direct fn." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (gradFlag || hessFlag) {
    Cerr << "Error: analytic derivatives not yet supported in salinas direct "
	 << "fn." <<std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // ------------------------
  // Salinas input processing
  // ------------------------

  // Set up dummy argc and argv with name of modified input file.
  // NOTE: for concurrent analyses within each fn eval, may want something like
  //       salinas.<eval>.<analysis>.inp (e.g., salinas.3.2.inp)
  int argc = 2;
  char* argv[3];
  argv[0] = "salinas"; // should be ignored
  char si[32];
  std::sprintf(si,"salinas%d.inp", evalServerId); // tag root name in root.inp
  argv[1] = si;
  argv[2] = NULL; // standard requires this, see Kern.&Ritchie, p. 115

  // Insert vars into Salinas input file (Exodus model file avoided for now).
  // Set up loop to process input file and match tokens to variable tags.  The
  // Salinas parser is not dependent on new lines, so don't worry about
  // formatting.
  std::ifstream fin("salinas.inp.template");
  std::ofstream fout(argv[1]);
  std::string token;
  while (fin) {
    fin >> token;
    if (token=="//")
      fin.ignore(256, '\n'); // comments will not be replicated in fout
    else if (token=="'./tagged.exo'") {
      // **** Issues with the Exodus input file.  Exodus input files must be
      //   tagged because the Exodus output uses the same name and must be
      //   protected from conflict with other concurrent simulations.  This
      //   requires the creation of these tagged files by DAKOTA or their
      //   existence a priori (which is a problem when tagging with open ended
      //   indices like evaluation id). A few approaches to handling this:
      // 1.) could use system("cp root.exo root#.exo"), but no good on TFLOP
      // 2.) could tag w/ evalServerId & put sal[0-9].exo out before launching,
      //   but Salinas must overwrite properly (it does) & data loss must be OK
      // 3.) could modify salinas to use (tagged) root.inp i/o root.exo in
      //   creating root-out.exo, thereby removing the need to tag Exodus input
      char se[32];
      std::sprintf(se,"'./salinas%d.exo' ", evalServerId); // tag root in root.exo
      fout << se;
    }
    else if (localDataView & VARIABLES_MAP) {
      // Use a map-based lookup if tokens of interest are added to varTypeMap
      std::map<String, var_t>::iterator v_it = varTypeMap.find(token);
      if (v_it == varTypeMap.end())
	fout << token << ' ';
      else {
	var_t vt = v_it->second;
	std::map<var_t, Real>::iterator xc_it = xCM.find(vt),
	  xdr_it = xDRM.find(vt);
	std::map<var_t, int>::iterator xdi_it = xDIM.find(vt);
	if (xc_it != xCM.end())
	  fout << xc_it->second << ' ';
	else if (xdi_it != xDIM.end())
	  fout << xdi_it->second << ' ';
	else if (xdr_it != xDRM.end())
	  fout << xdr_it->second << ' ';
	elseint TestDriverInterface::scalable_text_book()

	  fout << token << ' ';
      }
    }
    else {
      bool found = false;
      size_t i;
      for (i=0; i<numACV; ++i) // loop to remove any order dependency
        if (token == xCLabels[i]) // detect variable label
          { fout << xC[i] << ' '; found = true; break; }
      if (!found)
	for (i=0; i<numADIV; ++i) // loop to remove any order dependency
	  if (token == xDILabels[i]) // detect variable label
	    { fout << xDI[i] << ' '; found = true; break; }
      if (!found)
	for (i=0; i<numADRV; ++i) // loop to remove any order dependency
	  if (token == xDRLabels[i]) // detect variable label
	    { fout << xDR[i] << ' '; found = true; break; }
      if (!found)
        fout << token << ' ';
    }
  }
  fout << std::flush;

  // -----------------
  // Salinas execution
  // -----------------

  // salinas_main is a bare bones wrapper for salinas.  It is provided to
  // permit calling salinas as a subroutine.

  // analysis_comm may be invalid if multiProcAnalysisFlag is not true!
  const ParallelLevel& ea_pl
    = parallelLib.parallel_configuration().ea_parallel_level();
  MPI_Comm analysis_comm = ea_pl.server_intra_communicator();
  int fail_code = salinas_main(argc, argv, &analysis_comm);
  if (fail_code)
    return fail_code;

  // ---------------------------
  // Salinas response processing
  // ---------------------------

  // Compute margins and return lowest margin as objective function to be
  // _maximized_ (minimize opposite sign).  Constrain mass to be:
  // mass_low_bnd <= mass <= mass_upp_bnd
  //Real min_margin = 0.;
  Real lambda1, mass, mass_low_bnd=3., mass_upp_bnd=6.; // 4.608 nominal mass

  // Call EXODUS utilities to retrieve stress & g data

  // Retrieve data from salinas#.rslt
  char so[32];
  std::sprintf(so,"salinas%d.rslt",evalServerId);
  std::ifstream f2in(so);
  while (f2in) {
    f2in >> token;
    if (token=="Total") {
      for (size_t i=0; i<4; ++i) // After "Total", parse "Mass of Structure is"
        f2in >> token;
      f2in >> mass;
    }
    else if (token=="1:") {
      f2in >> lambda1;
    }
    else
      f2in.ignore(256, '\n');
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = -lambda1; // max fundamental freq. s.t. mass bounds
    //fnVals[0] = -min_margin;

  // **** c1:
  if (numFns > 1 && (directFnASV[1] & 1))
    fnVals[1] = (mass_low_bnd - mass)/mass_low_bnd;

  // **** c2:
  if (numFns > 2 && (directFnASV[2] & 1))
    fnVals[2] = (mass - mass_upp_bnd)/mass_upp_bnd;

  // **** df/dx:
  //if (directFnASV[0] & 2) {
  //}

  // **** dc1/dx:
  //if (numFns > 1 && (directFnASV[1] & 2)) {
  //}

  // **** dc2/dx:
  //if (numFns > 2 && (directFnASV[2] & 2)) {
  //}

  return 0;
}
#endif // DAKOTA_SALINAS


#ifdef DAKOTA_MODELCENTER
/** The ModelCenter interface doesn't have any specific construct
    vs. run time functions.  For now, we manage it along with the
    integrated test drivers */
int TestDriverInterface::mc_api_run()
{
  // ModelCenter interface through direct Dakota interface, HKIM 4/3/03
  // Modified to replace pxcFile with analysisComponents,   MSE 6/20/05

  if (multiProcAnalysisFlag) {
    Cerr << "Error: mc_api_run direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }

  int i, j, ireturn, iprint = 1;

  if(!mc_ptr_int) { // If null, instantiate ModelCenter
    // the pxcFile (Phoenix configuration file) is passed through the
    // analysis_components specification
    if (!analysisComponents.empty() &&
	!analysisComponents[analysisDriverIndex].empty())
      mc_load_model(ireturn,iprint,mc_ptr_int,
		    analysisComponents[analysisDriverIndex][0].c_str());
    else
      ireturn = -1;
    if(ireturn == -1 || mc_ptr_int == 0) abort_handler(INTERFACE_ERROR);
  }

  // continuous variables
  for(i=0; i<numACV; ++i) {
    const char* inStr = xCLabels[i].c_str();
    mc_set_value(ireturn,iprint,mc_ptr_int,xC[i],inStr);
    if(ireturn == -1) abort_handler(INTERFACE_ERROR);
  }

  // discrete, integer-valued variables (actual values sent, not indices)
  for(i=0; i<numADIV; ++i) {
    const char* inStr = xDILabels[i].c_str();
    mc_set_value(ireturn,iprint,mc_ptr_int,xDI[i],inStr);
    if(ireturn == -1) abort_handler(INTERFACE_ERROR);
  }

  // discrete, real-valued variables (actual values sent, not indices)
  for(i=0; i<numADRV; ++i) {
    const char* inStr = xDRLabels[i].c_str();
    mc_set_value(ireturn,iprint,mc_ptr_int,xDR[i],inStr);
    if(ireturn == -1) abort_handler(INTERFACE_ERROR);
  }

  int out_var_act_len = fnLabels.size();
  if (out_var_act_len != numFns) {
    Cerr << "Error: Mismatch in the number of responses in mc_api_run."
	 << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  for (i=0; i<out_var_act_len; ++i) {
    // **** f:
    if (directFnASV[i] & 1) {
      const char* outStr = fnLabels[i].c_str();
      mc_get_value(ireturn,iprint,mc_ptr_int,fnVals[i],outStr);
      if(ireturn == -1) {
	// Assume this is a failed function evaluation
	// TODO: check correctness / other possible return codes
	return(-1);
      }

    }
    // **** df/dx:
    if (directFnASV[i] & 2) {
      Cerr << "Error: Analytic gradients not supported in mc_api_run."
	   << std::endl;
      abort_handler(INTERFACE_ERROR);
    }
    // **** d^2f/dx^2:
    if (directFnASV[i] & 4) {
      Cerr << "Error: Analytic Hessians not supported in mc_api_run."
	   << std::endl;
      abort_handler(INTERFACE_ERROR);
    }

  }

  if(dc_ptr_int) {
    mc_store_current_design_point(ireturn,iprint,dc_ptr_int);
    // ignore ireturn for now.
  }

  return 0; // no failure
}
#endif // DAKOTA_MODELCENTER

int TestDriverInterface::aniso_quad_form()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: aniso_quad_form direct fn does not yet support "
   << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV || (gradFlag && numDerivVars != numVars)) {
    Cerr << "Error: Bad number of variables in aniso_quad_form direct fn."
         << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in aniso_quad_form direct fn."
         << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in aniso_quad_form direct fn."
         << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  static bool initialized = false;

  static RealMatrix q_mat(numVars, numVars);

  if(!initialized)
  {
    size_t seed = std::time(NULL);
    std::vector<RealMatrix::scalarType> eigenvals =
      boost::assign::list_of(7.0)(4.0)(1.0);

    if (!analysisComponents.empty() &&
        !analysisComponents[analysisDriverIndex].empty())
    {
      typedef boost::char_separator<char> sepT;
      typedef boost::tokenizer<sepT> tokenT;
      sepT sep(" :");

      const StringArray& anal_comps = analysisComponents[analysisDriverIndex];
      for(StringArray::const_iterator comp = anal_comps.begin();
          comp != anal_comps.end(); ++comp)
      {
        tokenT tokens(*comp, sep);
        tokenT::iterator tok = tokens.begin();
        if(*tok == "seed")
        {
          if(++tok == tokens.end())
          {
            Cerr << "Seed not specified for aniso_quad_form." << std::endl;
            abort_handler(INTERFACE_ERROR);
          }

          seed = boost::lexical_cast<size_t>(*tok);

          if(++tok != tokens.end())
          {
            Cerr << "Multiple fields given as a seed in analysis_components "
                    "for aniso_quad_form." << std::endl;
            abort_handler(INTERFACE_ERROR);
          }
        }
        else if(*tok == "eigenvals")
        {
          eigenvals.clear();

          ++tok;
          for(; tok != tokens.end(); ++tok)
          {
            eigenvals.push_back(boost::lexical_cast<RealMatrix::scalarType>(
              *tok));
          }

          if(eigenvals.size() == 0)
          {
            Cerr << "No eigenvalues specified in analysis_components "
                    "for aniso_quad_form." << std::endl;
            abort_handler(INTERFACE_ERROR);
          }
          else if(eigenvals.size() > numVars)
          {
            Cerr << "Too many eigenvalues specified in analysis_components "
                    "for aniso_quad_form." << std::endl;
            abort_handler(INTERFACE_ERROR);
          }
        }
        else
        {
          Cerr << "Aniso_quad_form received invalid analysis_components."
               << std::endl;
          abort_handler(INTERFACE_ERROR);
        }
      }
    }

    boost::random::mt19937 vec_RNG(seed);
    boost::random::normal_distribution<> sampler;

    size_t num_prin_direc = eigenvals.size();

    RealMatrix prin_vecs(numVars, num_prin_direc);
    size_t numElem = numVars * num_prin_direc;

    RealMatrix::scalarType* vals = prin_vecs.values();
    for(size_t i = 0; i < numElem; ++i)
    {
      vals[i] = sampler(vec_RNG);
    }

    for(int i = 0; i < num_prin_direc; ++i)
    {
      RealVector prev_ortho_vec = getCol(Teuchos::View, prin_vecs, i);
      prev_ortho_vec *= 1.0/prev_ortho_vec.normFrobenius();

      for(int j = i + 1; j < num_prin_direc; ++j)
      {
        RealVector::scalarType proj_val = getCol(Teuchos::View, prin_vecs,
          j).dot(prev_ortho_vec);

        RealVector prev_ortho_vec_copy(Teuchos::Copy,
                                       prev_ortho_vec.values(),
                                       prev_ortho_vec.length());
        prev_ortho_vec_copy *= proj_val;

        getCol(Teuchos::View, prin_vecs, j) -= prev_ortho_vec_copy;
      }

      q_mat.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, eigenvals[i],
        prev_ortho_vec, prev_ortho_vec, 1.0);
    }

    initialized = true;
  }

  RealMatrix quad_prod(numVars, 1);
  quad_prod.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, q_mat, xC,
    0.0);

  if (directFnASV[0] & 1) // **** f:
  {
    fnVals[0] =  getCol(Teuchos::View, quad_prod, 0).dot(xC);
  }

  if (directFnASV[0] & 2) // **** gradf
  {
    quad_prod *= 2.0;
    fnGrads = quad_prod;
  }

  return 0; // no failure
}

}  // namespace Dakota
