.. _`topic-sampling`:

Sampling
========

Sampling techniques are selected using the :dakkw:`method-sampling`
method selection. This method generates sets of samples according to
the probability distributions of the uncertain variables and maps them
into corresponding sets of response functions, where the number of
samples is specified by the :dakkw:`method-sampling-samples` integer specification.
Means, standard deviations, coefficients of variation (COVs), and 95\%
confidence intervals are computed for the response functions.
Probabilities and reliabilities may be computed for
:dakkw:`method-sampling-response_levels` specifications, and response levels may be
computed for either :dakkw:`method-sampling-probability_levels` or
:dakkw:`method-sampling-reliability_levels` specifications.

Currently, traditional Monte Carlo (MC) and Latin hypercube sampling
(LHS) are supported by Dakota and are chosen by specifying
:dakkw:`method-sampling-sample_type` as :dakkw:`method-sampling-sample_type-random`
or :dakkw:`method-sampling-sample_type-lhs`. In Monte
Carlo sampling, the samples are selected randomly according to the
user-specified probability distributions. Latin hypercube sampling is
a stratified sampling technique for which the range of each uncertain
variable is divided into :math:`N_{s}`  segments of equal probability, where
:math:`N_{s}`  is the number of samples requested. The relative lengths of
the segments are determined by the nature of the specified probability
distribution (e.g., uniform has segments of equal width, normal has
small segments near the mean and larger segments in the tails). For
each of the uncertain variables, a sample is selected randomly from
each of these equal probability segments. These :math:`N_{s}`  values for
each of the individual parameters are then combined in a shuffling
operation to create a set of :math:`N_{s}`  parameter vectors with a
specified correlation structure. A feature of the resulting sample set
is that
*every row and column in the hypercube of partitions has exactly one sample*.
Since the total number of samples is exactly equal
to the number of partitions used for each uncertain variable, an
arbitrary number of desired samples is easily accommodated (as
compared to less flexible approaches in which the total number of
samples is a product or exponential function of the number of
intervals for each variable, i.e., many classical design of
experiments methods).

Advantages of sampling-based methods include their relatively simple
implementation and their independence from the scientific disciplines
involved in the analysis. The main drawback of these techniques is the
large number of function evaluations needed to generate converged
statistics, which can render such an analysis computationally very
expensive, if not intractable, for real-world engineering
applications. LHS techniques, in general, require fewer samples than
traditional Monte Carlo for the same accuracy in statistics, but they
still can be prohibitively expensive. For further information on the
method and its relationship to other sampling techniques, one is
referred to the works by McKay, et al. :cite:p:`Mck79`, Iman and
Shortencarier :cite:p:`Ima84`, and Helton and Davis :cite:p:`Hel00`.
Note that under certain separability conditions associated with the
function to be sampled,
Latin hypercube sampling provides a more accurate estimate of the mean
value than does random sampling. That is, given an equal number of
samples, the LHS estimate of the mean will have less variance than the
mean value obtained through random sampling.