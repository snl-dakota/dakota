#pragma once
#include "dakota_data_types.hpp"


namespace Dakota {
    class Model;
    namespace ModelUtils {
        /// define and return discreteIntSets using active view from currentVariables
        const BitArray& discrete_int_sets(Model &model);
        /// define and return discreteIntSets using passed active view
        const BitArray& discrete_int_sets(Model &model, short active_view);
        /// return the sets of values available for each of the active
        /// discrete set integer variables (aggregated in activeDiscSetIntValues)
        const IntSetArray& discrete_set_int_values(Model &model);
        /// return the sets of values available for each of the active
        /// discrete set integer variables (aggregated in activeDiscSetIntValues)
        const IntSetArray& discrete_set_int_values(Model &model, short active_view);
        /// return the sets of values available for each of the active
        /// discrete set string variables (aggregated in activeDiscSetStringValues)
        const StringSetArray& discrete_set_string_values(Model &model);
        /// return the sets of values available for each of the active
        /// discrete set string variables (aggregated in activeDiscSetStringValues)
        const StringSetArray& discrete_set_string_values(Model &model, short active_view);
        /// return the sets of values available for each of the active
        /// discrete set real variables (aggregated in activeDiscSetRealValues)
        const RealSetArray& discrete_set_real_values(Model &model);
        /// return the sets of values available for each of the active
        /// discrete set real variables (aggregated in activeDiscSetRealValues)
        const RealSetArray& discrete_set_real_values(Model &model, short active_view);
    }
}
