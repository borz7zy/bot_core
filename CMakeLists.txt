cmake_minimum_required(VERSION 3.10)
project(bot_core)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(LIBS_PATH ${CMAKE_SOURCE_DIR}/libs)
set(THIRD_PARTY_LIBS ${LIBS_PATH}/third_party)

set(LOGPRINT_LIB ${LIBS_PATH}/logprint)

set(LUA_SOURCES ${THIRD_PARTY_LIBS}/lua)
set(LUA_BUILD 
 ${LUA_SOURCES}/lua.c
 ${LUA_SOURCES}/lapi.c
 ${LUA_SOURCES}/lauxlib.c
 ${LUA_SOURCES}/lbaselib.c
)
add_library(lua STATIC ${LUA_BUILD})

set(BOT_SRC
 main.cxx
 ${LOGPRINT_LIB}/logprint.cpp
)

add_executable(bot_core ${BOT_SRC})
target_include_directories(bot_core PRIVATE 
 ${LOGPRINT_LIB}
 ${LUA_SOURCES}
)
target_link_libraries(bot_core PRIVATE lua)