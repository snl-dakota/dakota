/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef LOW_DISCREPANCY_SEQUENCE_H
#define LOW_DISCREPANCY_SEQUENCE_H

#include "dakota_data_types.hpp"
#include "dakota_stat_util.hpp"
// #include "ProblemDescDB.hpp"

namespace Dakota {

/// Abstract class for low-discrepancy sequences

/** This abstract class provides uniform access to all low-discrepancy 
    sequences through the `get_points` function.

    Derived classes must provide implementations for the private virtual method 
    `unsafe_get_points(nMin, nMax, points)` and the public virtual method 
    `randomize()`
*/
class LowDiscrepancySequence
{
public:

  /// Default constructor
  LowDiscrepancySequence(
    int mMax,
    int dMax,
    int seedValue,
    short outputLevel
  ) :
  mMax(mMax),
  dMax(dMax),
  seedValue(seedValue),
  outputLevel(outputLevel)
  {
    /// Check inputs in separate functions so that they can be overriden in
    /// a derived class to print more useful error messages
    check_dMax();
    check_mMax();
    check_seedValue();
  }

  /// Get points from this low-discrepancy generator
  /// This function will store the points in-place in the matrix `points`
  /// Each column of `points` contains a `dimension`-dimensional point
  /// where `dimension` is equal to the number of rows of `points` and the
  /// number of points is equal to the number of columns of `points`
  void get_points(
    RealMatrix& points
  )
  {
    get_points(points.numCols(), points);
  }

  /// Get the first `n` points from this low-discrepancy generator
  /// This function will store the points in-place in the matrix `points`
  /// Each column of `points` contains a `dimension`-dimensional point
  /// where `dimension` is equal to the number of rows of `points` 
  void get_points(
    const size_t n,
    RealMatrix& points
  )
  {
    get_points(0, n, points);
  }

  /// Generates low-discrepancy points between given indices
  /// Returns the points with index `nMin`, `nMin` + 1, ..., `nMax` - 1
  /// This function will store the points in-place in the matrix `points`
  /// Each column of `points` contains a `dimension`-dimensional point
  void get_points(
    const size_t nMin,
    const size_t nMax, 
    RealMatrix& points
  )
  {
    /// Check sizes of the matrix `points`
    check_sizes(nMin, nMax, points);

    /// Get the low-discrepancy points
    unsafe_get_points(nMin, nMax, points);

    /// Print summary info
    if ( outputLevel >= Pecos::VERBOSE_OUTPUT )
    {
      Cout << "Successfully generated " << points.numCols()
        << " low-discrepancy points in " << points.numRows() << " dimensions:"
        << std::endl;
      for ( int col=0; col<points.numCols(); ++col )
      {
        Cout << col + nMin << ": ";
        for ( int row=0; row<points.numRows(); ++row )
        {
          Cout << points[col][row] << " ";
        }
        Cout << std::endl;
      }
    }
  }

  /// Returns the random seed value
  int get_seed() {
    return seedValue;
  }

  /// Sets the random seed value
  void set_seed(int seed) {
    seedValue = seed;
    reseed();
  }

  /// Reseed this low-discrepancy sequence
  /// NOTE: this function is required by `LDDriver` to change
  /// the random seed
  virtual void reseed() = 0;

protected:

  /// Maximum dimension of this low-discrepancy sequence
  int dMax;

  /// `log2` of the maximum number of points of this low-discrepancy sequence
  /// - For rank-1 lattices, this is also the length of the generating vector
  /// - For digital nets, this is also the number of columns of each generating
  ///   matrix
  int mMax;

  /// The seed of this low-discrepancy sequence
  int seedValue;

  /// The output verbosity level, can be one of
  /// {SILENT, QUIET, NORMAL, VERBOSE, DEBUG}_OUTPUT
  short outputLevel;

  /// Perform checks on the matrix `points`
  /// Each column of `points` contains a `dimension`-dimensional point
  /// where `dimension` is equal to the number of rows of `points` 
  /// The number of points `numPoints` is `nMax` - `nMin`
  /// Checks if the requested number of points `numPoints` exceeds the maximum 
  /// number of points allowed in this low-discrepancy sequence
  /// Checks if the number of rows of the matrix `points` exceeds the maximum
  /// dimension allowed in this low-discrepancy sequence
  /// Checks if the matrix `points` has `numPoints` columns
  void check_sizes(
    const size_t nMin,
    const size_t nMax, 
    RealMatrix& points
  )
  {
    /// Check if maximum number of points is exceeded
    auto maxPoints = UInt64(1) << mMax;
    if ( nMax > std::numeric_limits<UInt64>::max() || nMax > maxPoints )
    {
      Cerr << "\nError: requested number of samples " << nMax
        << " is larger than the maximum allowed number of points "
        << maxPoints << "." << std::endl;
      abort_handler(METHOD_ERROR);
    }

    /// Check if maximum dimension is exceeded
    auto dimension = points.numRows();
    if ( dimension > dMax )
    {
      Cerr << "\nError: this low-discrepancy sequence can only generate "
        << "points in dimension " << dMax << " or less, got " 
        << dimension << "." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    
    /// Check number of columns of points
    auto numPoints = points.numCols();
    if ( numPoints != nMax - nMin )
    {
      Cerr << "\nError: requested low-discrepancy points between index " 
        << nMin << " and " << nMax << ", but the provided matrix expects "
        << numPoints << " points." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  /// Generate points from this low-discrepancy sequence
  virtual void unsafe_get_points(
      const size_t nMin,
      const size_t nMax, 
      RealMatrix& points
  ) = 0;

private:

  /// Perform checks on dMax
  /// Checks if dMax is positive (> 0)
  void check_dMax()
  {
    if ( dMax < 1 )
    {
      Cerr << "\nError: maximum dimension 'dMax' must be positive (> 0), " 
        << "got " << dMax << ". Did you specify an empty generating vector "
        << "or empty generating matrices?" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  /// Perform checks on mMax
  /// Checks if mMax is positive (> 0)
  void check_mMax()
  {
    if ( mMax < 1 )
    {
      Cerr << "\nError: log2 of the maximum number of points 'mMax' must be "
        << "positive (> 0), got " << mMax << ". Did you provide a default "
        << "generating vector or default generating matrices, but forgot to "
        << "set the keyword 'm_max' in the input file?" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  /// Perform checks on seedValue
  /// Checks if seedValue is positive or zero (>= 0)
  void check_seedValue()
  {
    if ( seedValue < 0 )
    {
      Cerr << "\nError: value for random seed 'seedValue' must be 0 or more, "
        <<  "got " << seedValue << "." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

};

} // namespace Dakota

#endif
