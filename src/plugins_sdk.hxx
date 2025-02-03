#ifndef _PLUGINS_SDK_HXX
#define _PLUGINS_SDK_HXX

extern "C"
{
#include "lua.h"
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

class SDKBase
{
public:
    virtual ~SDKBase() = default;
    virtual void update() = 0;
    virtual void register_lua_natives(lua_State *L) = 0;
    virtual void onRun() = 0;
};

#define PLUGIN_ENTRY extern "C" PLUGIN_EXPORT SDKBase *PluginEntryGet

extern "C" void luaRegister(lua_State *L, const char *name, int (*func)(lua_State *));

#endif