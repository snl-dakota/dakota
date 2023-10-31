/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef RANK_1_LATTICE_H
#define RANK_1_LATTICE_H

#include "dakota_data_types.hpp"
#include "LowDiscrepancySequence.hpp"
#include "NonDSampling.hpp"

namespace Dakota {

/// Enum for rank-1 lattice rule ordering
enum Rank1LatticeOrdering {
  RANK_1_LATTICE_NATURAL_ORDERING,
  RANK_1_LATTICE_RADICAL_INVERSE_ORDERING
};

/// Class for rank-1 lattice rules in Dakota
class Rank1Lattice : public LowDiscrepancySequence
{
public:

  /// Default constructor
  Rank1Lattice(
    const UInt32Vector& generatingVector, /// Generating vector
    int mMax,                             /// log2 of maximum number of points
    bool randomShiftFlag,                 /// Use random shift if true
    int seedValue,                        /// Random seed value
    Rank1LatticeOrdering ordering,        /// Order of the lattice points
    short outputLevel                     /// Verbosity
  );

  /// A constructor that uses radical inverse ordering and a random shift
  Rank1Lattice(
    const UInt32Vector& generatingVector,
    int mMax
  );

  /// A constructor with only the random seed as argument
  Rank1Lattice(
    int seedValue
  );

  /// A constructor with no arguments
  Rank1Lattice();

  /// A constructor that takes a problem description database
  Rank1Lattice(
    ProblemDescDB& problem_db
  );

  /// A constructor that takes a tuple and a problem description database
  /// The tuple contains the generating vector and corresponding log2 of the
  /// maximum number of points
  Rank1Lattice(
    std::tuple<UInt32Vector, int> data,
    ProblemDescDB& problem_db
  );

  /// Destructor
  ~Rank1Lattice();

  /// Randomize this rank-1 lattice rule
  /// NOTE: required for 'unique' sampling in `NonDLowDiscrepancySampling`
  inline void randomize() { random_shift(); }

  /// Randomly shift this rank-1 lattice rule
  void random_shift() { random_shift(generate_system_seed()); }

  /// Do not randomly shift this rank-1 lattice rule
  void no_random_shift() { random_shift(-1); }

private:

  /// Generating vector of this rank-1 lattice rule
  UInt32Vector generatingVector;

  /// Randomize this rank-1 lattice rule if true
  bool randomShiftFlag;

  /// Random shift associated with this rank-1 lattice rule
  RealVector randomShift;

  /// Order of the points of this rank-1 lattice rule
  Rank1LatticeOrdering ordering;

  /// Scale factor for rank-1 lattice points
  Real scale;

  /// Extract the generating vector and log2 of the maximum number of points
  /// from the given problem description database
  std::tuple<UInt32Vector, int> get_data(
    ProblemDescDB& problem_db
  );

  /// Case I: the generating vector is provided in an external file
  const std::tuple<UInt32Vector, int> get_generating_vector_from_file(
    ProblemDescDB& problem_db
  );

  /// Case II: the generating vector is provided in the input file
  const std::tuple<UInt32Vector, int> get_inline_generating_vector(
    ProblemDescDB& problem_db
  );

  /// Case III: a default generating vector has been selected
  const std::tuple<UInt32Vector, int> get_default_generating_vector(
    ProblemDescDB& problem_db
  );

  /// Apply random shift to this rank-1 lattice rule
  /// Uses the given seed to initialize the RNG
  /// When the seed is < 0, the random shift will be removed
  void random_shift(
    int seed
  );

  /// Generates rank-1 lattice points without error checking
  void unsafe_get_points(
    const size_t nMin,
    const size_t nMax,
    RealMatrix& points
  );

  /// Position of the `k`th lattice point in RANK_1_LATTICE_NATURAL_ORDERING
  inline UInt32 reorder_natural(
    UInt32 k
  );

  /// Position of the `k`th lattice point in RANK_1_LATTICE_RADICAL_INVERSE_ORDERING
  inline UInt32 reorder_radical_inverse(
    UInt32 k
  );

  /// Function pointer to the chosen order of the points
  UInt32 (Rank1Lattice::*reorder)(UInt32);

};

} // namespace Dakota

#endif
