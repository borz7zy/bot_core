#ifndef _PLUGIN_MANAGER_HXX
#define _PLUGIN_MANAGER_HXX

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    void LoadPlugins(const std::string &directory);

private:
    std::vector<void *> loadedLibraries;

    void *LoadLibrary(const std::string &path);

    void UnloadAll();
};

#endif