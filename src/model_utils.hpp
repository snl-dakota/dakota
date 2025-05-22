/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once
#include "dakota_data_types.hpp"


namespace Dakota {
    class Model;
    class ProblemDescDB;
    class ParallelLibrary;
    
    namespace ModelUtils {
        /// define and return discreteIntSets using active view from currentVariables
        BitArray discrete_int_sets(const Model &model);

        /// define and return discreteIntSets using passed active view
        BitArray discrete_int_sets(const Model &model, short active_view);

        /// return the sets of values available for each of the active
        /// discrete set integer variables (aggregated in activeDiscSetIntValues)
        IntSetArray discrete_set_int_values(const Model &model);

        /// return the sets of values available for each of the active
        /// discrete set integer variables (aggregated in activeDiscSetIntValues)
        IntSetArray discrete_set_int_values(const Model &model, short active_view);

        /// return the sets of values available for each of the active
        /// discrete set string variables (aggregated in activeDiscSetStringValues)
        StringSetArray discrete_set_string_values(const Model &model);

        /// return the sets of values available for each of the active
        /// discrete set string variables (aggregated in activeDiscSetStringValues)
        StringSetArray discrete_set_string_values(const Model &model, short active_view);

        /// return the sets of values available for each of the active
        /// discrete set real variables (aggregated in activeDiscSetRealValues)
        RealSetArray discrete_set_real_values(const Model &model);

        /// return the sets of values available for each of the active
        /// discrete set real variables (aggregated in activeDiscSetRealValues)
        RealSetArray discrete_set_real_values(const Model &model, short active_view);

        /// returns total number of vars
        size_t tv(const Model &model);

        /// returns number of active continuous variables
        size_t cv(const Model &model);

        /// returns number of active discrete integer vars
        size_t div(const Model &model);

        /// returns number of active discrete string vars
        size_t dsv(const Model &model);

        /// returns number of active discrete real vars
        size_t drv(const Model &model);

        /// returns number of inactive continuous variables
        size_t icv(const Model &model);

        /// returns number of inactive discrete integer vars
        size_t idiv(const Model &model);

        /// returns number of inactive discrete string vars
        size_t idsv(const Model &model);

        /// returns number of inactive discrete real vars
        size_t idrv(const Model &model);

        /// returns total number of continuous variables
        size_t acv(const Model &model);

        /// returns total number of discrete integer vars
        size_t adiv(const Model &model);

        /// returns total number of discrete string vars
        size_t adsv(const Model &model);

        /// returns total number of discrete real vars
        size_t adrv(const Model &model);

        /// set the active variables in currentVariables
        void active_variables(Model &model, const Variables& vars);

        /// set the inactive variables in currentVariables
        void inactive_variables(Model &model, const Variables& vars);

        /// return the active continuous variables from currentVariables
        const RealVector& continuous_variables(const Model &model);

        /// return an active continuous variable from currentVariables
        Real continuous_variable(const Model &model, size_t i);

        /// set the active continuous variables in currentVariables
        void continuous_variables(Model &model, const RealVector& c_vars);

        /// set an active continuous variable in currentVariables
        void continuous_variable(Model &model, Real c_var, size_t i);

        /// return the active discrete integer variables from currentVariables
        const IntVector& discrete_int_variables(const Model &model);

        /// return an active discrete integer variable from currentVariables
        int discrete_int_variable(const Model &model, size_t i);

        /// set the active discrete integer variables in currentVariables
        void discrete_int_variables(Model &model, const IntVector& d_vars);

        /// set an active discrete integer variable in currentVariables
        void discrete_int_variable(Model &model, int d_var, size_t i);

        /// return the active discrete string variables from currentVariables
        StringMultiArrayConstView discrete_string_variables(const Model &model);

        /// return an active discrete string variable from currentVariables
        const String& discrete_string_variable(const Model &model, size_t i);

        /// set the active discrete string variables in currentVariables
        void discrete_string_variables(Model &model, StringMultiArrayConstView d_vars);

        /// set an active discrete string variable in currentVariables
        void discrete_string_variable(Model &model, const String& d_var, size_t i);

        /// return the active discrete real variables from currentVariables
        const RealVector& discrete_real_variables(const Model &model);

        /// return an active discrete real variable from currentVariables
        Real discrete_real_variable(const Model &model, size_t i);

        /// set the active discrete real variables in currentVariables
        void discrete_real_variables(Model &model, const RealVector& d_vars);

        /// set an active discrete real variable in currentVariables
        void discrete_real_variable(Model &model, Real d_var, size_t i);

        /// return the active continuous variable types from currentVariables
        UShortMultiArrayConstView continuous_variable_types(const Model &model);

        /// set the active continuous variable types in currentVariables
        void continuous_variable_types(Model &model, UShortMultiArrayConstView cv_types);

        /// set an active continuous variable type in currentVariables
        void continuous_variable_type(Model &model, unsigned short cv_type, size_t i);

        /// return the active discrete variable types from currentVariables
        UShortMultiArrayConstView discrete_int_variable_types(const Model &model);

        /// set the active discrete variable types in currentVariables
        void discrete_int_variable_types(Model &model, UShortMultiArrayConstView div_types);

        /// set an active discrete variable type in currentVariables
        void discrete_int_variable_type(Model &model, unsigned short div_type, size_t i);

        /// return the active discrete variable types from currentVariables
        UShortMultiArrayConstView discrete_string_variable_types(const Model &model);

        /// set the active discrete variable types in currentVariables
        void discrete_string_variable_types(Model &model, UShortMultiArrayConstView dsv_types);

        /// set an active discrete variable type in currentVariables
        void discrete_string_variable_type(Model &model, unsigned short dsv_type, size_t i);

        /// return the active discrete variable types from currentVariables
        UShortMultiArrayConstView discrete_real_variable_types(const Model &model);

        /// set the active discrete variable types in currentVariables
        void discrete_real_variable_types(Model &model, UShortMultiArrayConstView drv_types);

        /// set an active discrete variable type in currentVariables
        void discrete_real_variable_type(Model &model, unsigned short drv_type, size_t i);

        /// return the active continuous variable identifiers from currentVariables
        SizetMultiArrayConstView continuous_variable_ids(const Model &model);

        /// set the active continuous variable identifiers in currentVariables
        void continuous_variable_ids(Model &model, SizetMultiArrayConstView cv_ids);

        /// set an active continuous variable identifier in currentVariables
        void continuous_variable_id(Model &model, size_t cv_id, size_t i);

        /// return the inactive continuous variables in currentVariables
        const RealVector& inactive_continuous_variables(const Model &model);

        /// set the inactive continuous variables in currentVariables
        void inactive_continuous_variables(Model &model, const RealVector& i_c_vars);

        /// return the inactive discrete variables in currentVariables
        const IntVector& inactive_discrete_int_variables(const Model &model);

        /// set the inactive discrete variables in currentVariables
        void inactive_discrete_int_variables(Model &model, const IntVector& i_d_vars);

        /// return the inactive discrete variables in currentVariables
        StringMultiArrayConstView inactive_discrete_string_variables(const Model &model);

        /// set the inactive discrete variables in currentVariables
        void inactive_discrete_string_variables(Model &model, StringMultiArrayConstView i_d_vars);

        /// return the inactive discrete variables in currentVariables
        const RealVector& inactive_discrete_real_variables(const Model &model);

        /// set the inactive discrete variables in currentVariables
        void inactive_discrete_real_variables(Model &model, const RealVector& i_d_vars);

        /// return the inactive continuous variable types from currentVariables
        UShortMultiArrayConstView inactive_continuous_variable_types(const Model &model);

        /// return the inactive continuous variable identifiers from currentVariables
        SizetMultiArrayConstView inactive_continuous_variable_ids(const Model &model);

        /// return all continuous variables in currentVariables
        const RealVector& all_continuous_variables(const Model &model);

        /// set all continuous variables in currentVariables
        void all_continuous_variables(Model &model, const RealVector& a_c_vars);

        /// set a variable within the all continuous variables in currentVariables
        void all_continuous_variable(Model &model, Real a_c_var, size_t i);

        /// return all discrete variables in currentVariables
        const IntVector& all_discrete_int_variables(const Model &model);

        /// set all discrete variables in currentVariables
        void all_discrete_int_variables(Model &model, const IntVector& a_d_vars);

        /// set a variable within the all discrete variables in currentVariables
        void all_discrete_int_variable(Model &model, int a_d_var, size_t i);

        /// return all discrete variables in currentVariables
        StringMultiArrayConstView all_discrete_string_variables(const Model &model);

        /// set all discrete variables in currentVariables
        void all_discrete_string_variables(Model &model, StringMultiArrayConstView a_d_vars);

        /// set a variable within the all discrete variables in currentVariables
        void all_discrete_string_variable(Model &model, const String& a_d_var, size_t i);

        /// return all discrete variables in currentVariables
        const RealVector& all_discrete_real_variables(const Model &model);

        /// set all discrete variables in currentVariables
        void all_discrete_real_variables(Model &model, const RealVector& a_d_vars);

        /// set a variable within the all discrete variables in currentVariables
        void all_discrete_real_variable(Model &model, Real a_d_var, size_t i);

        /// return all continuous variable types from currentVariables
        UShortMultiArrayConstView all_continuous_variable_types(const Model &model);

        /// return all discrete variable types from currentVariables
        UShortMultiArrayConstView all_discrete_int_variable_types(const Model &model);

        /// return all discrete variable types from currentVariables
        UShortMultiArrayConstView all_discrete_string_variable_types(const Model &model);

        /// return all discrete variable types from currentVariables
        UShortMultiArrayConstView all_discrete_real_variable_types(const Model &model);

        /// return all continuous variable identifiers from currentVariables
        SizetMultiArrayConstView all_continuous_variable_ids(const Model &model);

        /// return the active continuous variable labels from currentVariables
        StringMultiArrayConstView continuous_variable_labels(const Model &model);

        /// set the active continuous variable labels in currentVariables
        void continuous_variable_labels(Model &model, StringMultiArrayConstView c_v_labels);

        /// return the active discrete variable labels from currentVariables
        StringMultiArrayConstView discrete_int_variable_labels(const Model &model);

        /// set the active discrete variable labels in currentVariables
        void discrete_int_variable_labels(Model &model, StringMultiArrayConstView d_v_labels);

        /// return the active discrete variable labels from currentVariables
        StringMultiArrayConstView discrete_string_variable_labels(const Model &model);

        /// set the active discrete variable labels in currentVariables
        void discrete_string_variable_labels(Model &model, StringMultiArrayConstView d_v_labels);

        /// return the active discrete variable labels from currentVariables
        StringMultiArrayConstView discrete_real_variable_labels(const Model &model);

        /// set the active discrete variable labels in currentVariables
        void discrete_real_variable_labels(Model &model, StringMultiArrayConstView d_v_labels);

        /// return the inactive continuous variable labels in currentVariables
        StringMultiArrayConstView inactive_continuous_variable_labels(const Model &model);

        /// set the inactive continuous variable labels in currentVariables
        void inactive_continuous_variable_labels(Model &model, StringMultiArrayConstView i_c_v_labels);

        /// return the inactive discrete variable labels in currentVariables
        StringMultiArrayConstView inactive_discrete_int_variable_labels(const Model &model);

        /// set the inactive discrete variable labels in currentVariables
        void inactive_discrete_int_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels);

        /// return the inactive discrete variable labels in currentVariables
        StringMultiArrayConstView inactive_discrete_string_variable_labels(const Model &model);

        /// set the inactive discrete variable labels in currentVariables
        void inactive_discrete_string_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels);

        /// return the inactive discrete variable labels in currentVariables
        StringMultiArrayConstView inactive_discrete_real_variable_labels(const Model &model);

        /// set the inactive discrete variable labels in currentVariables
        void inactive_discrete_real_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels);

        /// set all continuous variable labels in currentVariables
        StringMultiArrayConstView all_continuous_variable_labels(const Model &model);

        /// set a label within the all continuous labels in currentVariables
        void all_continuous_variable_labels(Model &model, StringMultiArrayConstView a_c_v_labels);

        /// return all discrete variable labels in currentVariables
        void all_continuous_variable_label(Model &model, const String& a_c_v_label, size_t i);

        /// set all discrete variable labels in currentVariables
        StringMultiArrayConstView all_discrete_int_variable_labels(const Model &model);

        /// set a label within the all discrete labels in currentVariables
        void all_discrete_int_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels);

        /// return all discrete variable labels in currentVariables
        void all_discrete_int_variable_label(Model &model, const String& a_d_v_label, size_t i);

        /// set all discrete variable labels in currentVariables
        StringMultiArrayConstView all_discrete_string_variable_labels(const Model &model);

        /// set a label within the all discrete labels in currentVariables
        void all_discrete_string_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels);

        /// return all discrete variable labels in currentVariables
        void all_discrete_string_variable_label(Model &model, const String& a_d_v_label, size_t i);

        /// set all discrete variable labels in currentVariables
        StringMultiArrayConstView all_discrete_real_variable_labels(const Model &model);

        /// set a label within the all discrete labels in currentVariables
        void all_discrete_real_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels);

        /// set a label within the all discrete labels in currentVariables
        void all_discrete_real_variable_label(Model &model, const String& a_d_v_label, size_t i);

        /// return the number of responses
        size_t response_size(const Model &model);

        /// return the response labels from currentResponse
        const StringArray& response_labels(const Model &model);

        /// set the response labels in currentResponse
        void response_labels(Model &model, const StringArray& resp_labels);

        /// return the active continuous lower bounds from userDefinedConstraints
        const RealVector& continuous_lower_bounds(const Model &model);

        /// return an active continuous lower bound from userDefinedConstraints
        Real continuous_lower_bound(const Model &model, size_t i);

        /// set the active continuous lower bounds in userDefinedConstraints
        void continuous_lower_bounds(Model &model, const RealVector& c_l_bnds);

        /// set the i-th active continuous lower bound in userDefinedConstraints
        void continuous_lower_bound(Model &model, Real c_l_bnd, size_t i);

        /// return the active continuous upper bounds from userDefinedConstraints
        const RealVector& continuous_upper_bounds(const Model &model);

        /// return an active continuous upper bound from userDefinedConstraints
        Real continuous_upper_bound(const Model &model, size_t i);

        /// set the active continuous upper bounds in userDefinedConstraints
        void continuous_upper_bounds(Model &model, const RealVector& c_u_bnds);

        /// set the i-th active continuous upper bound from userDefinedConstraints
        void continuous_upper_bound(Model &model, Real c_u_bnd, size_t i);

        /// return the active discrete int lower bounds from userDefinedConstraints
        const IntVector& discrete_int_lower_bounds(const Model &model);

        /// return an active discrete int lower bound from userDefinedConstraints
        int discrete_int_lower_bound(const Model &model, size_t i);

        /// set the active discrete int lower bounds in userDefinedConstraints
        void discrete_int_lower_bounds(Model &model, const IntVector& d_l_bnds);

        /// set the i-th active discrete int lower bound in userDefinedConstraints
        void discrete_int_lower_bound(Model &model, int d_l_bnd, size_t i);

        /// return the active discrete int upper bounds from userDefinedConstraints
        const IntVector& discrete_int_upper_bounds(const Model &model);

        /// return an active discrete int upper bound from userDefinedConstraints
        int discrete_int_upper_bound(const Model &model, size_t i);

        /// set the active discrete int upper bounds in userDefinedConstraints
        void discrete_int_upper_bounds(Model &model, const IntVector& d_u_bnds);

        /// set the i-th active discrete int upper bound in userDefinedConstraints
        void discrete_int_upper_bound(Model &model, int d_u_bnd, size_t i);

        /// return the active discrete real lower bounds from userDefinedConstraints
        const RealVector& discrete_real_lower_bounds(const Model &model);

        /// return an active discrete real lower bound from userDefinedConstraints
        Real discrete_real_lower_bound(const Model &model, size_t i);

        /// set the active discrete real lower bounds in userDefinedConstraints
        void discrete_real_lower_bounds(Model &model, const RealVector& d_l_bnds);

        /// set the i-th active discrete real lower bound in userDefinedConstraints
        void discrete_real_lower_bound(Model &model, Real d_l_bnd, size_t i);

        /// return the active discrete real upper bounds from userDefinedConstraints
        const RealVector& discrete_real_upper_bounds(const Model &model);

        /// return an active discrete real upper bound from userDefinedConstraints
        Real discrete_real_upper_bound(const Model &model, size_t i);

        /// set the active discrete real upper bounds in userDefinedConstraints
        void discrete_real_upper_bounds(Model &model, const RealVector& d_u_bnds);

        /// set the i-th active discrete real upper bound in userDefinedConstraints
        void discrete_real_upper_bound(Model &model, Real d_u_bnd, size_t i);

        /// return the inactive continuous lower bounds in userDefinedConstraints
        const RealVector& inactive_continuous_lower_bounds(const Model &model);

        /// set the inactive continuous lower bounds in userDefinedConstraints
        void inactive_continuous_lower_bounds(Model &model, const RealVector& i_c_l_bnds);

        /// return the inactive continuous upper bounds in userDefinedConstraints
        const RealVector& inactive_continuous_upper_bounds(const Model &model);

        /// set the inactive continuous upper bounds in userDefinedConstraints
        void inactive_continuous_upper_bounds(Model &model, const RealVector& i_c_u_bnds);

        /// return the inactive discrete lower bounds in userDefinedConstraints
        const IntVector& inactive_discrete_int_lower_bounds(const Model &model);

        /// set the inactive discrete lower bounds in userDefinedConstraints
        void inactive_discrete_int_lower_bounds(Model &model, const IntVector& i_d_l_bnds);

        /// return the inactive discrete upper bounds in userDefinedConstraints
        const IntVector& inactive_discrete_int_upper_bounds(const Model &model);

        /// set the inactive discrete upper bounds in userDefinedConstraints
        void inactive_discrete_int_upper_bounds(Model &model, const IntVector& i_d_u_bnds);

        /// return the inactive discrete lower bounds in userDefinedConstraints
        const RealVector& inactive_discrete_real_lower_bounds(const Model &model);

        /// set the inactive discrete lower bounds in userDefinedConstraints
        void inactive_discrete_real_lower_bounds(Model &model, const RealVector& i_d_l_bnds);

        /// return the inactive discrete upper bounds in userDefinedConstraints
        const RealVector& inactive_discrete_real_upper_bounds(Model &model);

        /// set the inactive discrete upper bounds in userDefinedConstraints
        void inactive_discrete_real_upper_bounds(Model &model, const RealVector& i_d_u_bnds);

        /// return all continuous lower bounds in userDefinedConstraints
        const RealVector& all_continuous_lower_bounds(const Model &model);

        /// set all continuous lower bounds in userDefinedConstraints
        void all_continuous_lower_bounds(Model &model, const RealVector& a_c_l_bnds);

        /// set a lower bound within continuous lower bounds in
        /// userDefinedConstraints
        void all_continuous_lower_bound(Model &model, Real a_c_l_bnd, size_t i);

        /// return all continuous upper bounds in userDefinedConstraints
        const RealVector& all_continuous_upper_bounds(const Model &model);

        /// set all continuous upper bounds in userDefinedConstraints
        void all_continuous_upper_bounds(Model &model, const RealVector& a_c_u_bnds);

        /// set an upper bound within all continuous upper bounds in
        /// userDefinedConstraints
        void all_continuous_upper_bound(Model &model, Real a_c_u_bnd, size_t i);

        /// return all discrete lower bounds in userDefinedConstraints
        const IntVector& all_discrete_int_lower_bounds(const Model &model);

        /// set all discrete lower bounds in userDefinedConstraints
        void all_discrete_int_lower_bounds(Model &model, const IntVector& a_d_l_bnds);

        /// set a lower bound within all discrete lower bounds in
        /// userDefinedConstraints
        void all_discrete_int_lower_bound(Model &model, int a_d_l_bnd, size_t i);

        /// return all discrete upper bounds in userDefinedConstraints
        const IntVector& all_discrete_int_upper_bounds(const Model &model);

        /// set all discrete upper bounds in userDefinedConstraints
        void all_discrete_int_upper_bounds(Model &model, const IntVector& a_d_u_bnds);

        /// set an upper bound within all discrete upper bounds in
        /// userDefinedConstraints
        void all_discrete_int_upper_bound(Model &model, int a_d_u_bnd, size_t i);

        /// return all discrete lower bounds in userDefinedConstraints
        const RealVector& all_discrete_real_lower_bounds(const Model &model);

        /// set all discrete lower bounds in userDefinedConstraints
        void all_discrete_real_lower_bounds(Model &model, const RealVector& a_d_l_bnds);

        /// set a lower bound within all discrete lower bounds in
        /// userDefinedConstraints
        void all_discrete_real_lower_bound(Model &model, Real a_d_l_bnd, size_t i);

        /// return all discrete upper bounds in userDefinedConstraints
        const RealVector& all_discrete_real_upper_bounds(const Model &model);

        /// set all discrete upper bounds in userDefinedConstraints
        void all_discrete_real_upper_bounds(Model &model, const RealVector& a_d_u_bnds);

        /// set an upper bound within all discrete upper bounds in
        /// userDefinedConstraints
        void all_discrete_real_upper_bound(Model &model, Real a_d_u_bnd, size_t i);


        /// return the number of linear inequality constraints   
        size_t num_linear_ineq_constraints(const Model &model);

        /// return the number of linear equality constraints
        size_t num_linear_eq_constraints(const Model &model);

        /// return the linear inequality constraint coefficients
        const RealMatrix& linear_ineq_constraint_coeffs(const Model &model);

        /// set the linear inequality constraint coefficients
        void linear_ineq_constraint_coeffs(Model &model, const RealMatrix& lin_ineq_coeffs);

        /// return the linear inequality constraint lower bounds
        const RealVector& linear_ineq_constraint_lower_bounds(const Model &model);

        /// set the linear inequality constraint lower bounds
        void linear_ineq_constraint_lower_bounds(Model &model, const RealVector& lin_ineq_l_bnds);

        /// return the linear inequality constraint upper bounds
        const RealVector& linear_ineq_constraint_upper_bounds(const Model &model);

        /// set the linear inequality constraint upper bounds
        void linear_ineq_constraint_upper_bounds(Model &model, const RealVector& lin_ineq_u_bnds);

        /// return the linear equality constraint coefficients
        const RealMatrix& linear_eq_constraint_coeffs(const Model &model);

        /// set the linear equality constraint coefficients
        void linear_eq_constraint_coeffs(Model &model, const RealMatrix& lin_eq_coeffs);

        /// return the linear equality constraint targets
        const RealVector& linear_eq_constraint_targets(const Model &model);

        /// set the linear equality constraint targets
        void linear_eq_constraint_targets(Model &model, const RealVector& lin_eq_targets);

        /// return the number of nonlinear inequality constraints
        size_t num_nonlinear_ineq_constraints(const Model &model);

        /// return the number of nonlinear equality constraints
        size_t num_nonlinear_eq_constraints(const Model &model);

        /// return the nonlinear inequality constraint lower bounds
        const RealVector& nonlinear_ineq_constraint_lower_bounds(const Model &model);

        /// set the nonlinear inequality constraint lower bounds
        void nonlinear_ineq_constraint_lower_bounds(Model &model, const RealVector& nln_ineq_l_bnds);

        /// return the nonlinear inequality constraint upper bounds
        const RealVector& nonlinear_ineq_constraint_upper_bounds(const Model &model);

        /// set the nonlinear inequality constraint upper bounds
        void nonlinear_ineq_constraint_upper_bounds(Model &model, const RealVector& nln_ineq_u_bnds);

        /// return the nonlinear equality constraint targets
        const RealVector& nonlinear_eq_constraint_targets(const Model &model);

        /// set the nonlinear equality constraint targets
        void nonlinear_eq_constraint_targets(Model &model, const RealVector& nln_eq_targets);

        /// construct the appropriate derived model type as given by the modelType attribute
        std::shared_ptr<Model> get_model(ProblemDescDB& problem_db, ParallelLibrary parallel_lib);

        /// dummy default empty string
        static const String empty_string = "";
    }
}
