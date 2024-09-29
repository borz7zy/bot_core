#ifndef _CORE_HXX
#define _CORE_HXX

#include <logprint.hxx>
#include "config_manager.hxx"
#include "minor_scripts.hxx"
#include "main_script.hxx"
#include "plugin_manager.hxx"
#include "lua_natives.hxx"
#include "tick_sys.hxx"
#include <vector>
#include <string>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class Core
{
public:
    Core() : LogsCore("CORE", "./core.log")
    {
        if (config->load("./bot_core.conf"))
        {
            std::string mainScriptPath = config->get("main_script_path");
            std::string misPath = config->get("minor_scripts_path");
            plPath = config->get("plugins_path");

            std::string minScripts = config->get("minor_scripts");
            std::string mainScriptNP = config->get("main_script");
            ticks = std::stoi(config->get("ticks"));

            processScripts(minScripts, misPath, minorScripts);

            mainScript = mainScriptPath + "/" + mainScriptNP;
        }
        else
        {
            if (config->generateConfig())
            {
                logger->LOGI("Directories and a standard config were generated. Please configure the config and run the core again!");
            }
            return;
        }

        TickSys tickSys(ticks, [this]()
                        { tick_update(); });

        for (const auto &minorScript : minorScripts)
        {
            preloadStates(minorScript.c_str());
        }

        preloadStates(mainScript.c_str());

        pluginManager.LoadPlugins(plPath);

        loadedLibraries = pluginManager.GetLoadedLibraries();

        for (const auto &stateInfo : luaStates)
        {
            for (const auto &[plugin, name] : loadedLibraries)
            {
                luaNatives.CallGetNatives(stateInfo.L, plugin, name.c_str());
            }

            lua_register(stateInfo.L, "load_module", safe_require);
        }

        for (const auto &minorScript : minorScripts)
        {
            try
            {
                lua_State *state = getStateFromVector(minorScript.c_str());
                loadMinor(state, minorScript.c_str());
            }
            catch (const std::runtime_error &e)
            {
                logger->LOGE("Error while searching state for script %s: %s", minorScript.c_str(), e.what());
            }
        }

        try
        {
            lua_State *state = getStateFromVector(mainScript.c_str());
            loadMain(state);
        }
        catch (const std::runtime_error &e)
        {
            logger->LOGE("Error while searching state for script %s: %s", mainScript.c_str(), e.what());
        }

        logger->LOGI("The core was initialized successfully.");

        tickSys.start();
    }
    ~Core()
    {
        for (const auto &stateInfo : luaStates)
        {
            if (stateInfo.L)
            {
                lua_close(stateInfo.L);
            }
        }
        logger->LOGI("Shutting down the core.");
    }

private:
    void processScripts(const std::string &scripts, const std::string &basePath, std::vector<std::string> &output);

    ConfigManager cManager;
    ConfigManager *config = &cManager;

    struct LuaStateInfo
    {
        lua_State *L;
        const char *script;
    };
    std::vector<LuaStateInfo> luaStates;

    logprint LogsCore;
    logprint *logger = &LogsCore;

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