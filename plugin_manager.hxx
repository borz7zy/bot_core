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
    void UnloadAll();
    std::vector<std::pair<void *, std::string>> GetLoadedLibraries();

private:
    std::vector<std::pair<void *, std::string>> loadedLibraries;

    void *LoadLibrary(const std::string &path);
};

#endif