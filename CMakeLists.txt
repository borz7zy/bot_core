cmake_minimum_required(VERSION 3.10)
project(bot_core)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(BOT_SRC
    main.cxx
    config_manager.cxx
    tick_sys.cxx
    core.cxx
    lua_natives.cxx
    plugin_manager.cxx
    minor_scripts.cxx
    main_script.cxx
)


add_executable(bot_core ${BOT_SRC})
target_include_directories(bot_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/logprint
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/third_party/lua
)
target_link_libraries(bot_core PRIVATE logprint lua::lib lua::header)