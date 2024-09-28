#include "core.hxx"

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
    MinorScripts minScripts(LogsCore);
    MinorScripts *minor = &minScripts;
    minor->load(L, minorScript);
}

void Core::loadMain(lua_State *L)
{
    MainScript mScript(LogsCore);
    MainScript *main = &mScript;

    main->load(L, mainScript.c_str());
}

void Core::callUpdatePlugin(void *plugin)
{
    typedef void (*VoidFunction)();
    VoidFunction func = (VoidFunction)dlsym(plugin, "update_ticks");

    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        return;
    }

    func();
}

void Core::preloadStates(const char *script)
{
    lua_State *L = luaL_newstate();
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
        logger->LOGE("Global %s not found", script);
        lua_pop(L, 1);
        return;
    }

    lua_getglobal(L, "UpdateTicks");
    if (lua_isnil(L, -1))
    {
        // logger->LOGE("Function UpdateTicks not found in %s", script);
        lua_pop(L, 1);
        return;
    }

    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *error = lua_tostring(L, -1);
            logger->LOGE("Error calling UpdateTicks on %s: %s", script, error);
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