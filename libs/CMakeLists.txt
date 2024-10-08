cmake_minimum_required(VERSION 3.10)
project(libs)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRD_PARTY_LIBS ${LIBS_PATH}/third_party)

set(LUA_SOURCES ${THIRD_PARTY_LIBS}/lua)
set(LUA_BUILD
    ${LUA_SOURCES}/lapi.c
    ${LUA_SOURCES}/lauxlib.c
    ${LUA_SOURCES}/lbaselib.c
    ${LUA_SOURCES}/lcode.c
    ${LUA_SOURCES}/lcorolib.c
    ${LUA_SOURCES}/lctype.c
    ${LUA_SOURCES}/ldblib.c
    ${LUA_SOURCES}/ldebug.c
    ${LUA_SOURCES}/ldo.c
    ${LUA_SOURCES}/ldump.c
    ${LUA_SOURCES}/lfunc.c
    ${LUA_SOURCES}/lgc.c
    ${LUA_SOURCES}/linit.c
    ${LUA_SOURCES}/liolib.c
    ${LUA_SOURCES}/llex.c
    ${LUA_SOURCES}/lmathlib.c
    ${LUA_SOURCES}/lmem.c
    ${LUA_SOURCES}/loadlib.c
    ${LUA_SOURCES}/lobject.c
    ${LUA_SOURCES}/lopcodes.c
    ${LUA_SOURCES}/loslib.c
    ${LUA_SOURCES}/lparser.c
    ${LUA_SOURCES}/lstate.c
    ${LUA_SOURCES}/lstring.c
    ${LUA_SOURCES}/lstrlib.c
    ${LUA_SOURCES}/ltable.c
    ${LUA_SOURCES}/ltablib.c
    ${LUA_SOURCES}/ltm.c
    ${LUA_SOURCES}/lundump.c
    ${LUA_SOURCES}/lutf8lib.c
    ${LUA_SOURCES}/lvm.c
    ${LUA_SOURCES}/lzio.c
)

set(LUA_HEADERS
    ${LUA_SOURCES}/lauxlib.h
    ${LUA_SOURCES}/lua.h
    ${LUA_SOURCES}/luaconf.h
    ${LUA_SOURCES}/lualib.h
)

add_library(lua STATIC ${LUA_BUILD})
if (WIN32)
    target_compile_options(lua PRIVATE /std:c99)
else()
    target_compile_options(lua PRIVATE -O2 -std=c99)
endif()

set(LUA_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
file(COPY ${LUA_HEADERS} DESTINATION ${LUA_INCLUDE_DIR})

add_library(lua-header INTERFACE)
target_include_directories(lua-header INTERFACE ${LUA_INCLUDE_DIR})

add_library(lua::lib ALIAS lua)
add_library(lua::header ALIAS lua-header)