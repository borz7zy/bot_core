#ifndef _MAIN_SCRIPTS_HXX
#define _MAIN_SCRIPTS_HXX

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class MainScript
{
public:
    MainScript()
    {
    }
    ~MainScript()
    {
    }
    void load(lua_State *L, const char *script);
    void callFunction(lua_State *L, const char *script, const char *functionName);

private:
    void unload(lua_State *L, const char *script);
};

#endif