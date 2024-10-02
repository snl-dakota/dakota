#include "model_utils.hpp"
#include "DakotaModel.hpp"

namespace Dakota {
    namespace ModelUtils {

        const BitArray& discrete_int_sets(Model &model) {
            Variables &cv = model.current_variables();
            return discrete_int_sets(model, cv.view().first);
        }


        const BitArray& discrete_int_sets(Model &model, short active_view) {
            // identify discrete integer sets within active discrete int variables
            // (excluding discrete integer ranges)
            Variables &cv = model.current_variables();

            bool relax = (active_view == RELAXED_ALL ||
                ( active_view >= RELAXED_DESIGN && active_view <= RELAXED_STATE ) );
            const SharedVariablesData&  svd = cv.shared_data();
            const SizetArray& active_totals = svd.active_components_totals();

            BitArray dis;
            dis.resize(cv.div());
            dis.reset();
            size_t i, di_cntr = 0;
            if (relax) {
                // This case is complicated by the promotion of active discrete variables
                // into active continuous variables.  all_relax_di and ardi_cntr operate
                // over all of the discrete variables from the input specification, but
                // discreteIntSets operates only over the non-relaxed/categorical active
                // discrete variables, for which it distinguishes sets from ranges.
                const BitArray& all_relax_di = svd.all_relaxed_discrete_int();
                const SizetArray& all_totals = svd.components_totals();
                size_t ardi_cntr = 0;
                // discrete design
                if (active_totals[TOTAL_DDIV]) {
                    size_t num_ddriv = svd.vc_lookup(DISCRETE_DESIGN_RANGE),
                        num_ddsiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT);
                    for (i=0; i<num_ddriv; ++i, ++ardi_cntr)
                        if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                            ++di_cntr;                  // leave bit as false
                        for (i=0; i<num_ddsiv; ++i, ++ardi_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                { dis.set(di_cntr); ++di_cntr; } // set bit to true
                }
                else
                    ardi_cntr += all_totals[TOTAL_DDIV];
                // discrete aleatory uncertain
                if (active_totals[TOTAL_DAUIV]) {
                    size_t num_dausiv = svd.vc_lookup(HISTOGRAM_POINT_UNCERTAIN_INT),
                        num_dauriv = all_totals[TOTAL_DAUIV] - num_dausiv; 
                    for (i=0; i<num_dauriv; ++i, ++ardi_cntr)
                        if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                            ++di_cntr;                  // leave bit as false
                    for (i=0; i<num_dausiv; ++i, ++ardi_cntr)
                        if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                            { dis.set(di_cntr); ++di_cntr; } // set bit to true
                }
                else
                    ardi_cntr += all_totals[TOTAL_DAUIV];
                // discrete epistemic uncertain
                if (active_totals[TOTAL_DEUIV]) {
                size_t num_deuriv = svd.vc_lookup(DISCRETE_INTERVAL_UNCERTAIN),
                    num_deusiv = svd.vc_lookup(DISCRETE_UNCERTAIN_SET_INT);
                for (i=0; i<num_deuriv; ++i, ++ardi_cntr)
                if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                ++di_cntr;                  // leave bit as false
                for (i=0; i<num_deusiv; ++i, ++ardi_cntr)
                if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                { dis.set(di_cntr); ++di_cntr; } // set bit to true
                }
                else ardi_cntr += all_totals[TOTAL_DEUIV];
                // discrete state
                if (active_totals[TOTAL_DSIV]) {
                size_t num_dsriv = svd.vc_lookup(DISCRETE_STATE_RANGE),
                    num_dssiv = svd.vc_lookup(DISCRETE_STATE_SET_INT);
                for (i=0; i<num_dsriv; ++i, ++ardi_cntr)
                if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                ++di_cntr;                  // leave bit as false
                for (i=0; i<num_dssiv; ++i, ++ardi_cntr)
                if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                { dis.set(di_cntr); ++di_cntr; } // set bit to true
                }
            }
            else { // MIXED_*
                size_t num_ddiv, num_dauiv, num_deuiv, num_dsiv;
                if (num_ddiv = active_totals[TOTAL_DDIV]) {
                        size_t set_ddiv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT);
                        di_cntr += num_ddiv - set_ddiv;//svd.vc_lookup(DISCRETE_DESIGN_RANGE)
                        for (i=0; i<set_ddiv; ++i, ++di_cntr)
                        dis.set(di_cntr);
                }
                if (num_dauiv = active_totals[TOTAL_DAUIV]) {
                    size_t set_dauiv = svd.vc_lookup(HISTOGRAM_POINT_UNCERTAIN_INT);
                    di_cntr += num_dauiv - set_dauiv; // range_dauiv
                    for (i=0; i<set_dauiv; ++i, ++di_cntr)
                        dis.set(di_cntr);
                }
                if (num_deuiv = active_totals[TOTAL_DEUIV]) {
                    size_t set_deuiv = svd.vc_lookup(DISCRETE_UNCERTAIN_SET_INT);
                    di_cntr += num_deuiv - set_deuiv;//vc_lookup(DISCRETE_INTERVAL_UNCERTAIN)
                    for (i=0; i<set_deuiv; ++i, ++di_cntr)
                        dis.set(di_cntr);
                }
                if (num_dsiv = active_totals[TOTAL_DSIV]) {
                    size_t set_dsiv = svd.vc_lookup(DISCRETE_STATE_SET_INT);
                    di_cntr += num_dsiv - set_dsiv;//svd.vc_lookup(DISCRETE_STATE_RANGE)
                    for (i=0; i<set_dsiv; ++i, ++di_cntr)
                        dis.set(di_cntr);
                }
            }

            return dis;
        }

    }
}