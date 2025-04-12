#ifndef DAKOTA_RESPONSE_CONTAINER_H
#define DAKOTA_RESPONSE_CONTAINER_H

#include <vector>
#include <memory> // Include for std::unique_ptr

#include "DakotaAccumulators.hpp"
#include "dakota_data_types.hpp"

namespace Dakota {

// Represents a group of models and their associated statistics.
class ModelGroup {
public:
    // Constructs a ModelGroup with the specified model indices and parameters.
    //
    // Args:
    //   model_indices: A vector of indices representing the models in this group.
    //   num_qois: The number of quantities of interest.
    //   num_moments: The number of moments to be calculated.
    //   num_replications: The number of replications for the statistics.
    ModelGroup(const std::vector<unsigned short>& model_indices, int num_qois, int num_moments, int num_replications);

    // Adds a sample of model evaluations to the group.
    //
    // Args:
    //   model_evaluations: A vector of model evaluation results.
    //   replication: The replication index for the sample.
    void addSample(const std::vector<double>& model_evaluations, int replication);

    // Gets the number of samples for a specific model, quantity of interest, moment, and replication.
    //
    // Args:
    //   model: The index of the model.
    //   qoi: The index of the quantity of interest.
    //   moment: The moment index (1-based).
    //   replication: The replication index.
    //
    // Returns:
    //   The number of samples for the specified parameters.
    int numSamples(int model = 0, int qoi = 0, int moment = 1, int replication = 0);

    // Gets the mean for a specific model, quantity of interest, moment, and replication.
    //
    // Args:
    //   model: The index of the model.
    //   qoi: The index of the quantity of interest.
    //   moment: The moment index (1-based).
    //   replication: The replication index.
    //
    // Returns:
    //   The computed mean for the specified parameters.
    Real getMean(int model, int qoi, int moment, int replication);

    // Gets the covariance between two models for a specific quantity of interest, moment, and replication.
    //
    // Args:
    //   model: The index of the first model.
    //   other_model: The index of the second model.
    //   qoi: The index of the quantity of interest.
    //   moment: The moment index (1-based).
    //   replication: The replication index.
    //
    // Returns:
    //   The computed covariance for the specified parameters.
    Real getCovariance(int model, int other_model, int qoi, int moment, int replication);

private:
    std::vector<unsigned short> model_indices_; // Indices of models in this group.
    int num_moments_; // Number of moments to be calculated.
    int num_qois_; // Number of quantities of interest.
    int num_replications_; // Number of replications for the statistics.

    // Accumulators for mean statistics.
    std::vector<std::vector<std::vector<std::vector<MeanAccumulator<Real>>>>> mean_accumulators_; // [moment][model][quantity_of_interest][replication]
    
    // Accumulators for covariance statistics.
    std::vector<std::vector<std::vector<std::vector<std::vector<CovarianceAccumulator<Real>>>>>> covariance_accumulators_; // [moment][model][other_model][quantity_of_interest][replication]
};

// Container for managing multiple ModelGroup instances.
class ResponseContainer {
public:
    // Constructs an empty ResponseContainer.
    ResponseContainer();

    // Adds a new ModelGroup to the container.
    //
    // Args:
    //   model_indices: A vector of indices representing the models in the new group.
    //   num_qois: The number of quantities of interest (default is 1).
    //   num_moments: The number of moments to be calculated (default is 1).
    //   num_replications: The number of replications for the statistics (default is 1).
    void addModelGroup(const std::vector<unsigned short>& model_indices, int num_qois = 1, int num_moments = 1, int num_replications = 1);

    // Adds samples to the model groups from a given map of samples.
    //
    // Args:
    //   samples: A map containing samples to be added.
    //   replication: The replication index for the samples (default is 0).
    void addSamples(IntIntResponse2DMap& samples, int replication = 0);

    // Gets the number of samples in the container for a specific model group.
    //
    // Args:
    //   group: The index of the model group.
    //
    // Returns:
    //   The number of samples.
    int numSamples(int group);

    // Gets the number of model groups in the container.
    //
    // Returns:
    //   The number of model groups.
    int numGroups();

    // Gets the mean for a specific group, model, quantity of interest, moment, and replication.
    //
    // Args:
    //   group: The index of the model group.
    //   model: The index of the model.
    //   qoi: The index of the quantity of interest (default is 0).
    //   moment: The moment index (default is 1).
    //   replication: The replication index (default is 0).
    //
    // Returns:
    //   The computed mean for the specified parameters.
    Real getMean(int group, int model, int qoi = 0, int moment = 1, int replication = 0);

    // Gets the covariance between two models for a specific group, quantity of interest, moment, and replication.
    //
    // Args:
    //   group: The index of the model group.
    //   model: The index of the first model.
    //   other_model: The index of the second model.
    //   qoi: The index of the quantity of interest (default is 0).
    //   moment: The moment index (default is 1).
    //   replication: The replication index (default is 0).
    //
    // Returns:
    //   The computed covariance for the specified parameters.
    Real getCovariance(int group, int model, int other_model, int qoi = 0, int moment = 1, int replication = 0);

private:
    std::vector<std::unique_ptr<ModelGroup>> model_groups_; // Unique pointers to model groups.
};

} // end namespace Dakota

#endif // DAKOTA_RESPONSE_CONTAINER_H