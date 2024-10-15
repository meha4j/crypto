cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")

project(burro C)

set(PKG_PATH ${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-linux)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_BUILD_TYPE Debug)
set(C_STANDARD 23)
set(COMPILE_OPTIONS -std=c23 -Wall -Wextra -Wpedantic -Werror)

set(SRC src/fit.c src/rot.c src/uc.c ${PKG_PATH}/lib/libunistring.a)
set(HDRS include/fit.h include/rot.h include/uc.h)

set(BURRO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/ ${PKG_PATH}/include/)

add_library(burro ${SRC} ${HDRS})

target_include_directories(burro PUBLIC ${BURRO_INCLUDE_DIR})
target_compile_options(burro PUBLIC ${COMPILE_OPTIONS})

add_subdirectory(app)
