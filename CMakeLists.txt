cmake_minimum_required(VERSION 3.10)
project(burro)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(SRC
  src/key.h
  src/key.c

  src/enc.h
  src/enc.c

  src/u8.h
  src/u8.c
  
  src/burro.c
)

add_executable(burro ${SRC})

target_link_libraries(burro unistring)
target_compile_definitions(burro PRIVATE ORIGIN=1072)
# target_compile_options(burro PRIVATE -Wall -Wextra -Wpedantic -Werror)

