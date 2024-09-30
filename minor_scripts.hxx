#ifndef _MINOR_SCRIPTS_HXX
#define _MINOR_SCRIPTS_HXX

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class MinorScripts
{
public:
    MinorScripts()
    {
    }
    ~MinorScripts()
    {
    }
    void load(lua_State *L, const char *script);
    void unload(lua_State *L, const char *script);
    void callFunction(lua_State *L, const char *script, const char *functionName);
};

#endif
