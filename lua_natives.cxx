#include "lua_natives.hxx"
#include "plugin_manager.hxx"
#include "globals.hxx"
#include "utils.hxx"
#include <sstream>

typedef Native_Function_List *(*RegisterNativesFunc)();

void LuaNatives::CallGetNatives(lua_State *L, void *plugin, const char *lib_name)
{
    RegisterNativesFunc registerNatives = nullptr;

#if defined(_WIN32)
    registerNatives = (RegisterNativesFunc)GetProcAddress((HMODULE)plugin, REGISTER_NATIVES_NAME);
#else
    registerNatives = (RegisterNativesFunc)dlsym(plugin, REGISTER_NATIVES_NAME);
#endif
    if (!registerNatives)
    {
#ifdef _WIN32
        int errorCode = static_cast<int>(GetLastError());
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf, 0, NULL);
        logp->printlf("Failed to get RegisterNatives function: %d - %s", errorCode, (char *)lpMsgBuf);
        LocalFree(lpMsgBuf);
#else
        logp->printlf("Failed to get RegisterNatives function: %s", dlerror());
#endif
        return;
    }
    Native_Function_List *nativeList = registerNatives();

    if (!nativeList)
    {
#ifdef DEBUG
        logp->printlf("RegisterNatives returned null!");
#endif
        return;
    }
    for (int i = 0; nativeList[i].nativeName != nullptr; ++i)
    {
        lua_register(L, nativeList[i].nativeName, nativeList[i].funcPtr);
    }
}

void LuaNatives::RegisterLuaNatives(lua_State *L)
{
    for (int i = 0; native_list[i].nativeName != nullptr; ++i)
    {
        lua_register(L, native_list[i].nativeName, native_list[i].funcPtr);
    }
}

int LuaNatives::safe_require(lua_State *L)
{
    const char *module_name = lua_tostring(L, 1);

    std::string module_path = "./modules/" + std::string(module_name) + ".luac";

    if (luaL_loadfile(L, module_path.c_str()) != LUA_OK)
    {
        const char *error = lua_tostring(L, -1);
        lua_pushstring(L, error);
        return lua_error(L);
    }

    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
    {
        const char *error = lua_tostring(L, -1);
        lua_pushstring(L, error);
        return lua_error(L);
    }

    lua_setglobal(L, module_name);

    return 0;
}

int LuaNatives::native_GetEnv(lua_State *L)
{
    Utils utils;
    const char *env_name = luaL_checkstring(L, 1);
    const char *env_result = utils.GetEnv(env_name);
    lua_pushstring(L, env_result);
    return 1;
}

int LuaNatives::native_print(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    logp->printlf(str);
    lua_pushinteger(L, true);
    return 1;
}

//---------------------Async HTTP functions-------------------------------------
// int LuaNatives::native_http_set_bind_address(lua_State *L)
// {
//     const char *address = luaL_optstring(L, 1, nullptr);
//     asyncHttp.SetBindAddress(address);
//     return 0;
// }

// int LuaNatives::native_http_request(lua_State *L)
// {
//     int method = luaL_checkinteger(L, 1);           // Get HTTP method type
//     const char *url = luaL_checkstring(L, 2);       // URL
//     const char *referer = luaL_optstring(L, 3, ""); // Optional referer

//     std::vector<PostParameter> params;
//     if (lua_istable(L, 4))
//     {
//         lua_pushnil(L); // push a nil key onto the stack to start iterate
//         while (lua_next(L, 4) != 0)
//         { // iterate over the table
//             //  key is at -2 and value at -1
//             if (lua_istable(L, -1))
//             {
//                 lua_getfield(L, -1, "name");
//                 const char *name = luaL_checkstring(L, -1);
//                 lua_pop(L, 1);
//                 lua_getfield(L, -1, "value");
//                 const char *value = luaL_optstring(L, -1, "");
//                 lua_pop(L, 1);
//                 lua_getfield(L, -1, "filename");
//                 const char *filename = luaL_optstring(L, -1, "");
//                 lua_pop(L, 1);
//                 lua_getfield(L, -1, "contentType");
//                 const char *contentType = luaL_optstring(L, -1, "");
//                 lua_pop(L, 1);
//                 lua_getfield(L, -1, "isFile");
//                 bool isFile = lua_toboolean(L, -1);
//                 lua_pop(L, 1);
//                 params.push_back({name, value, filename, contentType, isFile});
//             }
//             lua_pop(L, 1); // removes value
//         }
//     }

//     int id = asyncHttp.StartRequest(method, url, params, referer);
//     lua_pushinteger(L, id);
//     return 1;
// }

// int LuaNatives::native_http_is_complete(lua_State *L)
// {
//     int id = luaL_checkinteger(L, 1);
//     bool isComplete = asyncHttp.IsRequestComplete(id);
//     lua_pushboolean(L, isComplete);
//     return 1;
// }

// int LuaNatives::native_http_get_result(lua_State *L)
// {
//     int id = luaL_checkinteger(L, 1);
//     auto [status, response] = asyncHttp.GetRequestResult(id);

//     lua_pushinteger(L, status); // Push the HTTP status code
//     if (response)
//     {
//         lua_createtable(L, 0, 5); // Create table for response
//         // Header
//         lua_pushstring(L, response->header);
//         lua_setfield(L, -2, "header");
//         // Response body
//         lua_pushstring(L, response->response);
//         lua_setfield(L, -2, "body");
//         // Length of body
//         lua_pushinteger(L, response->response_len);
//         lua_setfield(L, -2, "body_len");
//         // Status Code
//         lua_pushinteger(L, response->response_code);
//         lua_setfield(L, -2, "status_code");
//         // Content type
//         lua_pushinteger(L, response->content_type);
//         lua_setfield(L, -2, "content_type");
//         return 2; // Return status code and response table
//     }
//     else
//     {
//         return 1; // Return status code if no response
//     }
// }