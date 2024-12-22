#ifndef _LUA_NATIVES_HXX
#define _LUA_NATIVES_HXX

#include <vector>
#include "global_sdk.hxx"
#include "asynchttp.hxx" // Include AsyncHttp
#include <string>
#include <memory>

#define REGISTER_NATIVES_NAME "RegisterNatives"

class LuaNatives
{
public:
    void CallGetNatives(lua_State *L, void *plugin, const char *lib_name);
    void RegisterLuaNatives(lua_State *L);

private:
    static int safe_require(lua_State *L);

    static int native_GetEnv(lua_State *L); // GetEnv
    static int native_print(lua_State *L);  // print

    // Async http
    static int native_http_request(lua_State *L);
    static int native_http_is_complete(lua_State *L);
    static int native_http_get_result(lua_State *L);
    static int native_http_set_bind_address(lua_State *L);

    const Native_Function_List native_list[8] = {
        {"load_module", safe_require},
        {"GetEnv", native_GetEnv},
        {"print", native_print},
        {"http_request", native_http_request},
        {"http_is_complete", native_http_is_complete},
        {"http_get_result", native_http_get_result},
        {"http_set_bind_address", native_http_set_bind_address},
        {nullptr, nullptr}};

    static AsyncHttp asyncHttp; // Static instance of AsyncHttp
};
#endif