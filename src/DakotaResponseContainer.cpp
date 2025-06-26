#include "DakotaResponseContainer.hpp"
#include "DakotaResponse.hpp"
#include <memory> // Include for std::unique_ptr

namespace Dakota {

// Constructs a ModelGroup with the specified model indices and parameters.
//
// Args:
//   model_indices: A vector of indices representing the models in this group.
//   num_qois: The number of quantities of interest.
//   num_moments: The number of moments to be calculated.
//   num_replications: The number of replications for the statistics.
ModelGroup::ModelGroup(const std::vector<unsigned short>& model_indices, int num_qois, int num_moments, int num_replications) :
    model_indices_(model_indices),
    num_moments_(num_moments),
    num_qois_(num_qois),
    num_replications_(num_replications),
    mean_accumulators_(
        num_moments, std::vector<std::vector<std::vector<MeanAccumulator<Real>>>>(
            model_indices.size(), std::vector<std::vector<MeanAccumulator<Real>>>(
                num_qois, std::vector<MeanAccumulator<Real>>(
                    num_replications
                )
            )
        )
    ),
    covariance_accumulators_(
        num_moments, std::vector<std::vector<std::vector<std::vector<CovarianceAccumulator<Real>>>>>(
            model_indices.size(), std::vector<std::vector<std::vector<CovarianceAccumulator<Real>>>>(
                model_indices.size(), std::vector<std::vector<CovarianceAccumulator<Real>>>(
                    num_qois, std::vector<CovarianceAccumulator<Real>>(
                        num_replications
                    )
                )
            )
        )
    )
{
}

// Adds a sample of model evaluations to the group.
//
// This method updates the mean and covariance accumulators with the provided
// model evaluations for the specified replication.
//
// Args:
//   model_evaluations: A vector of model evaluation results.
//   replication: The replication index for the sample.
void ModelGroup::addSample(const std::vector<Real>& model_evaluations, int replication) {
    // Update accumulators
    for (int moment = 0; moment < num_moments_; ++moment) {
        for (int model = 0; model < model_indices_.size(); ++model) {
            for (int qoi = 0; qoi < num_qois_; ++qoi) {

                // Update mean accumulator
                auto value = model_evaluations[model_indices_[model] * num_qois_ + qoi];
                if (!std::isnan(value)) { // or check out of bounds
                    auto value_pow = std::pow(value, moment + 1);
                    mean_accumulators_[moment][model][qoi][replication].addSample(value_pow);
                
                    // Update covariance accumulator
                    for (int other_model = 0; other_model <= model; ++other_model) {
                        auto other_value = model_evaluations[model_indices_[other_model] * num_qois_ + qoi];
                        if (!std::isnan(other_value)) {
                            auto other_value_pow = std::pow(other_value, moment + 1);
                            covariance_accumulators_[moment][model][other_model][qoi][replication].addSample(value_pow, other_value_pow);
                        }
                    }
                }
            }
        }
    }
}

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
int ModelGroup::numSamples(int model, int qoi, int moment, int replication) {
    return mean_accumulators_[moment - 1][model][qoi][replication].numSamples();
}

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
Real ModelGroup::getMean(int model, int qoi, int moment, int replication) {
    return mean_accumulators_[moment - 1][model][qoi][replication].computeStatistic();
}

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
Real ModelGroup::getCovariance(int model, int other_model, int qoi, int moment, int replication) {
    return covariance_accumulators_[moment - 1][model][other_model][qoi][replication].computeStatistic();
}

// Empties the model group of all stored samples.
void ModelGroup::reset() {
    for (int moment = 0; moment < num_moments_; ++moment) {
        for (int model = 0; model < model_indices_.size(); ++model) {
            for (int qoi = 0; qoi < num_qois_; ++qoi) {
                for (int replication = 0; replication < num_replications_; ++replication) {
                    mean_accumulators_[moment][model][qoi][replication].reset();
                    for (int other_model = 0; other_model <= model; ++other_model) {
                        covariance_accumulators_[moment][model][other_model][qoi][replication].reset();
                    }
                }
            }
        }
    }
}

// Constructs an empty ResponseContainer.
ResponseContainer::ResponseContainer() 
    : model_groups_() // Initialize the vector of unique pointers to model groups
{
}

// Adds a new ModelGroup to the container.
//
// Args:
//   model_indices: A vector of indices representing the models in the new group.
//   num_qois: The number of quantities of interest (default is 1).
//   num_moments: The number of moments to be calculated (default is 1).
//   num_replications: The number of replications for the statistics (default is 1).
void ResponseContainer::addModelGroup(const std::vector<unsigned short>& model_indices, int num_qois, int num_moments, int num_replications) {
    // Create a new ModelGroup and add it to the vector
    model_groups_.emplace_back(std::make_unique<ModelGroup>(model_indices, num_qois, num_moments, num_replications));
}

// Adds samples to the model groups from a given map of samples.
//
// Args:
//   samples: A map containing samples to be added.
//   replication: The replication index for the samples (default is 0).
void ResponseContainer::addSamples(IntIntResponse2DMap& samples, int replication) {
    for (const auto& grouped_samples : samples) {
        for (const auto& grouped_sample : grouped_samples.second) {
            std::vector<Real> function_values;
            auto tmp = grouped_sample.second.function_values();
            for (int j = 0; j < tmp.length(); ++j) {
                function_values.push_back(tmp(j));
            }
            model_groups_[grouped_samples.first]->addSample(function_values, replication);
        }
    }
}

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
Real ResponseContainer::getMean(int group, int model, int qoi, int moment, int replication) {
    return model_groups_[group]->getMean(model, qoi, moment, replication);
}

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
Real ResponseContainer::getCovariance(int group, int model, int other_model, int qoi, int moment, int replication) {
    return model_groups_[group]->getCovariance(model, other_model, qoi, moment, replication);
}

// Gets the number of samples in the container for a specific model group.
//
// Args:
//   group: The index of the model group.
//
// Returns:
//   The number of samples.
int ResponseContainer::numSamples(int group) {
    return model_groups_[group]->numSamples();
}

// Gets the number of model groups in the container.
//
// Returns:
//   The number of model groups.
int ResponseContainer::numGroups() {
    return model_groups_.size();
}

// Empties the response container of all stored samples.
void ResponseContainer::reset() {
    for (auto& model_group : model_groups_)
        model_group->reset();
}

} // end namespace Dakota