// #ifdef WIN32
// #pragma once
// #endif

// #ifndef _PLUGIN_MANAGER_HXX
// #define _PLUGIN_MANAGER_HXX

// #include <string>
// #include <vector>

// #ifdef _WIN32
// #include <windows.h>
// #else
// #include <dlfcn.h>
// #endif

// class PluginManager
// {
// public:
//     PluginManager();
//     ~PluginManager();

//     void LoadPlugins(const std::string &directory);
//     void UnloadAll();
//     std::vector<std::pair<void *, std::string>> GetLoadedLibraries();

// private:
//     std::vector<std::pair<void *, std::string>> loadedLibraries;

//     void *LoadLibrary(const std::string &path);
// };

// #endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#pragma once
#endif

#ifndef _PLUGIN_MANAGER_HXX
#define _PLUGIN_MANAGER_HXX

#include <vector>

#include "plugins_sdk.hxx"

class PluginManager
{
public:
    ~PluginManager();
    void LoadPlugins(const std::string &directory);
    void RunPlugins();
    void RegisterNatives(lua_State *L);
    void CallUpdateFunction();

private:
    std::vector<SDKBase *> plugins;
};

#endif