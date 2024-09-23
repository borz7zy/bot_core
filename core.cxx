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

void Core::loadMinor()
{
    MinorScripts minScripts(LogsCore);
    MinorScripts *minor = &minScripts;
    for (const auto &minScript : minorScripts)
    {
        minor->load(L, minScript.c_str());
    }
}

void Core::loadMain()
{
    MainScript mScript(LogsCore);
    MainScript *main = &mScript;

    main->load(L, mainScript.c_str());
}