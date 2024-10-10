#pragma once
#include "dakota_data_types.hpp"


namespace Dakota {
    class Model;
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


        size_t tv(const Model &model);

        size_t cv(const Model &model);

        size_t div(const Model &model);

        size_t dsv(const Model &model);

        size_t drv(const Model &model);

        size_t icv(const Model &model);

        size_t idiv(const Model &model);

        size_t idsv(const Model &model);

        size_t idrv(const Model &model);

        size_t acv(const Model &model);

        size_t adiv(const Model &model);

        size_t adsv(const Model &model);

        size_t adrv(const Model &model);

        void active_variables(Model &model, const Variables& vars);

        void inactive_variables(Model &model, const Variables& vars);

        const RealVector& continuous_variables(const Model &model);

        Real continuous_variable(const Model &model, size_t i);

        void continuous_variables(Model &model, const RealVector& c_vars);

        void continuous_variable(Model &model, Real c_var, size_t i);

        const IntVector& discrete_int_variables(const Model &model);

        int discrete_int_variable(const Model &model, size_t i);

        void discrete_int_variables(Model &model, const IntVector& d_vars);

        void discrete_int_variable(Model &model, int d_var, size_t i);

        StringMultiArrayConstView discrete_string_variables(const Model &model);

        const String& discrete_string_variable(const Model &model, size_t i);

        void discrete_string_variables(Model &model, StringMultiArrayConstView d_vars);

        void discrete_string_variable(Model &model, const String& d_var, size_t i);

        const RealVector& discrete_real_variables(const Model &model);

        Real discrete_real_variable(const Model &model, size_t i);

        void discrete_real_variables(Model &model, const RealVector& d_vars);

        void discrete_real_variable(Model &model, Real d_var, size_t i);

        UShortMultiArrayConstView continuous_variable_types(const Model &model);

        void continuous_variable_types(Model &model, UShortMultiArrayConstView cv_types);

        void continuous_variable_type(Model &model, unsigned short cv_type, size_t i);

        UShortMultiArrayConstView discrete_int_variable_types(const Model &model);

        void discrete_int_variable_types(Model &model, UShortMultiArrayConstView div_types);

        void discrete_int_variable_type(Model &model, unsigned short div_type, size_t i);

        UShortMultiArrayConstView discrete_string_variable_types(const Model &model);

        void discrete_string_variable_types(Model &model, UShortMultiArrayConstView dsv_types);

        void discrete_string_variable_type(Model &model, unsigned short dsv_type, size_t i);

        UShortMultiArrayConstView discrete_real_variable_types(const Model &model);

        void discrete_real_variable_types(Model &model, UShortMultiArrayConstView drv_types);

        void discrete_real_variable_type(Model &model, unsigned short drv_type, size_t i);

        SizetMultiArrayConstView continuous_variable_ids(const Model &model);

        void continuous_variable_ids(Model &model, SizetMultiArrayConstView cv_ids);

        void continuous_variable_id(Model &model, size_t cv_id, size_t i);

        const RealVector& inactive_continuous_variables(const Model &model);

        void inactive_continuous_variables(Model &model, const RealVector& i_c_vars);

        const IntVector& inactive_discrete_int_variables(const Model &model);

        void inactive_discrete_int_variables(Model &model, const IntVector& i_d_vars);

        StringMultiArrayConstView inactive_discrete_string_variables(const Model &model);

        void inactive_discrete_string_variables(Model &model, StringMultiArrayConstView i_d_vars);

        const RealVector& inactive_discrete_real_variables(const Model &model);

        void inactive_discrete_real_variables(Model &model, const RealVector& i_d_vars);

        UShortMultiArrayConstView inactive_continuous_variable_types(const Model &model);

        SizetMultiArrayConstView inactive_continuous_variable_ids(const Model &model);

        const RealVector& all_continuous_variables(const Model &model);

        void all_continuous_variables(Model &model, const RealVector& a_c_vars);

        void all_continuous_variable(Model &model, Real a_c_var, size_t i);

        const IntVector& all_discrete_int_variables(const Model &model);

        void all_discrete_int_variables(Model &model, const IntVector& a_d_vars);

        void all_discrete_int_variable(Model &model, int a_d_var, size_t i);

        StringMultiArrayConstView all_discrete_string_variables(const Model &model);

        void all_discrete_string_variables(Model &model, StringMultiArrayConstView a_d_vars);

        void all_discrete_string_variable(Model &model, const String& a_d_var, size_t i);

        const RealVector& all_discrete_real_variables(const Model &model);

        void all_discrete_real_variables(Model &model, const RealVector& a_d_vars);

        void all_discrete_real_variable(Model &model, Real a_d_var, size_t i);

        UShortMultiArrayConstView all_continuous_variable_types(const Model &model);

        UShortMultiArrayConstView all_discrete_int_variable_types(const Model &model);

        UShortMultiArrayConstView all_discrete_string_variable_types(const Model &model);

        UShortMultiArrayConstView all_discrete_real_variable_types(const Model &model);

        SizetMultiArrayConstView all_continuous_variable_ids(const Model &model);

        StringMultiArrayConstView continuous_variable_labels(const Model &model);

        void continuous_variable_labels(Model &model, StringMultiArrayConstView c_v_labels);

        StringMultiArrayConstView discrete_int_variable_labels(const Model &model);

        void discrete_int_variable_labels(Model &model, StringMultiArrayConstView d_v_labels);

        StringMultiArrayConstView discrete_string_variable_labels(const Model &model);

        void discrete_string_variable_labels(Model &model, StringMultiArrayConstView d_v_labels);

        StringMultiArrayConstView discrete_real_variable_labels(const Model &model);

        void discrete_real_variable_labels(Model &model, StringMultiArrayConstView d_v_labels);

        StringMultiArrayConstView inactive_continuous_variable_labels(const Model &model);

        void inactive_continuous_variable_labels(Model &model, StringMultiArrayConstView i_c_v_labels);

        StringMultiArrayConstView inactive_discrete_int_variable_labels(const Model &model);

        void inactive_discrete_int_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels);

        StringMultiArrayConstView inactive_discrete_string_variable_labels(const Model &model);

        void inactive_discrete_string_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels);

        StringMultiArrayConstView inactive_discrete_real_variable_labels(const Model &model);

        void inactive_discrete_real_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels);

        StringMultiArrayConstView all_continuous_variable_labels(const Model &model);

        void all_continuous_variable_labels(Model &model, StringMultiArrayConstView a_c_v_labels);

        void all_continuous_variable_label(Model &model, const String& a_c_v_label, size_t i);

        StringMultiArrayConstView all_discrete_int_variable_labels(const Model &model);

        void all_discrete_int_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels);

        void all_discrete_int_variable_label(Model &model, const String& a_d_v_label, size_t i);

        StringMultiArrayConstView all_discrete_string_variable_labels(const Model &model);

        void all_discrete_string_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels);

        void all_discrete_string_variable_label(Model &model, const String& a_d_v_label, size_t i);

        StringMultiArrayConstView all_discrete_real_variable_labels(const Model &model);

        void all_discrete_real_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels);

        void all_discrete_real_variable_label(Model &model, const String& a_d_v_label, size_t i);

        size_t response_size(const Model &model);

        const StringArray& response_labels(const Model &model);

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
    }
}
