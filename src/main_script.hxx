#ifdef WIN32
#pragma once
#endif

#ifndef _MAIN_SCRIPTS_HXX
#define _MAIN_SCRIPTS_HXX

#include "global_sdk.hxx"

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