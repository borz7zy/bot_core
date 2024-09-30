#ifndef _LUA_NATIVES_HXX
#define _LUA_NATIVES_HXX

#include <vector>
#include "global_sdk.hxx"

#define REGISTER_NATIVES_NAME "RegisterNatives"

class LuaNatives
{
public:
    void CallGetNatives(lua_State *L, void *plugin, const char *lib_name);
};

#endif