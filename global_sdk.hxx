#ifdef WIN32
#pragma once
#endif

#ifndef _GLOBAL_SDK_HXX
#define _GLOBAL_SDK_HXX

#ifndef PACKED
#if defined(_MSC_VER)
#define PACKED
#else
#define PACKED __attribute__((packed))
#endif
#endif

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "luaconf.h"
#include "lualib.h"
}

typedef int (*IntFunctionPtr)(lua_State *L);

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef PRAGMA_PACK_PUSH_ONCE
#define PRAGMA_PACK_PUSH_ONCE
#pragma pack(push, 1)
#endif
#endif

typedef struct Native_Function_List
{
    const char *nativeName PACKED;
    IntFunctionPtr funcPtr PACKED;
} PACKED Native_Function_List;

#ifdef _WIN32
#ifndef PRAGMA_PACK_POP_ONCE
#define PRAGMA_PACK_POP_ONCE
#pragma pack(pop)
#endif
#endif

#endif