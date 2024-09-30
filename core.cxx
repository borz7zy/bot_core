#include "core.hxx"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <cstring>
#include <sstream>
#include "globals.hxx"

Core::Core()
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
            logp->printlf("Directories and a standard config were generated. Please configure the config and run the core again!");
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
            logp->printlf("Error while searching state for script %s: %s", minorScript.c_str(), e.what());
        }
    }

    try
    {
        lua_State *state = getStateFromVector(mainScript.c_str());
        loadMain(state);
    }
    catch (const std::runtime_error &e)
    {
        logp->printlf("Error while searching state for script %s: %s", mainScript.c_str(), e.what());
    }

    logp->printlf("The core was initialized successfully.");

    tickSys.start();
}
Core::~Core()
{
    for (const auto &stateInfo : luaStates)
    {
        if (stateInfo.L)
        {
            lua_close(stateInfo.L);
        }
    }
    logp->printlf("Shutting down the core.");
}

void Core::processScripts(const std::string &scripts, const std::string &basePath, std::vector<std::string> &output)
{
    std::istringstream iss(scripts);
    std::string script;
    while (iss >> script)
    {
        output.push_back(basePath + "/" + script);
    }
}

int Core::safe_require(lua_State *L)
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

void Core::loadMinor(lua_State *L, const char *minorScript)
{
    MinorScripts minScripts;
    minScripts.load(L, minorScript);
}

void Core::loadMain(lua_State *L)
{
    MainScript mScript;
    mScript.load(L, mainScript.c_str());
}

void Core::callUpdatePlugin(void *plugin)
{
    typedef void (*VoidFunction)();
    VoidFunction func = nullptr;

#ifdef _WIN32
    HMODULE hModule = static_cast<HMODULE>(plugin);
    func = (VoidFunction)GetProcAddress(hModule, "update_ticks");

    if (!func)
    {
        DWORD error = GetLastError();
        std::cerr << "Failed to get function address: " << error << std::endl;
        return;
    }
#else
    func = (VoidFunction)dlsym(plugin, "update_ticks");

    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        std::cerr << "Failed to get function address: " << dlsym_error << std::endl;
        return;
    }
#endif

    func();
}

void Core::preloadStates(const char *script)
{
    lua_State *L = luaL_newstate();
    if (L == nullptr)
    {
        logp->printlf("Core initialization error: Lua could not allocate state");
        return;
    }
    luaL_openlibs(L);

    lua_pushnil(L);
    lua_setglobal(L, "io");
    lua_pushnil(L);
    lua_setglobal(L, "os");
    lua_pushnil(L);
    lua_setglobal(L, "package");

    LuaStateInfo stateInfo;
    stateInfo.L = L;
    stateInfo.script = script;

    luaStates.push_back(stateInfo);
}

lua_State *Core::getStateFromVector(const char *script)
{
    auto it = std::find_if(luaStates.begin(), luaStates.end(), [&](const LuaStateInfo &stateInfo)
                           { return std::strcmp(stateInfo.script, script) == 0; });

    if (it != luaStates.end())
    {
        return it->L;
    }
    else
    {
        throw std::runtime_error("Script not found");
    }
}

void Core::callUpdateInScript(const char *script)
{
    lua_State *L = getStateFromVector(script);
    lua_getglobal(L, script);
    if (lua_isnil(L, -1))
    {
        logp->printlf("Global %s not found", script);
        lua_pop(L, 1);
        return;
    }

    lua_getglobal(L, "UpdateTicks");
    if (lua_isnil(L, -1))
    {
        // logp->printlf("Function UpdateTicks not found in %s", script);
        lua_pop(L, 1);
        return;
    }

    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *error = lua_tostring(L, -1);
            logp->printlf("Error calling UpdateTicks on %s: %s", script, error);
            lua_pop(L, 1);
        }
    }

    lua_pop(L, 1);
}

void Core::tick_update()
{
    for (const auto &[plugin, name] : loadedLibraries)
    {
        callUpdatePlugin(plugin);
    }

    callUpdateInScript(mainScript.c_str());

    for (const auto &script : minorScripts)
    {
        callUpdateInScript(script.c_str());
    }
}