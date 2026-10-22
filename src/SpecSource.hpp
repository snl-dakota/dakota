/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SPEC_SOURCE_H
#define DAKOTA_SPEC_SOURCE_H

#include "IRStore.hpp"
#include "ProblemDescDB.hpp"

#include <type_traits>

namespace Dakota {

/// Read-only view of specification data from either a ProblemDescDB or a
/// block-local IRStore, so constructors can share one body across the
/// legacy and DI paths.

/** Keys use the ProblemDescDB form ("model.surrogate.nugget"); for an
    IRStore the block prefix is stripped ("surrogate.nugget").  Keys
    absent from the store yield a value-initialized T, matching the
    defaults the ProblemDescDB path supplies for unspecified keywords. */
class SpecSource
{
public:

  /// view a ProblemDescDB
  explicit SpecSource(const ProblemDescDB& problem_db):
    problemDB(&problem_db), irStore(nullptr)
  { }

  /// view a block-local IRStore
  explicit SpecSource(const IRStore& store):
    problemDB(nullptr), irStore(&store)
  { }

  /// return the value for a ProblemDescDB-style key
  template <typename T>
  std::remove_const_t<T> get(const String& key) const
  {
    using ValueT = std::remove_const_t<T>;
    if (problemDB)
      return problemDB->get<T>(key);
    const String local = key.substr(key.find('.') + 1);
    return irStore->contains(local) ? irStore->get<ValueT>(local) : ValueT();
  }

  /// true if viewing a ProblemDescDB
  bool is_problem_db() const
  { return problemDB != nullptr; }

  /// the viewed ProblemDescDB; only valid when is_problem_db()
  const ProblemDescDB& problem_db() const
  { return *problemDB; }

private:

  const ProblemDescDB* problemDB; ///< non-null on the legacy path
  const IRStore*       irStore;   ///< non-null on the DI path
};

} // namespace Dakota

#endif // DAKOTA_SPEC_SOURCE_H
