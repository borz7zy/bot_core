#ifndef _LUA_NATIVES_HXX
#define _LUA_NATIVES_HXX

#include <vector>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define REGISTER_NATIVES_NAME "RegisterNatives"

class LuaNatives
{
public:
    LuaNatives() {}
    ~LuaNatives() {}

    void CallGetNatives(lua_State *L, void *plugin, const char *lib_name);
};

#endif