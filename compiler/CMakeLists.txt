cmake_minimum_required(VERSION 3.10)
project(bot_core_compiler)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(bot_core_compiler_src
    compiler.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/logger.cxx
)

add_executable(${PROJECT_NAME} ${bot_core_compiler_src})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/third_party/luajit/src
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUAJIT_LIB})