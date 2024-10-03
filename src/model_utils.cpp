#include "model_utils.hpp"
#include "DakotaModel.hpp"
#include "MarginalsCorrDistribution.hpp"

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
                else
                    ardi_cntr += all_totals[TOTAL_DEUIV];
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


        const IntSetArray& discrete_set_int_values(Model &model) {
            discrete_set_int_values(model, model.current_variables().view().first);
        }

        const IntSetArray& discrete_set_int_values(Model &model, short active_view) {

            
            // aggregation of the admissible value sets for all active discrete
            // set integer variables
            IntSetArray adsi_vals;
            
            auto &mvd = model.multivariate_distribution();
            auto &cv = model.current_variables();

            std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
                std::static_pointer_cast<Pecos::MarginalsCorrDistribution>(mvd.multivar_dist_rep());
                
            const SharedVariablesData& svd = cv.shared_data();
            switch (active_view) {
                case MIXED_DESIGN: {
                    size_t num_rv = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
                        start_rv = svd.vc_lookup(CONTINUOUS_DESIGN)
                                + svd.vc_lookup(DISCRETE_DESIGN_RANGE);
                    mvd_rep->pull_parameters<IntSet>(start_rv, num_rv, Pecos::DSI_VALUES, adsi_vals);
                    break;
                }
                case MIXED_ALEATORY_UNCERTAIN: {
                    IntRealMapArray h_pt_prs;
                    mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
                    Pecos::H_PT_INT_PAIRS, h_pt_prs);
                    size_t i, num_dausiv = h_pt_prs.size();
                    adsi_vals.resize(num_dausiv);
                    for (i=0; i<num_dausiv; ++i)
                        map_keys_to_set(h_pt_prs[i], adsi_vals[i]);
                    break;
                }
                case MIXED_EPISTEMIC_UNCERTAIN: {
                    IntRealMapArray deusi_vals_probs;
                    mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
                    Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
                    size_t i, num_deusiv = deusi_vals_probs.size();
                    adsi_vals.resize(num_deusiv);
                    for (i=0; i<num_deusiv; ++i)
                        map_keys_to_set(deusi_vals_probs[i], adsi_vals[i]);
                    break;
                }
                case MIXED_UNCERTAIN: {
                    IntRealMapArray h_pt_prs, deusi_vals_probs;
                    mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
                    Pecos::H_PT_INT_PAIRS, h_pt_prs);
                    mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
                    Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
                    size_t i, num_dausiv = h_pt_prs.size(),num_deusiv = deusi_vals_probs.size();
                    adsi_vals.resize(num_dausiv+num_deusiv);
                    for (i=0; i<num_dausiv; ++i)
                        map_keys_to_set(h_pt_prs[i], adsi_vals[i]);
                    for (i=0; i<num_deusiv; ++i)
                        map_keys_to_set(deusi_vals_probs[i],adsi_vals[i+num_dausiv]);
                    break;
                }
                case MIXED_STATE: {
                    size_t num_cv, num_div, num_dsv, num_drv, start_rv = 0,
                    num_rv = svd.vc_lookup(DISCRETE_STATE_SET_INT);
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv
                    + svd.vc_lookup(CONTINUOUS_STATE) + svd.vc_lookup(DISCRETE_STATE_RANGE);
                    mvd_rep->pull_parameters<IntSet>(start_rv, num_rv, Pecos::DSI_VALUES,
                                    adsi_vals);
                    break;
                }
                case MIXED_ALL: {
                    IntRealMapArray h_pt_prs, deusi_vals_probs;
                    mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
                    Pecos::H_PT_INT_PAIRS, h_pt_prs);
                    mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
                    Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
                    size_t i, di_cntr = 0, num_ddsi = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
                    num_dausi = h_pt_prs.size(), num_deusi = deusi_vals_probs.size(),
                    num_dssi  = svd.vc_lookup(DISCRETE_STATE_SET_INT);
                    adsi_vals.resize(num_ddsi + num_dausi + num_deusi + num_dssi);
                    size_t num_cv, num_div, num_dsv, num_drv;
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    size_t rv_cntr = num_cv + num_div - num_ddsi;
                    for (i=0; i<num_ddsi; ++i, ++rv_cntr, ++di_cntr)
                        mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
                                    adsi_vals[di_cntr]);
                    rv_cntr += num_dsv + num_drv;
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    for (i=0; i<num_dausi; ++i, ++di_cntr)
                        map_keys_to_set(h_pt_prs[i], adsi_vals[di_cntr]);
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    for (i=0; i<num_deusi; ++i, ++di_cntr)
                        map_keys_to_set(deusi_vals_probs[i], adsi_vals[di_cntr]);
                    rv_cntr += num_cv + num_div + num_dsv + num_drv +
                    svd.vc_lookup(CONTINUOUS_STATE) + svd.vc_lookup(DISCRETE_STATE_RANGE);
                    for (i=0; i<num_dssi; ++i, ++rv_cntr, ++di_cntr)
                        mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
                                    adsi_vals[di_cntr]);
                    break;
                }
                default: { // RELAXED_*
                    const BitArray&    all_relax_di = svd.all_relaxed_discrete_int();
                    const SizetArray&    all_totals = svd.components_totals();
                    const SizetArray& active_totals = svd.active_components_totals();
                    size_t i, num_cv, num_div, num_dsv, num_drv,
                        di_cntr = 0, ardi_cntr = 0, rv_cntr = 0;      
                    // discrete design
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    if (active_totals[TOTAL_DDIV]) {
                        size_t num_ddsi = svd.vc_lookup(DISCRETE_DESIGN_SET_INT),
                            num_ddri = num_div - num_ddsi;
                        rv_cntr = num_cv;
                        for (i=0; i<num_ddri; ++i, ++ardi_cntr, ++rv_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                ++di_cntr;
                        for (i=0; i<num_ddsi; ++i, ++ardi_cntr, ++rv_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
                                        adsi_vals[di_cntr++]);
                        rv_cntr += num_dsv + num_drv;
                    }
                    else {
                        ardi_cntr += num_div;
                        rv_cntr   += num_cv + num_div + num_dsv + num_drv;
                    }
                    // discrete aleatory uncertain
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    if (active_totals[TOTAL_DAUIV]) {
                        IntRealMapArray h_pt_prs;
                        mvd_rep->pull_parameters<IntRealMap>(Pecos::HISTOGRAM_PT_INT,
                            Pecos::H_PT_INT_PAIRS, h_pt_prs);
                        size_t num_dausi = h_pt_prs.size(), num_dauri = num_div - num_dausi; 
                        for (i=0; i<num_dauri; ++i, ++ardi_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                ++di_cntr;
                        for (i=0; i<num_dausi; ++i, ++ardi_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                map_keys_to_set(h_pt_prs[i], adsi_vals[di_cntr++]);
                    }
                    else
                        ardi_cntr += num_div;
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    // discrete epistemic uncertain
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    if (active_totals[TOTAL_DEUIV]) {
                        IntRealMapArray deusi_vals_probs;
                        mvd_rep->pull_parameters<IntRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_INT,
                            Pecos::DUSI_VALUES_PROBS, deusi_vals_probs);
                        size_t num_deuri = svd.vc_lookup(DISCRETE_INTERVAL_UNCERTAIN),
                            num_deusi = deusi_vals_probs.size();
                        for (i=0; i<num_deuri; ++i, ++ardi_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                ++di_cntr;
                        for (i=0; i<num_deusi; ++i, ++ardi_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                map_keys_to_set(deusi_vals_probs[i],
                                    adsi_vals[di_cntr++]);
                    }
                    else
                        ardi_cntr += num_div;
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    // discrete state
                    if (active_totals[TOTAL_DSIV]) {
                        size_t num_dssi = svd.vc_lookup(DISCRETE_STATE_SET_INT),
                            num_dsri = all_totals[TOTAL_DSIV] - num_dssi;
                        rv_cntr += all_totals[TOTAL_CSV];
                        for (i=0; i<num_dsri; ++i, ++ardi_cntr, ++rv_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                ++di_cntr;                  // leave bit as false
                        for (i=0; i<num_dssi; ++i, ++ardi_cntr, ++rv_cntr)
                            if (!all_relax_di[ardi_cntr]) // part of active discrete vars
                                mvd_rep->pull_parameter<IntSet>(rv_cntr, Pecos::DSI_VALUES,
                                        adsi_vals[di_cntr++]);
                    }
                    break;
                }
            }
            return adsi_vals;
        }


    }
}