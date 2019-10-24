#ifndef SOSAGE_THIRD_PARTY_LUA_H
#define SOSAGE_THIRD_PARTY_LUA_H

#include <lua.hpp>

#include <string>

namespace Sosage::Third_party
{

class Lua
{

  lua_State* m_state;
  
public:

  Lua ();
  ~Lua ();

  void read (const std::string& file_name);
};

} // namespace Sosage::Third_party
  

#endif // SOSAGE_THIRD_PARTY_LUA_H
