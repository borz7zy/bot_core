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