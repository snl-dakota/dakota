/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDUnilevelRBDO
//- Description: Class for unilevel reliability-based design optimization (RBDO)
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_UNILEVEL_RBDO_H
#define NOND_UNILEVEL_RBDO_H

#include "DakotaNonD.H"
// fwd declarations insufficient for non-static attributes
#include "Epetra_SerialSymDenseMatrix.h"
// fwd declarations sufficient for static attributes
class Epetra_SerialDenseVector;
class Epetra_SerialDenseMatrix;
class Epetra_SerialDenseSolver;
class Epetra_SerialSpdDenseSolver;


/// Class for the unilevel RBDO approach to optimization under uncertainty

/** The NonDUnilevelRBDO class implements a single-level RBDO method
    in which the KKT equations from the MPP search are solved as
    equality constraints within the top level design optimization. */

class NonDUnilevelRBDO: public DakotaNonD
{
  public:

    //
    //- Heading: Constructors and destructor
    //

    NonDUnilevelRBDO(DakotaModel& model); ///< constructor
    ~NonDUnilevelRBDO();                  ///< destructor

    //
    //- Heading: Member functions
    //

    /// performs an uncertainty propagation using analytical reliability 
    /// methods which solve constrained optimization  problems to obtain
    /// approximations of the cumulative distribution function of response 
    void quantify_uncertainty(); // pure virtual, called by run_iterator

    /// print the approximate mean,standard deviation, and importance factors
    /// when using the mean value method (MV) or the CDF information when using
    /// other reliability methods (AMV,AMV+,FORM)
    void print_iterator_results(ostream& s) const;

  private:

    //
    //- Heading: Member functions
    //

    /// convenience function for encapsulating the simple Mean Value
    /// computation of approximate statistics and importance factors
    void mean_value();

    /// convenience function for encapsulating the iterated
    /// reliability methods (AMV, AMV+, FORM, SORM)
    void iterated_mean_value();

    //
    //- Heading: Objective/constraint eval fns passed by pointer to NPSOL/OPT++
    //

    // These functions need to be static so that they can be passed in
    // as function pointers without having to restrict the recipient
    // to functions from the NonDUnilevelRBDO class (see Stroustrup,
    // p.166 - pointers to member functions must use class scope
    // operators which would restrict the generality of the
    // NPSOLOptimizer/SNLLOptimizer "user_functions" interfaces).

#ifdef HAVE_NPSOL
    /// static function used by NPSOL as the objective function in the 
    /// RBDO problem formulation.  This portion is independent of RIA/PMA.
    static void RBDO_objective_eval(int& mode, int& n, Real* u, Real& f,
                                    Real* grad_f, int&);

    /// static function used by NPSOL as the RBDO constraint function
    /// in the Reliability Index Approach (RIA) problem formulation.
    static void RBDO_RIA_constraint_eval(int& mode, int& ncnln, int& n,
					 int& nrowj, int* needc, Real* u,
					 Real* c, Real* cjac, int& nstate);

    /// static function used by NPSOL as the RBDO constraint function
    /// in the Performance Measure Approach (PMA) problem formulation.
    static void RBDO_PMA_constraint_eval(int& mode, int& ncnln, int& n,
					 int& nrowj, int* needc, Real* u,
					 Real* c, Real* cjac, int& nstate);

#elif defined (HAVE_OPTPP)
    /// static function used by OPT++ as the objective function in the 
    /// Reliability Index Approach (RIA) problem formulation.  This
    /// equality-constrained optimization problem performs the search for the
    /// most probable point (MPP) with the objective function of (norm u)^2.
    static void RIA_objective_eval(int mode, int n, const ColumnVector& u,
                                   Real& f, ColumnVector& grad_f,
                                   int& result_mode);

    /// static function used by OPT++ as the constraint function in the 
    /// Reliability Index Approach (RIA) problem formulation.  This
    /// equality-constrained optimization problem performs the search for the
    /// most probable point (MPP) with the constraint of G(u) = response level.
    static void RIA_constraint_eval(int mode, int n,const ColumnVector& u, 
                                    ColumnVector& g, Matrix& grad_g,
                                    int& result_mode);

    /// static function used by OPT++ as the objective function in the 
    /// Performance Measure Approach (PMA) problem formulation.  This
    /// equality-constrained optimization problem performs the search for the
    /// most probable point (MPP) with the objective function of G(u).
    static void PMA_objective_eval(int mode, int n, const ColumnVector& u,
                                   Real& f, ColumnVector& grad_f,
                                   int& result_mode);

    /// static function used by OPT++ as the constraint function in the 
    /// Performance Measure Approach (PMA) problem formulation.  This
    /// equality-constrained optimization problem performs the search for the
    /// most probable point (MPP) with the constraint of (norm u)^2 = beta^2.
    static void PMA_constraint_eval(int mode, int n,const ColumnVector& u, 
                                    ColumnVector& g, Matrix& grad_g,
                                    int& result_mode);
#endif

    /// convenience function for evaluating G(u) and fnGradU(u).  Used by both
    /// RIA_constraint_eval() and both PMA_objective_eval() implementations.
    static void g_eval(int& mode, const Epetra_SerialDenseVector& u, Real& g);

    //
    //- Heading: Data members
    //

    /// static copy of response fn values evaluated in x-space at mean x
    static Epetra_SerialDenseVector fnValsMeanX;
    /// copy of response fn gradients evaluated in x-space at mean x
    Epetra_SerialDenseMatrix        fnGradsMeanX;
    /// Gradient of current response function in x-space
    static Epetra_SerialDenseVector fnGradX;
    /// Gradient of current response function in u-space
    static Epetra_SerialDenseVector fnGradU;
    /// vector of median values of functions used to determine which
    /// side of probability equal 0.5 the response level is
    DakotaRealVector                medianFnVals;

    /// petra copy of #uncertainCorrelations 
    Epetra_SerialSymDenseMatrix     petraCorrMatrix;
    /// cholesky factor of #petraCorrMatrix
    static Epetra_SerialDenseMatrix cholCorrMatrix;

    /// Location of MPP in x space
    static Epetra_SerialDenseVector mostProbPointX;
    /// Location of MPP in u space
    static Epetra_SerialDenseVector mostProbPointU;

    /// vector of indices indicating which type of uncertain variable
    static DakotaIntVector          ranVarType;
    /// Mean vector of all uncertain random variables 
    static Epetra_SerialDenseVector ranVarMeans;
    /// Standard deviation vector of all uncertain random variables
    static Epetra_SerialDenseVector ranVarSigmas;

    /// counter for which response function is being analyzed
    static int                      respFnCount;
    /// counter for which response/probability level is being analyzed
    static int                      levelCount;
    /// static copy of userDefinedModel used in RIA/PMA evaluators
    static DakotaModel              staticNonDModel;
    /// static copy of #numUncertainVars
    static size_t                   staticNumUncVars;
    /// static copy of #numFunctions
    static size_t                   staticNumFuncs;

    /// the current response level target for the current response function
    static Real                     requestedRespLevel;
    /// the current CDF probability level target for the current response fn
    static Real                     requestedCDFProbLevel;

    /// flag to represent which reliability method is being used
    static short                    mppSearchFlag;
    /// integration method identifier provided by integration specification
    DakotaString                    integrationMethod;
    /// importance factors predicted by MV
    DakotaRealMatrix                impFactor;
    /// derivative level for NPSOL executions (1 = analytic grads of objective
    /// fn, 2 = analytic grads of constraints, 3 = analytic grads of both).
    static int                      npsolDerivLevel;
};

#endif
