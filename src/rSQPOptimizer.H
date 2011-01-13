/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       rSQPOptimizer
//- Description: Wrapper class for rSQP++
//- Owner:       Roscoe Bartlett
//- Checked by:
//- Version: $Id: rSQPOptimizer.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef RSQP_OPTIMIZER_H
#define RSQP_OPTIMIZER_H

#include "DakotaOptimizer.H"
#include "NLPInterfacePack/src/serial/NLPSerialPreprocessExplJac.hpp"

namespace Dakota {
class rSQPOptimizer;
} // namespace Dakota

namespace NLPInterfacePack {

class NLPDakota : public NLPSerialPreprocessExplJac
{
public:

	/** @name Constructors / initializers */
	//@{

	///
	/** Constructor.
	 *
	 * ToDo: Finish documentation!
	 */
	NLPDakota(
		Dakota::Model               *model
		,rSQPOptimizer            *dakota_rsqp_opt
		,size_type                num_lin_eq
		,size_type                num_nonlin_eq
		,size_type                num_lin_ineq
		,size_type                num_nonlin_ineq
		);

	//@}

	/** @name Accessors */
	//@{

	///
	const Dakota::RealVector& dakota_x() const         { return dakota_x_; }
	///
	const Dakota::RealVector& dakota_functions() const { return dakota_functions_; }

	//@}

	/** @name Overridden public members from NLP */
	//@{

	///
	void initialize(bool test_setup);
	///
	bool is_initialized() const;
	///
	value_type max_var_bounds_viol() const;
	///
	void set_multi_calc(bool multi_calc) const;
	///
	bool multi_calc() const;

	//@}

	/** @name Overridden from NLPVarReductPerm */
	//@{
	
	///
	bool nlp_selects_basis() const;

	//@}

protected:

	/** @name Overridden protected methods from NLPSerialPreprocess */
	//@{

	///
	bool imp_nlp_has_changed() const;
	///
	size_type imp_n_orig() const;
	///
	size_type imp_m_orig() const;
	///
	size_type imp_mI_orig() const;
	///
	const DVectorSlice imp_xinit_orig() const;
	///
	bool imp_has_var_bounds() const;
	///
	const DVectorSlice imp_xl_orig() const;
	///
	const DVectorSlice imp_xu_orig() const;
	///
	const DVectorSlice imp_hl_orig() const;
	///
	const DVectorSlice imp_hu_orig() const;
	///
	void imp_calc_f_orig(
		const DVectorSlice            &x_full
		,bool                        newx
		,const ZeroOrderInfoSerial   &zero_order_info
		) const;
	///
	void imp_calc_c_orig(
		const DVectorSlice            &x_full
		,bool                        newx
		,const ZeroOrderInfoSerial   &zero_order_info
		) const;
	///
	void imp_calc_h_orig(
		const DVectorSlice            &x_full
		,bool                        newx
		,const ZeroOrderInfoSerial   &zero_order_info
		) const;
	///
	void imp_calc_Gf_orig(
		const DVectorSlice            &x_full
		,bool                        newx
		,const ObjGradInfoSerial     &obj_grad_info
		) const;
	///
	void imp_report_orig_final_solution(
		const DVectorSlice      &x_orig
		,const DVectorSlice     *lambda_orig
		,const DVectorSlice     *lambdaI_orig
		,const DVectorSlice     *nu_orig
		,bool                  is_optimal
		) const;

	//@}
	
	/** @name Overridden protected methods from NLPSerialPreprocessExplJac */
	//@{

	///
	size_type imp_Gc_nz_orig() const;
	///
	size_type imp_Gh_nz_orig() const;
	///
	void imp_calc_Gc_orig(
		const DVectorSlice& x_full, bool newx
		,const FirstOrderExplInfo& first_order_expl_info
		) const;
	///
	void imp_calc_Gh_orig(
		const DVectorSlice& x_full, bool newx
		,const FirstOrderExplInfo& first_order_expl_info
		) const;

	//@}

private:

	// /////////////////////////////////////////
	// Private types

	enum ERequiredFunc {
		CALC_F   = 0
		,CALC_C  = 1
		,CALC_H  = 2
		,CALC_GF = 3
		,CALC_GC = 4
		,CALC_GH = 5
	};

	// /////////////////////////////////////////
	// Private data members

	Dakota::Model     *model_;
	rSQPOptimizer   *dakota_rsqp_opt_;
	int              num_objectives_;

	bool             is_initialized_;

	bool             has_var_bounds_;

	mutable bool     multi_calc_;

	Dakota::RealVector multi_obj_weights_;
	size_type        num_lin_eq_;
	size_type        num_nonlin_eq_;
	size_type        num_lin_ineq_;
	size_type        num_nonlin_ineq_;

	value_type       invalid_func_value_;
	value_type       dakota_failed_value_;

	size_type        n_orig_;
	size_type        m_orig_;
	size_type        mI_orig_;

	size_type        Gc_orig_nz_;
	size_type        Gh_orig_nz_;

	DVector           xinit_orig_;
	DVector           xl_orig_;
	DVector           xu_orig_;
	DVector           hl_orig_;
	DVector           hu_orig_;

	mutable bool     f_orig_updated_;
	mutable bool     c_orig_updated_;
	mutable bool     h_orig_updated_;
	mutable bool     Gf_orig_updated_;
	mutable bool     Gc_orig_updated_;
	mutable bool     Gh_orig_updated_;
	
	mutable Dakota::RealVector dakota_x_;
	mutable Dakota::RealVector dakota_functions_;
	mutable Dakota::ShortArray dakota_asv_;

	// /////////////////////////////////////////
	// Private member functions

	///
	void assert_is_initialized() const;

	///
	void imp_calc_point(
		ERequiredFunc                required
		,const DVectorSlice           &x_full
		,bool                        newx
		,const ZeroOrderInfoSerial   *zero_order_info
		,const ObjGradInfoSerial     *obj_grad_info
		,const FirstOrderExplInfo    *first_order_expl_info
		) const;
	
	// Not defined and not to be called
	NLPDakota();
	NLPDakota(const NLPDakota&);
	NLPDakota& operator=(const NLPDakota&);

};	// end class NLPDakota

} // end namespace NLPInterfacePack


namespace Dakota {

///
/** Wrapper class for the rSQP++ optimization library.
 *
 * The rSQPOptimizer class provides a wrapper for rSQP++, a C++ 
 * sequential quadratic programming library written by Roscoe Bartlett. 
 * rSQP++ can currently be used in NAND mode, although use of its SAND 
 * mode for reduced-space SQP is planned.  rSQPOptimizer uses a
 * NLPDakota object to perform the function evaluations. 
 *
 * The user input mappings will ultimately include:
 * <tt>max_iterations</tt>, <tt>convergence_tolerance</tt>,
 * <tt>output_verbosity</tt>.
 */

class rSQPOptimizer: public Optimizer
{
public:
  
    ///
    rSQPOptimizer(Model& model);

    ///
    ~rSQPOptimizer();

    ///
    int num_objectives() const
    { return numObjectiveFns; }
    ///
    const RealVector& lin_ineq_lb() const
    { return iteratedModel.linear_ineq_constraint_lower_bounds(); }
    ///
    const RealVector& lin_ineq_ub() const
    { return iteratedModel.linear_ineq_constraint_upper_bounds(); }
    ///
    const RealVector& nonlin_ineq_lb() const
    { return iteratedModel.nonlinear_ineq_constraint_lower_bounds(); }
    ///
    const RealVector& nonlin_ineq_ub() const
    { return iteratedModel.nonlinear_ineq_constraint_upper_bounds(); }
    ///
    const RealVector& lin_eq_targ() const
    { return iteratedModel.linear_eq_constraint_targets(); }
    ///
    const RealVector& nonlin_eq_targ() const
    { return iteratedModel.nonlinear_eq_constraint_targets(); }
    ///
    const RealMatrix& lin_eq_jac() const
    { return iteratedModel.linear_eq_constraint_coeffs(); }
    ///
    const RealMatrix& lin_ineq_jac() const
    { return iteratedModel.linear_ineq_constraint_coeffs(); }
    
    /** @name Overridden from Optimizer */
    //@{

    ///
    void find_optimum();

    //@}

private:

    Model                          *model_;
    NLPInterfacePack::NLPDakota    nlp_;

};

} // namespace Dakota

#endif
