// #include "plugin_manager.hxx"

// #include <iostream>
// #include <filesystem>
// #include <any>
// #include <vector>
// #include <stdexcept>
// #ifdef _WIN32
// #include <windows.h>
// #else
// #include <dlfcn.h>
// #endif
// #include "globals.hxx"

// namespace fs = std::filesystem;

// PluginManager::PluginManager()
// {
// }

// PluginManager::~PluginManager()
// {
//     UnloadAll();
// }

// void PluginManager::LoadPlugins(const std::string &directory)
// {
//     for (const auto &entry : fs::directory_iterator(directory))
//     {
//         if (entry.is_regular_file())
//         {
//             const std::string filePath = entry.path().string();

// #ifdef _WIN32
//             if (filePath.substr(filePath.find_last_of(".")) == ".dll")
// #elif __APPLE__
//             if (filePath.substr(filePath.find_last_of(".")) == ".dylib")
// #else
//             if (filePath.substr(filePath.find_last_of(".")) == ".so")
// #endif
//             {
//                 logp->printlf("Loading plugin: %s", filePath.c_str());
//                 void *libHandle = LoadLibrary(filePath);
//                 if (libHandle)
//                 {
//                     loadedLibraries.emplace_back(libHandle, filePath);
//                 }
//                 else
//                 {
//                     logp->printlf("Failed to load plugin: %s", filePath.c_str());
//                 }
//             }
//         }
//     }
// }

// void *PluginManager::LoadLibrary(const std::string &path)
// {
// #ifdef _WIN32
//     HMODULE hLib = ::LoadLibraryA(path.c_str());
//     if (!hLib)
//     {
//         int errorCode = static_cast<int>(GetLastError());
//         LPVOID lpMsgBuf;
//         FormatMessage(
//             FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//             NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//             (LPTSTR)&lpMsgBuf, 0, NULL);
//         logp->printlf("Error loading plugin: %d - %s", errorCode, (char *)lpMsgBuf);
//         LocalFree(lpMsgBuf);
//     }
//     return hLib;
// #else
//     void *handle = dlopen(path.c_str(), RTLD_LAZY);
//     if (!handle)
//     {
//         logp->printlf("Error loading plugin: %s", dlerror());
//     }
//     return handle;
// #endif
// }

// void PluginManager::UnloadAll()
// {
//     for (const auto &[lib, name] : loadedLibraries)
//     {
// #ifdef _WIN32
//         ::FreeLibrary(static_cast<HMODULE>(lib));
// #else
//         dlclose(lib);
// #endif
//     }
//     loadedLibraries.clear();
// }

// std::vector<std::pair<void *, std::string>> PluginManager::GetLoadedLibraries()
// {
//     return loadedLibraries;
// }

#include "plugin_manager.hxx"
#include "globals.hxx"

#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

PluginManager::~PluginManager()
{
    for (auto &plugin : plugins)
    {
        delete plugin;
    }
}

void PluginManager::LoadPlugins(const std::string &directory)
{
    for (const auto &entry : std::filesystem::directory_iterator(directory))
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
                logp->printlf("Loading plugin: %s", filePath.c_str());
////////
#ifdef _WIN32
                HMODULE hLib = ::LoadLibraryA(filePath.c_str());
                if (!hLib)
                {
                    int errorCode = static_cast<int>(GetLastError());
                    LPVOID lpMsgBuf;
                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&lpMsgBuf, 0, NULL);
                    logp->printlf("Error loading plugin: %d - %s", errorCode, (char *)lpMsgBuf);
                    LocalFree(lpMsgBuf);
                    return;
                }

                auto entry_point = (SDKBase * (*)()) GetProcAddress(hLib, "PluginEntryGet");
                if (!entry_point)
                {
                    int errorCode = static_cast<int>(GetLastError());
                    LPVOID lpMsgBuf;
                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&lpMsgBuf, 0, NULL);
                    logp->printlf("Failed to find PLUGIN_ENTRY: %d - %s", errorCode, (char *)lpMsgBuf);
                    LocalFree(lpMsgBuf);
                    ::FreeLibrary(hLib);
                    return;
                }

                plugins.push_back(entry_point());
#else
                void *handle = dlopen(filePath.c_str(), RTLD_LAZY);
                if (!handle)
                {
                    logp->printlf("Error loading plugin: %s", dlerror());
                    return;
                }

                auto entry_point = (SDKBase * (*)()) dlsym(handle, "PluginEntryGet");
                if (!entry_point)
                {
                    logp->printlf("Failed to find PLUGIN_ENTRY: %s", dlerror());
                    dlclose(handle);
                    return;
                }

                plugins.push_back(entry_point());
#endif
            }
        }
    }
}

void PluginManager::RunPlugins()
{
    for (auto &plugin : plugins)
    {
        plugin->onRun();
    }
}

void PluginManager::RegisterNatives(lua_State *L)
{
    for (auto &plugin : plugins)
    {
        plugin->register_lua_natives(L);
    }
}

void PluginManager::CallUpdateFunction()
{
    for (auto &plugin : plugins)
    {
        plugin->update();
    }
}