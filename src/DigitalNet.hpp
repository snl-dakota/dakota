/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DIGITAL_NET_H
#define DIGITAL_NET_H

#include "dakota_data_types.hpp"
#include "LowDiscrepancySequence.hpp"
#include "NonDSampling.hpp"

namespace Dakota {

/// Enum for digital net ordering
enum DigitalNetOrdering {
  DIGITAL_NET_NATURAL_ORDERING,
  DIGITAL_NET_GRAY_CODE_ORDERING
};

/// Class for digital nets in Dakota
class DigitalNet : public LowDiscrepancySequence
{
public:

  /// Default constructor
  DigitalNet(
    const UInt64Matrix& generatingMatrices, /// Generating matrices
    int mMax,                               /// log2 of maximum number of points
    int tMax,                               /// Number of bits of integers in the generating matrices
    int tScramble,                          /// Number of rows in the linear scramble matrix
    bool digitalShiftFlag,                  /// Use digital shift if true
    bool scramblingFlag,                    /// Use linear matrix scramble if true
    int seedValue,                          /// Random seed value
    DigitalNetOrdering ordering,            /// Order of the digital net points
    bool mostSignificantBit,                /// Generating matrices are stored with most significant bit first if true
    short outputLevel                       /// Verbosity
  );

  /// A constructor that uses gray code ordering and a digital shift with linear
  /// matrix scramble
  DigitalNet(
    const UInt64Matrix& generatingMatrices,
    int mMax,
    int tMax
  );

  /// A constructor with only the random seed as argument
  DigitalNet(
    int seedValue
  );

  /// A constructor with no arguments
  DigitalNet();

  /// A constructor that takes a problem description database
  DigitalNet(
    ProblemDescDB& problem_db
  );

  /// A constructor that takes a tuple and a problem description database
  /// The tuple contains the generating matrices, corresponding log2 of the
  /// maximum number of points, number of bits in each integer of the
  /// generating matrices, and the number of rows in the linear scramble
  /// matrix
  DigitalNet(
    std::tuple<UInt64Matrix, int, int> data,
    ProblemDescDB& problem_db
  );

  /// Destructor
  ~DigitalNet();

  /// Randomize this digital net
  /// NOTE: required for 'unique' sampling in `NonDLowDiscrepancySampling`
  inline void randomize() { digital_shift(); scramble(); }

  /// Digitally shift this digital net
  void digital_shift() { digital_shift(generate_system_seed()); }

  /// Do not digitally shift this digital net
  void no_digital_shift() { digital_shift(-1); }

  /// Apply linear matrix scramble to this digital net
  void scramble() { scramble(generate_system_seed()); }

  /// Do not apply linear matrix scramble to this digital net
  void no_scrambling() { scramble(-1); }

private:

  /// Generating matrices of this digital net
  UInt64Matrix generatingMatrices;

  /// Scrambled generating matrices of this digital net
  UInt64Matrix scrambledGeneratingMatrices;

  /// Number of bits of each integer in generatingMatrices
  /// Also: number of rows in each generating matrix
  int tMax;

  /// Number of rows in the linear scramble matrix
  int tScramble;

  /// Digitally shift this digital net if true
  bool digitalShiftFlag;

  /// Perform linear matrix scramble if true
  bool scramblingFlag;

  /// Digital shift associated with this digital net
  UInt64Vector digitalShift;

  /// Order of the points of this digital net
  DigitalNetOrdering ordering;

  /// Most significant bit comes first in generatingMatrices when true
  bool mostSignificantBitFirst;

  /// Extract the generating matrices, corresponding log2 of the maximum number
  /// of points and number of bits in each integer of the generating matrices 
  /// from the given problem 
  std::tuple<UInt64Matrix, int, int> get_data(
    ProblemDescDB& problem_db
  );

  /// Case I: the generating matrices are provided in an external file
  const std::tuple<UInt64Matrix, int, int> get_generating_matrices_from_file(
    ProblemDescDB& problem_db
  );

  /// Case II: the generating matrices are provided in the input file
  const std::tuple<UInt64Matrix, int, int> get_inline_generating_matrices(
    ProblemDescDB& problem_db
  );

  /// Case III: a set of default generating matrices has been selected
  const std::tuple<UInt64Matrix, int, int> get_default_generating_matrices(
    ProblemDescDB& problem_db
  );

  /// Apply digital shift to this digital net
  /// Uses the given seed to initialize the RNG
  /// When the seed is < 0, the random shift will be removed
  void digital_shift(
    int seed
  );

  /// Scramble this digital net
  void scramble(
    int seed
  );

  /// Returns a set of linear scrambling matrices for this digital net
  UInt64Matrix get_linear_scrambling_matrices(
    int seed
  );

  /// Reverse the bits in the scrambled generating matrices
  void bitreverse_generating_matrices();

  /// Generates digital net points without error checking
  void unsafe_get_points(
    const size_t nMin,
    const size_t nMax,
    RealMatrix& points
  );

  /// Get the next point of the sequence represented as an unsigned integer
  /// vector
  void next(
    int n,
    UInt64Vector& current_point
  );

  /// Position of the `k`th digital net point in DIGITAL_NET_NATURAL_ORDERING
  inline UInt64 reorder_natural(
    UInt64 k
  );

  /// Position of the `k`th digital net point in DIGITAL_NET_GRAY_CODE_ORDERING
  inline UInt64 reorder_gray_code(
    UInt64 k
  );

  /// Function pointer to the chosen order of the points
  UInt64 (DigitalNet::*reorder)(UInt64);

};

} // namespace Dakota

#endif
