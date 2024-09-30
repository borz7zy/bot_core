#ifndef _CORE_HXX
#define _CORE_HXX

#include "minor_scripts.hxx"
#include "main_script.hxx"
#include "plugin_manager.hxx"
#include "lua_natives.hxx"
#include "tick_sys.hxx"
#include <vector>
#include <string>
#include <stdexcept>
#include "global_sdk.hxx"

class Core
{
public:
    Core();
    ~Core();

private:
    void processScripts(const std::string &scripts, const std::string &basePath, std::vector<std::string> &output);

    struct LuaStateInfo
    {
        lua_State *L;
        const char *script;
    };
    std::vector<LuaStateInfo> luaStates;

    std::string plPath;

    std::vector<std::string> minorScripts;
    std::string mainScript;

    PluginManager pluginManager;

    void preloadStates(const char *script);
    lua_State *getStateFromVector(const char *script);

    std::vector<std::pair<void *, std::string>> loadedLibraries;

    LuaNatives luaNatives;

    uint8_t ticks;

    void callUpdatePlugin(void *plugin);
    void callUpdateInScript(const char *script);

    void tick_update();

    static int safe_require(lua_State *L);

    void loadMinor(lua_State *L, const char *minorScript);
    void loadMain(lua_State *L);
};

#endif