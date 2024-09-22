#ifndef _MAIN_SCRIPTS_HXX
#define _MAIN_SCRIPTS_HXX

#include <logprint.hxx>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class MainScript
{
public:
    MainScript(logprint &LogsCore) : logger(&LogsCore)
    {
    }
    ~MainScript()
    {
    }
    void load(lua_State *L, const char *script);
    void unload(lua_State *L, const char *script);
    void callFunction(lua_State *L, const char *script, const char *functionName);

private:
    logprint *logger;
};

#endif