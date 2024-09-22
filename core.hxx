#ifndef _CORE_HXX
#define _CORE_HXX

#include <logprint.hxx>
#include "minor_scripts.hxx"
#include "main_script.hxx"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class Core
{
public:
    Core(const char *scrPath) : LogsCore("CORE", "./core.log"), scriptPath(scrPath)
    {
        L = luaL_newstate();
        if (L == nullptr)
        {
            logger->LOGE("Core initialization error: Lua could not allocate state");
            return;
        }
        luaL_openlibs(L);

        // minor

        // main

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