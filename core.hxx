#ifndef _CORE_HXX
#define _CORE_HXX

#include <logprint.hxx>
#include "minor_scripts.hxx"
#include "main_script.hxx"
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
    Core(std::vector<std::string> minScripts, std::string maScript) : LogsCore("CORE", "./core.log"),
                                                                      minorScripts(minScripts),
                                                                      mainScript(maScript)
    {
        L = luaL_newstate();
        if (L == nullptr)
        {
            logger->LOGE("Core initialization error: Lua could not allocate state");
            return;
        }
        luaL_openlibs(L);

        loadMinor();

        loadMain();

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

    std::vector<std::string> minorScripts;
    std::string mainScript;

    void loadMinor();
    void loadMain();
};

#endif