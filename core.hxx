#ifndef _CORE_HXX
#define _CORE_HXX

#include <logprint.hxx>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class Core
{
public:
    Core(const char *scrPath) : LogsCore("CORE", "./core.logs"), scriptPath(scrPath)
    {
        L = luaL_newstate();
        if (L == nullptr)
        {
            logger->LOGE("Core initialization error: Lua could not allocate state");
            return;
        }
        luaL_openlibs(L);

        if (luaL_loadfile(L, scriptPath) || lua_pcall(L, 0, LUA_MULTRET, 0))
        {
            logger->LOGE("Error opening LUAC: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
            return;
        }

        lua_getglobal(L, "main");
        if (lua_isfunction(L, -1))
        {
            if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
            {
                logger->LOGE("Error calling main function: %s", lua_tostring(L, -1));
                lua_pop(L, 1);
                return;
            }
        }
        else
        {
            logger->LOGE("The main function is not defined in the script!");
            return;
        }

        logger->LOGI("The core was initialized successfully.");
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
    const char *scriptPath;
};

#endif