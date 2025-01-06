#include "main_script.hxx"
#include "globals.hxx"

void MainScript::load(lua_State *L, const char *script)
{
    lua_newtable(L);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_setglobal(L, script);

    if (luaL_loadfile(L, script) || lua_pcall(L, 0, LUA_MULTRET, 0))
    {
        logp->printlf("Error opening LUAC: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    lua_getglobal(L, "main");
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            logp->printlf("Error calling main function: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
            unload(L, script);
            return;
        }
    }
    else
    {
        logp->printlf("The main function is not defined in the script!");
        return;
    }
}

void MainScript::unload(lua_State *L, const char *script)
{
    lua_getglobal(L, script);
    if (lua_istable(L, -1))
    {
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            lua_pushnil(L);
            lua_settable(L, -3);
        }
    }
    lua_pop(L, 1);
}

void MainScript::callFunction(lua_State *L, const char *script, const char *functionName)
{
    lua_getglobal(L, script);
    lua_getfield(L, -1, functionName);
    if (lua_isfunction(L, -1))
    {
        lua_pcall(L, 0, 0, 0);
    }
    else
    {
        logp->printlf("Function %s not found in %s script!", functionName, script);
        lua_pop(L, 1);
    }
}
