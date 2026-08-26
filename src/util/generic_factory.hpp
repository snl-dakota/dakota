#include <unordered_map>
#include <functional>
#include "functional.hpp"

namespace Dakota {
namespace Util {

/**
 * The default error policy for GenericFactory.
 * This error policy will return a default-constructed value of the same type as
 * the CreateFun's return type in the case of error.
 *
 * \tparam Key          the lookup key; any hashable type is allowed
 * \tparam CreateFun    the type of function for creating factory items
 */
template< typename Key, typename CreateFun >
struct DefaultGenericFactoryErrorPolicy
{
  using return_type = typename FunctionInfo<CreateFun>::return_type;

  template< typename... CreateArgs >
  return_type on_unknown_key(const Key &, CreateArgs &&...) const
  {
    return {};
  }
};

/**
 * A class for registering factory functions and calling them by providing a key.
 * Factory functions are added to the registry via register_key.
 * Providing a registered key to create will return the result of invoking the factory function registered with the key.
 *
 * \pre ErrorPolicy::on_unknown_key must have the same return type as CreateFun
 *
 * \tparam Key          the lookup key; any hashable type is allowed
 * \tparam CreateFun    the type of function for creating factory items
 * \tparam ErrorPolicy  a policy type that implements `on_unknown_key`
 */
template< typename Key, typename CreateFun, typename ErrorPolicy = DefaultGenericFactoryErrorPolicy<Key, CreateFun> >
class GenericFactory : public ErrorPolicy
{
public:

  using return_type = typename FunctionInfo<CreateFun>::return_type;

  /**
   * Register a factory function with the given key.
   * Future calls to create will invoke Fun when provided the same value for key.
   * If the key is already registered, Fun will not be inserted and this method will return false.
   *
   * \param key the key to associate the given function with
   * \param fun the function, functor, or function pointer associated with the key
   *
   * \return true if fun was successfully inserted for the given key; false if the key is already present in the registry
   */
  template< typename Fun >
  bool register_key( const Key &key, Fun &&fun )
  {
    return m_registrar.try_emplace( key, std::forward< Fun >( fun ) ).second;
  }

  /**
   * Returns whether a given key is registered.
   *
   * \param key the key to query
   *
   * \return true if the key is present in the registry; false otherwise
   */
  bool key_is_registered( const Key &key ) const noexcept { return m_registrar.find( key ) != m_registrar.end(); }

  /**
   * Invoke the factory function registered to the given key, returning its result if the key exists; otherwise returns ErrorPolicy::on_unknown_key(std::forward<Args>(args)...).
   *
   * \param key the key for looking up the factory function
   * \param args the arguments to forward to the factory function for the key if it exists; ErrorPolicy::on_unknown_key otherwise
   *
   * \return the result of invoking the factory function associated with key if it exists; ErrorPolicy::on_unknown_key otherwise
   */
  template< typename... Args >
  auto create( const Key &key, Args &&... args )
  {
    auto pos = m_registrar.find( key );
    if ( pos == m_registrar.end() )
    {
      return this->on_unknown_key(key, std::forward< Args >( args )...);
    }
    return pos->second(std::forward< Args >( args )...);
  }

private:

  std::unordered_map< Key, std::function< CreateFun > > m_registrar;
};
}
}
