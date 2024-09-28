#include "lua_natives.hxx"
#include "plugin_manager.hxx"
#include "global_sdk.hxx"

typedef Native_Function_List *(*RegisterNativesFunc)();

void LuaNatives::CallGetNatives(lua_State *L, void *plugin, const char *lib_name)
{
    RegisterNativesFunc registerNatives = nullptr;

#if defined(_WIN32)
    registerNatives = (RegisterNativesFunc)GetProcAddress((HMODULE)plugin, REGISTER_NATIVES_NAME);
#else
    registerNatives = (RegisterNativesFunc)dlsym(plugin, REGISTER_NATIVES_NAME);
#endif
    if (!registerNatives)
    {
        // std::cerr << "Failed to get RegisterNatives function: " << (handle ? dlerror() : GetLastError()) << std::endl;
        return;
    }
    Native_Function_List *nativeList = registerNatives();
    if (!nativeList)
    {
        // std::cerr << "RegisterNatives returned null!" << std::endl;
        return;
    }
    for (int i = 0; nativeList[i].nativeName != nullptr; ++i)
    {
        lua_register(L, nativeList[i].nativeName, nativeList[i].funcPtr);
    }
}