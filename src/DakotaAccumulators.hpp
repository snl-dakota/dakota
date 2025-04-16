#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <stdexcept>
#include <vector>

#include "dakota_data_types.hpp" // For Real data type

#ifndef DAKOTA_ACCUMULATORS_H
#define DAKOTA_ACCUMULATORS_H

using namespace boost::accumulators;

namespace Dakota {

// Abstract base class for all accumulators.
//
// This class defines the interface for all accumulator types. It includes pure
// virtual methods that must be implemented by derived classes to add samples
// and compute statistics.
template <typename T>
class Accumulator {
public:
    virtual ~Accumulator() = default;

    // Adds a sample to the accumulator.
    //
    // Args:
    //   sample: The sample to be added.
    virtual void addSample(T sample) = 0;

    // Returns the number of samples in this accumulator.
    //
    // Returns:
    //   The number of samples.
    virtual std::size_t numSamples() = 0;

    // Computes the final statistic based on the accumulated samples.
    //
    // Returns:
    //   The computed statistic of type T.
    virtual T computeStatistic() = 0;

    // Empties the accumulator of all stored samples.
    virtual void reset() = 0;
};

// Accumulator for computing the mean of a set of samples.
//
// This class inherits from `Accumulator` and implements the functionality to
// compute the mean of the samples added to it.
template <typename T>
class MeanAccumulator : public Accumulator<T> {
public:
    // Constructs a MeanAccumulator.
    MeanAccumulator() {}

    // Adds a sample to the mean accumulator.
    //
    // Args:
    //   sample: The sample to be added.
    void addSample(T sample) override {
        acc_(sample);
    }

    // Returns the number of samples in this mean accumulator.
    //
    // Returns:
    //   The number of samples.
    std::size_t numSamples() {
        return boost::accumulators::count(acc_);
    }

    // Computes the mean of the accumulated samples.
    //
    // Returns:
    //   The computed mean of type T.
    T computeStatistic() override {
        return mean(acc_);
    }

    // Empties the accumulator of all stored samples.
    void reset() override {
        acc_ = mean_accumulator();
    }

private:
    typedef mean_accumulator = accumulator_set<T, stats<tag::mean>>;
    mean_accumulator acc_;
};

// Accumulator for computing the variance of a set of samples.
//
// This class inherits from `Accumulator` and implements the functionality to
// compute the variance of the samples added to it.
template <typename T>
class VarianceAccumulator : public Accumulator<T> {
public:
    // Constructs a VarianceAccumulator.
    VarianceAccumulator() {}

    // Adds a sample to the variance accumulator.
    //
    // Args:
    //   sample: The sample to be added.
    void addSample(T sample) override {
        acc_(sample);
    }

    // Returns the number of samples in this variance accumulator.
    //
    // Returns:
    //   The number of samples.
    std::size_t numSamples() {
        return boost::accumulators::count(acc_);
    }

    // Computes the variance of the accumulated samples.
    //
    // Returns:
    //   The computed variance of type T.
    T computeStatistic() override {
        return variance(acc_);
    }

    // Empties the accumulator of all stored samples.
    void reset() override {
        acc_ = variance_accumulator();
    }

private:
    typedef variance_accumulator = accumulator_set<T, stats<tag::variance>>;
    variance_accumulator acc_;
};

// Accumulator for computing the covariance between two sets of samples.
//
// This class inherits from `Accumulator` and implements the functionality to
// compute the covariance of the samples added to it.
template <typename T>
class CovarianceAccumulator : public Accumulator<T> {
public:
    // Constructs a CovarianceAccumulator.
    CovarianceAccumulator() {}

    // Adds a pair of samples to the covariance accumulator.
    //
    // Args:
    //   sample_x: The first sample to be added.
    //   sample_y: The second sample to be added.
    void addSample(T sample_x, T sample_y) {
        acc_(sample_x, covariate1 = sample_y);
    }

    // Throws an exception when trying to add a single sample.
    //
    // Args:
    //   sample: The sample to be added (not used).
    // Throws:
    //   std::logic_error if called.
    void addSample(T sample) override {
        throw std::logic_error("CovarianceAccumulator requires two samples.");
    }
    
    // Returns the number of samples in this covariance accumulator.
    //
    // Returns:
    //   The number of samples.
    std::size_t numSamples() {
        return boost::accumulators::count(acc_);
    }

    // Computes the covariance of the accumulated samples.
    //
    // Returns:
    //   The computed covariance of type T.
    T computeStatistic() override {
        return covariance(acc_) * numSamples() / (numSamples() - 1);
    }

    // Empties the accumulator of all stored samples.
    void reset() override {
        acc_ = covariance_accumulator();
    }

private:
    typedef variance_accumulator = accumulator_set<T, stats<tag::covariance<T, tag::covariate1>>>co;
    covariance_accumulator acc_;
};

// Explicit instantiation for Real type
template class MeanAccumulator<Real>;
template class VarianceAccumulator<Real>;
template class CovarianceAccumulator<Real>;

}  // namespace Dakota

#endif  // DAKOTA_ACCUMULATORS_H
