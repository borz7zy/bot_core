#ifndef _LUA_NATIVES_HXX
#define _LUA_NATIVES_HXX

#include <vector>
#include "global_sdk.hxx"

#define REGISTER_NATIVES_NAME "RegisterNatives"

class LuaNatives
{
public:
    void CallGetNatives(lua_State *L, void *plugin, const char *lib_name);
    void RegisterLuaNatives(lua_State *L);

private:
    static int native_GetEnv(lua_State *L); // GetEnv
    static int native_print(lua_State *L);  // print

    const Native_Function_List native_list[3] = {
        {"GetEnv", native_GetEnv},
        {"print", native_print},
        {nullptr, nullptr}};
};

#endif