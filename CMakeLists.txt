cmake_minimum_required(VERSION 3.23)

project(dream_wayland_testbed 
        VERSION 1.0.0
        LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/wl_proto_gen.cmake)

option(BUILD_TESTS "Build tests" ON)

file(GLOB_RECURSE WAYLAND_CLIENT_LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.c*)

add_library(
    wayclient
    ${WAYLAND_CLIENT_LIB_SOURCES}
)

target_include_directories(
    wayclient
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    wayclient
    PRIVATE
    wayland-client
    wayland-egl
    EGL
    rt
    xkbcommon
    m
)

function(add_test_executable SRC_FILE)
    get_filename_component(TARGET_NAME ${SRC_FILE} NAME_WE)
    add_executable(
        ${TARGET_NAME}
        ${PROJECT_SOURCE_DIR}/tests/${SRC_FILE}
    )
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        wayclient
    )
endfunction()

if (BUILD_TESTS)
    add_test_executable(gcapi.c)
endif (BUILD_TESTS)
