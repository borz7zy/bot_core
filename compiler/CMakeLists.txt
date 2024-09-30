cmake_minimum_required(VERSION 3.10)
project(bot_core_compiler)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libs)

set(bot_core_compiler_src
    compiler.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/../logger.cxx
)

add_executable(bot_core_compiler ${bot_core_compiler_src})
target_include_directories(bot_core_compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/third_party/lua
)
target_link_libraries(bot_core_compiler PRIVATE lua::lib)