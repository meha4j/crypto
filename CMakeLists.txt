cmake_minimum_required(VERSION 3.10)
project(burro)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_BUILD_TYPE Debug)
set(C_STANDARD 23)

set(SRC
  src/log.h
  src/log.c

  src/key.h
  src/key.c
  src/enc.h
  src/enc.c
  src/u8.h
  src/u8.c
  src/crk.h
  src/crk.c

  src/burro.c
)

add_executable(burro ${SRC})

target_link_libraries(burro unistring)
target_compile_options(burro PRIVATE -std=c23)
target_compile_options(burro PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c23)

