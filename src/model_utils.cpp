#include "model_utils.hpp"
#include "DakotaModel.hpp"
#include "MarginalsCorrDistribution.hpp"

namespace Dakota {
    namespace ModelUtils {

        BitArray discrete_int_sets(const Model &model) {
            const Variables &cv = model.current_variables();
            return discrete_int_sets(model, cv.view().first);
        }


        BitArray discrete_int_sets(const Model &model, short active_view) {
            // identify discrete integer sets within active discrete int variables
            // (excluding discrete integer ranges)
            const Variables &cv = model.current_variables();

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


        IntSetArray discrete_set_int_values(const Model &model) {
            return discrete_set_int_values(model, model.current_variables().view().first);
        }

        IntSetArray discrete_set_int_values(const Model &model, short active_view) {

            
            // aggregation of the admissible value sets for all active discrete
            // set integer variables
            IntSetArray adsi_vals;
            
            const auto &mvd = model.multivariate_distribution();
            const auto &cv = model.current_variables();

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

        StringSetArray discrete_set_string_values(const Model &model) {
                return discrete_set_string_values(model, model.current_variables().view().first);
        }

        StringSetArray discrete_set_string_values(const Model &model, short active_view) {

            // aggregation of the admissible value sets for all active discrete
            // set string variables
            StringSetArray adss_vals;

            const auto &mvd = model.multivariate_distribution();
            const auto &cv = model.current_variables();

            std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
                std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
                (mvd.multivar_dist_rep());
            const SharedVariablesData& svd = cv.shared_data();
            switch (active_view) {
                case MIXED_DESIGN: case RELAXED_DESIGN: {
                    size_t num_cv, num_div, num_dsv, num_drv;      
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    mvd_rep->pull_parameters<StringSet>(num_cv + num_div, num_dsv,
                    Pecos::DSS_VALUES, adss_vals);
                    break;
                }
                case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN: {
                    StringRealMapArray h_pt_prs;
                    mvd_rep->pull_parameters<StringRealMap>(Pecos::HISTOGRAM_PT_STRING,
                    Pecos::H_PT_STR_PAIRS, h_pt_prs);
                    size_t i, num_dauss = h_pt_prs.size();
                    adss_vals.resize(num_dauss);
                    for (i=0; i<num_dauss; ++i)
                        map_keys_to_set(h_pt_prs[i], adss_vals[i]);
                    break;
                }
                case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN: {
                    StringRealMapArray deuss_vals_probs;
                    mvd_rep->pull_parameters<StringRealMap>(
                    Pecos::DISCRETE_UNCERTAIN_SET_STRING, Pecos::DUSS_VALUES_PROBS,
                    deuss_vals_probs);
                    size_t i, num_deuss = deuss_vals_probs.size();
                    adss_vals.resize(num_deuss);
                    for (i=0; i<num_deuss; ++i)
                        map_keys_to_set(deuss_vals_probs[i], adss_vals[i]);
                    break;
                }
                case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
                    StringRealMapArray h_pt_prs, deuss_vals_probs;
                    mvd_rep->pull_parameters<StringRealMap>(Pecos::HISTOGRAM_PT_STRING,
                    Pecos::H_PT_STR_PAIRS, h_pt_prs);
                    mvd_rep->pull_parameters<StringRealMap>(
                    Pecos::DISCRETE_UNCERTAIN_SET_STRING,
                    Pecos::DUSS_VALUES_PROBS, deuss_vals_probs);
                    size_t i, num_dauss = h_pt_prs.size(), num_deuss = deuss_vals_probs.size();
                    adss_vals.resize(num_dauss + num_deuss);
                    for (i=0; i<num_dauss; ++i)
                        map_keys_to_set(h_pt_prs[i], adss_vals[i]);
                    for (i=0; i<num_deuss; ++i)
                        map_keys_to_set(deuss_vals_probs[i],
                            adss_vals[i+num_dauss]);
                    break;
                }
                case MIXED_STATE: case RELAXED_STATE: {
                    size_t num_cv, num_div, num_dsv, num_drv, start_rv = 0;
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div;
                    mvd_rep->pull_parameters<StringSet>(start_rv, num_dsv, Pecos::DSS_VALUES,
                                    adss_vals);
                    break;
                }
                case MIXED_ALL: case RELAXED_ALL: {
                    StringRealMapArray h_pt_prs, deuss_vals_probs;
                    mvd_rep->pull_parameters<StringRealMap>(Pecos::HISTOGRAM_PT_STRING,
                    Pecos::H_PT_STR_PAIRS, h_pt_prs);
                    mvd_rep->pull_parameters<StringRealMap>(
                    Pecos::DISCRETE_UNCERTAIN_SET_STRING, Pecos::DUSS_VALUES_PROBS,
                    deuss_vals_probs);
                    size_t i, ds_cntr = 0,
                    num_ddss  = svd.vc_lookup(DISCRETE_DESIGN_SET_STRING),
                    num_dauss = h_pt_prs.size(), num_deuss= deuss_vals_probs.size(),
                    num_dsss  = svd.vc_lookup(DISCRETE_STATE_SET_STRING);
                    adss_vals.resize(num_ddss + num_dauss +
                                    num_deuss + num_dsss);
                    size_t num_cv, num_div, num_dsv, num_drv;
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    size_t rv_cntr = num_cv + num_div;
                    for (i=0; i<num_ddss; ++i, ++rv_cntr, ++ds_cntr)
                        mvd_rep->pull_parameter<StringSet>(rv_cntr, Pecos::DSS_VALUES,
                                    adss_vals[ds_cntr]);
                    rv_cntr += num_drv;
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    for (i=0; i<num_dauss; ++i, ++ds_cntr)
                        map_keys_to_set(h_pt_prs[i], adss_vals[ds_cntr]);
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    for (i=0; i<num_deuss; ++i, ++ds_cntr)
                        map_keys_to_set(deuss_vals_probs[i], adss_vals[ds_cntr]);
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
                    rv_cntr += num_cv + num_div;
                    for (i=0; i<num_dsss; ++i, ++rv_cntr, ++ds_cntr)
                        mvd_rep->pull_parameter<StringSet>(rv_cntr, Pecos::DSS_VALUES,
                                    adss_vals[ds_cntr]);
                    break;
                }
            }


            return adss_vals;
        }


        RealSetArray discrete_set_real_values(const Model &model) {
                return discrete_set_real_values(model, model.current_variables().view().first);
        }

        RealSetArray discrete_set_real_values(const Model &model, short active_view) {
            // aggregation of the admissible value sets for all active discrete
            // set real variables
            RealSetArray adsr_vals;

            const auto &mvd = model.multivariate_distribution();
            const auto &cv = model.current_variables();


            std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
                std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
                (mvd.multivar_dist_rep());
            const SharedVariablesData& svd = cv.shared_data();
            switch (active_view) {
                case MIXED_DESIGN: {
                    size_t num_cv, num_div, num_dsv, num_drv;      
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    mvd_rep->pull_parameters<RealSet>(num_cv + num_div + num_dsv, num_drv,
                    Pecos::DSR_VALUES, adsr_vals);
                    break;
                }
                case MIXED_ALEATORY_UNCERTAIN: {
                    RealRealMapArray h_pt_prs;
                    mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
                    Pecos::H_PT_REAL_PAIRS, h_pt_prs);
                    size_t i, num_dausr = h_pt_prs.size();
                    adsr_vals.resize(num_dausr);
                    for (i=0; i<num_dausr; ++i)
                        map_keys_to_set(h_pt_prs[i], adsr_vals[i]);
                    break;
                }
                case MIXED_EPISTEMIC_UNCERTAIN: {
                    RealRealMapArray deusr_vals_probs;
                    mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
                    Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
                    size_t i, num_deusr = deusr_vals_probs.size();
                    adsr_vals.resize(num_deusr);
                    for (i=0; i<num_deusr; ++i)
                        map_keys_to_set(deusr_vals_probs[i], adsr_vals[i]);
                    break;
                }
                case MIXED_UNCERTAIN: {
                    RealRealMapArray h_pt_prs, deusr_vals_probs;
                    mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
                    Pecos::H_PT_REAL_PAIRS, h_pt_prs);
                    mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
                    Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
                    size_t i, num_dausr = h_pt_prs.size(), num_deusr = deusr_vals_probs.size();
                    adsr_vals.resize(num_dausr + num_deusr);
                    for (i=0; i<num_dausr; ++i)
                        map_keys_to_set(h_pt_prs[i], adsr_vals[i]);
                    for (i=0; i<num_deusr; ++i)
                        map_keys_to_set(deusr_vals_probs[i],adsr_vals[i+num_dausr]);
                    break;
                }
                case MIXED_STATE: {
                    size_t num_cv, num_div, num_dsv, num_drv, start_rv = 0;
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv + num_drv;
                    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
                    start_rv += num_cv + num_div + num_dsv;
                    mvd_rep->pull_parameters<RealSet>(start_rv, num_drv, Pecos::DSR_VALUES,
                                    adsr_vals);
                    break;
                }
                case MIXED_ALL: {
                    RealRealMapArray h_pt_prs, deusr_vals_probs;
                    mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
                    Pecos::H_PT_REAL_PAIRS, h_pt_prs);
                    mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
                    Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
                    size_t i, dr_cntr = 0, num_dausr = h_pt_prs.size(),
                    num_deusr = deusr_vals_probs.size(),
                    num_dssr  = svd.vc_lookup(DISCRETE_STATE_SET_REAL);
                    size_t num_cv, num_div, num_dsv, num_drv;
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    adsr_vals.resize(num_drv + num_dausr + num_deusr + num_dssr);
                    size_t rv_cntr = num_cv + num_div + num_dsv;
                    for (i=0; i<num_drv; ++i, ++rv_cntr, ++dr_cntr)
                        mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
                                    adsr_vals[dr_cntr]);
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    for (i=0; i<num_dausr; ++i, ++dr_cntr)
                        map_keys_to_set(h_pt_prs[i], adsr_vals[dr_cntr]);
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    for (i=0; i<num_deusr; ++i, ++dr_cntr)
                        map_keys_to_set(deusr_vals_probs[i], adsr_vals[dr_cntr]);
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
                    rv_cntr += num_cv + num_div + num_dsv;
                    for (i=0; i<num_drv; ++i, ++rv_cntr, ++dr_cntr)
                        mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
                                    adsr_vals[dr_cntr]);
                    break;
                }
                default: { // RELAXED_*
                    const BitArray&    all_relax_dr = svd.all_relaxed_discrete_real();
                    const SizetArray&    all_totals = svd.components_totals();
                    const SizetArray& active_totals = svd.active_components_totals();
                    size_t i, num_cv, num_div, num_dsv, num_drv,
                        dr_cntr = 0, ardr_cntr = 0, rv_cntr = 0;
                    // discrete design
                    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
                    if (active_totals[TOTAL_DDRV]) {
                        rv_cntr = num_cv + num_div + num_dsv;
                        for (i=0; i<num_drv; ++i, ++ardr_cntr, ++rv_cntr)
                            if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
                                mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
                                        adsr_vals[dr_cntr++]);
                    } else {
                        ardr_cntr += num_drv;
                        rv_cntr   += num_cv + num_div + num_dsv + num_drv;
                    }
                    // discrete aleatory uncertain
                    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    if (active_totals[TOTAL_DAURV]) {
                        RealRealMapArray h_pt_prs;
                        mvd_rep->pull_parameters<RealRealMap>(Pecos::HISTOGRAM_PT_REAL,
                            Pecos::H_PT_REAL_PAIRS, h_pt_prs);
                        size_t num_dausr = h_pt_prs.size(); 
                        for (i=0; i<num_dausr; ++i, ++ardr_cntr)
                            if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
                                map_keys_to_set(h_pt_prs[i], adsr_vals[dr_cntr++]);
                    } else
                        ardr_cntr += num_drv;
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    // discrete epistemic uncertain
                    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
                    if (active_totals[TOTAL_DEURV]) {
                        RealRealMapArray deusr_vals_probs;
                        mvd_rep->pull_parameters<RealRealMap>(Pecos::DISCRETE_UNCERTAIN_SET_REAL,
                            Pecos::DUSR_VALUES_PROBS, deusr_vals_probs);
                        size_t num_deusr = deusr_vals_probs.size();
                        for (i=0; i<num_deusr; ++i, ++ardr_cntr)
                            if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
                                map_keys_to_set(deusr_vals_probs[i],
                                    adsr_vals[dr_cntr++]);
                    } else
                        ardr_cntr += num_drv;
                    rv_cntr += num_cv + num_div + num_dsv + num_drv;
                    // discrete state
                    if (active_totals[TOTAL_DSRV]) {
                        svd.state_counts(num_cv, num_div, num_dsv, num_drv);
                        rv_cntr += num_cv + num_div + num_dsv;
                        for (i=0; i<num_drv; ++i, ++ardr_cntr, ++rv_cntr)
                            if (!all_relax_dr[ardr_cntr]) // part of active discrete vars
                                mvd_rep->pull_parameter<RealSet>(rv_cntr, Pecos::DSR_VALUES,
                                        adsr_vals[dr_cntr++]);
                    }
                    break;
                }
            }

            return adsr_vals;
        }


        size_t tv(const Model &model) {
            return model.current_variables().tv();
        }

        size_t cv(const Model &model) {
            return model.current_variables().cv();
        }

        size_t div(const Model &model) {
            return model.current_variables().div();
        }

        size_t dsv(const Model &model) {
            return model.current_variables().dsv();
        }

        size_t drv(const Model &model) {
            return model.current_variables().drv();
        }

        size_t icv(const Model &model) {
            return model.current_variables().icv();
        }

        size_t idiv(const Model &model) {
            return model.current_variables().idiv();
        }

        size_t idsv(const Model &model) {
            return model.current_variables().idsv();
        }

        size_t idrv(const Model &model) {
            return model.current_variables().idrv();
        }

        size_t acv(const Model &model) {
            return model.current_variables().acv();
        }

        size_t adiv(const Model &model) {
            return model.current_variables().adiv();
        }

        size_t adsv(const Model &model) {
            return model.current_variables().adsv();
        }

        size_t adrv(const Model &model) {
            return model.current_variables().adrv();
        }

        void active_variables(Model &model, const Variables& vars) {
            model.current_variables().active_variables(vars);
        }

        void inactive_variables(Model &model, const Variables& vars) {
            model.current_variables().inactive_variables(vars);
        }

        const RealVector& continuous_variables(const Model &model) {
            return model.current_variables().continuous_variables();
        }

        Real continuous_variable(const Model &model, size_t i) {
            return model.current_variables().continuous_variable(i);
        }

        void continuous_variables(Model &model, const RealVector& c_vars) {
            model.current_variables().continuous_variables(c_vars);
        }

        void continuous_variable(Model &model, Real c_var, size_t i) {
            model.current_variables().continuous_variable(c_var, i);
        }

        const IntVector& discrete_int_variables(const Model &model) {
            return model.current_variables().discrete_int_variables();
        }

        int discrete_int_variable(const Model &model, size_t i) {
            return model.current_variables().discrete_int_variable(i);
        }

        void discrete_int_variables(Model &model, const IntVector& d_vars) {
            model.current_variables().discrete_int_variables(d_vars);
        }

        void discrete_int_variable(Model &model, int d_var, size_t i) {
            model.current_variables().discrete_int_variable(d_var, i);
        }

        StringMultiArrayConstView discrete_string_variables(const Model &model) {
            return model.current_variables().discrete_string_variables();
        }

        const String& discrete_string_variable(const Model &model, size_t i) {
            return model.current_variables().discrete_string_variable(i);
        }

        void discrete_string_variables(Model &model, StringMultiArrayConstView d_vars) {
            model.current_variables().discrete_string_variables(d_vars);
        }

        void discrete_string_variable(Model &model, const String& d_var, size_t i) {
            model.current_variables().discrete_string_variable(d_var, i);
        }

        const RealVector& discrete_real_variables(const Model &model) {
            return model.current_variables().discrete_real_variables();
        }

        Real discrete_real_variable(const Model &model, size_t i) {
            return model.current_variables().discrete_real_variable(i);
        }

        void discrete_real_variables(Model &model, const RealVector& d_vars) {
            model.current_variables().discrete_real_variables(d_vars);
        }

        void discrete_real_variable(Model &model, Real d_var, size_t i) {
            model.current_variables().discrete_real_variable(d_var, i);
        }

        UShortMultiArrayConstView continuous_variable_types(const Model &model) {
            return model.current_variables().continuous_variable_types();
        }

        void continuous_variable_types(Model &model, UShortMultiArrayConstView cv_types) {
            model.current_variables().continuous_variable_types(cv_types);
        }

        void continuous_variable_type(Model &model, unsigned short cv_type, size_t i) {
            model.current_variables().continuous_variable_type(cv_type, i);
        }

        UShortMultiArrayConstView discrete_int_variable_types(const Model &model) {
            return model.current_variables().discrete_int_variable_types();
        }

        void discrete_int_variable_types(Model &model, UShortMultiArrayConstView div_types) {
            model.current_variables().discrete_int_variable_types(div_types);
        }

        void discrete_int_variable_type(Model &model, unsigned short div_type, size_t i) {
            model.current_variables().discrete_int_variable_type(div_type, i);
        }

        UShortMultiArrayConstView discrete_string_variable_types(const Model &model) {
            return model.current_variables().discrete_string_variable_types();
        }

        void discrete_string_variable_types(Model &model, UShortMultiArrayConstView dsv_types) {
            model.current_variables().discrete_string_variable_types(dsv_types);
        }

        void discrete_string_variable_type(Model &model, unsigned short dsv_type, size_t i) {
            model.current_variables().discrete_string_variable_type(dsv_type, i);
        }

        UShortMultiArrayConstView discrete_real_variable_types(const Model &model) {
            return model.current_variables().discrete_real_variable_types();
        }

        void discrete_real_variable_types(Model &model, UShortMultiArrayConstView drv_types) {
            model.current_variables().discrete_real_variable_types(drv_types);
        }

        void discrete_real_variable_type(Model &model, unsigned short drv_type, size_t i) {
            model.current_variables().discrete_real_variable_type(drv_type, i);
        }

        SizetMultiArrayConstView continuous_variable_ids(const Model &model) {
            return model.current_variables().continuous_variable_ids();
        }

        void continuous_variable_ids(Model &model, SizetMultiArrayConstView cv_ids) {
            model.current_variables().continuous_variable_ids(cv_ids);
        }

        void continuous_variable_id(Model &model, size_t cv_id, size_t i) {
            model.current_variables().continuous_variable_id(cv_id, i);
        }

        const RealVector& inactive_continuous_variables(const Model &model) {
            return model.current_variables().inactive_continuous_variables();
        }

        void inactive_continuous_variables(Model &model, const RealVector& i_c_vars) {
            model.current_variables().inactive_continuous_variables(i_c_vars);
        }

        const IntVector& inactive_discrete_int_variables(const Model &model) {
            return model.current_variables().inactive_discrete_int_variables();
        }

        void inactive_discrete_int_variables(Model &model, const IntVector& i_d_vars) {
            model.current_variables().inactive_discrete_int_variables(i_d_vars);
        }

        StringMultiArrayConstView inactive_discrete_string_variables(const Model &model) {
            return model.current_variables().inactive_discrete_string_variables();
        }

        void inactive_discrete_string_variables(Model &model, StringMultiArrayConstView i_d_vars) {
            model.current_variables().inactive_discrete_string_variables(i_d_vars);
        }

        const RealVector& inactive_discrete_real_variables(const Model &model) {
            return model.current_variables().inactive_discrete_real_variables();
        }

        void inactive_discrete_real_variables(Model &model, const RealVector& i_d_vars) {
            model.current_variables().inactive_discrete_real_variables(i_d_vars);
        }

        UShortMultiArrayConstView inactive_continuous_variable_types(const Model &model) {
            return model.current_variables().inactive_continuous_variable_types();
        }

        SizetMultiArrayConstView inactive_continuous_variable_ids(const Model &model) {
            return model.current_variables().inactive_continuous_variable_ids();
        }

        const RealVector& all_continuous_variables(const Model &model) {
            return model.current_variables().all_continuous_variables();
        }

        void all_continuous_variables(Model &model, const RealVector& a_c_vars) {
            model.current_variables().all_continuous_variables(a_c_vars);
        }

        void all_continuous_variable(Model &model, Real a_c_var, size_t i) {
            model.current_variables().all_continuous_variable(a_c_var, i);
        }

        const IntVector& all_discrete_int_variables(const Model &model) {
            return model.current_variables().all_discrete_int_variables();
        }

        void all_discrete_int_variables(Model &model, const IntVector& a_d_vars) {
            model.current_variables().all_discrete_int_variables(a_d_vars);
        }

        void all_discrete_int_variable(Model &model, int a_d_var, size_t i) {
            model.current_variables().all_discrete_int_variable(a_d_var, i);
        }

        StringMultiArrayConstView all_discrete_string_variables(const Model &model) {
            return model.current_variables().all_discrete_string_variables();
        }

        void all_discrete_string_variables(Model &model, StringMultiArrayConstView a_d_vars) {
            model.current_variables().all_discrete_string_variables(a_d_vars);
        }

        void all_discrete_string_variable(Model &model, const String& a_d_var, size_t i) {
            model.current_variables().all_discrete_string_variable(a_d_var, i);
        }

        const RealVector& all_discrete_real_variables(const Model &model) {
            return model.current_variables().all_discrete_real_variables();
        }

        void all_discrete_real_variables(Model &model, const RealVector& a_d_vars) {
            model.current_variables().all_discrete_real_variables(a_d_vars);
        }

        void all_discrete_real_variable(Model &model, Real a_d_var, size_t i) {
            model.current_variables().all_discrete_real_variable(a_d_var, i);
        }

        UShortMultiArrayConstView all_continuous_variable_types(const Model &model) {
            return model.current_variables().all_continuous_variable_types();
        }

        UShortMultiArrayConstView all_discrete_int_variable_types(const Model &model) {
            return model.current_variables().all_discrete_int_variable_types();
        }

        UShortMultiArrayConstView all_discrete_string_variable_types(const Model &model) {
            return model.current_variables().all_discrete_string_variable_types();
        }

        UShortMultiArrayConstView all_discrete_real_variable_types(const Model &model) {
            return model.current_variables().all_discrete_real_variable_types();
        }

        SizetMultiArrayConstView all_continuous_variable_ids(const Model &model) {
            return model.current_variables().all_continuous_variable_ids();
        }


        StringMultiArrayConstView continuous_variable_labels(const Model &model) {
            return model.current_variables().continuous_variable_labels();
        }

        void continuous_variable_labels(Model &model, StringMultiArrayConstView c_v_labels) {
            model.current_variables().continuous_variable_labels(c_v_labels);
        }

        StringMultiArrayConstView discrete_int_variable_labels(const Model &model) {
            return model.current_variables().discrete_int_variable_labels();
        }

        void discrete_int_variable_labels(Model &model, StringMultiArrayConstView d_v_labels) {
            model.current_variables().discrete_int_variable_labels(d_v_labels);
        }

        StringMultiArrayConstView discrete_string_variable_labels(const Model &model) {
            return model.current_variables().discrete_string_variable_labels();
        }

        void discrete_string_variable_labels(Model &model, StringMultiArrayConstView d_v_labels) {
            model.current_variables().discrete_string_variable_labels(d_v_labels);
        }

        StringMultiArrayConstView discrete_real_variable_labels(const Model &model) {
            return model.current_variables().discrete_real_variable_labels();
        }

        void discrete_real_variable_labels(Model &model, StringMultiArrayConstView d_v_labels) {
            model.current_variables().discrete_real_variable_labels(d_v_labels);
        }

        StringMultiArrayConstView inactive_continuous_variable_labels(const Model &model) {
            return model.current_variables().inactive_continuous_variable_labels();
        }

        void inactive_continuous_variable_labels(Model &model, StringMultiArrayConstView i_c_v_labels) {
            model.current_variables().inactive_continuous_variable_labels(i_c_v_labels);
        }

        StringMultiArrayConstView inactive_discrete_int_variable_labels(const Model &model) {
            return model.current_variables().inactive_discrete_int_variable_labels();
        }

        void inactive_discrete_int_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels) {
            model.current_variables().inactive_discrete_int_variable_labels(i_d_v_labels);
        }

        StringMultiArrayConstView inactive_discrete_string_variable_labels(const Model &model) {
            return model.current_variables().inactive_discrete_string_variable_labels();
        }

        void inactive_discrete_string_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels) {
            model.current_variables().inactive_discrete_string_variable_labels(i_d_v_labels);
        }

        StringMultiArrayConstView inactive_discrete_real_variable_labels(const Model &model) {
            return model.current_variables().inactive_discrete_real_variable_labels();
        }

        void inactive_discrete_real_variable_labels(Model &model, StringMultiArrayConstView i_d_v_labels) {
            model.current_variables().inactive_discrete_real_variable_labels(i_d_v_labels);
        }

        StringMultiArrayConstView all_continuous_variable_labels(const Model &model) {
            return model.current_variables().all_continuous_variable_labels();
        }

        void all_continuous_variable_labels(Model &model, StringMultiArrayConstView a_c_v_labels) {
            model.current_variables().all_continuous_variable_labels(a_c_v_labels);
        }

        void all_continuous_variable_label(Model &model, const String& a_c_v_label, size_t i) {
            model.current_variables().all_continuous_variable_label(a_c_v_label, i);
        }

        StringMultiArrayConstView all_discrete_int_variable_labels(const Model &model) {
            return model.current_variables().all_discrete_int_variable_labels();
        }

        void all_discrete_int_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels) {
            model.current_variables().all_discrete_int_variable_labels(a_d_v_labels);
        }

        void all_discrete_int_variable_label(Model &model, const String& a_d_v_label, size_t i) {
            model.current_variables().all_discrete_int_variable_label(a_d_v_label, i);
        }

        StringMultiArrayConstView all_discrete_string_variable_labels(const Model &model) {
            return model.current_variables().all_discrete_string_variable_labels();
        }

        void all_discrete_string_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels) {
            model.current_variables().all_discrete_string_variable_labels(a_d_v_labels);
        }

        void all_discrete_string_variable_label(Model &model, const String& a_d_v_label, size_t i) {
            model.current_variables().all_discrete_string_variable_label(a_d_v_label, i);
        }

        StringMultiArrayConstView all_discrete_real_variable_labels(const Model &model) {
            return model.current_variables().all_discrete_real_variable_labels();
        }

        void all_discrete_real_variable_labels(Model &model, StringMultiArrayConstView a_d_v_labels) {
            model.current_variables().all_discrete_real_variable_labels(a_d_v_labels);
        }

        void all_discrete_real_variable_label(Model &model, const String& a_d_v_label, size_t i) {
            model.current_variables().all_discrete_real_variable_label(a_d_v_label, i);
        }


        size_t response_size(const Model &model) {
            return model.current_response().num_functions();
        }

        const StringArray& response_labels(const Model &model) {
            return model.current_response().function_labels();
        }

        void response_labels(Model &model, const StringArray& resp_labels) {
            model.current_response().function_labels(resp_labels);
        }





        const RealVector& continuous_lower_bounds(const Model &model) {
            model.user_defined_constraints().continuous_lower_bounds();
        }


        Real continuous_lower_bound(const Model &model, size_t i) {
            return model.user_defined_constraints().continuous_lower_bound(i);
        }


        void continuous_lower_bounds(Model &model, const RealVector& c_l_bnds) {

            model.user_defined_constraints().continuous_lower_bounds(c_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(c_l_bnds, 
                    model.current_variables().shared_data().cv_to_all_mask());
        }


        void continuous_lower_bound(Model &model, Real c_l_bnd, size_t i) {
            model.user_defined_constraints().continuous_lower_bound(c_l_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bound(c_l_bnd,
                    model.current_variables().shared_data().cv_index_to_all_index(i));
        }


        const RealVector& continuous_upper_bounds(const Model &model) {
            return model.user_defined_constraints().continuous_upper_bounds();
        }


        Real continuous_upper_bound(const Model &model, size_t i) {
            return model.user_defined_constraints().continuous_upper_bound(i);
        }


        void continuous_upper_bounds(Model &model, const RealVector& c_u_bnds) {
            model.user_defined_constraints().continuous_upper_bounds(c_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(c_u_bnds,
                    model.current_variables().shared_data().cv_to_all_mask());

        }


        void continuous_upper_bound(Model &model, Real c_u_bnd, size_t i) {
            model.user_defined_constraints().continuous_upper_bound(c_u_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bound(c_u_bnd,
                    model.current_variables().shared_data().cv_index_to_all_index(i));
        }


        const IntVector& discrete_int_lower_bounds(const Model &model) {
            return model.user_defined_constraints().discrete_int_lower_bounds();
        }


        int discrete_int_lower_bound(const Model &model, size_t i) {
            return model.user_defined_constraints().discrete_int_lower_bound(i);
        }


        void discrete_int_lower_bounds(Model &model, const IntVector& d_l_bnds) {

            model.user_defined_constraints().discrete_int_lower_bounds(d_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(d_l_bnds,
                    model.current_variables().shared_data().div_to_all_mask());
        }


        void discrete_int_lower_bound(Model &model, int d_l_bnd, size_t i) {
            model.user_defined_constraints().discrete_int_lower_bound(d_l_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bound(d_l_bnd,
                    model.current_variables().shared_data().div_index_to_all_index(i));
        }


        const IntVector& discrete_int_upper_bounds(const Model &model) {
            return model.user_defined_constraints().discrete_int_upper_bounds();
        }


        int discrete_int_upper_bound(const Model &model, size_t i) {
            return model.user_defined_constraints().discrete_int_upper_bound(i);
        }


        void discrete_int_upper_bounds(Model &model, const IntVector& d_u_bnds) {

            model.user_defined_constraints().discrete_int_upper_bounds(d_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(d_u_bnds,
                    model.current_variables().shared_data().div_to_all_mask());
        }


        void discrete_int_upper_bound(Model &model, int d_u_bnd, size_t i) {
            model.user_defined_constraints().discrete_int_upper_bound(d_u_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bound(d_u_bnd,
                    model.current_variables().shared_data().div_index_to_all_index(i));
        }


        const RealVector& discrete_real_lower_bounds(const Model &model) {
            return model.user_defined_constraints().discrete_real_lower_bounds();
        }


        Real discrete_real_lower_bound(const Model &model, size_t i) {
            return model.user_defined_constraints().discrete_real_lower_bound(i);
        }


        void discrete_real_lower_bounds(Model &model, const RealVector& d_l_bnds) {
            model.user_defined_constraints().discrete_real_lower_bounds(d_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(d_l_bnds,
                    model.current_variables().shared_data().drv_to_all_mask());
        }


        void discrete_real_lower_bound(Model &model, Real d_l_bnd, size_t i) {
            model.user_defined_constraints().discrete_real_lower_bound(d_l_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bound(d_l_bnd,
                    model.current_variables().shared_data().drv_index_to_all_index(i));
        }


        const RealVector& discrete_real_upper_bounds(const Model &model) {
            return model.user_defined_constraints().discrete_real_upper_bounds();
        }


        Real discrete_real_upper_bound(const Model &model, size_t i) {
            return model.user_defined_constraints().discrete_real_upper_bound(i);
        }


        void discrete_real_upper_bounds(Model &model, const RealVector& d_u_bnds) {
            model.user_defined_constraints().discrete_real_upper_bounds(d_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(d_u_bnds,
                    model.current_variables().shared_data().drv_to_all_mask());
        }


        void discrete_real_upper_bound(Model &model, Real d_u_bnd, size_t i) {
            model.user_defined_constraints().discrete_real_upper_bound(d_u_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bound(d_u_bnd,
                    model.current_variables().shared_data().drv_index_to_all_index(i));
        }


        const RealVector& inactive_continuous_lower_bounds(const Model &model) {
            return model.user_defined_constraints().inactive_continuous_lower_bounds();
        }


        void 
        inactive_continuous_lower_bounds(Model &model, const RealVector& i_c_l_bnds) {
        
            model.user_defined_constraints().inactive_continuous_lower_bounds(i_c_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(i_c_l_bnds,
                    model.current_variables().shared_data().icv_to_all_mask());
        }


        const RealVector& inactive_continuous_upper_bounds(const Model &model) {
            return model.user_defined_constraints().inactive_continuous_upper_bounds();
        }


        void 
        inactive_continuous_upper_bounds(Model &model, const RealVector& i_c_u_bnds) {
            model.user_defined_constraints().inactive_continuous_upper_bounds(i_c_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(i_c_u_bnds,
                    model.current_variables().shared_data().icv_to_all_mask());
        }


        const IntVector& inactive_discrete_int_lower_bounds(const Model &model) {
            return model.user_defined_constraints().inactive_discrete_int_lower_bounds();
        }


        void 
        inactive_discrete_int_lower_bounds(Model &model, const IntVector& i_d_l_bnds) {
            model.user_defined_constraints().inactive_discrete_int_lower_bounds(i_d_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(i_d_l_bnds,
                    model.current_variables().shared_data().idiv_to_all_mask());
        }


        const IntVector& inactive_discrete_int_upper_bounds(const Model &model) {
            return model.user_defined_constraints().inactive_discrete_int_upper_bounds();
        }


        void 
        inactive_discrete_int_upper_bounds(Model &model, const IntVector& i_d_u_bnds) {
            model.user_defined_constraints().inactive_discrete_int_upper_bounds(i_d_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(i_d_u_bnds,
                    model.current_variables().shared_data().idiv_to_all_mask());
        
        }


        const RealVector& inactive_discrete_real_lower_bounds(const Model &model) {
            return model.user_defined_constraints().inactive_discrete_real_lower_bounds();
        }


        void 
        inactive_discrete_real_lower_bounds(Model &model, const RealVector& i_d_l_bnds) {
            model.user_defined_constraints().inactive_discrete_real_lower_bounds(i_d_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(i_d_l_bnds,
                    model.current_variables().shared_data().idrv_to_all_mask());
        }


        const RealVector& inactive_discrete_real_upper_bounds(const Model &model) {
            return model.user_defined_constraints().inactive_discrete_real_upper_bounds();
        }


        void 
        inactive_discrete_real_upper_bounds(Model &model, const RealVector& i_d_u_bnds) {
            model.user_defined_constraints().inactive_discrete_real_upper_bounds(i_d_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(i_d_u_bnds,
                    model.current_variables().shared_data().idrv_to_all_mask());
        }


        const RealVector& all_continuous_lower_bounds(const Model &model) {
            return  model.user_defined_constraints().all_continuous_lower_bounds();
        }


        void all_continuous_lower_bounds(Model &model, const RealVector& a_c_l_bnds) {
            model.user_defined_constraints().all_continuous_lower_bounds(a_c_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(a_c_l_bnds,
                    model.current_variables().shared_data().acv_to_all_mask());
        }


        void all_continuous_lower_bound(Model &model, Real a_c_l_bnd, size_t i) {
            model.user_defined_constraints().all_continuous_lower_bound(a_c_l_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bound(a_c_l_bnd,
                    model.current_variables().shared_data().acv_index_to_all_index(i));
        }


        const RealVector& all_continuous_upper_bounds(const Model &model) {
            return model.user_defined_constraints().all_continuous_upper_bounds();
        }


        void all_continuous_upper_bounds(Model &model, const RealVector& a_c_u_bnds) {
            model.user_defined_constraints().all_continuous_upper_bounds(a_c_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(a_c_u_bnds,
                    model.current_variables().shared_data().acv_to_all_mask());
        }


        void all_continuous_upper_bound(Model &model, Real a_c_u_bnd, size_t i) {
            model.user_defined_constraints().all_continuous_upper_bound(a_c_u_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bound(a_c_u_bnd,
                    model.current_variables().shared_data().acv_index_to_all_index(i));
        }


        const IntVector& all_discrete_int_lower_bounds(const Model &model) {
            return model.user_defined_constraints().all_discrete_int_lower_bounds();
        }


        void all_discrete_int_lower_bounds(Model &model, const IntVector& a_d_l_bnds) {
            model.user_defined_constraints().all_discrete_int_lower_bounds(a_d_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(a_d_l_bnds,
                    model.current_variables().shared_data().adiv_to_all_mask());
        }


        void all_discrete_int_lower_bound(Model &model, int a_d_l_bnd, size_t i) {
            model.user_defined_constraints().all_discrete_int_lower_bound(a_d_l_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bound(a_d_l_bnd,
                    model.current_variables().shared_data().adiv_index_to_all_index(i));
        }


        const IntVector& all_discrete_int_upper_bounds(const Model &model) {
        return model.user_defined_constraints().all_discrete_int_upper_bounds();
        }


        void all_discrete_int_upper_bounds(Model &model, const IntVector& a_d_u_bnds) {
            model.user_defined_constraints().all_discrete_int_upper_bounds(a_d_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(a_d_u_bnds,
                    model.current_variables().shared_data().adiv_to_all_mask());
        }


        void all_discrete_int_upper_bound(Model &model, int a_d_u_bnd, size_t i) {
            model.user_defined_constraints().all_discrete_int_upper_bound(a_d_u_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bound(a_d_u_bnd,
                    model.current_variables().shared_data().adiv_index_to_all_index(i));
        }


        const RealVector& all_discrete_real_lower_bounds(const Model &model) {
            return model.user_defined_constraints().all_discrete_real_lower_bounds();
        }


        void all_discrete_real_lower_bounds(Model &model, const RealVector& a_d_l_bnds) {
            model.user_defined_constraints().all_discrete_real_lower_bounds(a_d_l_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bounds(a_d_l_bnds,
                    model.current_variables().shared_data().adrv_to_all_mask());
        }


        void all_discrete_real_lower_bound(Model &model, Real a_d_l_bnd, size_t i) {
            model.user_defined_constraints().all_discrete_real_lower_bound(a_d_l_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.lower_bound(a_d_l_bnd,
                    model.current_variables().shared_data().adrv_index_to_all_index(i));
        }


        const RealVector& all_discrete_real_upper_bounds(const Model &model) {
            return model.user_defined_constraints().all_discrete_real_upper_bounds();
        }


        void all_discrete_real_upper_bounds(Model &model, const RealVector& a_d_u_bnds) {
            model.user_defined_constraints().all_discrete_real_upper_bounds(a_d_u_bnds);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bounds(a_d_u_bnds,
                    model.current_variables().shared_data().adrv_to_all_mask());
        }


        void all_discrete_real_upper_bound(Model &model, Real a_d_u_bnd, size_t i) {
            model.user_defined_constraints().all_discrete_real_upper_bound(a_d_u_bnd, i);
            auto & mvd = model.multivariate_distribution();
            if (mvd.global_bounds())
                mvd.upper_bound(a_d_u_bnd,
                    model.current_variables().shared_data().adrv_index_to_all_index(i));
        }

    }
}