#ifndef _MAIN_HXX
#define _MAIN_HXX

#include "globals.hxx"

#include "minor_scripts.hxx"
#include "main_script.hxx"
#include "plugin_manager.hxx"
#include "lua_natives.hxx"
#include "tick_sys.hxx"
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <csignal>
#include "global_sdk.hxx"

std::atomic<bool> g_shutdownRequested = false;
void shutdownCore();
void signalHandler(int signal);

void processScripts(const std::string &scripts, const std::string &basePath, std::vector<std::string> &output);

struct LuaStateInfo
{
    lua_State *L;
    const char *script;
};
std::vector<LuaStateInfo> luaStates;

std::string plPath;

std::vector<std::string> minorScripts;
std::string mainScript;

PluginManager pluginManager;

void preloadStates(const char *script);
lua_State *getStateFromVector(const char *script);

std::vector<std::pair<void *, std::string>> loadedLibraries;

LuaNatives luaNatives;

uint8_t ticks;

void callUpdatePlugin(void *plugin);
void callUpdateInScript(const char *script);

void tick_update();

void loadMinor(lua_State *L, const char *minorScript);
void loadMain(lua_State *L);

#endif