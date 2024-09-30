#include "lua_natives.hxx"
#include "plugin_manager.hxx"
#include "global_sdk.hxx"
#include "globals.hxx"

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
#ifdef _WIN32
        DWORD errorCode = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf, 0, NULL);
        logp->printlf("Failed to get RegisterNatives function: %s", (char *)lpMsgBuf);
        LocalFree(lpMsgBuf);
#else
        logp->printlf("Failed to get RegisterNatives function: %s", dlerror());
#endif
        return;
    }
    Native_Function_List *nativeList = registerNatives();
    if (!nativeList)
    {
        logp->printlf("RegisterNatives returned null!");
        return;
    }
    for (int i = 0; nativeList[i].nativeName != nullptr; ++i)
    {
        lua_register(L, nativeList[i].nativeName, nativeList[i].funcPtr);
    }
}