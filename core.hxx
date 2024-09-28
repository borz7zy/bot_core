#ifndef _CORE_HXX
#define _CORE_HXX

#include <logprint.hxx>
#include "minor_scripts.hxx"
#include "main_script.hxx"
#include "plugin_manager.hxx"
#include "lua_natives.hxx"
#include "tick_sys.hxx"
#include <vector>
#include <string>
#include <iostream>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class Core
{
public:
    Core(
        std::string pPath,
        std::vector<std::string> minScripts,
        std::string maScript,
        uint8_t tickpeersecond) : LogsCore("CORE", "./core.log"),
                                  minorScripts(minScripts),
                                  mainScript(maScript),
                                  plPath(pPath),
                                  tickSys(tickpeersecond, [this]()
                                          { tick_update(); })
    {
        L = luaL_newstate();
        if (L == nullptr)
        {
            logger->LOGE("Core initialization error: Lua could not allocate state");
            return;
        }
        luaL_openlibs(L);

        lua_pushnil(L);
        lua_setglobal(L, "io");
        lua_pushnil(L);
        lua_setglobal(L, "os");
        lua_pushnil(L);
        lua_setglobal(L, "package");

        pluginManager.LoadPlugins(plPath);

        loadedLibraries = pluginManager.GetLoadedLibraries();

        for (const auto &[plugin, name] : loadedLibraries)
        {
            luaNatives.CallGetNatives(L, plugin, name.c_str());
        }

        lua_register(L, "load_module", safe_require);

        loadMinor();

        loadMain();

        logger->LOGI("The core was initialized successfully.");

        tickSys.start();
    }
    ~Core()
    {
        if (L)
        {
            lua_close(L);
        }
        logger->LOGI("Shutting down the core.");
    }

private:
    lua_State *L;

    logprint LogsCore;
    logprint *logger = &LogsCore;

    std::string plPath;

    std::vector<std::string> minorScripts;
    std::string mainScript;

    PluginManager pluginManager;

    std::vector<std::pair<void *, std::string>> loadedLibraries;

    LuaNatives luaNatives;

    void tick_update();
    TickSys tickSys;

    static int safe_require(lua_State *L);

    void loadMinor();
    void loadMain();
};

#endif