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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(PREBUILDEDLIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/prebuilded-libs)

set(wolfssl_root_dir ${PREBUILDEDLIBS_DIR}/wolfssl)
list(APPEND CMAKE_PREFIX_PATH ${wolfssl_root_dir}/lib/cmake/wolfssl)
find_package(Threads REQUIRED)
find_package(wolfssl REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/compiler)

set(BOT_SRC
    main.cxx
    utils.cxx
    globals.cxx
    logger.cxx
    config_manager.cxx
    tick_sys.cxx
    lua_natives.cxx
    plugin_manager.cxx
    minor_scripts.cxx
    main_script.cxx
    httpclient.cxx
    asynchttp.cxx
)

add_executable(bot_core ${BOT_SRC})
target_include_directories(bot_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/third_party/lua
)
target_link_libraries(bot_core PRIVATE lua::lib lua::header wolfssl::wolfssl)