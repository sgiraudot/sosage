#include <Sosage/Engine.h>
#include <Sosage/Third_party/Lua.h>

namespace Sosage::Third_party
{

Lua::Lua ()
{
  m_state = luaL_newstate();

  static const luaL_Reg lualibs[] =
    {
      { "base", luaopen_base },
      { NULL, NULL}
    };

  const luaL_Reg *lib = lualibs;
  for(; lib->func != NULL; lib++)
  {
    lib->func(m_state);
    lua_settop(m_state, 0);
  }

  lua_register
    (m_state, "set_image",
     [](lua_State* L) -> int
     {
       int n = lua_gettop(L);
       if(n != 5)
       {
         lua_pushstring(L, "Not enough parameter.");
         lua_error(L);
       }

       const char* key = lua_tostring(L, 1);
       const char* file_name = lua_tostring(L, 2);
       int x = std::atoi(lua_tostring(L, 3));
       int y = std::atoi(lua_tostring(L, 4));
       int z = std::atoi(lua_tostring(L, 5));

       engine().set_image(key, file_name, x, y, z);

       return 0;
     });
  
  lua_register
    (m_state, "set_ground_map",
     [](lua_State* L) -> int
     {
       int n = lua_gettop(L);
       if(n != 2)
       {
         lua_pushstring(L, "Not enough parameter.");
         lua_error(L);
       }

       const char* key = lua_tostring(L, 1);
       const char* file_name = lua_tostring(L, 2);

       engine().set_ground_map(key, file_name);

       return 0;
     });

  lua_register
    (m_state, "pick",
     [](lua_State* L) -> int
     {

     });

  lua_register
    (m_state, "animate",
     [](lua_State* L) -> int
     {

     });

  lua_register
    (m_state, "open_window",
     [](lua_State* L) -> int
     {

     });
}

Lua::~Lua ()
{
  if (m_state)
    lua_close(m_state);
}

void Lua::read (const std::string& file_name)
{
  if (luaL_loadfile (m_state, file_name.c_str())
      || lua_pcall (m_state, 0, 0, 0))
    error ("Cannot read " + file_name + ":\n" + lua_tostring(m_state, -1));
}

} // namespace Sosage::Third_party
