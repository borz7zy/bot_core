cmake_minimum_required(VERSION 3.10)
project(bot_core)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building bot core in Debug mode")
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building bot core in Release mode")
    add_definitions(-DRELEASE)
endif()

#macos
#       export MACOSX_DEPLOYMENT_TARGET=$(sw_vers -productVersion | cut -d'.' -f1-2)
set(LUAJIT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/libs/third_party/luajit/src/libluajit.a)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/compiler)

set(BOT_SRC
    src/main.cxx
    src/utils.cxx
    src/globals.cxx
    src/logger.cxx
    src/config_manager.cxx
    src/tick_sys.cxx
    src/lua_natives.cxx
    src/plugin_manager.cxx
    src/minor_scripts.cxx
    src/main_script.cxx
    src/plugins_sdk.cxx
)

add_executable(${PROJECT_NAME} ${BOT_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/third_party/luajit/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LUAJIT_LIB})