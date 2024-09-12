/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_bit_utils.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_types.hpp"
#include "dakota_mersenne_twister.hpp"
#include "dakota_tabular_io.hpp"
#include "low_discrepancy_data.hpp"

#include "LowDiscrepancySequence.hpp"
#include "Rank1Lattice.hpp"

#include <boost/random/uniform_01.hpp>

namespace Dakota {

/// Default constructor
Rank1Lattice::Rank1Lattice(
  const UInt32Vector& generatingVector, /// Generating vector
  int mMax,                             /// log2 of maximum number of points
  bool randomShiftFlag,                 /// Use random shift if true
  int seedValue,                        /// Random seed value
  Rank1LatticeOrdering ordering,        /// Order of the lattice points
  short outputLevel                     /// Verbosity
) :
LowDiscrepancySequence(
  mMax,
  generatingVector.length(),
  seedValue,
  outputLevel
),
generatingVector(generatingVector),
randomShiftFlag(randomShiftFlag),
ordering(ordering)
{
  /// Print summary info when debugging
  if ( outputLevel >= DEBUG_OUTPUT )
  {
    Cout << "The maximum dimension of this rank-1 lattice rule is "
      << dMax << "." << std::endl;
    Cout << "The log2 of the maximum number of points of this rank-1 "
      << "lattice rule is " << mMax << "." << std::endl;
    Cout << "The value of the random seed is " << seedValue << "."
      << std::endl;
    auto length = generatingVector.length();
    Cout << "Found generating vector of length " << length << ":";
    for (size_t j=0; j < length; ++j)
      Cout << generatingVector[j] << " ";
    Cout << std::endl;
  }

  ///
  /// Options for setting the random shift of this rank-1 lattice rule
  ///
  random_shift(randomShiftFlag ? seedValue : -1);

  if ( randomShiftFlag )
  {
    /// Print random shift vector when debugging
    if ( outputLevel >= DEBUG_OUTPUT )
    {
      Cout << "Using random shift ";
      for (size_t j=0; j < dMax; ++j)
      {
        Cout << randomShift[j] << " ";
      }
      Cout << std::endl;
    }
  }
  else
  {
    /// Print warning about missing random shift (will include the 0 point)
    if ( outputLevel >= QUIET_OUTPUT )
    {
      Cout << "WARNING: This lattice rule will not be randomized, samples "
        << "will include zeros as the first point!" << std::endl;
    }
  }

  ///
  /// Options for setting the ordering of this rank-1 lattice rule
  ///
  if ( ordering == RANK_1_LATTICE_NATURAL_ORDERING )
  {
    reorder = &Rank1Lattice::reorder_natural;
    scale = 1 / Real(1 << mMax);
  }
  else if ( ordering == RANK_1_LATTICE_RADICAL_INVERSE_ORDERING )
  {
    reorder = &Rank1Lattice::reorder_radical_inverse;
    scale = 1 / Real(4294967296L); // 1 / 2^32
  }
  else
  {
    Cerr << "Unknown ordering (" << ordering << ") requested." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  if ( outputLevel >= DEBUG_OUTPUT )
  {
    if ( ordering == RANK_1_LATTICE_NATURAL_ORDERING )
    {
      Cout << "Using natural ordering of the lattice points" << std::endl;
    }
    else
    {
      Cout << "Using radical inverse ordering of the lattice points" 
        << std::endl;
    }
  }
}

/// A constructor that uses radical inverse ordering and a random shift
Rank1Lattice::Rank1Lattice(
  const UInt32Vector& generatingVector,
  int mMax
) :
Rank1Lattice(
  generatingVector,
  mMax,
  true,
  generate_system_seed(),
  RANK_1_LATTICE_RADICAL_INVERSE_ORDERING,
  NORMAL_OUTPUT
)
{

}

/// A constructor with only the random seed as argument
Rank1Lattice::Rank1Lattice(
  int seedValue
) :
Rank1Lattice(
  UInt32Vector(Teuchos::View, &cools_kuo_nuyens_d250_m20[0], 250),
  20,
  true,
  seedValue,
  RANK_1_LATTICE_RADICAL_INVERSE_ORDERING,
  NORMAL_OUTPUT
)
{

}

/// A constructor with no arguments
Rank1Lattice::Rank1Lattice(
) :
Rank1Lattice(
  UInt32Vector(Teuchos::View, &cools_kuo_nuyens_d250_m20[0], 250),
  20,
  true,
  generate_system_seed(),
  RANK_1_LATTICE_RADICAL_INVERSE_ORDERING,
  NORMAL_OUTPUT
)
{

}

/// A constructor that takes a problem description database
Rank1Lattice::Rank1Lattice(
  ProblemDescDB& problem_db
) :
Rank1Lattice(
  get_data(problem_db),
  problem_db
)
{

}

/// A constructor that takes a tuple and a problem description database
Rank1Lattice::Rank1Lattice(
  std::tuple<UInt32Vector, int> data,
  ProblemDescDB& problem_db
) :
Rank1Lattice(
  std::get<0>(data), /// Unpack generating vector
  std::get<1>(data), /// Unpack log2 of maximum number of points
  !problem_db.get_bool("method.no_random_shift"),
  problem_db.get_int("method.random_seed") ?
    problem_db.get_int("method.random_seed") :
    generate_system_seed(),
  problem_db.get_bool("method.ordering.natural") ? 
    RANK_1_LATTICE_NATURAL_ORDERING :
    RANK_1_LATTICE_RADICAL_INVERSE_ORDERING,
  problem_db.get_short("method.output")
)
{

}

/// Destructor
Rank1Lattice::~Rank1Lattice()
{

}

/// Extract the generating vector and log2 of the maximum number of points from
/// the given problem description database
/// There are 3 different ways to specify a generating vector:
///
/// +--------------------------------------------------+
/// | Case I:  "generating_vector file [...]"          |
/// +--------------------------------------------------+
/// The generating vector will be read from the file with the given name
///
/// +--------------------------------------------------+
/// | Case II: "generating_vector inline [...]"        |
/// +--------------------------------------------------+
/// Assumes the generating vector is given as an inline sequence of
/// integers
///
/// +--------------------------------------------------+
/// | Case III:   "generating_vector predefined [...]" |
/// +--------------------------------------------------+
/// The given name should match the name of a predefined generating vector
/// Choose one of:
///  * cools_kuo_nuyens
///  * kuo
std::tuple<UInt32Vector, int> Rank1Lattice::get_data(
  ProblemDescDB& problem_db
)
{
  /// Name of the file with the generating vector
  String file = problem_db.get_string("method.generating_vector.file");

  /// Get the inline generating vector
  IntVector inlineVector = 
    problem_db.get_iv("method.generating_vector.inline");
  size_t len = inlineVector.length();

  /// NOTE: outputLevel has not been set yet, so gettting it directly from
  /// the 'problem_db' instead
  bool outputLevel = problem_db.get_short("method.output");

  /// Case I: the generating vector is provided in an external file
  if ( !file.empty() )
  {
    if ( outputLevel >= DEBUG_OUTPUT )
    {
      Cout << "Reading generating vector from file " << file << "..."
        << std::endl;
    }
    
    return get_generating_vector_from_file(problem_db);
  }

  /// Case II: the generating vector is provided in the input file
  else if ( len > 0 )
  {
    if ( outputLevel >= DEBUG_OUTPUT )
    {
      Cout << "Reading inline generating vector..." << std::endl;
    }

    return get_inline_generating_vector(problem_db);
  }

  /// Case III: a default generating vector has been selected
  else
  {
    /// Verify that `mMax` has not been provided
    if ( problem_db.get_int("method.m_max") )
    {
      Cerr << "\nError: you can't specify a default generating vector and "
        << "the log2 of the maximum number of points 'm_max' at the same "
        << "time." << std::endl;
      abort_handler(METHOD_ERROR);
    }

    return get_default_generating_vector(problem_db);
  }
}

/// Case I: the generating vector is provided in an external file
const std::tuple<UInt32Vector, int> Rank1Lattice::get_generating_vector_from_file(
  ProblemDescDB& problem_db
)
{
  String fileName = problem_db.get_string("method.generating_vector.file");

  /// Wrap in try-block
  try{
    int nbOfRows = count_rows(fileName);
    UInt32Vector generatingVector(nbOfRows);
    std::fstream file(fileName);
    String line;
    int j = 0;
    while (std::getline(file, line))
    {
      generatingVector[j++] = std::stoull(line);
    }

    return std::make_tuple(
      generatingVector,
      problem_db.get_int("method.m_max")
    );
  }
  catch (...) /// Catch-all handler
  {
    Cerr << "Error: error while parsing generating vector from file '"
      << fileName << "'" << std::endl;
    abort_handler(METHOD_ERROR);

  }
}

/// Case II: the generating vector is provided in the input file
const std::tuple<UInt32Vector, int> Rank1Lattice::get_inline_generating_vector(
  ProblemDescDB& problem_db
)
{
  /// Get the inline generating vector
  IntVector inlineVector = 
    problem_db.get_iv("method.generating_vector.inline");
  size_t len = inlineVector.length();
  
  /// Can't get away without making a copy here, conversion from
  /// int to UInt32, maybe there's a smarter way to do this?
  /// NOTE: that smarter way would probably mean implementing
  /// 'UINT32LIST' instead of 'INTEGERLIST'
  UInt32Vector generatingVector;
  generatingVector.resize(len);
  for (size_t j = 0; j < len; ++j)
  {
    generatingVector[j] = inlineVector[j];
  }

  return std::make_tuple(
    generatingVector,
    problem_db.get_int("method.m_max")
  );
}

/// Case III: a default generating vector has been selected
const std::tuple<UInt32Vector, int> Rank1Lattice::get_default_generating_vector(
  ProblemDescDB& problem_db
)
{
  /// NOTE: outputLevel has not been set yet, so gettting it directly from
  /// the 'problem_db' instead
  bool outputLevel = problem_db.get_short("method.output");

  /// Select predefined generating vector
  if ( problem_db.get_bool("method.kuo") )
  {
    if ( outputLevel >= DEBUG_OUTPUT )
    {
      Cout << "Found predefined generating vector 'kuo'"
        << std::endl;
    }

    return std::make_tuple(
      UInt32Vector(Teuchos::View, &kuo_d3600_m20[0], 3600),
      20
    );
  }
  else
  {
    if ( outputLevel >= DEBUG_OUTPUT )
    {
      if ( problem_db.get_bool("method.cools_kuo_nuyens") )
      {
        Cout << "Found predefined generating vector 'cools_kuo_nuyens'"
          << std::endl;
      }
      else
      {
        Cout << "No generating vector provided, using fall-back option "
          << "'cools_kuo_nuyens'" << std::endl;
      }
    }

    return std::make_tuple(
      UInt32Vector(Teuchos::View, &cools_kuo_nuyens_d250_m20[0], 250),
      20
    );
  }
}

/// Randomize this low-discrepancy sequence
/// Uses the given seed to initialize the RNG
/// When the seed is < 0, the random shift will be removed
void Rank1Lattice::random_shift(
  int seed
)
{
  /// NOTE: lhsDriver is really slow, switching to boost since
  /// variables are uncorrelated
  // Real zeros[dMax] = { }; std::fill(zeros, zeros + dMax, 0);
  // Real ones[dMax] = { }; std::fill(ones, ones + dMax, 1);
  // const RealVector lower(Teuchos::View, zeros, dMax);
  // const RealVector upper(Teuchos::View, ones, dMax);
  // Pecos::LHSDriver lhsDriver("random");
  // RealSymMatrix corr; // Uncorrelated random variables
  // lhsDriver.seed(seedValue);
  // lhsDriver.generate_uniform_samples(lower, upper, corr, 1, randomShift);
  randomShift.resize(dMax);
  if ( !randomShiftFlag || seed < 0 )
  {
    randomShift = 0; /// Sets all entries to 0
  }
  else
  {
    boost::random::mt19937 rng(seed);
    boost::uniform_01<boost::mt19937> sampler(rng);
    for (size_t j = 0; j < dMax; ++j)
    {
      randomShift[j] = sampler();
    }
  }
}

/// Generates rank-1 lattice points without error checking
/// Returns the points with index `nMin`, `nMin` + 1, ..., `nMax` - 1
/// This function will store the points in-place in the matrix `points`
/// Each column of `points` contains a `dimension`-dimensional point
/// where `dimension` is equal to the number of rows of `points`
void Rank1Lattice::unsafe_get_points(
  const size_t nMin,
  const size_t nMax, 
  RealMatrix& points
)
{
  for ( UInt32 k = nMin; k < nMax; ++k ) /// Loop over all points
  {
    Real phik = (this->*reorder)(k) * scale; /// phi(k)
    for ( int j = 0; j < points.numRows(); ++j ) /// Loop over all dimensions
    {
      Real point = phik * generatingVector[j] + randomShift[j];
      points[k - nMin][j] = point - std::floor(point); /// Map to [0, 1)
    }
  }
}

/// Position of the `k`th lattice point in RANK_1_LATTICE_NATURAL_ORDERING
inline UInt32 Rank1Lattice::reorder_natural(
  UInt32 k
)
{
  return k;
}

/// Position of the `k`th lattice point in RANK_1_LATTICE_RADICAL_INVERSE_ORDERING
inline UInt32 Rank1Lattice::reorder_radical_inverse(
  UInt32 k
)
{
  return bitreverse(k);
}

} // namespace Dakota