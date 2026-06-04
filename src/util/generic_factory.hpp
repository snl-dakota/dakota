#include <unordered_map>
#include <functional>
#include "functional.hpp"

namespace Dakota {
namespace Util {
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

template< typename Key, typename CreateFun, typename ErrorPolicy = DefaultGenericFactoryErrorPolicy<Key, CreateFun> >
class GenericFactory : public ErrorPolicy
{
public:

  using return_type = typename FunctionInfo<CreateFun>::return_type;
  static_assert(std::is_same_v<return_type, typename FunctionInfo<decltype(&ErrorPolicy::on_unknown_key)>::return_type>);

  template< typename Fun >
  bool register_key( const Key &key, Fun &&fun )
  {
    return m_registrar.try_emplace( key, std::forward< Fun >( fun ) ).second;
  }

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
