#include <tuple>
#include <type_traits>

namespace Dakota {
namespace Util {
  /**
   * Utility to introspect various parts of a function type, functor, function pointer type, and member function type.
   *
   * \tparam F  The type to introspect
   */
  template<typename F>
  struct FunctionInfo : FunctionInfo<decltype(&std::remove_reference_t<F>::operator())>
  {};

  // There follows a number of specializations according to the type we are introspecting
  // and the const/noxcept/lvalue/rvalue specifiers. We need a specialization for each one
  // so this part is a bit verbose, though mostly copy-and-paste boilerplate.
  //
  // In the future we can easily extend this to have a data member specifying whether the function is
  // const, noexcept, the class type, etc.

  // Free function specializations
  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...)>
  {
    using return_type = Return;             ///< The return type of the function
    using arg_tuple = std::tuple<Args...>;  ///< A tuple containing the argument types of the function
  };

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) noexcept> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) &> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) & noexcept> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) &&> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) && noexcept> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) const> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) const noexcept> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) const &> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) const & noexcept> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) const &&> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...) const && noexcept> : FunctionInfo<Return(Args...)>
  {};

  // Function pointer specializations
  // We only need two of these since function pointers cannot be `const` or l/r-value qualified

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(*)(Args...)> : FunctionInfo<Return(Args...)>
  {};

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(*)(Args...) noexcept> : FunctionInfo<Return(Args...) noexcept>
  {};

  // Member function specializations

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...)> : FunctionInfo<Return(Args...)>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) noexcept> : FunctionInfo<Return(Args...) noexcept>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) &> : FunctionInfo<Return(Args...) &>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) & noexcept> : FunctionInfo<Return(Args...) & noexcept>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) &&> : FunctionInfo<Return(Args...) &&>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) && noexcept> : FunctionInfo<Return(Args...) && noexcept>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const> : FunctionInfo<Return(Args...) const>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const noexcept> : FunctionInfo<Return(Args...) const noexcept>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const &> : FunctionInfo<Return(Args...) const &>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const & noexcept> : FunctionInfo<Return(Args...) const & noexcept>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const &&> : FunctionInfo<Return(Args...) const &&>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const && noexcept> : FunctionInfo<Return(Args...) const && noexcept>
  {};
}
}
