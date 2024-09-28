#include "plugin_manager.hxx"

#include <iostream>
#include <filesystem>
#include <logprint.hxx>
#include <any>
#include <vector>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

logprint logger("PLUGINS", "./plugins.log");
logprint *logs = &logger;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    UnloadAll();
}

void PluginManager::LoadPlugins(const std::string &directory)
{
    for (const auto &entry : fs::directory_iterator(directory))
    {
        if (entry.is_regular_file())
        {
            const std::string filePath = entry.path().string();

#ifdef _WIN32
            if (filePath.substr(filePath.find_last_of(".")) == ".dll")
#elif __APPLE__
            if (filePath.substr(filePath.find_last_of(".")) == ".dylib")
#else
            if (filePath.substr(filePath.find_last_of(".")) == ".so")
#endif
            {
                logs->LOGI("Loading: %s", filePath.c_str());
                void *libHandle = LoadLibrary(filePath);
                if (libHandle)
                {
                    loadedLibraries.emplace_back(libHandle, filePath);
                }
                else
                {
                    logs->LOGE("Failed to load: %s", filePath.c_str());
                }
            }
        }
    }
}

void *PluginManager::LoadLibrary(const std::string &path)
{
#ifdef _WIN32
    HMODULE hLib = ::LoadLibraryA(path.c_str());
    if (!hLib)
    {
        logs->LOGE("Error loading plugin: %s", GetLastError());
    }
    return hLib;
#elif __APPLE__
    void *handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle)
    {
        logs->LOGE("Error loading plugin: %s", dlerror());
    }
    return handle;
#else
    void *handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle)
    {
        logs->LOGE("Error loading plugin: %s", dlerror());
    }
    return handle;
#endif
}

void PluginManager::UnloadAll()
{
    for (const auto &[lib, name] : loadedLibraries)
    {
#ifdef _WIN32
        ::FreeLibrary(static_cast<HMODULE>(lib));
#else
        dlclose(lib);
#endif
    }
    loadedLibraries.clear();
}

std::vector<std::pair<void *, std::string>> PluginManager::GetLoadedLibraries()
{
    return loadedLibraries;
}
