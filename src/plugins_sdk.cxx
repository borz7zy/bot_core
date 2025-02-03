#include "plugins_sdk.hxx"

// extern "C" void luaRegister(lua_State *L, const char *name, int (*func)(lua_State *));
void luaRegister(lua_State *L, const char *name, int (*func)(lua_State *))
{
    lua_register(L, name, func);
}