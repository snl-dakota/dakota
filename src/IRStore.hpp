/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_IR_STORE_H
#define DAKOTA_IR_STORE_H

#include "dakota_data_types.hpp"
#include "generated_ir_types.hpp"

#include <stdexcept>
#include <unordered_map>

namespace Dakota {

namespace irgen = dakota::irgen;

/// Unified IR value representation (shared with generated contract defaults).
using IRValue = dakota::irgen::IRValue;

/// Block-local key/value storage for materialized IR data.
class IRStore
{
public:
  using Map = std::map<String, IRValue>;

  bool contains(const String& local_key) const
  { return values_.find(local_key) != values_.end(); }

  void set_value(const String& local_key, IRValue value)
  {
    values_[local_key] = std::move(value);
  }

  template <class T>
  const T& get(const String& local_key) const
  {
    auto it = values_.find(local_key);
    if (it == values_.end())
      throw std::runtime_error("IRStore::get: missing key '" + local_key + "'");

    const T* ptr = std::get_if<T>(&it->second);
    if (!ptr)
      throw std::runtime_error("IRStore::get: type mismatch for key '" + local_key + "'");

    return *ptr;
  }

  const Map& values() const
  { return values_; }

private:
  Map values_;
};

} // namespace Dakota

#endif // DAKOTA_IR_STORE_H
