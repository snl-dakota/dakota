#pragma once

namespace Dakota {

/// Runtime response implementations.
enum { BASE_RESPONSE = 0, SIMULATION_RESPONSE, EXPERIMENT_RESPONSE };

/// Primary response function categories.
enum { GENERIC_FNS = 0, OBJECTIVE_FNS, CALIB_TERMS };

} // namespace Dakota
