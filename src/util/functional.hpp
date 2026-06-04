#include <tuple>

namespace Dakota {
namespace Util {
  template<typename F>
  struct FunctionInfo;

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(Args...)>
  {
    using return_type = Return;
    using arg_tuple = std::tuple<Args...>;
  };

  template<typename Return, typename... Args>
  struct FunctionInfo<Return(*)(Args...)> : FunctionInfo<Return(Args...)>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...)> : FunctionInfo<Return(Args...)>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const> : FunctionInfo<Return(Args...)>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) noexcept> : FunctionInfo<Return(Args...)>
  {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionInfo<Return(Class::*)(Args...) const noexcept> : FunctionInfo<Return(Args...)>
  {};
}
}
