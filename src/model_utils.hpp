#pragma once
#include "dakota_data_types.hpp"


namespace Dakota {
    class Model;
    namespace ModelUtils {
        /// define and return discreteIntSets using active view from currentVariables
        const BitArray& discrete_int_sets(Model &model);
        /// define and return discreteIntSets using passed active view
        const BitArray& discrete_int_sets(Model &model, short active_view);
    }
}