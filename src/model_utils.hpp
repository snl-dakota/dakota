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
