#ifndef BOOL_DISPATCH_HPP
#define BOOL_DISPATCH_HPP

#include <type_traits>
#include <utility>

namespace rol_interface {

struct BoolDispatch {
  constexpr explicit BoolDispatch( bool value_in ) noexcept : value{value_in} {}
  constexpr operator bool () const noexcept { return value; }

  constexpr BoolDispatch operator ! () const noexcept {
    return BoolDispatch(!value);
  }

  constexpr BoolDispatch operator && ( BoolDispatch other ) const noexcept {
    return BoolDispatch(value && other.value);
  } 

  constexpr BoolDispatch operator || ( BoolDispatch other ) const noexcept {
    return BoolDispatch(value || other.value);
  } 

  constexpr BoolDispatch operator ^ ( BoolDispatch other ) const noexcept {
    return BoolDispatch(static_cast<bool>(value ^ other.value));
  }

  template<class F>
  void receive( F&& f ) const {
    static_assert(std::is_invocable_v<decltype(f),std::bool_constant<true>>,
                  "Error: Function must be callable with an argument of type std::bool_constant<true>");
    static_assert(std::is_invocable_v<decltype(f),std::bool_constant<false>>,
                  "Error: Function must be callable with an argument of type std::bool_constant<true>");
    if(value) std::forward<decltype(f)>(f)(std::bool_constant<true>{});
    else      std::forward<decltype(f)>(f)(std::bool_constant<false>{});
  }  

  const bool value;
}; // struct BoolDispatch



} // namewspace rol_interface

#endif // BOOL_DISPATCH_HPP

